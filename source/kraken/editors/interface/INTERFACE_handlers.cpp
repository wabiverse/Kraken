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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kraken/kraken.h"

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_factory.h"
#include "USD_operator.h"
#include "USD_userpref.h"
#include "USD_types.h"
#include "USD_object.h"
#include "USD_curveprofile_types.h"
#include "USD_texture_types.h"

#include "WM_cursors.h"
#include "WM_cursors_api.h"
#include "WM_window.h"
#include "WM_event_system.h"
#include "WM_tooltip.h"

#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "KLI_math.h"
#include "KLI_rect.h"
#include "KLI_string_utf8.h"
#include "KLI_time.h"

#include "KKE_context.h"

#include "LUXO_access.h"

#include "interface_intern.h"

/* -------------------------------------------------------------------- */
/** \name Feature Defines
 *
 * These defines allow developers to locally toggle functionality which
 * may be useful for testing (especially conflicts in dragging).
 * Ideally the code would be refactored to support this functionality in a less fragile way.
 * Until then keep these defines.
 * \{ */

/** Place the mouse at the scaled down location when un-grabbing. */
#define USE_CONT_MOUSE_CORRECT
/** Support dragging toggle buttons. */
#define USE_DRAG_TOGGLE

/** Support dragging multiple number buttons at once. */
#define USE_DRAG_MULTINUM

/** Allow dragging/editing all other selected items at once. */
#define USE_ALLSELECT

/**
 * Check to avoid very small mouse-moves from jumping away from keyboard navigation,
 * while larger mouse motion will override keyboard input, see: T34936.
 */
#define USE_KEYNAV_LIMIT

/** Support dragging popups by their header. */
#define USE_DRAG_POPUP

/** \} */

/* -------------------------------------------------------------------- */
/** \name Local Defines
 * \{ */

/**
 * The buffer side used for password strings, where the password is stored internally,
 * but not displayed.
 */
#define UI_MAX_PASSWORD_STR 128

/**
 * This is a lower limit on the soft minimum of the range.
 * Usually the derived lower limit from the visible precision is higher,
 * so this number is the backup minimum.
 *
 * Logarithmic scale does not work with a minimum value of zero,
 * but we want to support it anyway. It is set to 0.5e... for
 * correct rounding since when the tweaked value is lower than
 * the log minimum (lower limit), it will snap to 0.
 */
#define UI_PROP_SCALE_LOG_MIN 0.5e-8f
/**
 * This constant defines an offset for the precision change in
 * snap rounding, when going to higher values. It is set to
 * `0.5 - log10(3) = 0.03` to make the switch at `0.3` values.
 */
#define UI_PROP_SCALE_LOG_SNAP_OFFSET 0.03f

/**
 * When #USER_CONTINUOUS_MOUSE is disabled or tablet input is used,
 * Use this as a maximum soft range for mapping cursor motion to the value.
 * Otherwise min/max of #FLT_MAX, #INT_MAX cause small adjustments to jump to large numbers.
 *
 * This is needed for values such as location & dimensions which don't have a meaningful min/max,
 * Instead of mapping cursor motion to the min/max, map the motion to the click-step.
 *
 * This value is multiplied by the click step to calculate a range to clamp the soft-range by.
 * See: T68130
 */
#define UI_DRAG_MAP_SOFT_RANGE_PIXEL_MAX 1000

/** \} */

/* -------------------------------------------------------------------- */
/** \name Structs & Defines
 * \{ */

#define BUTTON_FLASH_DELAY 0.020
#define MENU_SCROLL_INTERVAL 0.1
#define PIE_MENU_INTERVAL 0.01
#define BUTTON_AUTO_OPEN_THRESH 0.2
#define BUTTON_MOUSE_TOWARDS_THRESH 1.0
/** Pixels to move the cursor to get out of keyboard navigation. */
#define BUTTON_KEYNAV_PX_LIMIT 8

/** Margin around the menu, use to check if we're moving towards this rectangle (in pixels). */
#define MENU_TOWARDS_MARGIN 20
/** Tolerance for closing menus (in pixels). */
#define MENU_TOWARDS_WIGGLE_ROOM 64
/** Drag-lock distance threshold (in pixels). */
#define BUTTON_DRAGLOCK_THRESH 3

/** \} */

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

#ifdef USE_ALLSELECT

/* Unfortunately there's no good way handle more generally:
 * (propagate single clicks on layer buttons to other objects) */
#  define USE_ALLSELECT_LAYER_HACK

struct uiSelectContextElem
{
  KrakenPRIM ptr;
  union
  {
    bool val_b;
    int val_i;
    float val_f;
  };
};

struct uiSelectContextStore
{
  uiSelectContextElem *elems;
  int elems_len;
  bool do_free;
  bool is_enabled;
  /* When set, simply copy values (don't apply difference).
   * Rules are:
   * - dragging numbers uses delta.
   * - typing in values will assign to all. */
  bool is_copy;
};

static bool ui_selectcontext_begin(kContext *C, uiBut *but, uiSelectContextStore *selctx_data);
static void ui_selectcontext_end(uiBut *but, uiSelectContextStore *selctx_data);
static void ui_selectcontext_apply(kContext *C,
                                   uiBut *but,
                                   uiSelectContextStore *selctx_data,
                                   const double value,
                                   const double value_orig);

#  define IS_ALLSELECT_EVENT(event) (((event)->modifier & KM_ALT) != 0)

/** just show a tinted color so users know its activated */
#  define UI_BUT_IS_SELECT_CONTEXT UI_BUT_NODE_ACTIVE

#endif /* USE_ALLSELECT */

#ifdef USE_DRAG_MULTINUM

/**
 * how far to drag before we check for gesture direction (in pixels),
 * NOTE: half the height of a button is about right... */
#  define DRAG_MULTINUM_THRESHOLD_DRAG_X (UI_UNIT_Y / 4)

/**
 * How far to drag horizontally
 * before we stop checking which buttons the gesture spans (in pixels),
 * locking down the buttons so we can drag freely without worrying about vertical movement.
 */
#  define DRAG_MULTINUM_THRESHOLD_DRAG_Y (UI_UNIT_Y / 4)

/**
 * How strict to be when detecting a vertical gesture:
 * [0.5 == sloppy], [0.9 == strict], (unsigned dot-product).
 *
 * @note We should be quite strict here,
 * since doing a vertical gesture by accident should be avoided,
 * however with some care a user should be able to do a vertical movement without _missing_.
 */
#  define DRAG_MULTINUM_THRESHOLD_VERTICAL (0.75f)

/* a simple version of uiHandleButtonData when accessing multiple buttons */
struct uiButMultiState
{
  double origvalue;
  uiBut *but;

#  ifdef USE_ALLSELECT
  uiSelectContextStore select_others;
#  endif
};

enum eHandleButtonMultiInit
{
  /** gesture direction unknown, wait until mouse has moved enough... */
  BUTTON_MULTI_INIT_UNSET = 0,
  /** vertical gesture detected, flag buttons interactively (UI_BUT_DRAG_MULTI) */
  BUTTON_MULTI_INIT_SETUP,
  /** flag buttons finished, apply horizontal motion to active and flagged */
  BUTTON_MULTI_INIT_ENABLE,
  /** vertical gesture _not_ detected, take no further action */
  BUTTON_MULTI_INIT_DISABLE,
};

struct uiHandleButtonMulti
{
  eHandleButtonMultiInit init;

  bool has_mbuts; /* any buttons flagged UI_BUT_DRAG_MULTI */
  std::vector<uiButMultiState *> mbuts;
  uiButStore *bs_mbuts;

  bool is_proportional;

  /* In some cases we directly apply the changes to multiple buttons,
   * so we don't want to do it twice. */
  bool skip;

  /* before activating, we need to check gesture direction accumulate signed cursor movement
   * here so we can tell if this is a vertical motion or not. */
  float drag_dir[2];

  /* values copied direct from event->mouse_pos
   * used to detect buttons between the current and initial mouse position */
  int drag_start[2];

  /* store x location once BUTTON_MULTI_INIT_SETUP is set,
   * moving outside this sets BUTTON_MULTI_INIT_ENABLE */
  int drag_lock_x;
};

#endif /* USE_DRAG_MULTINUM */

struct uiUndoStack_Text_State
{
  struct uiUndoStack_Text_State *next, *prev;
  int cursor_index;
  char text[0];
};

struct uiUndoStack_Text
{
  std::vector<uiUndoStack_Text_State *> states;
  uiUndoStack_Text_State *current;
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

  /* post activate */
  uiButtonActivateType posttype;
  uiBut *postbut;

#ifdef USE_DRAG_MULTINUM
  /* Multi-buttons will be updated in unison with the active button. */
  uiHandleButtonMulti multi_data;
#endif

#ifdef USE_ALLSELECT
  uiSelectContextStore select_others;
#endif

#ifdef USE_CONT_MOUSE_CORRECT
  /* when ungrabbing buttons which are #ui_but_is_cursor_warp(),
   * we may want to position them.
   * FLT_MAX signifies do-nothing, use #ui_block_to_window_fl()
   * to get this into a usable space. */
  float ungrab_mval[2];
#endif

  /* Text field undo. */
  struct uiUndoStack_Text *undo_stack_text;
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
  kraken::eWmOperatorContext opcontext;
  KrakenPRIM *opptr;

  KrakenPRIM stagepoin;
  KrakenPROP *stageprop;

  void *search_arg;
  uiFreeArgFunc search_arg_free_fn;

  uiBlockInteraction_CallbackData custom_interaction_callbacks;
  uiBlockInteraction_Handle *custom_interaction_handle;

  kContextStore *context;

  char undostr[KKE_UNDO_STR_MAX];
  char drawstr[UI_MAX_DRAW_STR];
};

static void button_activate_init(kContext *C,
                                 ARegion *region,
                                 uiBut *but,
                                 uiButtonActivateType type);
static void button_activate_state(kContext *C, uiBut *but, uiHandleButtonState state);
static void button_activate_exit(kContext *C,
                                 uiBut *but,
                                 uiHandleButtonData *data,
                                 const bool mousemove,
                                 const bool onfree);
static int ui_handler_region_menu(kContext *C, const wmEvent *event, void *userdata);
static void ui_handle_button_activate(kContext *C,
                                      ARegion *region,
                                      uiBut *but,
                                      uiButtonActivateType type);
static bool ui_do_but_extra_operator_icon(kContext *C,
                                          uiBut *but,
                                          uiHandleButtonData *data,
                                          const wmEvent *event);
static void ui_do_but_extra_operator_icons_mousemove(uiBut *but,
                                                     uiHandleButtonData *data,
                                                     const wmEvent *event);
static void ui_numedit_begin_set_values(uiBut *but, uiHandleButtonData *data);
#ifdef USE_DRAG_MULTINUM
static void ui_multibut_restore(kContext *C, uiHandleButtonData *data, uiBlock *block);
static uiButMultiState *ui_multibut_lookup(uiHandleButtonData *data, const uiBut *but);
#endif

