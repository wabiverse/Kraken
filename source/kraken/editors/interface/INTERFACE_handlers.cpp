/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Editors.
 * Tools for Artists.
 */

#include "kraken/kraken.h"

#include "USD_factory.h"
#include "USD_operator.h"
#include "USD_userpref.h"
#include "USD_wm_types.h"

#include "WM_cursors_api.h"
#include "WM_window.h"
#include "WM_event_system.h"
#include "WM_tooltip.h"

#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "KLI_rect.h"

#include "KKE_context.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

enum uiButtonActivateType
{
  BUTTON_ACTIVATE_OVER,
  BUTTON_ACTIVATE,
  BUTTON_ACTIVATE_APPLY,
  BUTTON_ACTIVATE_TEXT_EDITING,
  BUTTON_ACTIVATE_OPEN,
};

enum uiHandleButtonState
{
  BUTTON_STATE_INIT,
  BUTTON_STATE_HIGHLIGHT,
  BUTTON_STATE_WAIT_FLASH,
  BUTTON_STATE_WAIT_RELEASE,
  BUTTON_STATE_WAIT_KEY_EVENT,
  BUTTON_STATE_NUM_EDITING,
  BUTTON_STATE_TEXT_EDITING,
  BUTTON_STATE_TEXT_SELECTING,
  BUTTON_STATE_MENU_OPEN,
  BUTTON_STATE_WAIT_DRAG,
  BUTTON_STATE_EXIT,
};

enum uiMenuScrollType
{
  MENU_SCROLL_UP,
  MENU_SCROLL_DOWN,
  MENU_SCROLL_TOP,
  MENU_SCROLL_BOTTOM,
};

struct uiBlockInteraction_Handle
{
  struct uiBlockInteraction_Params params;
  void *user_data;
  /**
   * This is shared between #uiHandleButtonData and #uiAfterFunc,
   * the last user runs the end callback and frees the data.
   *
   * This is needed as the order of freeing changes depending on
   * accepting/canceling the operation.
   */
  int user_count;
};

struct uiHandleButtonData
{
  wmWindowManager *wm;
  wmWindow *window;
  ScrArea *area;
  ARegion *region;

  bool interactive;

  /* overall state */
  uiHandleButtonState state;
  int retval;
  /* booleans (could be made into flags) */
  bool cancel, escapecancel;
  bool applied, applied_interactive;
  /* Button is being applied through an extra icon. */
  bool apply_through_extra_icon;
  bool changed_cursor;
  wmTimer *flashtimer;

  /* edited value */
  /* use 'ui_textedit_string_set' to assign new strings */
  char *str;
  char *origstr;
  UsdAttribute value, origvalue, startvalue;
  float vec[3], origvec[3];
  ColorBand *coba;

  /* True when alt is held and the preference for displaying tooltips should be ignored. */
  bool tooltip_force;
  /**
   * Behave as if #UI_BUT_DISABLED is set (without drawing grayed out).
   * Needed so non-interactive labels can be activated for the purpose of showing tool-tips,
   * without them blocking interaction with nodes, see: T97386.
   */
  bool disable_force;

  /* auto open */
  bool used_mouse;
  wmTimer *autoopentimer;

  /* auto open (hold) */
  wmTimer *hold_action_timer;

  /* text selection/editing */
  /* size of 'str' (including terminator) */
  int maxlen;
  /* Button text selection:
   * extension direction, selextend, inside ui_do_but_TEX */
  int sel_pos_init;
  /* Allow reallocating str/editstr and using 'maxlen' to track alloc size (maxlen + 1) */
  bool is_str_dynamic;

  /* number editing / dragging */
  /* coords are Window/uiBlock relative (depends on the button) */
  int draglastx, draglasty;
  int dragstartx, dragstarty;
  int draglastvalue;
  int dragstartvalue;
  bool dragchange, draglock;
  int dragsel;
  float dragf, dragfstart;
  CBData *dragcbd;

  /** Soft min/max with #UI_DRAG_MAP_SOFT_RANGE_PIXEL_MAX applied. */
  float drag_map_soft_min;
  float drag_map_soft_max;

  /* Menu open, see: #UI_screen_free_active_but_highlight. */
  uiPopupBlockHandle *menu;
  int menuretval;

  /* Search box see: #UI_screen_free_active_but_highlight. */
  ARegion *searchbox;

  struct uiBlockInteraction_Handle *custom_interaction_handle;

  /* Text field undo. */
  // struct uiUndoStack_Text *undo_stack_text;

  /* post activate */
  uiButtonActivateType posttype;
  uiBut *postbut;
};

struct uiAfterFunc
{
  uiButHandleFunc func;
  void *func_arg1;
  void *func_arg2;

  uiButHandleNFunc funcN;
  void *func_argN;

  uiButHandleRenameFunc rename_func;
  void *rename_arg1;
  void *rename_orig;

  uiBlockHandleFunc handle_func;
  void *handle_func_arg;
  int retval;

  uiMenuHandleFunc butm_func;
  void *butm_func_arg;
  int a2;

  wmOperator *popup_op;
  wmOperatorType *optype;
  // wmOperatorCallContext opcontext;
  KrakenPRIM *opptr;

  KrakenPRIM stagepoin;
  KrakenPROP *stageprop;

  void *search_arg;
  uiFreeArgFunc search_arg_free_fn;

  uiBlockInteraction_CallbackData custom_interaction_callbacks;
  uiBlockInteraction_Handle *custom_interaction_handle;

  // kContextStore *context;

  char undostr[KKE_UNDO_STR_MAX];
  char drawstr[UI_MAX_DRAW_STR];
};