/* buttons clipboard */
static ColorBand but_copypaste_coba = {0};
static CurveMapping but_copypaste_curve = {0};
static bool but_copypaste_curve_alive = false;
static CurveProfile but_copypaste_profile = {0};
static bool but_copypaste_profile_alive = false;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Button Drag Multi-Number
 * \{ */

#ifdef USE_DRAG_MULTINUM

/* small multi-but api */
static void ui_multibut_add(uiHandleButtonData *data, uiBut *but)
{
  KLI_assert(but->flag & UI_BUT_DRAG_MULTI);
  KLI_assert(data->multi_data.has_mbuts);

  uiButMultiState *mbut_state = (uiButMultiState *)MEM_callocN(sizeof(*mbut_state), __func__);
  mbut_state->but = but;
  mbut_state->origvalue = ui_but_value_get(but);
#  ifdef USE_ALLSELECT
  mbut_state->select_others.is_copy = data->select_others.is_copy;
#  endif

  data->multi_data.mbuts.insert(data->multi_data.mbuts.begin(), mbut_state);

  UI_butstore_register(data->multi_data.bs_mbuts, &mbut_state->but);
}

static uiButMultiState *ui_multibut_lookup(uiHandleButtonData *data, const uiBut *but)
{
  for (auto &mbut_state : data->multi_data.mbuts) {

    if (mbut_state->but == but) {
      return mbut_state;
    }
  }

  return NULL;
}

static void ui_multibut_restore(kContext *C, uiHandleButtonData *data, uiBlock *block)
{
  for (auto &but : block->buttons) {
    if (but->flag & UI_BUT_DRAG_MULTI) {
      uiButMultiState *mbut_state = ui_multibut_lookup(data, but);
      if (mbut_state) {
        ui_but_value_set(but, mbut_state->origvalue);

#  ifdef USE_ALLSELECT
        if (mbut_state->select_others.elems_len > 0) {
          ui_selectcontext_apply(C,
                                 but,
                                 &mbut_state->select_others,
                                 mbut_state->origvalue,
                                 mbut_state->origvalue);
        }
#  else
        UNUSED_VARS(C);
#  endif
      }
    }
  }
}

static void ui_multibut_free(uiHandleButtonData *data, uiBlock *block)
{
#  ifdef USE_ALLSELECT
  if (!data->multi_data.mbuts.empty()) {
    const auto &it = data->multi_data.mbuts.begin();
    while (it != data->multi_data.mbuts.end()) {
      uiButMultiState *mbut_state = (*it);

      if (mbut_state->select_others.elems) {
        MEM_freeN(mbut_state->select_others.elems);
      }

      MEM_freeN((*it));
      MEM_freeN(data->multi_data.mbuts.data());
      (*it)++;
    }
  }
#  else
  KLI_linklist_freeN(data->multi_data.mbuts);
#  endif

  data->multi_data.mbuts = {NULL};

  if (data->multi_data.bs_mbuts) {
    UI_butstore_free(block, data->multi_data.bs_mbuts);
    data->multi_data.bs_mbuts = NULL;
  }
}

static bool ui_multibut_states_tag(uiBut *but_active,
                                   uiHandleButtonData *data,
                                   const wmEvent *event)
{
  float seg[2][2];
  bool changed = false;

  seg[0][0] = data->multi_data.drag_start[0];
  seg[0][1] = data->multi_data.drag_start[1];

  seg[1][0] = event->mouse_pos[0];
  seg[1][1] = event->mouse_pos[1];

  KLI_assert(data->multi_data.init == BUTTON_MULTI_INIT_SETUP);

  ui_window_to_block_fl(data->region, but_active->block, &seg[0][0], &seg[0][1]);
  ui_window_to_block_fl(data->region, but_active->block, &seg[1][0], &seg[1][1]);

  data->multi_data.has_mbuts = false;

  /* follow ui_but_find_mouse_over_ex logic */
  for (auto &but : but_active->block->buttons) {
    bool drag_prev = false;
    bool drag_curr = false;

    /* re-set each time */
    if (but->flag & UI_BUT_DRAG_MULTI) {
      but->flag &= ~UI_BUT_DRAG_MULTI;
      drag_prev = true;
    }

    if (ui_but_is_interactive(but, false)) {

      /* drag checks */
      if (but_active != but) {
        if (ui_but_is_compatible(but_active, but)) {

          KLI_assert(but->active == NULL);

          /* finally check for overlap */
          if (KLI_rctf_isect_segment(&but->rect, seg[0], seg[1])) {

            but->flag |= UI_BUT_DRAG_MULTI;
            data->multi_data.has_mbuts = true;
            drag_curr = true;
          }
        }
      }
    }

    changed |= (drag_prev != drag_curr);
  }

  return changed;
}

static void ui_multibut_states_create(uiBut *but_active, uiHandleButtonData *data)
{
  KLI_assert(data->multi_data.init == BUTTON_MULTI_INIT_SETUP);
  KLI_assert(data->multi_data.has_mbuts);

  data->multi_data.bs_mbuts = UI_butstore_create(but_active->block);

  for (auto &but : but_active->block->buttons) {
    if (but->flag & UI_BUT_DRAG_MULTI) {
      ui_multibut_add(data, but);
    }
  }

  /* Edit buttons proportionally to each other.
   * NOTE: if we mix buttons which are proportional and others which are not,
   * this may work a bit strangely. */
  if ((but_active->stageprop && (but_active->stageprop->flag & PROP_PROPORTIONAL)) ||
      ELEM(but_active->unit_type, PROP_UNIT_LENGTH)) {
    double origvalue;
    data->origvalue.Get(&origvalue);
    if (origvalue != 0.0) {
      data->multi_data.is_proportional = true;
    }
  }
}

static void ui_multibut_states_apply(kContext *C, uiHandleButtonData *data, uiBlock *block)
{
  ARegion *region = data->region;

  double value;
  data->value.Get(&value);

  double origvalue;
  data->origvalue.Get(&origvalue);

  const double value_delta = value - origvalue;
  const double value_scale = data->multi_data.is_proportional ? (value / origvalue) : 0.0;

  KLI_assert(data->multi_data.init == BUTTON_MULTI_INIT_ENABLE);
  KLI_assert(data->multi_data.skip == false);

  for (auto &but : block->buttons) {
    if (!(but->flag & UI_BUT_DRAG_MULTI)) {
      continue;
    }

    uiButMultiState *mbut_state = ui_multibut_lookup(data, but);

    if (mbut_state == NULL) {
      /* Highly unlikely. */
      printf("%s: Can't find button\n", __func__);
      /* While this avoids crashing, multi-button dragging will fail,
       * which is still a bug from the user perspective. See T83651. */
      continue;
    }

    void *active_back;
    ui_but_execute_begin(C, region, but, &active_back);

#  ifdef USE_ALLSELECT
    if (data->select_others.is_enabled) {
      /* init once! */
      if (mbut_state->select_others.elems_len == 0) {
        ui_selectcontext_begin(C, but, &mbut_state->select_others);
      }
      if (mbut_state->select_others.elems_len == 0) {
        mbut_state->select_others.elems_len = -1;
      }
    }

    /* Needed so we apply the right deltas. */
    but->active->origvalue.Set(mbut_state->origvalue);
    but->active->select_others = mbut_state->select_others;
    but->active->select_others.do_free = false;
#  endif

    KLI_assert(active_back == NULL);
    /* No need to check 'data->state' here. */
    if (data->str) {
      /* Entering text (set all). */
      but->active->value = data->value;
      ui_but_string_set(C, but, data->str);
    } else {
      /* Dragging (use delta). */
      if (data->multi_data.is_proportional) {
        but->active->value.Set(mbut_state->origvalue * value_scale);
      } else {
        but->active->value.Set(mbut_state->origvalue + value_delta);
      }

      /* Clamp based on soft limits, see T40154. */
      double value;
      but->active->value.Get(&value);
      CLAMP(value, (double)but->softmin, (double)but->softmax);
    }

    ui_but_execute_end(C, region, but, active_back);
  }
}

#endif /* USE_DRAG_MULTINUM */

/** \} */

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
  after->opcontext = but->opcontext;
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

  if (but->context) {
    after->context = CTX_store_copy(but->context);
  }

  ui_but_drawstr_without_sep_char(but, after->drawstr, sizeof(after->drawstr));

  but->optype = NULL;
  but->opcontext = WM_OP_INVOKE_DEFAULT;
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
  ui_but_value_set(but, but->hardmin);
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
        ui_but_value_set(but, typedVal);
        ui_but_update_edited(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Int) {
      int typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        ui_but_value_set(but, typedVal);
        ui_but_update_edited(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Float) {
      float typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        ui_but_value_set(but, typedVal);
        ui_but_update_edited(but);
      }

    } else if (data->value.GetTypeName() == SdfValueTypeNames->Token) {
      TfToken typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        ui_but_value_set(but, (double)typedVal.Hash());
        ui_but_update_edited(but);
      }

    } else {
      double typedVal = FormFactory(data->value);
      if (but->type == UI_BTYPE_MENU) {
        ui_but_value_set(but, typedVal);
        ui_but_update_edited(but);
      }
    }
  }

  ui_apply_but_func(C, but);
  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_TOG(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  const bool value = ui_but_value_get(but);
  int value_toggle;
  if (but->bit) {
    value_toggle = UI_BITBUT_VALUE_TOGGLED((int)value, but->bitnr);
  } else {
    value_toggle = (value == 0.0);
    if (ELEM(but->type, UI_BTYPE_TOGGLE_N, UI_BTYPE_ICON_TOGGLE_N, UI_BTYPE_CHECKBOX_N)) {
      value_toggle = !value_toggle;
    }
  }

  ui_but_value_set(but, (bool)value_toggle);
  if (ELEM(but->type, UI_BTYPE_ICON_TOGGLE, UI_BTYPE_ICON_TOGGLE_N)) {
    ui_but_update_edited(but);
  }

  ui_apply_but_func(C, but);

  data->retval = but->retval;
  data->applied = true;
}

static void ui_apply_but_ROW(kContext *C, uiBlock *block, uiBut *but, uiHandleButtonData *data)
{
  ui_but_value_set(but, but->hardmax);

  ui_apply_but_func(C, but);

  /* states of other row buttons */
  for (auto &bt : block->buttons) {
    if (bt != but && bt->poin == but->poin && ELEM(bt->type, UI_BTYPE_ROW, UI_BTYPE_LISTROW)) {
      ui_but_update_edited(bt);
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

#ifdef USE_DRAG_TOGGLE
static bool ui_drag_toggle_but_is_supported(const uiBut *but)
{
  if (but->flag & UI_BUT_DISABLED) {
    return false;
  }
  if (ui_but_is_bool(but)) {
    return true;
  }
  if (UI_but_is_decorator(but)) {
    return ELEM(but->icon,
                ICON_DECORATE,
                ICON_DECORATE_KEYFRAME,
                ICON_DECORATE_ANIMATE,
                ICON_DECORATE_OVERRIDE);
  }
  return false;
}

static bool ui_but_is_drag_toggle(const uiBut *but)
{
  return ((ui_drag_toggle_but_is_supported(but) == true) &&
          /* Menu check is important so the button dragged over isn't removed instantly. */
          (ui_block_is_menu(but->block) == false));
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
  double value;
  data->value.Get(&value);

  double origvalue;
  data->origvalue.Get(&origvalue);

  ui_selectcontext_apply(C, but, &data->select_others, value, origvalue);
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

static int ui_do_but_EXIT(kContext *C, uiBut *but, uiHandleButtonData *data, const wmEvent *event)
{
  if (data->state == BUTTON_STATE_HIGHLIGHT) {

    /* First handle click on icon-drag type button. */
    if ((event->type == LEFTMOUSE) && (event->val == KM_PRESS) && ui_but_drag_is_draggable(but)) {
      if (ui_but_contains_point_px_icon(but, data->region, event)) {

        /* tell the button to wait and keep checking further events to
         * see if it should start dragging */
        button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
        data->dragstartx = event->mouse_pos[0];
        data->dragstarty = event->mouse_pos[1];
        return WM_UI_HANDLER_CONTINUE;
      }
    }
#ifdef USE_DRAG_TOGGLE
    if ((event->type == LEFTMOUSE) && (event->val == KM_PRESS) && ui_but_is_drag_toggle(but)) {
      button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
      data->dragstartx = event->mouse_pos[0];
      data->dragstarty = event->mouse_pos[1];
      return WM_UI_HANDLER_CONTINUE;
    }
#endif

    if (ELEM(event->type, LEFTMOUSE, EVT_PADENTER, EVT_RETKEY) && event->val == KM_PRESS) {
      int ret = WM_UI_HANDLER_BREAK;
      /* XXX: (a bit ugly) Special case handling for file-browser drag button. */
      if (ui_but_drag_is_draggable(but) && but->imb &&
          ui_but_contains_point_px_icon(but, data->region, event)) {
        ret = WM_UI_HANDLER_CONTINUE;
      }
      /* Same special case handling for UI lists. Return CONTINUE so that a tweak or CLICK event
       * will be sent for the list to work with. */
      const uiBut *listbox = ui_list_find_mouse_over(data->region, event);
      if (listbox) {
        const uiList *ui_list = (uiList *)listbox->custom_data;
        if (ui_list /*&& ui_list->dyn_data->custom_drag_optype*/) {
          ret = WM_UI_HANDLER_CONTINUE;
        }
      }
      const uiBut *view_but = ui_view_item_find_mouse_over(data->region, event->mouse_pos.data());
      if (view_but) {
        ret = WM_UI_HANDLER_CONTINUE;
      }
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      return ret;
    }
  } else if (data->state == BUTTON_STATE_WAIT_DRAG) {

    /* this function also ends state */
    // if (ui_but_drag_init(C, but, data, event)) {
    //   return WM_UI_HANDLER_BREAK;
    // }

    /* If the mouse has been pressed and released, getting to
     * this point without triggering a drag, then clear the
     * drag state for this button and continue to pass on the event */
    if (event->type == LEFTMOUSE && event->val == KM_RELEASE) {
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      return WM_UI_HANDLER_CONTINUE;
    }

    /* while waiting for a drag to be triggered, always block
     * other events from getting handled */
    return WM_UI_HANDLER_BREAK;
  }

  return WM_UI_HANDLER_CONTINUE;
}

/* Shared by any button that supports drag-toggle. */
static bool ui_do_but_ANY_drag_toggle(kContext *C,
                                      uiBut *but,
                                      uiHandleButtonData *data,
                                      const wmEvent *event,
                                      int *r_retval)
{
  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (event->type == LEFTMOUSE && event->val == KM_PRESS && ui_but_is_drag_toggle(but)) {
      ui_apply_but(C, but->block, but, data, true);
      button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
      data->dragstartx = event->mouse_pos[0];
      data->dragstarty = event->mouse_pos[1];
      *r_retval = WM_UI_HANDLER_BREAK;
      return true;
    }
  } else if (data->state == BUTTON_STATE_WAIT_DRAG) {
    /* NOTE: the 'BUTTON_STATE_WAIT_DRAG' part of 'ui_do_but_EXIT' could be refactored into
     * its own function */
    data->applied = false;
    *r_retval = ui_do_but_EXIT(C, but, data, event);
    return true;
  }
  return false;
}
#endif /* USE_DRAG_TOGGLE */

static int ui_do_but_BUT(kContext *C, uiBut *but, uiHandleButtonData *data, const wmEvent *event)
{
#ifdef USE_DRAG_TOGGLE
  {
    int retval;
    if (ui_do_but_ANY_drag_toggle(C, but, data, event, &retval)) {
      return retval;
    }
  }
#endif

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (event->type == LEFTMOUSE && event->val == KM_PRESS) {
      button_activate_state(C, but, BUTTON_STATE_WAIT_RELEASE);
      return WM_UI_HANDLER_BREAK;
    }
    if (event->type == LEFTMOUSE && event->val == KM_RELEASE && but->block->handle) {
      /* regular buttons will be 'UI_SELECT', menu items 'UI_ACTIVE' */
      if (!(but->flag & (UI_SELECT | UI_ACTIVE))) {
        data->cancel = true;
      }
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      return WM_UI_HANDLER_BREAK;
    }
    if (ELEM(event->type, EVT_PADENTER, EVT_RETKEY) && event->val == KM_PRESS) {
      button_activate_state(C, but, BUTTON_STATE_WAIT_FLASH);
      return WM_UI_HANDLER_BREAK;
    }
  } else if (data->state == BUTTON_STATE_WAIT_RELEASE) {
    if (event->type == LEFTMOUSE && event->val == KM_RELEASE) {
      if (!(but->flag & UI_SELECT)) {
        data->cancel = true;
      }
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      return WM_UI_HANDLER_BREAK;
    }
  }

  return WM_UI_HANDLER_CONTINUE;
}

static int ui_do_but_HOTKEYEVT(kContext *C,
                               uiBut *but,
                               uiHandleButtonData *data,
                               const wmEvent *event)
{
  uiButHotkeyEvent *hotkey_but = (uiButHotkeyEvent *)but;
  KLI_assert(but->type == UI_BTYPE_HOTKEY_EVENT);

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (ELEM(event->type, LEFTMOUSE, EVT_PADENTER, EVT_RETKEY, EVT_BUT_OPEN) &&
        (event->val == KM_PRESS)) {
      but->drawstr[0] = 0;
      hotkey_but->modifier_key = 0;
      button_activate_state(C, but, BUTTON_STATE_WAIT_KEY_EVENT);
      return WM_UI_HANDLER_BREAK;
    }
  } else if (data->state == BUTTON_STATE_WAIT_KEY_EVENT) {
    if (ISMOUSE_MOTION(event->type)) {
      return WM_UI_HANDLER_CONTINUE;
    }
    if (event->type == EVT_UNKNOWNKEY) {
      WM_report(RPT_WARNING, "Unsupported key: Unknown");
      return WM_UI_HANDLER_CONTINUE;
    }
    if (event->type == EVT_CAPSLOCKKEY) {
      WM_report(RPT_WARNING, "Unsupported key: CapsLock");
      return WM_UI_HANDLER_CONTINUE;
    }

    if (event->type == LEFTMOUSE && event->val == KM_PRESS) {
      /* only cancel if click outside the button */
      if (ui_but_contains_point_px(but, but->active->region, event->mouse_pos.data()) == false) {
        data->cancel = true;
        /* Close the containing popup (if any). */
        data->escapecancel = true;
        button_activate_state(C, but, BUTTON_STATE_EXIT);
        return WM_UI_HANDLER_BREAK;
      }
    }

    /* always set */
    hotkey_but->modifier_key = event->modifier;

    ui_but_update(but);
    ED_region_tag_redraw(data->region);

    if (event->val == KM_PRESS) {
      if (ISHOTKEY(event->type) && (event->type != EVT_ESCKEY)) {
        if (WM_key_event_string(event->type, false)[0]) {
          ui_but_value_set(but, event->type);
        } else {
          data->cancel = true;
        }

        button_activate_state(C, but, BUTTON_STATE_EXIT);
        return WM_UI_HANDLER_BREAK;
      }
      if (event->type == EVT_ESCKEY) {
        if (event->val == KM_PRESS) {
          data->cancel = true;
          data->escapecancel = true;
          button_activate_state(C, but, BUTTON_STATE_EXIT);
        }
      }
    }
  }

  return WM_UI_HANDLER_CONTINUE;
}

static int ui_do_but_KEYEVT(kContext *C,
                            uiBut *but,
                            uiHandleButtonData *data,
                            const wmEvent *event)
{
  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (ELEM(event->type, LEFTMOUSE, EVT_PADENTER, EVT_RETKEY) && event->val == KM_PRESS) {
      button_activate_state(C, but, BUTTON_STATE_WAIT_KEY_EVENT);
      return WM_UI_HANDLER_BREAK;
    }
  } else if (data->state == BUTTON_STATE_WAIT_KEY_EVENT) {
    if (ISMOUSE_MOTION(event->type)) {
      return WM_UI_HANDLER_CONTINUE;
    }

    if (event->val == KM_PRESS) {
      if (WM_key_event_string(event->type, false)[0]) {
        ui_but_value_set(but, event->type);
      } else {
        data->cancel = true;
      }

      button_activate_state(C, but, BUTTON_STATE_EXIT);
    }
  }

  return WM_UI_HANDLER_CONTINUE;
}