/* returns the active button with an optional checking function */
static uiBut *ui_context_button_active(const ARegion *region, bool (*but_check_cb)(const uiBut *))
{
  uiBut *but_found = nullptr;

  while (region) {
    /**
     * Follow this exact priority (from highest to lowest priority):
     * 1) Active-override button (#UI_BUT_ACTIVE_OVERRIDE).
     * 2) The real active button.
     * 3) The previously active button (#UI_BUT_LAST_ACTIVE). */

    uiBut *active_but_override = nullptr;
    uiBut *active_but_real = nullptr;
    uiBut *active_but_last = nullptr;

    /* find active button */
    for (auto &block : region->uiblocks) {
      for (auto &but : block->buttons) {
        if (but->flag & UI_BUT_ACTIVE_OVERRIDE) {
          active_but_override = but;
        }
        if (but->active) {
          active_but_real = but;
        }
        if (but->flag & UI_BUT_LAST_ACTIVE) {
          active_but_last = but;
        }
      }
    }

    uiBut *activebut = active_but_override;
    if (!activebut) {
      activebut = active_but_real;
    }
    if (!activebut) {
      activebut = active_but_last;
    }

    if (activebut && (but_check_cb == NULL || but_check_cb(activebut))) {
      uiHandleButtonData *data = activebut->active;

      but_found = activebut;

      /* Recurse into opened menu, like color-picker case. */
      if (data && data->menu && (region != data->menu->region)) {
        region = data->menu->region;
      } else {
        return but_found;
      }
    } else {
      /* no active button */
      return but_found;
    }
  }

  return but_found;
}

uiBut *UI_context_active_but_get(const kContext *C)
{
  return ui_context_button_active(CTX_wm_region(C), NULL);
}

void UI_but_tooltip_timer_remove(kContext *C, uiBut *but)
{
  uiHandleButtonData *data = but->active;
  if (data) {
    if (data->autoopentimer) {
      WM_event_remove_timer(data->wm, data->window, data->autoopentimer);
      data->autoopentimer = NULL;
    }

    if (data->window) {
      WM_tooltip_clear(C, data->window);
    }
  }
}

static inline void copy_v3_v3(float r[3], const float a[3])
{
  r[0] = a[0];
  r[1] = a[1];
  r[2] = a[2];
}

static inline void zero_v3(float r[3])
{
  r[0] = 0.0f;
  r[1] = 0.0f;
  r[2] = 0.0f;
}

/**
 * Check if a #uiAfterFunc is needed for this button.
 */
static bool ui_afterfunc_check(const uiBlock *block, const uiBut *but)
{
  return (but->func || but->funcN || but->rename_func || but->optype || but->stageprop ||
          block->handle_func || (but->type == UI_BTYPE_BUT_MENU && block->butm_func) ||
          (block->handle && block->handle->popup_op));
}

/* -------------------------------------------------------------------- */
/** \name Button Apply/Revert
 * \{ */

static std::vector<uiAfterFunc *> UIAfterFuncs = {};

static uiAfterFunc *ui_afterfunc_new(void)
{
  uiAfterFunc *after = new uiAfterFunc();

  UIAfterFuncs.push_back(after);

  return after;
}

/**
 * These functions are postponed and only executed after all other
 * handling is done, i.e. menus are closed, in order to avoid conflicts
 * with these functions removing the buttons we are working with.
 */
static void ui_apply_but_func(kContext *C, uiBut *but)
{
  uiBlock *block = but->block;
  if (!ui_afterfunc_check(block, but)) {
    return;
  }

  uiAfterFunc *after = ui_afterfunc_new();

  if (but->func && ELEM(but, but->func_arg1, but->func_arg2)) {
    /* exception, this will crash due to removed button otherwise */
    but->func(C, but->func_arg1, but->func_arg2);
  } else {
    after->func = but->func;
  }

  after->func_arg1 = but->func_arg1;
  after->func_arg2 = but->func_arg2;

  after->funcN = but->funcN;
  after->func_argN = (but->func_argN) ?
                       std::memcpy(after->func_argN, but->func_argN, sizeof(but->func_argN)) :
                       NULL;

  after->rename_func = but->rename_func;
  after->rename_arg1 = but->rename_arg1;
  after->rename_orig = but->rename_orig; /* needs free! */

  after->handle_func = block->handle_func;
  after->handle_func_arg = block->handle_func_arg;
  after->retval = but->retval;

  if (but->type == UI_BTYPE_BUT_MENU) {
    after->butm_func = block->butm_func;
    after->butm_func_arg = block->butm_func_arg;
    after->a2 = but->a2;
  }

  if (block->handle) {
    after->popup_op = block->handle->popup_op;
  }

  after->optype = but->optype;
  // after->opcontext = but->opcontext;
  after->opptr = but->opptr;

  after->stagepoin = but->stagepoin;
  after->stageprop = but->stageprop;

  if (but->type == UI_BTYPE_SEARCH_MENU) {
    uiButSearch *search_but = (uiButSearch *)but;
    after->search_arg_free_fn = search_but->arg_free_fn;
    after->search_arg = search_but->arg;
    search_but->arg_free_fn = NULL;
    search_but->arg = NULL;
  }

  if (but->active != NULL) {
    uiHandleButtonData *data = but->active;
    if (data->custom_interaction_handle != NULL) {
      after->custom_interaction_callbacks = block->custom_interaction_callbacks;
      after->custom_interaction_handle = data->custom_interaction_handle;

      /* Ensure this callback runs once and last. */
      uiAfterFunc *after_prev = UIAfterFuncs.back();
      if (after_prev &&
          (after_prev->custom_interaction_handle == data->custom_interaction_handle)) {
        after_prev->custom_interaction_handle = NULL;
        memset(&after_prev->custom_interaction_callbacks,
               0x0,
               sizeof(after_prev->custom_interaction_callbacks));
      } else {
        after->custom_interaction_handle->user_count++;
      }
    }
  }

  // if (but->context) {
  //   after->context = CTX_store_copy(but->context);
  // }

  UI_but_drawstr_without_sep_char(but, after->drawstr, sizeof(after->drawstr));

  but->optype = NULL;
  // but->opcontext = 0;
  but->opptr = NULL;
}