static void ui_do_but_textedit_select(kContext *C,
                                      uiBlock *block,
                                      uiBut *but,
                                      uiHandleButtonData *data,
                                      const wmEvent *event)
{
  int retval = WM_UI_HANDLER_CONTINUE;

  switch (event->type) {
    case MOUSEMOVE: {
      int mx = event->mouse_pos[0];
      int my = event->mouse_pos[1];
      ui_window_to_block(data->region, block, &mx, &my);

      // ui_textedit_set_cursor_select(but, data, event->mouse_pos[0]);
      retval = WM_UI_HANDLER_BREAK;
      break;
    }
    case LEFTMOUSE:
      if (event->val == KM_RELEASE) {
        button_activate_state(C, but, BUTTON_STATE_TEXT_EDITING);
      }
      retval = WM_UI_HANDLER_BREAK;
      break;
  }

  if (retval == WM_UI_HANDLER_BREAK) {
    ui_but_update(but);
    ED_region_tag_redraw(data->region);
  }
}

static int ui_do_but_TAB(kContext *C,
                         uiBlock *block,
                         uiBut *but,
                         uiHandleButtonData *data,
                         const wmEvent *event)
{
  const bool is_property = (but->stageprop != NULL);

#ifdef USE_DRAG_TOGGLE
  if (is_property) {
    int retval;
    if (ui_do_but_ANY_drag_toggle(C, but, data, event, &retval)) {
      return retval;
    }
  }
#endif

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    const int rna_type = but->stageprop ? LUXO_property_type(but->stageprop) : 0;

    if (is_property && ELEM(rna_type, PROP_POINTER, PROP_STRING) && (but->custom_data != NULL) &&
        (event->type == LEFTMOUSE) &&
        ((event->val == KM_DBL_CLICK) || (event->modifier & KM_CTRL))) {
      button_activate_state(C, but, BUTTON_STATE_TEXT_EDITING);
      return WM_UI_HANDLER_BREAK;
    }
    if (ELEM(event->type, LEFTMOUSE, EVT_PADENTER, EVT_RETKEY)) {
      const int event_val = (is_property) ? KM_PRESS : KM_CLICK;
      if (event->val == event_val) {
        button_activate_state(C, but, BUTTON_STATE_EXIT);
        return WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_TEXT_EDITING) {
    // ui_do_but_textedit(C, block, but, data, event);
    return WM_UI_HANDLER_BREAK;
  } else if (data->state == BUTTON_STATE_TEXT_SELECTING) {
    ui_do_but_textedit_select(C, block, but, data, event);
    return WM_UI_HANDLER_BREAK;
  }

  return WM_UI_HANDLER_CONTINUE;
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
  ui_window_to_block(region, but->block, &x, &y);
  if (!KLI_rctf_isect_pt(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax),
                         x,
                         y)) {
    return NULL;
  }

  const float icon_size = 0.8f *
                          KLI_rctf_size_y(GfVec4f(but->rect.xmin,
                                                  but->rect.xmax,
                                                  but->rect.ymin,
                                                  but->rect.ymax)); /* ICON_SIZE_FROM_BUTRECT */
  float xmax = but->rect.xmax;
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

static int ui_do_but_TEX(kContext *C,
                         uiBlock *block,
                         uiBut *but,
                         uiHandleButtonData *data,
                         const wmEvent *event)
{
  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (ELEM(event->type, LEFTMOUSE, EVT_BUT_OPEN, EVT_PADENTER, EVT_RETKEY) &&
        event->val == KM_PRESS) {
      if (ELEM(event->type, EVT_PADENTER, EVT_RETKEY) && (!UI_but_is_utf8(but))) {
        /* pass - allow filesel, enter to execute */
      } else if (ELEM(but->emboss, UI_EMBOSS_NONE, UI_EMBOSS_NONE_OR_STATUS) &&
                 ((event->modifier & KM_CTRL) == 0)) {
        /* pass */
      } else {
        if (!ui_but_extra_operator_icon_mouse_over_get(but, data->region, event)) {
          button_activate_state(C, but, BUTTON_STATE_TEXT_EDITING);
        }
        return WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_TEXT_EDITING) {
    // ui_do_but_textedit(C, block, but, data, event);
    return WM_UI_HANDLER_BREAK;
  } else if (data->state == BUTTON_STATE_TEXT_SELECTING) {
    ui_do_but_textedit_select(C, block, but, data, event);
    return WM_UI_HANDLER_BREAK;
  }

  return WM_UI_HANDLER_CONTINUE;
}

static int ui_do_but_SEARCH_UNLINK(kContext *C,
                                   uiBlock *block,
                                   uiBut *but,
                                   uiHandleButtonData *data,
                                   const wmEvent *event)
{
  /* unlink icon is on right */
  if (ELEM(event->type, LEFTMOUSE, EVT_BUT_OPEN, EVT_PADENTER, EVT_RETKEY)) {
    /* doing this on KM_PRESS calls eyedropper after clicking unlink icon */
    if ((event->val == KM_RELEASE) && ui_do_but_extra_operator_icon(C, but, data, event)) {
      return WM_UI_HANDLER_BREAK;
    }
  }
  return ui_do_but_TEX(C, block, but, data, event);
}

static int ui_do_but_TOG(kContext *C, uiBut *but, uiHandleButtonData *data, const wmEvent *event)
{
#ifdef USE_DRAG_TOGGLE
  {
    int retval;
    if (ui_do_but_ANY_drag_toggle(C, but, data, event, &retval)) {
      return retval;
    }
  }
#endif

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    bool do_activate = false;
    if (ELEM(event->type, EVT_PADENTER, EVT_RETKEY)) {
      if (event->val == KM_PRESS) {
        do_activate = true;
      }
    } else if (event->type == LEFTMOUSE) {
      if (ui_block_is_menu(but->block)) {
        /* Behave like other menu items. */
        do_activate = (event->val == KM_RELEASE);
      } else if (!ui_do_but_extra_operator_icon(C, but, data, event)) {
        /* Also use double-clicks to prevent fast clicks to leak to other handlers (T76481). */
        do_activate = ELEM(event->val, KM_PRESS, KM_DBL_CLICK);
      }
    }

    if (do_activate) {
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      return WM_UI_HANDLER_BREAK;
    }
    if (ELEM(event->type, MOUSEPAN, WHEELDOWNMOUSE, WHEELUPMOUSE) && (event->modifier & KM_CTRL)) {
      /* Support Ctrl-Wheel to cycle values on expanded enum rows. */
      if (but->type == UI_BTYPE_ROW) {
        int type = event->type;
        int val = event->val;

        /* Convert pan to scroll-wheel. */
        if (type == MOUSEPAN) {
          ui_pan_to_scroll(event, &type, &val);

          if (type == MOUSEPAN) {
            return WM_UI_HANDLER_BREAK;
          }
        }

        const int direction = (type == WHEELDOWNMOUSE) ? -1 : 1;
        uiBut *but_select = ui_but_find_select_in_enum(but, direction);
        if (but_select) {
          uiBut *but_other = (direction == -1) ? but_select->next : but_select->prev;
          if (but_other /*&& ui_but_find_select_in_enum__cmp(but, but_other)*/) {
            ARegion *region = data->region;

            data->cancel = true;
            button_activate_exit(C, but, data, false, false);

            /* Activate the text button. */
            button_activate_init(C, region, but_other, BUTTON_ACTIVATE_OVER);
            data = but_other->active;
            if (data) {
              ui_apply_but(C, but->block, but_other, but_other->active, true);
              button_activate_exit(C, but_other, data, false, false);

              /* restore active button */
              button_activate_init(C, region, but, BUTTON_ACTIVATE_OVER);
            } else {
              /* shouldn't happen */
              KLI_assert(0);
            }
          }
        }
        return WM_UI_HANDLER_BREAK;
      }
    }
  }
  return WM_UI_HANDLER_CONTINUE;
}

static int ui_do_but_VIEW_ITEM(kContext *C,
                               uiBut *but,
                               uiHandleButtonData *data,
                               const wmEvent *event)
{
  uiButViewItem *view_item_but = (uiButViewItem *)but;
  KLI_assert(view_item_but->but.type == UI_BTYPE_VIEW_ITEM);

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (event->type == LEFTMOUSE) {
      switch (event->val) {
        case KM_PRESS:
          /* Extra icons have priority, don't mess with them. */
          if (ui_but_extra_operator_icon_mouse_over_get(but, data->region, event)) {
            return WM_UI_HANDLER_BREAK;
          }
          button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
          data->dragstartx = event->mouse_pos[0];
          data->dragstarty = event->mouse_pos[1];
          return WM_UI_HANDLER_CONTINUE;

        case KM_CLICK:
          button_activate_state(C, but, BUTTON_STATE_EXIT);
          return WM_UI_HANDLER_BREAK;

        case KM_DBL_CLICK:
          data->cancel = true;
          UI_view_item_begin_rename(view_item_but->view_item);
          ED_region_tag_redraw(CTX_wm_region(C));
          return WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_WAIT_DRAG) {
    /* Let "default" button handling take care of the drag logic. */
    return ui_do_but_EXIT(C, but, data, event);
  }

  return WM_UI_HANDLER_CONTINUE;
}

static int ui_do_but_SCROLL(kContext *C,
                            uiBlock *block,
                            uiBut *but,
                            uiHandleButtonData *data,
                            const wmEvent *event)
{
  int retval = WM_UI_HANDLER_CONTINUE;
  const bool horizontal =
    (KLI_rctf_size_x(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)) >
     KLI_rctf_size_y(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)));

  int mx = event->mouse_pos[0];
  int my = event->mouse_pos[1];
  ui_window_to_block(data->region, block, &mx, &my);

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (event->val == KM_PRESS) {
      if (event->type == LEFTMOUSE) {
        if (horizontal) {
          data->dragstartx = mx;
          data->draglastx = mx;
        } else {
          data->dragstartx = my;
          data->draglastx = my;
        }
        // button_activate_state(C, but, BUTTON_STATE_NUM_EDITING);
        retval = WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_NUM_EDITING) {
    if (event->type == EVT_ESCKEY) {
      if (event->val == KM_PRESS) {
        data->cancel = true;
        data->escapecancel = true;
        // button_activate_state(C, but, BUTTON_STATE_EXIT);
      }
    } else if (event->type == LEFTMOUSE && event->val == KM_RELEASE) {
      // button_activate_state(C, but, BUTTON_STATE_EXIT);
    } else if (event->type == MOUSEMOVE) {
      const bool is_motion = (event->type == MOUSEMOVE);
      // if (ui_numedit_but_SLI(but,
      //                        data,
      //                        (horizontal) ? mx : my,
      //                        horizontal,
      //                        is_motion,
      //                        false,
      //                        false)) {
      //   ui_numedit_apply(C, block, but, data);
      // }
    }

    retval = WM_UI_HANDLER_BREAK;
  }

  return retval;
}

static int ui_do_but_GRIP(kContext *C,
                          uiBlock *block,
                          uiBut *but,
                          uiHandleButtonData *data,
                          const wmEvent *event)
{
  int retval = WM_UI_HANDLER_CONTINUE;
  const bool horizontal =
    (KLI_rctf_size_x(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)) <
     KLI_rctf_size_y(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)));

  /* NOTE: Having to store org point in window space and recompute it to block "space" each time
   *       is not ideal, but this is a way to hack around behavior of ui_window_to_block(), which
   *       returns different results when the block is inside a panel or not...
   *       See T37739.
   */

  int mx = event->mouse_pos[0];
  int my = event->mouse_pos[1];
  ui_window_to_block(data->region, block, &mx, &my);

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    if (event->val == KM_PRESS) {
      if (event->type == LEFTMOUSE) {
        data->dragstartx = event->mouse_pos[0];
        data->dragstarty = event->mouse_pos[1];
        button_activate_state(C, but, BUTTON_STATE_NUM_EDITING);
        retval = WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_NUM_EDITING) {
    if (event->type == EVT_ESCKEY) {
      if (event->val == KM_PRESS) {
        data->cancel = true;
        data->escapecancel = true;
        button_activate_state(C, but, BUTTON_STATE_EXIT);
      }
    } else if (event->type == LEFTMOUSE && event->val == KM_RELEASE) {
      button_activate_state(C, but, BUTTON_STATE_EXIT);
    } else if (event->type == MOUSEMOVE) {
      int dragstartx = data->dragstartx;
      int dragstarty = data->dragstarty;
      ui_window_to_block(data->region, block, &dragstartx, &dragstarty);

      double origvalue;
      data->origvalue.Get(&origvalue);
      data->value.Set(origvalue + (horizontal ? mx - dragstartx : dragstarty - my));
      // ui_numedit_apply(C, block, but, data);
    }

    retval = WM_UI_HANDLER_BREAK;
  }

  return retval;
}