static void ui_apply_but_BUT(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  ui_apply_but_func(C, but);

  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_BUTM(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  UI_but_value_set(but, but->hardmin);
  ui_apply_but_func(C, but);

  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_BLOCK(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  /**
   * Now we strongly type the scene description type name
   * into a respective UI representation of the authored type. */

  if (data->value.IsValid()) {
    if (data->value.GetTypeName() == SdfValueTypeNames->Bool) {
      bool typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        UI_but_value_set(but, typedVal);
        UI_but_update_edited<bool>(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Int) {
      int typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        UI_but_value_set(but, typedVal);
        UI_but_update_edited<int>(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Float) {
      float typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        UI_but_value_set(but, typedVal);
        UI_but_update_edited<float>(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Token) {
      TfToken typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        UI_but_value_set(but, typedVal);
        UI_but_update_edited<TfToken>(but);
      }

    } else {
      double typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        UI_but_value_set(but, typedVal);
        UI_but_update_edited<double>(but);
      }
    }
  }

  ui_apply_but_func(C, but);
  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_TOG(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  const bool value = UI_but_value_get<bool>(but);
  int value_toggle;
  if (but->bit) {
    value_toggle = UI_BITBUT_VALUE_TOGGLED((int)value, but->bitnr);
  } else {
    value_toggle = (value == 0.0);
    if (ELEM(but->type, UI_BTYPE_TOGGLE_N, UI_BTYPE_ICON_TOGGLE_N, UI_BTYPE_CHECKBOX_N)) {
      value_toggle = !value_toggle;
    }
  }

  UI_but_value_set(but, (bool)value_toggle);
  if (ELEM(but->type, UI_BTYPE_ICON_TOGGLE, UI_BTYPE_ICON_TOGGLE_N)) {
    UI_but_update_edited<bool>(but);
  }

  ui_apply_but_func(C, but);

  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_ROW(kContext *C, uiBlock *block, uiBut *but, uiHandleButtonData *data)
{
  UI_but_value_set(but, but->hardmax);

  ui_apply_but_func(C, but);

  /* states of other row buttons */
  for (auto &bt : block->buttons) {
    if (bt != but && bt->poin == but->poin && ELEM(bt->type, UI_BTYPE_ROW, UI_BTYPE_LISTROW)) {
      UI_but_update_edited<wabi::TfToken>(bt);
    }
  }

  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_VIEW_ITEM(kContext *C,
                                   uiBlock *block,
                                   uiBut *but,
                                   uiHandleButtonData *data)
{
  if (data->apply_through_extra_icon) {
    /* Don't apply this, it would cause unintended tree-row toggling when clicking on extra icons.
     */
    return;
  }
  ui_apply_but_ROW(C, block, but, data);
}

static void ui_block_interaction_update(kContext *C,
                                        uiBlockInteraction_CallbackData *callbacks,
                                        uiBlockInteraction_Handle *interaction)
{
  KLI_assert(callbacks->update_fn != NULL);
  callbacks->update_fn(C, &interaction->params, callbacks->arg1, interaction->user_data);
}

static void ui_apply_but(kContext *C,
                         uiBlock *block,
                         uiBut *but,
                         uiHandleButtonData *data,
                         const bool interactive)
{
  const eButType but_type = but->type; /* Store as const to quiet maybe uninitialized warning. */

  data->retval = 0;

  /* if we cancel and have not applied yet, there is nothing to do,
   * otherwise we have to restore the original value again */
  if (data->cancel) {
    if (!data->applied) {
      return;
    }

    if (data->str) {
      delete data->str;
    }
    data->str = data->origstr;
    data->origstr = NULL;
    data->value = data->origvalue;
    copy_v3_v3(data->vec, data->origvec);
    /* postpone clearing origdata */
  } else {
    /* We avoid applying interactive edits a second time
     * at the end with the #uiHandleButtonData.applied_interactive flag. */
    if (interactive) {
      data->applied_interactive = true;
    } else if (data->applied_interactive) {
      return;
    }
  }

  /* ensures we are writing actual values */
  char *editstr = but->editstr;
  double *editval = but->editval;
  float *editvec = but->editvec;
  ColorBand *editcoba;
  CurveMapping *editcumap;
  CurveProfile *editprofile;
  if (but_type == UI_BTYPE_COLORBAND) {
    uiButColorBand *but_coba = (uiButColorBand *)but;
    editcoba = but_coba->edit_coba;
  } else if (but_type == UI_BTYPE_CURVE) {
    uiButCurveMapping *but_cumap = (uiButCurveMapping *)but;
    editcumap = but_cumap->edit_cumap;
  } else if (but_type == UI_BTYPE_CURVEPROFILE) {
    uiButCurveProfile *but_profile = (uiButCurveProfile *)but;
    editprofile = but_profile->edit_profile;
  }
  but->editstr = NULL;
  but->editval = NULL;
  but->editvec = NULL;
  if (but_type == UI_BTYPE_COLORBAND) {
    uiButColorBand *but_coba = (uiButColorBand *)but;
    but_coba->edit_coba = NULL;
  } else if (but_type == UI_BTYPE_CURVE) {
    uiButCurveMapping *but_cumap = (uiButCurveMapping *)but;
    but_cumap->edit_cumap = NULL;
  } else if (but_type == UI_BTYPE_CURVEPROFILE) {
    uiButCurveProfile *but_profile = (uiButCurveProfile *)but;
    but_profile->edit_profile = NULL;
  }

  /* handle different types */
  switch (but_type) {
    case UI_BTYPE_BUT:
    case UI_BTYPE_DECORATOR:
      ui_apply_but_BUT(C, but, data);
      break;
    case UI_BTYPE_TEXT:
    case UI_BTYPE_SEARCH_MENU:
      printf("todo: need to handle button type for text and search.\n");
      // ui_apply_but_TEX(C, but, data);
      break;
    case UI_BTYPE_BUT_TOGGLE:
    case UI_BTYPE_TOGGLE:
    case UI_BTYPE_TOGGLE_N:
    case UI_BTYPE_ICON_TOGGLE:
    case UI_BTYPE_ICON_TOGGLE_N:
    case UI_BTYPE_CHECKBOX:
    case UI_BTYPE_CHECKBOX_N:
      ui_apply_but_TOG(C, but, data);
      break;
    case UI_BTYPE_ROW:
      ui_apply_but_ROW(C, block, but, data);
      break;
    case UI_BTYPE_VIEW_ITEM:
      ui_apply_but_VIEW_ITEM(C, block, but, data);
      break;
    case UI_BTYPE_LISTROW:
      printf("todo: need to handle button type for listrow.\n");
      // ui_apply_but_LISTROW(C, block, but, data);
      break;
    case UI_BTYPE_TAB:
      printf("todo: need to handle button type for tab.\n");
      // ui_apply_but_TAB(C, but, data);
      break;
    case UI_BTYPE_SCROLL:
    case UI_BTYPE_GRIP:
    case UI_BTYPE_NUM:
    case UI_BTYPE_NUM_SLIDER:
      printf("todo: need to handle button type for scroll, grip, num, and slider.\n");
      // ui_apply_but_NUM(C, but, data);
      break;
    case UI_BTYPE_MENU:
    case UI_BTYPE_BLOCK:
    case UI_BTYPE_PULLDOWN:
      ui_apply_but_BLOCK(C, but, data);
      break;
    case UI_BTYPE_COLOR:
      if (data->cancel) {
        printf("todo: need to handle button type for vec.\n");
        // ui_apply_but_VEC(C, but, data);
      } else {
        ui_apply_but_BLOCK(C, but, data);
      }
      break;
    case UI_BTYPE_BUT_MENU:
      ui_apply_but_BUTM(C, but, data);
      break;
    case UI_BTYPE_UNITVEC:
    case UI_BTYPE_HSVCUBE:
    case UI_BTYPE_HSVCIRCLE:
      printf("todo: need to handle button type for vec.\n");
      // ui_apply_but_VEC(C, but, data);
      break;
    case UI_BTYPE_COLORBAND:
      printf("todo: need to handle button type for colorband.\n");
      // ui_apply_but_COLORBAND(C, but, data);
      break;
    case UI_BTYPE_CURVE:
      printf("todo: need to handle button type for curve.\n");
      // ui_apply_but_CURVE(C, but, data);
      break;
    case UI_BTYPE_CURVEPROFILE:
      printf("todo: need to handle button type for curveprofile.\n");
      // ui_apply_but_CURVEPROFILE(C, but, data);
      break;
    case UI_BTYPE_KEY_EVENT:
    case UI_BTYPE_HOTKEY_EVENT:
      ui_apply_but_BUT(C, but, data);
      break;
    case UI_BTYPE_IMAGE:
      printf("todo: need to handle button type for image.\n");
      // ui_apply_but_IMAGE(C, but, data);
      break;
    case UI_BTYPE_HISTOGRAM:
      printf("todo: need to handle button type for histogram.\n");
      // ui_apply_but_HISTOGRAM(C, but, data);
      break;
    case UI_BTYPE_WAVEFORM:
      printf("todo: need to handle button type for waveform.\n");
      // ui_apply_but_WAVEFORM(C, but, data);
      break;
    case UI_BTYPE_TRACK_PREVIEW:
      printf("todo: need to handle button type for track preview.\n");
      // ui_apply_but_TRACKPREVIEW(C, but, data);
      break;
    default:
      break;
  }

#ifdef USE_DRAG_MULTINUM
  if (data->multi_data.has_mbuts) {
    if ((data->multi_data.init == BUTTON_MULTI_INIT_ENABLE) && (data->multi_data.skip == false)) {
      if (data->cancel) {
        ui_multibut_restore(C, data, block);
      } else {
        ui_multibut_states_apply(C, data, block);
      }
    }
  }
#endif

#ifdef USE_ALLSELECT
  ui_selectcontext_apply(C, but, &data->select_others, data->value, data->origvalue);
#endif

  if (data->cancel) {
    data->origvalue.Set(double(0.0));
    zero_v3(data->origvec);
  }

  but->editstr = editstr;
  but->editval = editval;
  but->editvec = editvec;
  if (but_type == UI_BTYPE_COLORBAND) {
    uiButColorBand *but_coba = (uiButColorBand *)but;
    but_coba->edit_coba = editcoba;
  } else if (but_type == UI_BTYPE_CURVE) {
    uiButCurveMapping *but_cumap = (uiButCurveMapping *)but;
    but_cumap->edit_cumap = editcumap;
  } else if (but_type == UI_BTYPE_CURVEPROFILE) {
    uiButCurveProfile *but_profile = (uiButCurveProfile *)but;
    but_profile->edit_profile = editprofile;
  }

  if (data->custom_interaction_handle != NULL) {
    ui_block_interaction_update(C,
                                &block->custom_interaction_callbacks,
                                data->custom_interaction_handle);
  }
}

uiBut *UI_region_active_but_get(const ARegion *region)
{
  return ui_context_button_active(region, NULL);
}

/* -------------------------------------------------------------------- */
/** \name Events for Various Button Types
 * \{ */

static uiButExtraOpIcon *ui_but_extra_operator_icon_mouse_over_get(uiBut *but,
                                                                   ARegion *region,
                                                                   const wmEvent *event)
{
  if (but->extra_op_icons.empty()) {
    return NULL;
  }

  int x = event->mouse_pos[0], y = event->mouse_pos[1];
  UI_window_to_block(region, but->block, &x, &y);
  if (!KLI_rctf_isect_pt(but->rect, x, y)) {
    return NULL;
  }

  const float icon_size = 0.8f * KLI_rctf_size_y(but->rect); /* ICON_SIZE_FROM_BUTRECT */
  float xmax = but->rect[1];
  /* Same as in 'widget_draw_extra_icons', icon padding from the right edge. */
  xmax -= 0.2 * icon_size;

  /* Handle the padding space from the right edge as the last button. */
  if (x > xmax) {
    return but->extra_op_icons.back();
  }

  /* Inverse order, from right to left. */
  for (auto &op_icon : but->extra_op_icons) {
    if ((x > (xmax - icon_size)) && x <= xmax) {
      return op_icon;
    }
    xmax -= icon_size;
  }

  return NULL;
}

static ARegion *ui_but_tooltip_init(kContext *C, ARegion *region, int *pass, double *r_pass_delay, bool *r_exit_on_event)
{
  bool is_label = false;
  if (*pass == 1) {
    is_label = true;
    (*pass)--;
    (*r_pass_delay) = UI_TOOLTIP_DELAY - UI_TOOLTIP_DELAY_LABEL;
  }

  uiBut *but = UI_region_active_but_get(region);
  *r_exit_on_event = false;
  if (but) {
    const wmWindow *win = CTX_wm_window(C);
    uiButExtraOpIcon *extra_icon = ui_but_extra_operator_icon_mouse_over_get(
        but, but->active ? but->active->region : region, win->eventstate);

    return UI_tooltip_create_from_button_or_extra_icon(C, region, but, extra_icon, is_label);
  }
  return NULL;
}

static void ui_block_interaction_end(kContext *C,
                                     uiBlockInteraction_CallbackData *callbacks,
                                     uiBlockInteraction_Handle *interaction)
{
  KLI_assert(callbacks->end_fn != NULL);
  callbacks->end_fn(C, &interaction->params, callbacks->arg1, interaction->user_data);
  delete interaction->params.unique_retval_ids;
  delete interaction;
}

static void button_tooltip_timer_reset(kContext *C, uiBut *but)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  uiHandleButtonData *data = but->active;

  WM_tooltip_timer_clear(C, data->window);

  if ((UI_FLAG & USER_TOOLTIPS) || (data->tooltip_force)) {
    if (!but->block->tooltipdisabled) {
      if (!wm->drags.front()) {
        const bool is_label = UI_but_has_tooltip_label(but);
        const double delay = is_label ? UI_TOOLTIP_DELAY_LABEL : UI_TOOLTIP_DELAY;
        WM_tooltip_timer_init_ex(C,
                                 data->window,
                                 data->area,
                                 data->region,
                                 ui_but_tooltip_init,
                                 delay);
        if (is_label) {
          kScreen *screen = WM_window_get_active_screen(data->window);
          if (screen->tool_tip) {
            screen->tool_tip->pass = 1;
          }
        }
      }
    }
  }
}

static void ui_def_but_rna__panel_type(kContext *C, uiLayout *layout, void *but_p)
{
  uiBut *but = static_cast<uiBut *>(but_p);
  const char *panel_type = static_cast<const char *>(but->func_argN);
  PanelType *pt = WM_paneltype_find(panel_type, true);
  if (pt) {
    ui_item_paneltype_func(C, layout, pt);
  }
  else {
    char msg[256];
    SNPRINTF(msg, TIP_("Missing Panel: %s"), panel_type);
    uiItemL(layout, msg, ICON_NONE);
  }
}

bool UI_but_menu_draw_as_popover(const uiBut *but)
{
  return (but->menu_create_func == ui_def_but_rna__panel_type);
}

static void button_activate_state(kContext *C, uiBut *but, uiHandleButtonState state)
{
  uiHandleButtonData *data = but->active;
  if (data->state == state) {
    return;
  }

  /* Highlight has timers for tool-tips and auto open. */
  if (state == BUTTON_STATE_HIGHLIGHT) {
    but->flag &= ~UI_SELECT;

    button_tooltip_timer_reset(C, but);

    /* Automatic open pull-down block timer. */
    if (ELEM(but->type, UI_BTYPE_BLOCK, UI_BTYPE_PULLDOWN, UI_BTYPE_POPOVER) ||
        /* Menu button types may draw as popovers, check for this case
         * ignoring other kinds of menus (mainly enums). (see T66538). */
        ((but->type == UI_BTYPE_MENU) &&
         (UI_but_paneltype_get(but) || ui_but_menu_draw_as_popover(but)))) {
      if (data->used_mouse && !data->autoopentimer) {
        int time;

        if (but->block->auto_open == true) { /* test for toolbox */
          time = 1;
        } else if ((but->block->flag & UI_BLOCK_LOOP && but->type != UI_BTYPE_BLOCK) ||
                   (but->block->auto_open == true)) {
          time = 5 * U.menuthreshold2;
        } else if (U.uiflag & USER_MENUOPENAUTO) {
          time = 5 * U.menuthreshold1;
        } else {
          time = -1; /* do nothing */
        }

        if (time >= 0) {
          data->autoopentimer = WM_event_add_timer(data->wm,
                                                   data->window,
                                                   TIMER,
                                                   0.02 * (double)time);
        }
      }
    }
  } else {
    but->flag |= UI_SELECT;
    UI_but_tooltip_timer_remove(C, but);
  }

  /* text editing */
  if (state == BUTTON_STATE_TEXT_EDITING && data->state != BUTTON_STATE_TEXT_SELECTING) {
    ui_textedit_begin(C, but, data);
  } else if (data->state == BUTTON_STATE_TEXT_EDITING && state != BUTTON_STATE_TEXT_SELECTING) {
    ui_textedit_end(C, but, data);
  } else if (data->state == BUTTON_STATE_TEXT_SELECTING && state != BUTTON_STATE_TEXT_EDITING) {
    ui_textedit_end(C, but, data);
  }

  /* number editing */
  if (state == BUTTON_STATE_NUM_EDITING) {
    if (ui_but_is_cursor_warp(but)) {
      WM_cursor_grab_enable(CTX_wm_window(C), WM_CURSOR_WRAP_XY, true, NULL);
    }
    ui_numedit_begin(but, data);
  } else if (data->state == BUTTON_STATE_NUM_EDITING) {
    ui_numedit_end(but, data);

    if (but->flag & UI_BUT_DRIVEN) {
      /* Only warn when editing stepping/dragging the value.
       * No warnings should show for editing driver expressions though!
       */
      if (state != BUTTON_STATE_TEXT_EDITING) {
        WM_report(RPT_INFO,
                  "Can't edit driven number value, see graph editor for the driver setup.");
      }
    }

    if (ui_but_is_cursor_warp(but)) {

#ifdef USE_CONT_MOUSE_CORRECT
      /* stereo3d has issues with changing cursor location so rather avoid */
      if (data->ungrab_mval[0] != FLT_MAX && !WM_stereo3d_enabled(data->window, false)) {
        int mouse_ungrab_xy[2];
        ui_block_to_window_fl(data->region,
                              but->block,
                              &data->ungrab_mval[0],
                              &data->ungrab_mval[1]);
        mouse_ungrab_xy[0] = data->ungrab_mval[0];
        mouse_ungrab_xy[1] = data->ungrab_mval[1];

        WM_cursor_grab_disable(data->window, mouse_ungrab_xy);
      } else {
        WM_cursor_grab_disable(data->window, NULL);
      }
#else
      WM_cursor_grab_disable(data->window, NULL);
#endif
    }
  }
  /* menu open */
  if (state == BUTTON_STATE_MENU_OPEN) {
    ui_block_open_begin(C, but, data);
  } else if (data->state == BUTTON_STATE_MENU_OPEN) {
    ui_block_open_end(C, but, data);
  }

  /* add a short delay before exiting, to ensure there is some feedback */
  if (state == BUTTON_STATE_WAIT_FLASH) {
    data->flashtimer = WM_event_add_timer(data->wm, data->window, TIMER, BUTTON_FLASH_DELAY);
  } else if (data->flashtimer) {
    WM_event_remove_timer(data->wm, data->window, data->flashtimer);
    data->flashtimer = NULL;
  }

  /* add hold timer if it's used */
  if (state == BUTTON_STATE_WAIT_RELEASE && (but->hold_func != NULL)) {
    data->hold_action_timer = WM_event_add_timer(data->wm,
                                                 data->window,
                                                 TIMER,
                                                 BUTTON_AUTO_OPEN_THRESH);
  } else if (data->hold_action_timer) {
    WM_event_remove_timer(data->wm, data->window, data->hold_action_timer);
    data->hold_action_timer = NULL;
  }

  /* Add a blocking ui handler at the window handler for blocking, modal states
   * but not for popups, because we already have a window level handler. */
  if (!(but->block->handle && but->block->handle->popup)) {
    if (button_modal_state(state)) {
      if (!button_modal_state(data->state)) {
        WM_event_add_ui_handler(C,
                                &data->window->modalhandlers,
                                ui_handler_region_menu,
                                NULL,
                                data,
                                0);
      }
    } else {
      if (button_modal_state(data->state)) {
        /* true = postpone free */
        WM_event_remove_ui_handler(&data->window->modalhandlers,
                                   ui_handler_region_menu,
                                   NULL,
                                   data,
                                   true);
      }
    }
  }

  /* Wait for mouse-move to enable drag. */
  if (state == BUTTON_STATE_WAIT_DRAG) {
    but->flag &= ~UI_SELECT;
  }

  if (state == BUTTON_STATE_TEXT_EDITING) {
    ui_block_interaction_begin_ensure(C, but->block, data, true);
  } else if (state == BUTTON_STATE_EXIT) {
    if (data->state == BUTTON_STATE_NUM_EDITING) {
      /* This happens on pasting values for example. */
      ui_block_interaction_begin_ensure(C, but->block, data, true);
    }
  }

  data->state = state;

  if (state != BUTTON_STATE_EXIT) {
    /* When objects for eg. are removed, running ui_but_update() can access
     * the removed data - so disable update on exit. Also in case of
     * highlight when not in a popup menu, we remove because data used in
     * button below popup might have been removed by action of popup. Needs
     * a more reliable solution... */
    if (state != BUTTON_STATE_HIGHLIGHT || (but->block->flag & UI_BLOCK_LOOP)) {
      ui_but_update(but);
    }
  }

  /* redraw */
  ED_region_tag_redraw_no_rebuild(data->region);
}

static void button_activate_exit(kContext *C,
                                 uiBut *but,
                                 uiHandleButtonData *data,
                                 const bool mousemove,
                                 const bool onfree)
{
  wmWindow *win = data->window;
  uiBlock *block = but->block;

  if (but->type == UI_BTYPE_GRIP) {
    WM_cursor_modal_restore(win);
  }

  /* ensure we are in the exit state */
  if (data->state != BUTTON_STATE_EXIT) {
    button_activate_state(C, but, BUTTON_STATE_EXIT);
  }

  /* apply the button action or value */
  if (!onfree) {
    ui_apply_but(C, block, but, data, false);
  }

#ifdef USE_DRAG_MULTINUM
  if (data->multi_data.has_mbuts) {
    for(auto &bt, block->buttons)
    {
      if (bt->flag & UI_BUT_DRAG_MULTI) {
        bt->flag &= ~UI_BUT_DRAG_MULTI;

        if (!data->cancel) {
          ui_apply_but_autokey(C, bt);
        }
      }
    }

    ui_multibut_free(data, block);
  }
#endif

  /* if this button is in a menu, this will set the button return
   * value to the button value and the menu return value to ok, the
   * menu return value will be picked up and the menu will close */
  if (block->handle && !(block->flag & UI_BLOCK_KEEP_OPEN)) {
    if (!data->cancel || data->escapecancel) {
      uiPopupBlockHandle *menu;

      menu = block->handle;
      menu->butretval = data->retval;
      menu->menuretval = (data->cancel) ? UI_RETURN_CANCEL : UI_RETURN_OK;
    }
  }

  if (!onfree && !data->cancel) {
    /* autokey & undo push */
    ui_apply_but_undo(but);
    ui_apply_but_autokey(C, but);

#ifdef USE_ALLSELECT
    {
      /* only RNA from this button is used */
      uiBut but_temp = *but;
      uiSelectContextStore *selctx_data = &data->select_others;
      for (int i = 0; i < selctx_data->elems_len; i++) {
        uiSelectContextElem *other = &selctx_data->elems[i];
        but_temp.rnapoin = other->ptr;
        ui_apply_but_autokey(C, &but_temp);
      }
    }
#endif

    /* popup menu memory */
    if (block->flag & UI_BLOCK_POPUP_MEMORY) {
      ui_popup_menu_memory_set(block, but);
    }

    if (U.runtime.is_dirty == false) {
      ui_but_update_preferences_dirty(but);
    }
  }

  /* Disable tool-tips until mouse-move + last active flag. */
  for(auto &block_iter : data->region->uiblocks)
  {
    for(auto &bt : block_iter->buttons)
    {
      bt->flag &= ~UI_BUT_LAST_ACTIVE;
    }

    block_iter->tooltipdisabled = true;
  }

  ui_blocks_set_tooltips(data->region, false);

  /* clean up */
  if (data->str) {
    delete data->str;
  }
  if (data->origstr) {
    delete data->origstr;
  }

  if (data->changed_cursor) {
    WM_cursor_modal_restore(data->window);
  }

  /* redraw and refresh (for popups) */
  ED_region_tag_redraw_no_rebuild(data->region);
  ED_region_tag_refresh_ui(data->region);

  if ((but->flag & UI_BUT_DRAG_MULTI) == 0) {
    if (data->custom_interaction_handle != NULL) {
      /* Should only set when the button is modal. */
      KLI_assert(but->active != NULL);
      data->custom_interaction_handle->user_count--;

      KLI_assert(data->custom_interaction_handle->user_count >= 0);
      if (data->custom_interaction_handle->user_count == 0) {
        ui_block_interaction_end(C,
                                 &but->block->custom_interaction_callbacks,
                                 data->custom_interaction_handle);
      }
      data->custom_interaction_handle = NULL;
    }
  }

  /* clean up button */
  MEM_SAFE_FREE(but->active);

  but->flag &= ~(UI_ACTIVE | UI_SELECT);
  but->flag |= UI_BUT_LAST_ACTIVE;
  if (!onfree) {
    ui_but_update(but);
  }

  /* Adds empty mouse-move in queue for re-initialize handler, in case mouse is
   * still over a button. We cannot just check for this ourselves because
   * at this point the mouse may be over a button in another region. */
  if (mousemove) {
    WM_event_add_mousemove(CTX_wm_window(C));
  }
}

static void ui_handle_button_return_submenu(kContext *C, const wmEvent *event, uiBut *but)
{
  uiHandleButtonData *data = but->active;
  uiPopupBlockHandle *menu = data->menu;

  /* copy over return values from the closing menu */
  if ((menu->menuretval & UI_RETURN_OK) || (menu->menuretval & UI_RETURN_UPDATE)) {
    if (but->type == UI_BTYPE_COLOR) {
      copy_v3_v3(data->vec, menu->retvec);
    } else if (but->type == UI_BTYPE_MENU) {
      data->value.Set((int)menu->retvalue);
    }
  }

  if (menu->menuretval & UI_RETURN_UPDATE) {
    if (data->interactive) {
      ui_apply_but(C, but->block, but, data, true);
    } else {
      UI_but_update<int>(but);
    }

    menu->menuretval = 0;
  }

  /* now change button state or exit, which will close the submenu */
  if ((menu->menuretval & UI_RETURN_OK) || (menu->menuretval & UI_RETURN_CANCEL)) {
    if (menu->menuretval != UI_RETURN_OK) {
      data->cancel = true;
    }

    button_activate_exit(C, but, data, true, false);
  } else if (menu->menuretval & UI_RETURN_OUT) {
    if (event->type == MOUSEMOVE && ui_but_contains_point_px(but, data->region, event->xy)) {
      button_activate_state(C, but, BUTTON_STATE_HIGHLIGHT);
    } else {
      if (ISKEYBOARD(event->type)) {
        /* keyboard menu hierarchy navigation, going back to previous level */
        but->active->used_mouse = false;
        button_activate_state(C, but, BUTTON_STATE_HIGHLIGHT);
      } else {
        data->cancel = true;
        button_activate_exit(C, but, data, true, false);
      }
    }
  }
}

static int ui_handle_menu_return_submenu(kContext *C,
                                         const wmEvent *event,
                                         uiPopupBlockHandle *menu)
{
  ARegion *region = menu->region;
  uiBlock *block = region->uiblocks.front();

  uiBut *but = UI_region_find_active_but(region);

  KLI_assert(but);

  uiHandleButtonData *data = but->active;
  uiPopupBlockHandle *submenu = data->menu;

  if (submenu->menuretval) {
    bool update;

    /* first decide if we want to close our own menu cascading, if
     * so pass on the sub menu return value to our own menu handle */
    if ((submenu->menuretval & UI_RETURN_OK) || (submenu->menuretval & UI_RETURN_CANCEL)) {
      if (!(block->flag & UI_BLOCK_KEEP_OPEN)) {
        menu->menuretval = submenu->menuretval;
        menu->butretval = data->retval;
      }
    }

    update = (submenu->menuretval & UI_RETURN_UPDATE) != 0;

    /* now let activated button in this menu exit, which
     * will actually close the submenu too */
    ui_handle_button_return_submenu(C, event, but);

    if (update) {
      submenu->menuretval = 0;
    }
  }

  if (block->flag & (UI_BLOCK_MOVEMOUSE_QUIT | UI_BLOCK_POPOVER)) {
    /* for cases where close does not cascade, allow the user to
     * move the mouse back towards the menu without closing */
    ui_mouse_motion_towards_reinit(menu, event->xy);
  }

  if (menu->menuretval) {
    return WM_UI_HANDLER_CONTINUE;
  }
  return WM_UI_HANDLER_BREAK;
}

static int ui_handle_menus_recursive(kContext *C,
                                     const wmEvent *event,
                                     uiPopupBlockHandle *menu,
                                     int level,
                                     const bool is_parent_inside,
                                     const bool is_parent_menu,
                                     const bool is_floating)
{
  int retval = WM_UI_HANDLER_CONTINUE;
  bool do_towards_reinit = false;

  /* check if we have a submenu, and handle events for it first */
  uiBut *but = UI_region_find_active_but(menu->region);
  uiHandleButtonData *data = (but) ? but->active : NULL;
  uiPopupBlockHandle *submenu = (data) ? data->menu : NULL;

  if (submenu) {
    uiBlock *block = menu->region->uiblocks.front();
    const bool is_menu = UI_block_is_menu(block);
    bool inside = false;
    /* root pie menus accept the key that spawned
     * them as double click to improve responsiveness */
    const bool do_recursion = (!(block->flag & UI_BLOCK_RADIAL) ||
                               event->type != block->pie_data.event_type);

    if (do_recursion) {
      if (is_parent_inside == false) {
        int mx = event->mouse_pos[0];
        int my = event->mouse_pos[1];
        UI_window_to_block(menu->region, block, &mx, &my);
        inside = KLI_rctf_isect_pt(block->rect, mx, my);
      }

      retval = ui_handle_menus_recursive(C,
                                         event,
                                         submenu,
                                         level + 1,
                                         is_parent_inside || inside,
                                         is_menu,
                                         false);
    }
  }

  /* now handle events for our own menu */
  if (retval == WM_UI_HANDLER_CONTINUE || event->type == TIMER) {
    const bool do_but_search = (but && (but->type == UI_BTYPE_SEARCH_MENU));
    if (submenu && submenu->menuretval) {
      const bool do_ret_out_parent = (submenu->menuretval & UI_RETURN_OUT_PARENT) != 0;
      retval = ui_handle_menu_return_submenu(C, event, menu);
      submenu = NULL; /* hint not to use this, it may be freed by call above */
      (void)submenu;
      /* we may want to quit the submenu and handle the even in this menu,
       * if its important to use it, check 'data->menu' first */
      if (((retval == WM_UI_HANDLER_BREAK) && do_ret_out_parent) == false) {
        /* skip applying the event */
        return retval;
      }
    }

    if (do_but_search) {
      uiBlock *block = menu->region->uiblocks.first;

      retval = ui_handle_menu_button(C, event, menu);

      if (block->flag & (UI_BLOCK_MOVEMOUSE_QUIT | UI_BLOCK_POPOVER)) {
        /* when there is a active search button and we close it,
         * we need to reinit the mouse coords T35346. */
        if (ui_region_find_active_but(menu->region) != but) {
          do_towards_reinit = true;
        }
      }
    } else {
      uiBlock *block = menu->region->uiblocks.first;
      uiBut *listbox = ui_list_find_mouse_over(menu->region, event);

      if (block->flag & UI_BLOCK_RADIAL) {
        retval = ui_pie_handler(C, event, menu);
      } else if (event->type == LEFTMOUSE || event->val != KM_DBL_CLICK) {
        bool handled = false;

        if (listbox) {
          const int retval_test = ui_handle_list_event(C, event, menu->region, listbox);
          if (retval_test != WM_UI_HANDLER_CONTINUE) {
            retval = retval_test;
            handled = true;
          }
        }

        if (handled == false) {
          retval = ui_handle_menu_event(C,
                                        event,
                                        menu,
                                        level,
                                        is_parent_inside,
                                        is_parent_menu,
                                        is_floating);
        }
      }
    }
  }

  if (do_towards_reinit) {
    ui_mouse_motion_towards_reinit(menu, event->xy);
  }

  return retval;
}

/* two types of popups, one with operator + enum, other with regular callbacks */
static int ui_popup_handler(kContext *C, const wmEvent *event, void *userdata)
{
  uiPopupBlockHandle *menu = (uiPopupBlockHandle *)userdata;
  /* we block all events, this is modal interaction,
   * except for drop events which is described below */
  int retval = WM_UI_HANDLER_BREAK;
  bool reset_pie = false;

  ARegion *menu_region = CTX_wm_menu(C);
  CTX_wm_menu_set(C, menu->region);

  if (event->type == EVT_DROP || event->val == KM_DBL_CLICK) {
    /* EVT_DROP:
     *   If we're handling drop event we'll want it to be handled by popup callee as well,
     *   so it'll be possible to perform such operations as opening .blend files by dropping
     *   them into blender, even if there's opened popup like splash screen (sergey).
     * KM_DBL_CLICK:
     *   Continue in case of double click so wm_handlers_do calls handler again with KM_PRESS
     *   event. This is needed to ensure correct button handling for fast clicking (T47532).
     */

    retval = WM_UI_HANDLER_CONTINUE;
  }

  ui_handle_menus_recursive(C, event, menu, 0, false, false, true);

  /* free if done, does not free handle itself */
  if (menu->menuretval) {
    wmWindow *win = CTX_wm_window(C);
    /* copy values, we have to free first (closes region) */
    const uiPopupBlockHandle temp = *menu;
    uiBlock *block = menu->region->uiblocks.first;

    /* set last pie event to allow chained pie spawning */
    if (block->flag & UI_BLOCK_RADIAL) {
      win->pie_event_type_last = block->pie_data.event_type;
      reset_pie = true;
    }

    ui_popup_block_free(C, menu);
    UI_popup_handlers_remove(&win->modalhandlers, menu);
    CTX_wm_menu_set(C, NULL);

#ifdef USE_DRAG_TOGGLE
    {
      WM_event_free_ui_handler_all(C,
                                   &win->modalhandlers,
                                   ui_handler_region_drag_toggle,
                                   ui_handler_region_drag_toggle_remove);
    }
#endif

    if ((temp.menuretval & UI_RETURN_OK) || (temp.menuretval & UI_RETURN_POPUP_OK)) {
      if (temp.popup_func) {
        temp.popup_func(C, temp.popup_arg, temp.retvalue);
      }
    } else if (temp.cancel_func) {
      temp.cancel_func(C, temp.popup_arg);
    }

    WM_event_add_mousemove(win);
  } else {
    /* Re-enable tool-tips */
    if (event->type == MOUSEMOVE &&
        (event->xy[0] != event->prev_xy[0] || event->xy[1] != event->prev_xy[1])) {
      ui_blocks_set_tooltips(menu->region, true);
    }
  }

  /* delayed apply callbacks */
  ui_apply_but_funcs_after(C);

  if (reset_pie) {
    /* Reacquire window in case pie invalidates it somehow. */
    wmWindow *win = CTX_wm_window(C);

    if (win) {
      win->pie_event_type_last = EVENT_NONE;
    }
  }

  CTX_wm_region_set(C, menu_region);

  return retval;
}

void UI_popup_handlers_add(kContext *C,
                           std::vector<wmEventHandler *> handlers,
                           uiPopupBlockHandle *popup,
                           char flag)
{
  WM_event_add_ui_handler(C, handlers, ui_popup_handler, ui_popup_handler_remove, popup, flag);
}

/* -------------------------------------------------------------------- */
/** \name UI Block Interaction API
 * \{ */

void UI_block_interaction_set(uiBlock *block, uiBlockInteraction_CallbackData *callbacks)
{
  block->custom_interaction_callbacks = *callbacks;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name UI Queries
 * \{ */

bool UI_but_is_editing(const uiBut *but)
{
  const uiHandleButtonData *data = but->active;
  return (data && ELEM(data->state, BUTTON_STATE_TEXT_EDITING, BUTTON_STATE_NUM_EDITING));
}

/** \} */

KRAKEN_NAMESPACE_END