static int ui_do_but_BLOCK(kContext *C, uiBut *but, uiHandleButtonData *data, const wmEvent *event)
{
  if (data->state == BUTTON_STATE_HIGHLIGHT) {

    /* First handle click on icon-drag type button. */
    if (event->type == LEFTMOUSE && ui_but_drag_is_draggable(but) && event->val == KM_PRESS) {
      if (ui_but_contains_point_px_icon(but, data->region, event)) {
        button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
        data->dragstartx = event->mouse_pos[0];
        data->dragstarty = event->mouse_pos[1];
        return WM_UI_HANDLER_BREAK;
      }
    }
#ifdef USE_DRAG_TOGGLE
    if (event->type == LEFTMOUSE && event->val == KM_PRESS && (ui_but_is_drag_toggle(but))) {
      button_activate_state(C, but, BUTTON_STATE_WAIT_DRAG);
      data->dragstartx = event->mouse_pos[0];
      data->dragstarty = event->mouse_pos[1];
      return WM_UI_HANDLER_BREAK;
    }
#endif
    /* regular open menu */
    if (ELEM(event->type, LEFTMOUSE, EVT_PADENTER, EVT_RETKEY) && event->val == KM_PRESS) {
      button_activate_state(C, but, BUTTON_STATE_MENU_OPEN);
      return WM_UI_HANDLER_BREAK;
    }
    if (ui_but_supports_cycling(but)) {
      if (ELEM(event->type, MOUSEPAN, WHEELDOWNMOUSE, WHEELUPMOUSE) &&
          (event->modifier & KM_CTRL)) {
        int type = event->type;
        int val = event->val;

        /* Convert pan to scroll-wheel. */
        if (type == MOUSEPAN) {
          ui_pan_to_scroll(event, &type, &val);

          if (type == MOUSEPAN) {
            return WM_UI_HANDLER_BREAK;
          }
        }

        const int direction = (type == WHEELDOWNMOUSE) ? 1 : -1;

        data->value.Set(ui_but_menu_step(but, direction));

        button_activate_state(C, but, BUTTON_STATE_EXIT);
        ui_apply_but(C, but->block, but, data, true);

        /* Button's state need to be changed to EXIT so moving mouse away from this mouse
         * wouldn't lead to cancel changes made to this button, but changing state to EXIT also
         * makes no button active for a while which leads to triggering operator when doing fast
         * scrolling mouse wheel. using post activate stuff from button allows to make button be
         * active again after checking for all that mouse leave and cancel stuff, so quick
         * scroll wouldn't be an issue anymore. Same goes for scrolling wheel in another
         * direction below (sergey).
         */
        data->postbut = but;
        data->posttype = BUTTON_ACTIVATE_OVER;

        /* without this, a new interface that draws as result of the menu change
         * won't register that the mouse is over it, eg:
         * Alt+MouseWheel over the render slots, without this,
         * the slot menu fails to switch a second time.
         *
         * The active state of the button could be maintained some other way
         * and remove this mouse-move event.
         */
        WM_event_add_mousemove(data->window);

        return WM_UI_HANDLER_BREAK;
      }
    }
  } else if (data->state == BUTTON_STATE_WAIT_DRAG) {

    /* this function also ends state */
    // if (ui_but_drag_init(C, but, data, event)) {
    //   return WM_UI_HANDLER_BREAK;
    // }

    /* outside icon quit, not needed if drag activated */
    if (0 == ui_but_contains_point_px_icon(but, data->region, event)) {
      button_activate_state(C, but, BUTTON_STATE_EXIT);
      data->cancel = true;
      return WM_UI_HANDLER_BREAK;
    }

    if (event->type == LEFTMOUSE && event->val == KM_RELEASE) {
      button_activate_state(C, but, BUTTON_STATE_MENU_OPEN);
      return WM_UI_HANDLER_BREAK;
    }
  }

  return WM_UI_HANDLER_CONTINUE;
}

uiBut *UI_region_active_but_get(const ARegion *region)
{
  return ui_context_button_active(region, NULL);
}

static void ui_numedit_set_active(uiBut *but)
{
  const int oldflag = but->drawflag;
  but->drawflag &= ~(UI_BUT_ACTIVE_LEFT | UI_BUT_ACTIVE_RIGHT);

  uiHandleButtonData *data = but->active;
  if (!data) {
    return;
  }

  /* Ignore once we start dragging. */
  if (data->dragchange == false) {
    const float handle_width = min_ff(
      KLI_rctf_size_x(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)) / 3,
      KLI_rctf_size_y(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)) *
        0.7f);
    /* we can click on the side arrows to increment/decrement,
     * or click inside to edit the value directly */
    int mx = data->window->eventstate->mouse_pos[0];
    int my = data->window->eventstate->mouse_pos[1];
    ui_window_to_block(data->region, but->block, &mx, &my);

    if (mx < (but->rect.xmin + handle_width)) {
      but->drawflag |= UI_BUT_ACTIVE_LEFT;
    } else if (mx > (but->rect.xmax - handle_width)) {
      but->drawflag |= UI_BUT_ACTIVE_RIGHT;
    }
  }

  /* Don't change the cursor once pressed. */
  if ((but->flag & UI_SELECT) == 0) {
    if ((but->drawflag & UI_BUT_ACTIVE_LEFT) || (but->drawflag & UI_BUT_ACTIVE_RIGHT)) {
      if (data->changed_cursor) {
        WM_cursor_modal_restore(data->window);
        data->changed_cursor = false;
      }
    } else {
      if (data->changed_cursor == false) {
        WM_cursor_modal_set(data->window, WM_CURSOR_X_MOVE);
        data->changed_cursor = true;
      }
    }
  }

  if (but->drawflag != oldflag) {
    ED_region_tag_redraw(data->region);
  }
}

/* -------------------------------------------------------------------- */
/** \name Button State Handling
 * \{ */

static bool button_modal_state(uiHandleButtonState state)
{
  return ELEM(state,
              BUTTON_STATE_WAIT_RELEASE,
              BUTTON_STATE_WAIT_KEY_EVENT,
              BUTTON_STATE_NUM_EDITING,
              BUTTON_STATE_TEXT_EDITING,
              BUTTON_STATE_TEXT_SELECTING,
              BUTTON_STATE_MENU_OPEN);
}

static ARegion *ui_but_tooltip_init(kContext *C,
                                    ARegion *region,
                                    int *pass,
                                    double *r_pass_delay,
                                    bool *r_exit_on_event)
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
      but,
      but->active ? but->active->region : region,
      win->eventstate);

    // return UI_tooltip_create_from_button_or_extra_icon(C, region, but, extra_icon, is_label);
  }
  return NULL;
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

/* -------------------------------------------------------------------- */
/** \name Button Number Editing (various types)
 * \{ */

static void ui_numedit_begin_set_values(uiBut *but, uiHandleButtonData *data)
{
  data->startvalue.Set(ui_but_value_get(but));
  data->origvalue = data->startvalue;
  data->value = data->origvalue;
}

static void ui_numedit_begin(uiBut *but, uiHandleButtonData *data)
{
  if (but->type == UI_BTYPE_CURVE) {
    uiButCurveMapping *but_cumap = (uiButCurveMapping *)but;
    but_cumap->edit_cumap = (CurveMapping *)but->poin;
  } else if (but->type == UI_BTYPE_CURVEPROFILE) {
    uiButCurveProfile *but_profile = (uiButCurveProfile *)but;
    but_profile->edit_profile = (CurveProfile *)but->poin;
  } else if (but->type == UI_BTYPE_COLORBAND) {
    uiButColorBand *but_coba = (uiButColorBand *)but;
    data->coba = (ColorBand *)but->poin;
    but_coba->edit_coba = data->coba;
  } else if (ELEM(but->type,
                  UI_BTYPE_UNITVEC,
                  UI_BTYPE_HSVCUBE,
                  UI_BTYPE_HSVCIRCLE,
                  UI_BTYPE_COLOR)) {
    ui_but_v3_get(but, data->origvec);
    copy_v3_v3(data->vec, data->origvec);
    but->editvec = data->vec;
  } else {
    ui_numedit_begin_set_values(but, data);

    double editval;
    data->value.Get(&editval);
    (*but->editval) = editval;

    float softmin = but->softmin;
    float softmax = but->softmax;
    float softrange = softmax - softmin;
    const PropertyScaleType scale_type = ui_but_scale_type(but);

    float log_min = (scale_type == PROP_SCALE_LOG) ? max_ff(softmin, UI_PROP_SCALE_LOG_MIN) : 0.0f;

    if ((but->type == UI_BTYPE_NUM) && (ui_but_is_cursor_warp(but) == false)) {
      uiButNumber *number_but = (uiButNumber *)but;

      if (scale_type == PROP_SCALE_LOG) {
        log_min = max_ff(log_min, powf(10, -number_but->precision) * 0.5f);
      }
      /* Use a minimum so we have a predictable range,
       * otherwise some float buttons get a large range. */
      const float value_step_float_min = 0.1f;
      const bool is_float = ui_but_is_float(but);
      const double value_step = is_float ?
                                  (double)(number_but->step_size * UI_PRECISION_FLOAT_SCALE) :
                                  (int)number_but->step_size;
      const float drag_map_softrange_max = UI_DRAG_MAP_SOFT_RANGE_PIXEL_MAX * UI_DPI_FAC;
      const float softrange_max = min_ff(softrange,
                                         2 * (is_float ?
                                                min_ff(value_step, value_step_float_min) *
                                                  (drag_map_softrange_max / value_step_float_min) :
                                                drag_map_softrange_max));

      if (softrange > softrange_max) {

        double origvalue;
        data->origvalue.Get(&origvalue);

        /* Center around the value, keeping in the real soft min/max range. */
        softmin = origvalue - (softrange_max / 2);
        softmax = origvalue + (softrange_max / 2);
        if (!isfinite(softmin)) {
          softmin = (origvalue > 0.0f ? FLT_MAX : -FLT_MAX);
        }
        if (!isfinite(softmax)) {
          softmax = (origvalue > 0.0f ? FLT_MAX : -FLT_MAX);
        }

        if (softmin < but->softmin) {
          softmin = but->softmin;
          softmax = softmin + softrange_max;
        } else if (softmax > but->softmax) {
          softmax = but->softmax;
          softmin = softmax - softrange_max;
        }

        /* Can happen at extreme values. */
        if (UNLIKELY(softmin == softmax)) {
          if (origvalue > 0.0) {
            softmin = nextafterf(softmin, -FLT_MAX);
          } else {
            softmax = nextafterf(softmax, FLT_MAX);
          }
        }

        softrange = softmax - softmin;
      }
    }

    if (softrange == 0.0f) {
      data->dragfstart = 0.0f;
    } else {
      switch (scale_type) {
        case PROP_SCALE_LINEAR: {
          double value;
          data->value.Get(&value);
          data->dragfstart = ((float)value - softmin) / softrange;
          break;
        }
        case PROP_SCALE_LOG: {
          KLI_assert(log_min != 0.0f);
          const float base = softmax / log_min;
          double value;
          data->value.Get(&value);
          data->dragfstart = logf((float)value / log_min) / logf(base);
          break;
        }
        case PROP_SCALE_CUBIC: {
          const float cubic_min = cube_f(softmin);
          const float cubic_max = cube_f(softmax);
          const float cubic_range = cubic_max - cubic_min;
          double value;
          data->value.Get(&value);
          const float f = ((float)value - softmin) * cubic_range / softrange + cubic_min;
          data->dragfstart = (cbrtf(f) - softmin) / softrange;
          break;
        }
      }
    }
    data->dragf = data->dragfstart;

    data->drag_map_soft_min = softmin;
    data->drag_map_soft_max = softmax;
  }

  data->dragchange = false;
  data->draglock = true;
}

static void ui_numedit_end(uiBut *but, uiHandleButtonData *data)
{
  but->editval = NULL;
  but->editvec = NULL;
  if (but->type == UI_BTYPE_COLORBAND) {
    uiButColorBand *but_coba = (uiButColorBand *)but;
    but_coba->edit_coba = NULL;
  } else if (but->type == UI_BTYPE_CURVE) {
    uiButCurveMapping *but_cumap = (uiButCurveMapping *)but;
    but_cumap->edit_cumap = NULL;
  } else if (but->type == UI_BTYPE_CURVEPROFILE) {
    uiButCurveProfile *but_profile = (uiButCurveProfile *)but;
    but_profile->edit_profile = NULL;
  }
  data->dragstartx = 0;
  data->draglastx = 0;
  data->dragchange = false;
  data->dragcbd = NULL;
  data->dragsel = 0;
}

static void ui_textedit_begin(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  wmWindow *win = data->window;
  const bool is_num_but = ELEM(but->type, UI_BTYPE_NUM, UI_BTYPE_NUM_SLIDER);
  bool no_zero_strip = false;

  MEM_SAFE_FREE(data->str);

#ifdef USE_DRAG_MULTINUM
  /* this can happen from multi-drag */
  if (data->applied_interactive) {
    /* remove any small changes so canceling edit doesn't restore invalid value: T40538 */
    data->cancel = true;
    ui_apply_but(C, but->block, but, data, true);
    data->cancel = false;

    data->applied_interactive = false;
  }
#endif

#ifdef USE_ALLSELECT
  if (is_num_but) {
    if (IS_ALLSELECT_EVENT(win->eventstate)) {
      data->select_others.is_enabled = true;
      data->select_others.is_copy = true;
    }
  }
#endif

  /* retrieve string */
  data->maxlen = ui_but_string_get_max_length(but);
  if (data->maxlen != 0) {
    data->str = (char *)MEM_callocN(sizeof(char) * data->maxlen, "textedit str");
    /* We do not want to truncate precision to default here, it's nice to show value,
     * not to edit it - way too much precision is lost then. */
    ui_but_string_get_ex(but,
                         data->str,
                         data->maxlen,
                         UI_PRECISION_FLOAT_MAX,
                         true,
                         &no_zero_strip);
  } else {
    data->is_str_dynamic = true;
    data->str = ui_but_string_get_dynamic(but, &data->maxlen);
  }

  if (ui_but_is_float(but) && !ui_but_is_unit(but) && !ui_but_anim_expression_get(but, NULL, 0) &&
      !no_zero_strip) {
    KLI_str_rstrip_float_zero(data->str, '\0');
  }

  if (is_num_but) {
    KLI_assert(data->is_str_dynamic == false);
    ui_but_convert_to_unit_alt_name(but, data->str, data->maxlen);

    ui_numedit_begin_set_values(but, data);
  }

  /* won't change from now on */
  const int len = strlen(data->str);

  data->origstr = KLI_strdupn(data->str, len);
  data->sel_pos_init = 0;

  /* set cursor pos to the end of the text */
  but->editstr = data->str;
  but->pos = len;
  but->selsta = 0;
  but->selend = len;

  /* Initialize undo history tracking. */
  data->undo_stack_text = ui_textedit_undo_stack_create();
  ui_textedit_undo_push(data->undo_stack_text, but->editstr, but->pos);

  /* optional searchbox */
  if (but->type == UI_BTYPE_SEARCH_MENU) {
    uiButSearch *search_but = (uiButSearch *)but;

    data->searchbox = search_but->popup_create_fn(C, data->region, search_but);
    ui_searchbox_update(C, data->searchbox, but, true); /* true = reset */
  }

  /* reset alert flag (avoid confusion, will refresh on exit) */
  but->flag &= ~UI_BUT_REDALERT;

  ui_but_update(but);

  /* Make sure the edited button is in view. */
  if (data->searchbox) {
    /* Popup blocks don't support moving after creation, so don't change the view for them. */
  } else if (UI_block_layout_needs_resolving(but->block)) {
    /* Layout isn't resolved yet (may happen when activating while drawing through
     * #UI_but_active_only()), so can't move it into view yet. This causes
     * #ui_but_update_view_for_active() to run after the layout is resolved. */
    but->changed = true;
  } else {
    UI_but_ensure_in_view(C, data->region, but);
  }

  WM_cursor_modal_set(win, WM_CURSOR_TEXT_EDIT);

#ifdef WITH_INPUT_IME
  if (!is_num_but) {
    ui_textedit_ime_begin(win, but);
  }
#endif
}

static void ui_textedit_end(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  wmWindow *win = data->window;

  if (but) {
    if (UI_but_is_utf8(but)) {
      const int strip = KLI_str_utf8_invalid_strip(but->editstr, strlen(but->editstr));
      /* not a file?, strip non utf-8 chars */
      if (strip) {
        /* won't happen often so isn't that annoying to keep it here for a while */
        printf("%s: invalid utf8 - stripped chars %d\n", __func__, strip);
      }
    }

    if (data->searchbox) {
      if (data->cancel == false) {
        KLI_assert(but->type == UI_BTYPE_SEARCH_MENU);
        uiButSearch *but_search = (uiButSearch *)but;

        if ((ui_searchbox_apply(but, data->searchbox) == false) &&
            (ui_searchbox_find_index(data->searchbox, but->editstr) == -1) &&
            !but_search->results_are_suggestions) {

          if (but->flag & UI_BUT_VALUE_CLEAR) {
            /* It is valid for _VALUE_CLEAR flavor to have no active element
             * (it's a valid way to unlink). */
            but->editstr[0] = '\0';
          }
          data->cancel = true;

          /* ensure menu (popup) too is closed! */
          data->escapecancel = true;

          WM_reportf(RPT_ERROR, "Failed to find '%s'", but->editstr);
          WM_report_banner_show();
        }
      }

      ui_searchbox_free(C, data->searchbox);
      data->searchbox = NULL;
    }

    but->editstr = NULL;
    but->pos = -1;
  }

  WM_cursor_modal_restore(win);

  /* Free text undo history text blocks. */
  ui_textedit_undo_stack_destroy(data->undo_stack_text);
  data->undo_stack_text = NULL;

#ifdef WITH_INPUT_IME
  if (win->ime_data) {
    ui_textedit_ime_end(win, but);
  }
#endif
}

/* -------------------------------------------------------------------- */
/** \name Menu/Popup Begin/End (various popup types)
 * \{ */

static void ui_block_open_begin(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  uiBlockCreateFunc func = NULL;
  uiBlockHandleCreateFunc handlefunc = NULL;
  uiMenuCreateFunc menufunc = NULL;
  uiMenuCreateFunc popoverfunc = NULL;
  void *arg = NULL;

  switch (but->type) {
    case UI_BTYPE_BLOCK:
    case UI_BTYPE_PULLDOWN:
      if (but->menu_create_func) {
        menufunc = but->menu_create_func;
        arg = but->poin;
      } else {
        func = but->block_create_func;
        arg = but->poin ? but->poin : but->func_argN;
      }
      break;
    case UI_BTYPE_MENU:
    case UI_BTYPE_POPOVER:
      KLI_assert(but->menu_create_func);
      if ((but->type == UI_BTYPE_POPOVER) || ui_but_menu_draw_as_popover(but)) {
        popoverfunc = but->menu_create_func;
      } else {
        menufunc = but->menu_create_func;
      }
      arg = but->poin;
      break;
    case UI_BTYPE_COLOR:
      ui_but_v3_get(but, data->origvec);
      copy_v3_v3(data->vec, data->origvec);
      but->editvec = data->vec;

      if (ui_but_menu_draw_as_popover(but)) {
        popoverfunc = but->menu_create_func;
      } else {
        handlefunc = ui_block_func_COLOR;
      }
      arg = but;
      break;

      /* quiet warnings for unhandled types */
    default:
      break;
  }

  if (func || handlefunc) {
    data->menu = ui_popup_block_create(C, data->region, but, func, handlefunc, arg, NULL);
    if (but->block->handle) {
      data->menu->popup = but->block->handle->popup;
    }
  } else if (menufunc) {
    data->menu = ui_popup_menu_create(C, data->region, but, menufunc, arg);
    if (but->block->handle) {
      data->menu->popup = but->block->handle->popup;
    }
  } else if (popoverfunc) {
    data->menu = ui_popover_panel_create(C, data->region, but, popoverfunc, arg);
    if (but->block->handle) {
      data->menu->popup = but->block->handle->popup;
    }
  }

#ifdef USE_ALLSELECT
  {
    if (IS_ALLSELECT_EVENT(data->window->eventstate)) {
      data->select_others.is_enabled = true;
    }
  }
#endif

  /* this makes adjacent blocks auto open from now on */
  // if (but->block->auto_open == 0) {
  //  but->block->auto_open = 1;
  //}
}

static void ui_block_open_end(kContext *C, uiBut *but, uiHandleButtonData *data)
{
  if (but) {
    but->editval = NULL;
    but->editvec = NULL;

    but->block->auto_open_last = PIL_check_seconds_timer();
  }

  if (data->menu) {
    ui_popup_block_free(C, data->menu);
    data->menu = NULL;
  }
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
          time = 5 * UI_MENU_THRESHOLD2;
        } else if (UI_FLAG & USER_MENUOPENAUTO) {
          time = 5 * UI_MENU_THRESHOLD1;
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
      if (data->ungrab_mval[0] != FLT_MAX /*&& !WM_stereo3d_enabled(data->window, false)*/) {
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
                                data->window->modalhandlers,
                                ui_handler_region_menu,
                                NULL,
                                data,
                                0);
      }
    } else {
      if (button_modal_state(data->state)) {
        /* true = postpone free */
        // WM_event_remove_ui_handler(data->window->modalhandlers,
        //                            ui_handler_region_menu,
        //                            NULL,
        //                            data,
        //                            true);
      }
    }
  }

  /* Wait for mouse-move to enable drag. */
  if (state == BUTTON_STATE_WAIT_DRAG) {
    but->flag &= ~UI_SELECT;
  }

  if (state == BUTTON_STATE_TEXT_EDITING) {
    // ui_block_interaction_begin_ensure(C, but->block, data, true);
  } else if (state == BUTTON_STATE_EXIT) {
    if (data->state == BUTTON_STATE_NUM_EDITING) {
      /* This happens on pasting values for example. */
      // ui_block_interaction_begin_ensure(C, but->block, data, true);
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

static void button_activate_init(kContext *C,
                                 ARegion *region,
                                 uiBut *but,
                                 uiButtonActivateType type)
{
  /* Only ever one active button! */
  KLI_assert(ui_region_find_active_but(region) == NULL);

  /* setup struct */
  uiHandleButtonData *data = (uiHandleButtonData *)MEM_callocN(sizeof(uiHandleButtonData),
                                                               "uiHandleButtonData");
  data->wm = CTX_wm_manager(C);
  data->window = CTX_wm_window(C);
  data->area = CTX_wm_area(C);
  KLI_assert(region != NULL);
  data->region = region;

#ifdef USE_CONT_MOUSE_CORRECT
  copy_v2_fl(data->ungrab_mval, FLT_MAX);
#endif

  if (ELEM(but->type, UI_BTYPE_CURVE, UI_BTYPE_CURVEPROFILE, UI_BTYPE_SEARCH_MENU)) {
    /* XXX curve is temp */
  } else {
    if ((but->flag & UI_BUT_UPDATE_DELAY) == 0) {
      data->interactive = true;
    }
  }

  data->state = BUTTON_STATE_INIT;

  /* activate button */
  but->flag |= UI_ACTIVE;

  but->active = data;

  /* we disable auto_open in the block after a threshold, because we still
   * want to allow auto opening adjacent menus even if no button is activated
   * in between going over to the other button, but only for a short while */
  if (type == BUTTON_ACTIVATE_OVER && but->block->auto_open == true) {
    if (but->block->auto_open_last + BUTTON_AUTO_OPEN_THRESH < PIL_check_seconds_timer()) {
      but->block->auto_open = false;
    }
  }

  if (type == BUTTON_ACTIVATE_OVER) {
    data->used_mouse = true;
  }
  button_activate_state(C, but, BUTTON_STATE_HIGHLIGHT);

  if (type == BUTTON_ACTIVATE_OPEN) {
    button_activate_state(C, but, BUTTON_STATE_MENU_OPEN);

    /* activate first button in submenu */
    if (data->menu && data->menu->region) {
      ARegion *subar = data->menu->region;
      uiBlock *subblock = subar->uiblocks.front();
      uiBut *subbut;

      if (subblock) {
        subbut = ui_but_first(subblock);

        if (subbut) {
          ui_handle_button_activate(C, subar, subbut, BUTTON_ACTIVATE);
        }
      }
    }
  } else if (type == BUTTON_ACTIVATE_TEXT_EDITING) {
    button_activate_state(C, but, BUTTON_STATE_TEXT_EDITING);
  } else if (type == BUTTON_ACTIVATE_APPLY) {
    button_activate_state(C, but, BUTTON_STATE_WAIT_FLASH);
  }

  if (but->type == UI_BTYPE_GRIP) {
    const bool horizontal =
      (KLI_rctf_size_x(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)) <
       KLI_rctf_size_y(GfVec4f(but->rect.xmin, but->rect.xmax, but->rect.ymin, but->rect.ymax)));
    WM_cursor_modal_set(data->window, horizontal ? WM_CURSOR_X_MOVE : WM_CURSOR_Y_MOVE);
  } else if (but->type == UI_BTYPE_NUM) {
    ui_numedit_set_active(but);
  }

  if (UI_but_has_tooltip_label(but)) {
    /* Show a label for this button. */
    kScreen *screen = WM_window_get_active_screen(data->window);
    if ((PIL_check_seconds_timer() - WM_tooltip_time_closed()) < 0.1) {
      WM_tooltip_immediate_init(C, CTX_wm_window(C), data->area, region, ui_but_tooltip_init);
      if (screen->tool_tip) {
        screen->tool_tip->pass = 1;
      }
    }
  }
}

static void ui_handle_button_activate(kContext *C,
                                      ARegion *region,
                                      uiBut *but,
                                      uiButtonActivateType type)
{
  uiBut *oldbut = ui_region_find_active_but(region);
  if (oldbut) {
    uiHandleButtonData *data = oldbut->active;
    data->cancel = true;
    button_activate_exit(C, oldbut, data, false, false);
  }

  button_activate_init(C, region, but, type);
}

/* -------------------------------------------------------------------- */
/** \name Button Tool Tip
 * \{ */

static void ui_blocks_set_tooltips(ARegion *region, const bool enable)
{
  if (!region) {
    return;
  }

  /* We disabled buttons when they were already shown, and re-enable them on mouse move. */
  for (auto &block : region->uiblocks) {
    block->tooltipdisabled = !enable;
  }
}

static void ui_but_extra_operator_icon_apply(kContext *C, uiBut *but, uiButExtraOpIcon *op_icon)
{
  but->active->apply_through_extra_icon = true;

  if (but->active->interactive) {
    ui_apply_but(C, but->block, but, but->active, true);
  }
  button_activate_state(C, but, BUTTON_STATE_EXIT);
  // WM_operator_name_call_ptr_with_depends_on_cursor(C,
  //                                                  op_icon->optype_params->optype,
  //                                                  op_icon->optype_params->opcontext,
  //                                                  op_icon->optype_params->opptr,
  //                                                  NULL,
  //                                                  NULL);

  /* Force recreation of extra operator icons (pseudo update). */
  ui_but_extra_operator_icons_free(but);

  WM_event_add_mousemove(CTX_wm_window(C));
}

static bool ui_do_but_extra_operator_icon(kContext *C,
                                          uiBut *but,
                                          uiHandleButtonData *data,
                                          const wmEvent *event)
{
  uiButExtraOpIcon *op_icon = ui_but_extra_operator_icon_mouse_over_get(but, data->region, event);

  if (!op_icon) {
    return false;
  }

  /* Only act on release, avoids some glitches. */
  if (event->val != KM_RELEASE) {
    /* Still swallow events on the icon. */
    return true;
  }

  ED_region_tag_redraw(data->region);
  button_tooltip_timer_reset(C, but);

  ui_but_extra_operator_icon_apply(C, but, op_icon);
  /* NOTE: 'but', 'data' may now be freed, don't access. */

  return true;
}

static void ui_do_but_extra_operator_icons_mousemove(uiBut *but,
                                                     uiHandleButtonData *data,
                                                     const wmEvent *event)
{
  uiButExtraOpIcon *old_highlighted = NULL;

  /* Unset highlighting of all first. */
  for (auto &op_icon : but->extra_op_icons) {
    if (op_icon->highlighted) {
      old_highlighted = op_icon;
    }
    op_icon->highlighted = false;
  }

  uiButExtraOpIcon *hovered = ui_but_extra_operator_icon_mouse_over_get(but, data->region, event);

  if (hovered) {
    hovered->highlighted = true;
  }

  if (old_highlighted != hovered) {
    ED_region_tag_redraw_no_rebuild(data->region);
  }
}

/**
 * Hack for #uiList #UI_BTYPE_LISTROW buttons to "give" events to overlaying #UI_BTYPE_TEXT
 * buttons (Ctrl-Click rename feature & co).
 */
static uiBut *ui_but_list_row_text_activate(kContext *C,
                                            uiBut *but,
                                            uiHandleButtonData *data,
                                            const wmEvent *event,
                                            uiButtonActivateType activate_type)
{
  ARegion *region = CTX_wm_region(C);
  uiBut *labelbut =
    ui_but_find_mouse_over_ex(region, event->mouse_pos.data(), true, false, NULL, NULL);

  if (labelbut && labelbut->type == UI_BTYPE_TEXT && !(labelbut->flag & UI_BUT_DISABLED)) {
    /* exit listrow */
    data->cancel = true;
    button_activate_exit(C, but, data, false, false);

    /* Activate the text button. */
    button_activate_init(C, region, labelbut, activate_type);

    return labelbut;
  }
  return NULL;
}

static void ui_but_copy_numeric_array(uiBut *but, char *output, int output_len_max)
{
  // const int values_len = get_but_property_array_length(but);
  // float *values = alloca(values_len * sizeof(float));
  // LUXO_property_float_get_array(&but->rnapoin, but->stageprop, values);
  // float_array_to_string(values, values_len, output, output_len_max);
}

static void ui_but_copy_numeric_value(uiBut *but, char *output, int output_len_max)
{
  /* Get many decimal places, then strip trailing zeros.
   * NOTE: too high values start to give strange results. */
  ui_but_string_get_ex(but, output, output_len_max, UI_PRECISION_FLOAT_MAX, false, NULL);
  KLI_str_rstrip_float_zero(output, '\0');
}

static void float_array_to_string(const float *values,
                                  const int values_len,
                                  char *output,
                                  int output_len_max)
{
  const int values_end = values_len - 1;
  int ofs = 0;
  output[ofs++] = '[';
  for (int i = 0; i < values_len; i++) {
    ofs += KLI_snprintf_rlen(output + ofs,
                             output_len_max - ofs,
                             (i != values_end) ? "%f, " : "%f]",
                             values[i]);
  }
}

static void ui_but_copy_text(uiBut *but, char *output, int output_len_max)
{
  ui_but_string_get(but, output, output_len_max);
}

static void ui_but_copy_colorband(uiBut *but)
{
  if (but->poin != NULL) {
    memcpy(&but_copypaste_coba, but->poin, sizeof(ColorBand));
  }
}

static void ui_but_copy_curvemapping(uiBut *but)
{
  if (but->poin != NULL) {
    but_copypaste_curve_alive = true;
    // KKE_curvemapping_free_data(&but_copypaste_curve);
    // KKE_curvemapping_copy_data(&but_copypaste_curve, (CurveMapping *)but->poin);
  }
}

static void ui_but_copy_CurveProfile(uiBut *but)
{
  if (but->poin != NULL) {
    but_copypaste_profile_alive = true;
    // KKE_curveprofile_free_data(&but_copypaste_profile);
    // KKE_curveprofile_copy_data(&but_copypaste_profile, (CurveProfile *)but->poin);
  }
}

static void ui_but_copy_operator(kContext *C, uiBut *but, char *output, int output_len_max)
{
  KrakenPRIM *opptr = UI_but_operator_ptr_get(but);

  char *str;
  str = WM_operator_pystring_ex(C, NULL, false, true, but->optype, opptr);
  KLI_strncpy(output, str, output_len_max);
  MEM_freeN(str);
}

static bool ui_but_copy_menu(uiBut *but, char *output, int output_len_max)
{
  MenuType *mt = UI_but_menutype_get(but);
  if (mt) {
    KLI_snprintf(output,
                 output_len_max,
                 "kpy.ops.wm.call_menu(name=\"%s\")",
                 mt->idname.GetText());
    return true;
  }
  return false;
}

static bool ui_but_copy_popover(uiBut *but, char *output, int output_len_max)
{
  PanelType *pt = UI_but_paneltype_get(but);
  if (pt) {
    KLI_snprintf(output,
                 output_len_max,
                 "kpy.ops.wm.call_panel(name=\"%s\")",
                 pt->idname.GetText());
    return true;
  }
  return false;
}

static void ui_but_copy_color(uiBut *but, char *output, int output_len_max)
{
  float rgba[4];

  if (but->stageprop && (int)but->stageprop->GetTypeName().GetDimensions().size == 4) {
    GfVec4f rgba_gf;
    but->stageprop->Get(&rgba_gf);
    rgba[0] = rgba_gf[0];
    rgba[1] = rgba_gf[1];
    rgba[2] = rgba_gf[2];
  } else {
    rgba[3] = 1.0f;
  }

  ui_but_v3_get(but, rgba);

  /* convert to linear color to do compatible copy between gamma and non-gamma */
  if (but->stageprop && LUXO_property_subtype(but->stageprop) == PROP_COLOR_GAMMA) {
    srgb_to_linearrgb_v3_v3(rgba, rgba);
  }

  float_array_to_string(rgba, 4, output, output_len_max);
}

static void ui_but_copy(kContext *C, uiBut *but, const bool copy_array)
{
  if (ui_but_contains_password(but)) {
    return;
  }

  /* Arbitrary large value (allow for paths: 'PATH_MAX') */
  char buf[4096] = {0};
  const int buf_max_len = sizeof(buf);

  /* Left false for copying internal data (color-band for eg). */
  bool is_buf_set = false;

  const bool has_required_data = !(but->poin == NULL && but->stagepoin.data == NULL);

  switch (but->type) {
    case UI_BTYPE_NUM:
    case UI_BTYPE_NUM_SLIDER:
      if (!has_required_data) {
        break;
      }
      if (copy_array && ui_but_has_array_value(but)) {
        ui_but_copy_numeric_array(but, buf, buf_max_len);
      } else {
        ui_but_copy_numeric_value(but, buf, buf_max_len);
      }
      is_buf_set = true;
      break;

    case UI_BTYPE_UNITVEC:
      if (!has_required_data) {
        break;
      }
      ui_but_copy_numeric_array(but, buf, buf_max_len);
      is_buf_set = true;
      break;

    case UI_BTYPE_COLOR:
      if (!has_required_data) {
        break;
      }
      ui_but_copy_color(but, buf, buf_max_len);
      is_buf_set = true;
      break;

    case UI_BTYPE_TEXT:
    case UI_BTYPE_SEARCH_MENU:
      if (!has_required_data) {
        break;
      }
      ui_but_copy_text(but, buf, buf_max_len);
      is_buf_set = true;
      break;

    case UI_BTYPE_COLORBAND:
      ui_but_copy_colorband(but);
      break;

    case UI_BTYPE_CURVE:
      ui_but_copy_curvemapping(but);
      break;

    case UI_BTYPE_CURVEPROFILE:
      ui_but_copy_CurveProfile(but);
      break;

    case UI_BTYPE_BUT:
      if (!but->optype) {
        break;
      }
      ui_but_copy_operator(C, but, buf, buf_max_len);
      is_buf_set = true;
      break;

    case UI_BTYPE_MENU:
    case UI_BTYPE_PULLDOWN:
      if (ui_but_copy_menu(but, buf, buf_max_len)) {
        is_buf_set = true;
      }
      break;
    case UI_BTYPE_POPOVER:
      if (ui_but_copy_popover(but, buf, buf_max_len)) {
        is_buf_set = true;
      }
      break;

    default:
      break;
  }

  if (is_buf_set) {
    WM_clipboard_text_set(buf, 0);
  }
}

static void ui_but_paste(kContext *C, uiBut *but, uiHandleButtonData *data, const bool paste_array)
{
  KLI_assert((but->flag & UI_BUT_DISABLED) == 0); /* caller should check */

  // int buf_paste_len = 0;
  // char *buf_paste;
  // ui_but_get_pasted_text_from_clipboard(&buf_paste, &buf_paste_len);

  // const bool has_required_data = !(but->poin == NULL && but->stagepoin.data == NULL);

  // switch (but->type) {
  //   case UI_BTYPE_NUM:
  //   case UI_BTYPE_NUM_SLIDER:
  //     if (!has_required_data) {
  //       break;
  //     }
  //     if (paste_array && ui_but_has_array_value(but)) {
  //       ui_but_paste_numeric_array(C, but, data, buf_paste);
  //     }
  //     else {
  //       ui_but_paste_numeric_value(C, but, data, buf_paste);
  //     }
  //     break;

  //   case UI_BTYPE_UNITVEC:
  //     if (!has_required_data) {
  //       break;
  //     }
  //     ui_but_paste_normalized_vector(C, but, data, buf_paste);
  //     break;

  //   case UI_BTYPE_COLOR:
  //     if (!has_required_data) {
  //       break;
  //     }
  //     ui_but_paste_color(C, but, buf_paste);
  //     break;

  //   case UI_BTYPE_TEXT:
  //   case UI_BTYPE_SEARCH_MENU:
  //     if (!has_required_data) {
  //       break;
  //     }
  //     ui_but_paste_text(C, but, data, buf_paste);
  //     break;

  //   case UI_BTYPE_COLORBAND:
  //     ui_but_paste_colorband(C, but, data);
  //     break;

  //   case UI_BTYPE_CURVE:
  //     ui_but_paste_curvemapping(C, but);
  //     break;

  //   case UI_BTYPE_CURVEPROFILE:
  //     ui_but_paste_CurveProfile(C, but);
  //     break;

  //   default:
  //     break;
  // }

  // MEM_freeN((void *)buf_paste);
}

static int ui_do_button(kContext *C, uiBlock *block, uiBut *but, const wmEvent *event)
{
  uiHandleButtonData *data = but->active;
  int retval = WM_UI_HANDLER_CONTINUE;

  const bool is_disabled = but->flag & UI_BUT_DISABLED || data->disable_force;

  /* if but->pointype is set, but->poin should be too */
  KLI_assert(!but->pointype || but->poin);

  /* Only hard-coded stuff here, button interactions with configurable
   * keymaps are handled using operators (see #ED_keymap_ui). */

  if (data->state == BUTTON_STATE_HIGHLIGHT) {

    /* handle copy and paste */
    bool is_press_ctrl_but_no_shift = (event->val == KM_PRESS) &&
                                      (event->modifier & (KM_CTRL | KM_OSKEY)) &&
                                      (event->modifier & KM_SHIFT) == 0;
    const bool do_copy = event->type == EVT_CKEY && is_press_ctrl_but_no_shift;
    const bool do_paste = event->type == EVT_VKEY && is_press_ctrl_but_no_shift;

    /* Specific handling for list-rows, we try to find their overlapping text button. */
    if ((do_copy || do_paste) && but->type == UI_BTYPE_LISTROW) {
      uiBut *labelbut = ui_but_list_row_text_activate(C, but, data, event, BUTTON_ACTIVATE_OVER);
      if (labelbut) {
        but = labelbut;
        data = but->active;
      }
    }

    /* do copy first, because it is the only allowed operator when disabled */
    if (do_copy) {
      ui_but_copy(C, but, event->modifier & KM_ALT);
      return WM_UI_HANDLER_BREAK;
    }

    /* handle menu */

    if ((event->type == RIGHTMOUSE) &&
        (event->modifier & (KM_SHIFT | KM_CTRL | KM_ALT | KM_OSKEY)) == 0 &&
        (event->val == KM_PRESS)) {
      /* For some button types that are typically representing entire sets of data, right-clicking
       * to spawn the context menu should also activate the item. This makes it clear which item
       * will be operated on.
       * Apply the button immediately, so context menu polls get the right active item. */
      if (ELEM(but->type, UI_BTYPE_VIEW_ITEM)) {
        ui_apply_but(C, but->block, but, but->active, true);
      }

      /* RMB has two options now */
      if (ui_popup_context_menu_for_button(C, but, event)) {
        return WM_UI_HANDLER_BREAK;
      }
    }

    if (is_disabled) {
      return WM_UI_HANDLER_CONTINUE;
    }

    if (do_paste) {
      ui_but_paste(C, but, data, event->modifier & KM_ALT);
      return WM_UI_HANDLER_BREAK;
    }

    if ((data->state == BUTTON_STATE_HIGHLIGHT) &&
        ELEM(event->type, LEFTMOUSE, EVT_BUT_OPEN, EVT_PADENTER, EVT_RETKEY) &&
        (event->val == KM_RELEASE) &&
        /* Only returns true if the event was handled. */
        ui_do_but_extra_operator_icon(C, but, data, event)) {
      return WM_UI_HANDLER_BREAK;
    }
  }

  if (but->flag & UI_BUT_DISABLED) {
    /* It's important to continue here instead of breaking since breaking causes the event to be
     * considered "handled", preventing further click/drag events from being generated.
     *
     * An example of where this is needed is dragging node-sockets, where dragging a node-socket
     * could exit the button before the drag threshold was reached, disable the button then break
     * handling of the #MOUSEMOVE event preventing the socket being dragged entirely, see: T96255.
     *
     * Region level event handling is responsible for preventing events being passed
     * through to parts of the UI that are logically behind this button, see: T92364. */
    return WM_UI_HANDLER_CONTINUE;
  }

  switch (but->type) {
    case UI_BTYPE_BUT:
    case UI_BTYPE_DECORATOR:
      retval = ui_do_but_BUT(C, but, data, event);
      break;
    case UI_BTYPE_KEY_EVENT:
      retval = ui_do_but_KEYEVT(C, but, data, event);
      break;
    case UI_BTYPE_HOTKEY_EVENT:
      retval = ui_do_but_HOTKEYEVT(C, but, data, event);
      break;
    case UI_BTYPE_TAB:
      retval = ui_do_but_TAB(C, block, but, data, event);
      break;
    case UI_BTYPE_BUT_TOGGLE:
    case UI_BTYPE_TOGGLE:
    case UI_BTYPE_ICON_TOGGLE:
    case UI_BTYPE_ICON_TOGGLE_N:
    case UI_BTYPE_TOGGLE_N:
    case UI_BTYPE_CHECKBOX:
    case UI_BTYPE_CHECKBOX_N:
    case UI_BTYPE_ROW:
      retval = ui_do_but_TOG(C, but, data, event);
      break;
    case UI_BTYPE_VIEW_ITEM:
      retval = ui_do_but_VIEW_ITEM(C, but, data, event);
      break;
    case UI_BTYPE_SCROLL:
      retval = ui_do_but_SCROLL(C, block, but, data, event);
      break;
    case UI_BTYPE_GRIP:
      retval = ui_do_but_GRIP(C, block, but, data, event);
      break;
    case UI_BTYPE_NUM:
      // retval = ui_do_but_NUM(C, block, but, data, event);
      break;
    case UI_BTYPE_NUM_SLIDER:
      // retval = ui_do_but_SLI(C, block, but, data, event);
      break;
    case UI_BTYPE_LISTBOX:
      /* Nothing to do! */
      break;
    case UI_BTYPE_LISTROW:
      // retval = ui_do_but_LISTROW(C, but, data, event);
      break;
    case UI_BTYPE_ROUNDBOX:
    case UI_BTYPE_LABEL:
    case UI_BTYPE_IMAGE:
    case UI_BTYPE_PROGRESS_BAR:
    case UI_BTYPE_NODE_SOCKET:
    case UI_BTYPE_PREVIEW_TILE:
      retval = ui_do_but_EXIT(C, but, data, event);
      break;
    case UI_BTYPE_HISTOGRAM:
      // retval = ui_do_but_HISTOGRAM(C, block, but, data, event);
      break;
    case UI_BTYPE_WAVEFORM:
      // retval = ui_do_but_WAVEFORM(C, block, but, data, event);
      break;
    case UI_BTYPE_VECTORSCOPE:
      /* Nothing to do! */
      break;
    case UI_BTYPE_TEXT:
    case UI_BTYPE_SEARCH_MENU:
      if ((but->type == UI_BTYPE_SEARCH_MENU) && (but->flag & UI_BUT_VALUE_CLEAR)) {
        retval = ui_do_but_SEARCH_UNLINK(C, block, but, data, event);
        if (retval & WM_UI_HANDLER_BREAK) {
          break;
        }
      }
      retval = ui_do_but_TEX(C, block, but, data, event);
      break;
    case UI_BTYPE_MENU:
    case UI_BTYPE_POPOVER:
    case UI_BTYPE_BLOCK:
    case UI_BTYPE_PULLDOWN:
      retval = ui_do_but_BLOCK(C, but, data, event);
      break;
    case UI_BTYPE_BUT_MENU:
      retval = ui_do_but_BUT(C, but, data, event);
      break;
    case UI_BTYPE_COLOR:
      // retval = ui_do_but_COLOR(C, but, data, event);
      break;
    case UI_BTYPE_UNITVEC:
      // retval = ui_do_but_UNITVEC(C, block, but, data, event);
      break;
    case UI_BTYPE_COLORBAND:
      // retval = ui_do_but_COLORBAND(C, block, but, data, event);
      break;
    case UI_BTYPE_CURVE:
      // retval = ui_do_but_CURVE(C, block, but, data, event);
      break;
    case UI_BTYPE_CURVEPROFILE:
      // retval = ui_do_but_CURVEPROFILE(C, block, but, data, event);
      break;
    case UI_BTYPE_HSVCUBE:
      // retval = ui_do_but_HSVCUBE(C, block, but, data, event);
      break;
    case UI_BTYPE_HSVCIRCLE:
      // retval = ui_do_but_HSVCIRCLE(C, block, but, data, event);
      break;
    case UI_BTYPE_TRACK_PREVIEW:
      // retval = ui_do_but_TRACKPREVIEW(C, block, but, data, event);
      break;

      /* quiet warnings for unhandled types */
    case UI_BTYPE_SEPR:
    case UI_BTYPE_SEPR_LINE:
    case UI_BTYPE_SEPR_SPACER:
    case UI_BTYPE_EXTRA:
      break;
  }

#ifdef USE_DRAG_MULTINUM
  data = but->active;
  if (data) {
    if (ISMOUSE_MOTION(event->type) ||
        /* if we started dragging, progress on any event */
        (data->multi_data.init == BUTTON_MULTI_INIT_SETUP)) {
      if (ELEM(but->type, UI_BTYPE_NUM, UI_BTYPE_NUM_SLIDER) &&
          ELEM(data->state, BUTTON_STATE_TEXT_EDITING, BUTTON_STATE_NUM_EDITING)) {
        /* initialize! */
        if (data->multi_data.init == BUTTON_MULTI_INIT_UNSET) {
          /* --> (BUTTON_MULTI_INIT_SETUP | BUTTON_MULTI_INIT_DISABLE) */

          const float margin_y = DRAG_MULTINUM_THRESHOLD_DRAG_Y / sqrtf(block->aspect);

          /* check if we have a vertical gesture */
          if (len_squared_v2(data->multi_data.drag_dir) > (margin_y * margin_y)) {
            const float dir_nor_y[2] = {0.0, 1.0f};
            float dir_nor_drag[2];

            normalize_v2_v2(dir_nor_drag, data->multi_data.drag_dir);

            if (fabsf(dot_v2v2(dir_nor_drag, dir_nor_y)) > DRAG_MULTINUM_THRESHOLD_VERTICAL) {
              data->multi_data.init = BUTTON_MULTI_INIT_SETUP;
              data->multi_data.drag_lock_x = event->mouse_pos[0];
            } else {
              data->multi_data.init = BUTTON_MULTI_INIT_DISABLE;
            }
          }
        } else if (data->multi_data.init == BUTTON_MULTI_INIT_SETUP) {
          /* --> (BUTTON_MULTI_INIT_ENABLE) */
          const float margin_x = DRAG_MULTINUM_THRESHOLD_DRAG_X / sqrtf(block->aspect);
          /* Check if we're don't setting buttons. */
          if ((data->str &&
               ELEM(data->state, BUTTON_STATE_TEXT_EDITING, BUTTON_STATE_NUM_EDITING)) ||
              ((abs(data->multi_data.drag_lock_x - event->mouse_pos[0]) > margin_x) &&
               /* Just to be sure, check we're dragging more horizontally then vertically. */
               abs(event->prev_mouse_pos[0] - event->mouse_pos[0]) >
                 abs(event->prev_mouse_pos[1] - event->mouse_pos[1]))) {
            if (data->multi_data.has_mbuts) {
              ui_multibut_states_create(but, data);
              data->multi_data.init = BUTTON_MULTI_INIT_ENABLE;
            } else {
              data->multi_data.init = BUTTON_MULTI_INIT_DISABLE;
            }
          }
        }

        if (data->multi_data.init == BUTTON_MULTI_INIT_SETUP) {
          if (ui_multibut_states_tag(but, data, event)) {
            ED_region_tag_redraw(data->region);
          }
        }
      }
    }
  }
#endif /* USE_DRAG_MULTINUM */

  return retval;
}

static int ui_handle_button_event(kContext *C, const wmEvent *event, uiBut *but)
{
  uiHandleButtonData *data = but->active;
  const uiHandleButtonState state_orig = data->state;

  uiBlock *block = but->block;
  ARegion *region = data->region;

  int retval = WM_UI_HANDLER_CONTINUE;

  if (data->state == BUTTON_STATE_HIGHLIGHT) {
    switch (event->type) {
      case WINDEACTIVATE:
      case EVT_BUT_CANCEL:
        data->cancel = true;
        button_activate_state(C, but, BUTTON_STATE_EXIT);
        break;
#ifdef USE_UI_POPOVER_ONCE
      case LEFTMOUSE: {
        if (event->val == KM_RELEASE) {
          if (block->flag & UI_BLOCK_POPOVER_ONCE) {
            if (!(but->flag & UI_BUT_DISABLED)) {
              if (ui_but_is_popover_once_compat(but)) {
                data->cancel = false;
                button_activate_state(C, but, BUTTON_STATE_EXIT);
                retval = WM_UI_HANDLER_BREAK;
                /* Cancel because this `but` handles all events and we don't want
                 * the parent button's update function to do anything.
                 *
                 * Causes issues with buttons defined by #uiItemFullR_with_popover. */
                block->handle->menuretval = UI_RETURN_CANCEL;
              } else if (ui_but_is_editable_as_text(but)) {
                ui_handle_button_activate(C, region, but, BUTTON_ACTIVATE_TEXT_EDITING);
                retval = WM_UI_HANDLER_BREAK;
              }
            }
          }
        }
        break;
      }
#endif
      case MOUSEMOVE: {
        uiBut *but_other = ui_but_find_mouse_over(region, event);
        bool exit = false;

        /* always deactivate button for pie menus,
         * else moving to blank space will leave activated */
        if ((!ui_block_is_menu(block) || ui_block_is_pie_menu(block)) &&
            !ui_but_contains_point_px(but, region, event->mouse_pos.data())) {
          exit = true;
        } else if (but_other && ui_but_is_editable(but_other) && (but_other != but)) {
          exit = true;
        }

        if (exit) {
          data->cancel = true;
          button_activate_state(C, but, BUTTON_STATE_EXIT);
        } else if (event->mouse_pos[0] != event->prev_mouse_pos[0] ||
                   event->mouse_pos[1] != event->prev_mouse_pos[1]) {
          /* Re-enable tool-tip on mouse move. */
          ui_blocks_set_tooltips(region, true);
          button_tooltip_timer_reset(C, but);
        }

        /* Update extra icons states. */
        ui_do_but_extra_operator_icons_mousemove(but, data, event);

        break;
      }
      case TIMER: {
        /* Handle menu auto open timer. */
        if (event->customdata == data->autoopentimer) {
          WM_event_remove_timer(data->wm, data->window, data->autoopentimer);
          data->autoopentimer = NULL;

          if (ui_but_contains_point_px(but, region, event->mouse_pos.data()) || but->active) {
            button_activate_state(C, but, BUTTON_STATE_MENU_OPEN);
          }
        }

        break;
      }
      /* XXX hardcoded keymap check... but anyway,
       * while view changes, tool-tips should be removed */
      case WHEELUPMOUSE:
      case WHEELDOWNMOUSE:
      case MIDDLEMOUSE:
      case MOUSEPAN:
        UI_but_tooltip_timer_remove(C, but);
        ATTR_FALLTHROUGH;
      default:
        break;
    }

    /* handle button type specific events */
    retval = ui_do_button(C, block, but, event);
  } else if (data->state == BUTTON_STATE_WAIT_RELEASE) {
    switch (event->type) {
      case WINDEACTIVATE:
        data->cancel = true;
        button_activate_state(C, but, BUTTON_STATE_EXIT);
        break;

      case TIMER: {
        if (event->customdata == data->hold_action_timer) {
          if (true) {
            data->cancel = true;
            button_activate_state(C, but, BUTTON_STATE_EXIT);
          } else {
            /* Do this so we can still mouse-up, closing the menu and running the button.
             * This is nice to support but there are times when the button gets left pressed.
             * Keep disabled for now. */
            WM_event_remove_timer(data->wm, data->window, data->hold_action_timer);
            data->hold_action_timer = NULL;
          }
          retval = WM_UI_HANDLER_CONTINUE;
          but->hold_func(C, data->region, but);
        }
        break;
      }
      case MOUSEMOVE: {
        /* deselect the button when moving the mouse away */
        /* also de-activate for buttons that only show highlights */
        if (ui_but_contains_point_px(but, region, event->mouse_pos.data())) {

          /* Drag on a hold button (used in the toolbar) now opens it immediately. */
          if (data->hold_action_timer) {
            if (but->flag & UI_SELECT) {
              if (len_manhattan_v2v2_int(event->mouse_pos.data(), event->prev_mouse_pos.data()) <=
                  WM_EVENT_CURSOR_MOTION_THRESHOLD) {
                /* pass */
              } else {
                WM_event_remove_timer(data->wm, data->window, data->hold_action_timer);
                data->hold_action_timer = WM_event_add_timer(data->wm, data->window, TIMER, 0.0f);
              }
            }
          }

          if (!(but->flag & UI_SELECT)) {
            but->flag |= (UI_SELECT | UI_ACTIVE);
            data->cancel = false;
            ED_region_tag_redraw_no_rebuild(data->region);
          }
        } else {
          if (but->flag & UI_SELECT) {
            but->flag &= ~(UI_SELECT | UI_ACTIVE);
            data->cancel = true;
            ED_region_tag_redraw_no_rebuild(data->region);
          }
        }
        break;
      }
      default:
        /* otherwise catch mouse release event */
        ui_do_button(C, block, but, event);
        break;
    }

    retval = WM_UI_HANDLER_BREAK;
  } else if (data->state == BUTTON_STATE_WAIT_FLASH) {
    switch (event->type) {
      case TIMER: {
        if (event->customdata == data->flashtimer) {
          button_activate_state(C, but, BUTTON_STATE_EXIT);
        }
        break;
      }
    }

    retval = WM_UI_HANDLER_CONTINUE;
  } else if (data->state == BUTTON_STATE_MENU_OPEN) {
    /* check for exit because of mouse-over another button */
    switch (event->type) {
      case MOUSEMOVE: {
        uiBut *bt;

        if (data->menu && data->menu->region) {
          if (ui_region_contains_point_px(data->menu->region, event->mouse_pos.data())) {
            break;
          }
        }

        bt = ui_but_find_mouse_over(region, event);

        if (bt && bt->active != data) {
          if (but->type != UI_BTYPE_COLOR) { /* exception */
            data->cancel = true;
          }
          button_activate_state(C, but, BUTTON_STATE_EXIT);
        }
        break;
      }
      case RIGHTMOUSE: {
        if (event->val == KM_PRESS) {
          uiBut *bt = ui_but_find_mouse_over(region, event);
          if (bt && bt->active == data) {
            button_activate_state(C, bt, BUTTON_STATE_HIGHLIGHT);
          }
        }
        break;
      }
    }

    ui_do_button(C, block, but, event);
    retval = WM_UI_HANDLER_CONTINUE;
  } else {
    retval = ui_do_button(C, block, but, event);
    // retval = WM_UI_HANDLER_BREAK; XXX why ?
  }

  /* may have been re-allocated above (eyedropper for eg) */
  data = but->active;
  if (data && data->state == BUTTON_STATE_EXIT) {
    uiBut *post_but = data->postbut;
    const uiButtonActivateType post_type = data->posttype;

    /* Reset the button value when empty text is typed. */
    if ((data->cancel == false) && (data->str != NULL) && (data->str[0] == '\0') &&
        (but->stageprop && ELEM(LUXO_property_type(but->stageprop), PROP_FLOAT, PROP_INT))) {
      MEM_SAFE_FREE(data->str);
      // ui_button_value_default(but, &data->value);

#ifdef USE_DRAG_MULTINUM
      if (!data->multi_data.mbuts.empty()) {
        for (auto &state : data->multi_data.mbuts) {
          uiBut *but_iter = state->but;
          double default_value;

          // if (ui_button_value_default(but_iter, &default_value)) {
          //   ui_but_value_set(but_iter, default_value);
          // }
        }
      }
      data->multi_data.skip = true;
#endif
    }

    button_activate_exit(C, but, data, (post_but == NULL), false);

    /* for jumping to the next button with tab while text editing */
    if (post_but) {
      /* The post_but still has previous ranges (without the changes in active button considered),
       * needs refreshing the ranges. */
      ui_but_range_set_soft(post_but);
      ui_but_range_set_hard(post_but);

      button_activate_init(C, region, post_but, post_type);
    } else if (!((event->type == EVT_BUT_CANCEL) && (event->val == 1))) {
      /* XXX issue is because WM_event_add_mousemove(wm) is a bad hack and not reliable,
       * if that gets coded better this bypass can go away too.
       *
       * This is needed to make sure if a button was active,
       * it stays active while the mouse is over it.
       * This avoids adding mouse-moves, see: T33466. */
      if (ELEM(state_orig, BUTTON_STATE_INIT, BUTTON_STATE_HIGHLIGHT, BUTTON_STATE_WAIT_DRAG)) {
        if (ui_but_find_mouse_over(region, event) == but) {
          button_activate_init(C, region, but, BUTTON_ACTIVATE_OVER);
        }
      }
    }
  }

  return retval;
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
      ui_but_update(but);
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
    if (event->type == MOUSEMOVE &&
        ui_but_contains_point_px(but, data->region, event->mouse_pos.data())) {
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

static void ui_block_interaction_end(kContext *C,
                                     uiBlockInteraction_CallbackData *callbacks,
                                     uiBlockInteraction_Handle *interaction)
{
  KLI_assert(callbacks->end_fn != NULL);
  callbacks->end_fn(C, &interaction->params, callbacks->arg1, interaction->user_data);
  delete interaction->params.unique_retval_ids;
  delete interaction;
}

static void ui_apply_but_funcs_after(kContext *C)
{
  /* Copy to avoid recursive calls. */
  std::vector<uiAfterFunc *> funcs = UIAfterFuncs;
  UIAfterFuncs.clear();

  size_t afterfIdx = 0;
  for (auto &afterf : funcs) {
    uiAfterFunc after = *afterf; /* copy to avoid memleak on exit() */
    funcs.erase(funcs.begin() + afterfIdx);

    if (after.context) {
      CTX_store_set(C, after.context);
    }

    if (after.popup_op) {
      // popup_check(C, after.popup_op);
    }

    KrakenPRIM opptr;
    if (after.opptr) {
      /* free in advance to avoid leak on exit */
      opptr = *after.opptr;
      MEM_freeN(after.opptr);
    }

    if (after.optype) {
      // WM_operator_name_call_ptr_with_depends_on_cursor(C,
      //                                                  after.optype,
      //                                                  after.opcontext,
      //                                                  (after.opptr) ? &opptr : NULL,
      //                                                  NULL,
      //                                                  after.drawstr);
    }

    if (after.opptr) {
      WM_operator_properties_free(&opptr);
    }

    if (after.stagepoin.data) {
      // LUXO_property_update(C, &after.stagepoin, after.stageprop);
    }

    if (after.context) {
      CTX_store_set(C, NULL);
      CTX_store_free(after.context);
    }

    if (after.func) {
      after.func(C, after.func_arg1, after.func_arg2);
    }
    if (after.funcN) {
      after.funcN(C, after.func_argN, after.func_arg2);
    }
    if (after.func_argN) {
      MEM_freeN(after.func_argN);
    }

    if (after.handle_func) {
      after.handle_func(C, after.handle_func_arg, after.retval);
    }
    if (after.butm_func) {
      after.butm_func(C, after.butm_func_arg, after.a2);
    }

    if (after.rename_func) {
      after.rename_func(C, after.rename_arg1, (char *)after.rename_orig);
    }
    if (after.rename_orig) {
      MEM_freeN(after.rename_orig);
    }

    if (after.search_arg_free_fn) {
      after.search_arg_free_fn(after.search_arg);
    }

    if (after.custom_interaction_handle != NULL) {
      after.custom_interaction_handle->user_count--;
      KLI_assert(after.custom_interaction_handle->user_count >= 0);
      if (after.custom_interaction_handle->user_count == 0) {
        ui_block_interaction_update(C,
                                    &after.custom_interaction_callbacks,
                                    after.custom_interaction_handle);
        ui_block_interaction_end(C,
                                 &after.custom_interaction_callbacks,
                                 after.custom_interaction_handle);
      }
      after.custom_interaction_handle = NULL;
    }

    // ui_afterfunc_update_preferences_dirty(&after);

    if (after.undostr[0]) {
      // ED_undo_push(C, after.undostr);
    }

    ++afterfIdx;
  }
}

static void ui_def_but_rna__panel_type(kContext *C, uiLayout *layout, void *but_p)
{
  uiBut *but = static_cast<uiBut *>(but_p);
  const char *panel_type = static_cast<const char *>(but->func_argN);
  PanelType *pt = WM_paneltype_find(panel_type, true);
  if (pt) {
    ui_item_paneltype_func(C, layout, pt);
  } else {
    char msg[256];
    SNPRINTF(msg, TIP_("Missing Panel: %s"), panel_type);
    uiItemL(layout, msg, ICON_NONE);
  }
}

bool ui_but_menu_draw_as_popover(const uiBut *but)
{
  return (but->menu_create_func == ui_def_but_rna__panel_type);
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
    for (auto &bt : block->buttons) {
      if (bt->flag & UI_BUT_DRAG_MULTI) {
        bt->flag &= ~UI_BUT_DRAG_MULTI;

        if (!data->cancel) {
          // ui_apply_but_autokey(C, bt);
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
    // ui_apply_but_undo(but);
    // ui_apply_but_autokey(C, but);

#ifdef USE_ALLSELECT
    {
      /* only RNA from this button is used */
      uiBut but_temp = *but;
      uiSelectContextStore *selctx_data = &data->select_others;
      for (int i = 0; i < selctx_data->elems_len; i++) {
        uiSelectContextElem *other = &selctx_data->elems[i];
        but_temp.stagepoin = other->ptr;
        // ui_apply_but_autokey(C, &but_temp);
      }
    }
#endif

    /* popup menu memory */
    if (block->flag & UI_BLOCK_POPUP_MEMORY) {
      ui_popup_menu_memory_set(block, but);
    }

    if (UI_RUNTIME_IS_DIRTY == false) {
      // ui_but_update_preferences_dirty(but);
    }
  }

  /* Disable tool-tips until mouse-move + last active flag. */
  for (auto &block_iter : data->region->uiblocks) {
    for (auto &bt : block_iter->buttons) {
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

static int ui_handle_menu_return_submenu(kContext *C,
                                         const wmEvent *event,
                                         uiPopupBlockHandle *menu)
{
  ARegion *region = menu->region;
  uiBlock *block = region->uiblocks.front();

  uiBut *but = ui_region_find_active_but(region);

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
    // ui_mouse_motion_towards_reinit(menu, event->mouse_pos);
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
  uiBut *but = ui_region_find_active_but(menu->region);
  uiHandleButtonData *data = (but) ? but->active : NULL;
  uiPopupBlockHandle *submenu = (data) ? data->menu : NULL;

  if (submenu) {
    uiBlock *block = menu->region->uiblocks.front();
    const bool is_menu = ui_block_is_menu(block);
    bool inside = false;
    /* root pie menus accept the key that spawned
     * them as double click to improve responsiveness */
    const bool do_recursion = (!(block->flag & UI_BLOCK_RADIAL) ||
                               event->type != block->pie_data.event_type);

    if (do_recursion) {
      if (is_parent_inside == false) {
        int mx = event->mouse_pos[0];
        int my = event->mouse_pos[1];
        ui_window_to_block(menu->region, block, &mx, &my);
        inside = KLI_rctf_isect_pt(
          GfVec4f(block->rect.xmin, block->rect.xmax, block->rect.ymin, block->rect.ymax),
          mx,
          my);
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
      uiBlock *block = menu->region->uiblocks.front();

      // retval = ui_handle_menu_button(C, event, menu);

      if (block->flag & (UI_BLOCK_MOVEMOUSE_QUIT | UI_BLOCK_POPOVER)) {
        /* when there is a active search button and we close it,
         * we need to reinit the mouse coords T35346. */
        if (ui_region_find_active_but(menu->region) != but) {
          do_towards_reinit = true;
        }
      }
    } else {
      uiBlock *block = menu->region->uiblocks.front();
      uiBut *listbox = ui_list_find_mouse_over(menu->region, event);

      if (block->flag & UI_BLOCK_RADIAL) {
        // retval = ui_pie_handler(C, event, menu);
      } else if (event->type == LEFTMOUSE || event->val != KM_DBL_CLICK) {
        bool handled = false;

        if (listbox) {
          // const int retval_test = ui_handle_list_event(C, event, menu->region, listbox);
          // if (retval_test != WM_UI_HANDLER_CONTINUE) {
          //   retval = retval_test;
          //   handled = true;
          // }
        }

        if (handled == false) {
          // retval = ui_handle_menu_event(C,
          //                               event,
          //                               menu,
          //                               level,
          //                               is_parent_inside,
          //                               is_parent_menu,
          //                               is_floating);
        }
      }
    }
  }

  if (do_towards_reinit) {
    // ui_mouse_motion_towards_reinit(menu, event->mouse_pos);
  }

  return retval;
}

/* handle buttons at the window level, modal, for example while
 * number sliding, text editing, or when a menu block is open */
static int ui_handler_region_menu(kContext *C, const wmEvent *event, void *UNUSED(userdata))
{
  ARegion *menu_region = CTX_wm_menu(C);
  ARegion *region = menu_region ? menu_region : CTX_wm_region(C);
  int retval = WM_UI_HANDLER_CONTINUE;

  uiBut *but = ui_region_find_active_but(region);

  if (but) {
    kScreen *screen = CTX_wm_screen(C);
    uiBut *but_other;

    /* handle activated button events */
    uiHandleButtonData *data = but->active;

    if ((data->state == BUTTON_STATE_MENU_OPEN) &&
        /* Make sure this popup isn't dragging a button.
         * can happen with popovers (see T67882). */
        (ui_region_find_active_but(data->menu->region) == NULL) &&
        /* make sure mouse isn't inside another menu (see T43247) */
        (ui_screen_region_find_mouse_over(screen, event) == NULL) &&
        (ELEM(but->type, UI_BTYPE_PULLDOWN, UI_BTYPE_POPOVER, UI_BTYPE_MENU)) &&
        (but_other = ui_but_find_mouse_over(region, event)) && (but != but_other) &&
        (ELEM(but_other->type, UI_BTYPE_PULLDOWN, UI_BTYPE_POPOVER, UI_BTYPE_MENU)) &&
        /* Hover-opening menu's doesn't work well for buttons over one another
         * along the same axis the menu is opening on (see T71719). */
        (((data->menu->direction & (UI_DIR_LEFT | UI_DIR_RIGHT)) &&
          KLI_rctf_isect_rect_x(&but->rect, &but_other->rect, NULL)) ||
         ((data->menu->direction & (UI_DIR_DOWN | UI_DIR_UP)) &&
          KLI_rctf_isect_rect_y(&but->rect, &but_other->rect, NULL)))) {
      /* if mouse moves to a different root-level menu button,
       * open it to replace the current menu */
      if ((but_other->flag & UI_BUT_DISABLED) == 0) {
        ui_handle_button_activate(C, region, but_other, BUTTON_ACTIVATE_OVER);
        button_activate_state(C, but_other, BUTTON_STATE_MENU_OPEN);
        retval = WM_UI_HANDLER_BREAK;
      }
    } else if (data->state == BUTTON_STATE_MENU_OPEN) {
      /* handle events for menus and their buttons recursively,
       * this will handle events from the top to the bottom menu */
      if (data->menu) {
        retval = ui_handle_menus_recursive(C, event, data->menu, 0, false, false, false);
      }

      /* handle events for the activated button */
      if ((data->menu && (retval == WM_UI_HANDLER_CONTINUE)) || (event->type == TIMER)) {
        if (data->menu && data->menu->menuretval) {
          ui_handle_button_return_submenu(C, event, but);
          retval = WM_UI_HANDLER_BREAK;
        } else {
          retval = ui_handle_button_event(C, event, but);
        }
      }
    } else {
      /* handle events for the activated button */
      retval = ui_handle_button_event(C, event, but);
    }
  }

  /* Re-enable tool-tips. */
  if (event->type == MOUSEMOVE && (event->mouse_pos[0] != event->prev_mouse_pos[0] ||
                                   event->mouse_pos[1] != event->prev_mouse_pos[1])) {
    ui_blocks_set_tooltips(region, true);
  }

  if (but && but->active && but->active->menu) {
    /* Set correct context menu-region. The handling button above breaks if we set the region
     * first, so only set it for executing the after-funcs. */
    CTX_wm_menu_set(C, but->active->menu->region);
  }

  /* delayed apply callbacks */
  ui_apply_but_funcs_after(C);

  /* Reset to previous context region. */
  CTX_wm_menu_set(C, menu_region);

  /* Don't handle double-click events,
   * these will be converted into regular clicks which we handle. */
  if (retval == WM_UI_HANDLER_CONTINUE) {
    if (event->val == KM_DBL_CLICK) {
      return WM_UI_HANDLER_CONTINUE;
    }
  }

  /* we block all events, this is modal interaction */
  return WM_UI_HANDLER_BREAK;
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
     *   them into kraken, even if there's opened popup like splash screen (sergey).
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
    uiBlock *block = menu->region->uiblocks.front();

    /* set last pie event to allow chained pie spawning */
    if (block->flag & UI_BLOCK_RADIAL) {
      // win->pie_event_type_last = block->pie_data.event_type;
      reset_pie = true;
    }

    ui_popup_block_free(C, menu);
    const auto &mnu = win->modalhandlers.begin();
    while (mnu != win->modalhandlers.end()) {
      win->modalhandlers.erase(mnu);
    }
    CTX_wm_menu_set(C, NULL);

#ifdef USE_DRAG_TOGGLE
    {
      // WM_event_free_ui_handler_all(C,
      //                              &win->modalhandlers,
      //                              ui_handler_region_drag_toggle,
      //                              ui_handler_region_drag_toggle_remove);
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
    if (event->type == MOUSEMOVE && (event->mouse_pos[0] != event->prev_mouse_pos[0] ||
                                     event->mouse_pos[1] != event->prev_mouse_pos[1])) {
      ui_blocks_set_tooltips(menu->region, true);
    }
  }

  /* delayed apply callbacks */
  ui_apply_but_funcs_after(C);

  if (reset_pie) {
    /* Reacquire window in case pie invalidates it somehow. */
    wmWindow *win = CTX_wm_window(C);

    if (win) {
      // win->pie_event_type_last = EVENT_NONE;
    }
  }

  CTX_wm_region_set(C, menu_region);

  return retval;
}

static void ui_popup_handler_remove(kContext *C, void *userdata)
{
  uiPopupBlockHandle *menu = (uiPopupBlockHandle *)userdata;

  /* More correct would be to expect UI_RETURN_CANCEL here, but not wanting to
   * cancel when removing handlers because of file exit is a rare exception.
   * So instead of setting cancel flag for all menus before removing handlers,
   * just explicitly flag menu with UI_RETURN_OK to avoid canceling it. */
  if ((menu->menuretval & UI_RETURN_OK) == 0 && menu->cancel_func) {
    menu->cancel_func(C, menu->popup_arg);
  }

  /* free menu block if window is closed for some reason */
  ui_popup_block_free(C, menu);

  /* delayed apply callbacks */
  ui_apply_but_funcs_after(C);
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