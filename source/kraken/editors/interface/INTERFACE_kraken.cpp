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

#include "USD_operator.h"
#include "USD_wm_types.h"
#include "USD_factory.h"
#include "USD_userpref.h"

#include "WM_window.h"
#include "WM_event_system.h"

#include "LUXO_access.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KLI_rect.h"
#include "KLI_string_utils.h"

#include "KKE_scene.h"
#include "KKE_context.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

/* avoid unneeded calls to ui_but_value_get */
#define UI_BUT_VALUE_UNSET DBL_MAX
#define UI_GET_BUT_VALUE_INIT(_but, _t, _value) \
  (_value) = UI_but_value_get<_t>(_but);        \
  ((void)0)

#define B_NOP -1

/* Auto-complete helper functions. */
struct AutoComplete
{
  size_t maxlen;
  int matches;
  char *truncate;
  const char *startname;
};

void UI_window_to_block_fl(const ARegion *region, uiBlock *block, float *r_x, float *r_y)
{
  GfVec4i coords = FormFactory(region->coords);

  const int getsizex = GET_Y(coords) - GET_X(coords) + 1;
  const int getsizey = GET_Z(coords) - GET_W(coords) + 1;
  const int sx = GET_X(coords);
  const int sy = GET_Z(coords);

  const float a = 0.5f * ((float)getsizex) * block->winmat[0][0];
  const float b = 0.5f * ((float)getsizex) * block->winmat[1][0];
  const float c = 0.5f * ((float)getsizex) * (1.0f + block->winmat[3][0]);

  const float d = 0.5f * ((float)getsizey) * block->winmat[0][1];
  const float e = 0.5f * ((float)getsizey) * block->winmat[1][1];
  const float f = 0.5f * ((float)getsizey) * (1.0f + block->winmat[3][1]);

  const float px = *r_x - sx;
  const float py = *r_y - sy;

  *r_y = (a * (py - f) + d * (c - px)) / (a * e - d * b);
  *r_x = (px - b * (*r_y) - c) / a;

  if (block->panel) {
    *r_x -= block->panel->ofsx;
    *r_y -= block->panel->ofsy;
  }
}

void UI_window_to_block(const ARegion *region, uiBlock *block, int *r_x, int *r_y)
{
  float fx = *r_x;
  float fy = *r_y;

  UI_window_to_block_fl(region, block, &fx, &fy);

  *r_x = (int)lround(fx);
  *r_y = (int)lround(fy);
}

/* ************* window matrix ************** */

void ui_block_to_region_fl(const ARegion *region, uiBlock *block, float *r_x, float *r_y)
{
  GfVec4i coords;
  region->coords.Get(&coords);
  const int getsizex = KLI_rcti_size_x(coords) + 1;
  const int getsizey = KLI_rcti_size_y(coords) + 1;

  float gx = *r_x;
  float gy = *r_y;

  if (block->panel) {
    gx += block->panel->ofsx;
    gy += block->panel->ofsy;
  }

  *r_x = ((float)getsizex) * (0.5f + 0.5f * (gx * block->winmat[0][0] + gy * block->winmat[1][0] +
                                             block->winmat[3][0]));
  *r_y = ((float)getsizey) * (0.5f + 0.5f * (gx * block->winmat[0][1] + gy * block->winmat[1][1] +
                                             block->winmat[3][1]));
}

void ui_block_to_window_fl(const ARegion *region, uiBlock *block, float *r_x, float *r_y)
{
  GfVec4i coords;
  region->coords.Get(&coords);
  ui_block_to_region_fl(region, block, r_x, r_y);
  *r_x += coords[0];
  *r_y += coords[2];
}

/* ************* EVENTS ************* */

uiBlock *UI_block_begin(const kContext *C, ARegion *region, const char *name, eUIEmbossType emboss)
{
  wmWindow *window = CTX_wm_window(C);
  Scene *scene = CTX_data_scene(C);

  uiBlock *block = new uiBlock();
  block->active = true;
  block->emboss = emboss;
  block->evil_C = (void *)C; /* XXX */

  block->button_groups.clear();

  if (scene) {
    /* store display device name, don't lookup for transformations yet
     * block could be used for non-color displays where looking up for transformation
     * would slow down redraw, so only lookup for actual transform when it's indeed
     * needed
     */
    STRNCPY(block->display_device, scene->display_settings.display_device);

    /* copy to avoid crash when scene gets deleted with ui still open */
    block->unit = new UnitSettings();
    memcpy(block->unit, &scene->unit, sizeof(scene->unit));
  }
  else {
    STRNCPY(block->display_device, IMB_colormanagement_display_get_default_name());
  }

  KLI_strncpy(block->name, name, sizeof(block->name));

  if (region) {
    UI_block_region_set(block, region);
  }

  /* Set window matrix and aspect for region and OpenGL state. */
  ui_update_window_matrix(window, region, block);

  /* Tag as popup menu if not created within a region. */
  if (!(region && region->visible)) {
    block->auto_open = true;
    block->flag |= UI_BLOCK_LOOP;
  }

  return block;
}

template<typename T> int ui_but_is_pushed_ex(uiBut *but, T *value)
{
  int is_push = 0;
  if (but->pushed_state_func) {
    return but->pushed_state_func(but, but->pushed_state_arg);
  }

  if (but->bit) {
    const bool state = !ELEM(but->type,
                             UI_BTYPE_TOGGLE_N,
                             UI_BTYPE_ICON_TOGGLE_N,
                             UI_BTYPE_CHECKBOX_N);
    T lvalue;
    UI_GET_BUT_VALUE_INIT(but, T, *value);
    lvalue = (T)*value;
    if (UI_BITBUT_TEST(lvalue, (but->bitnr))) {
      is_push = state;
    } else {
      is_push = !state;
    }
  } else {
    switch (but->type) {
      case UI_BTYPE_BUT:
      case UI_BTYPE_HOTKEY_EVENT:
      case UI_BTYPE_KEY_EVENT:
      case UI_BTYPE_COLOR:
      case UI_BTYPE_DECORATOR:
        is_push = -1;
        break;
      case UI_BTYPE_BUT_TOGGLE:
      case UI_BTYPE_TOGGLE:
      case UI_BTYPE_ICON_TOGGLE:
      case UI_BTYPE_CHECKBOX:
        UI_GET_BUT_VALUE_INIT(but, T, *value);
        if (*value != (T)but->hardmin) {
          is_push = true;
        }
        break;
      case UI_BTYPE_ICON_TOGGLE_N:
      case UI_BTYPE_TOGGLE_N:
      case UI_BTYPE_CHECKBOX_N:
        UI_GET_BUT_VALUE_INIT(but, T, *value);
        if (*value == 0) {
          is_push = true;
        }
        break;
      case UI_BTYPE_ROW:
      case UI_BTYPE_LISTROW:
      case UI_BTYPE_TAB:
        if ((but->type == UI_BTYPE_TAB) && but->stageprop && but->custom_data) {
          /* uiBut.custom_data points to data this tab represents (e.g. workspace).
           * uiBut.stagepoin/prop store an active value (e.g. active workspace). */
          if (but->stageprop->type == PROP_POINTER) {
            const KrakenPRIM active_ptr = CreationFactory::PTR::Set(&but->stagepoin,
                                                                    but->stageprop->name,
                                                                    but->stageprop);
            if (active_ptr.data == but->custom_data) {
              is_push = true;
            }
          }
          break;
        } else if (but->optype) {
          break;
        }

        UI_GET_BUT_VALUE_INIT(but, T, *value);
        /* support for stage enum buts */
        if (but->stageprop && (but->stageprop->type & PROP_ENUM_FLAG)) {
          if ((T)*value & (int)but->hardmax) {
            is_push = true;
          }
        } else {
          if (*value == (double)but->hardmax) {
            is_push = true;
          }
        }
        break;
      case UI_BTYPE_VIEW_ITEM: {
        const uiButViewItem *view_item_but = (const uiButViewItem *)but;

        is_push = -1;
        if (view_item_but->view_item) {
          is_push = UI_view_item_is_active(view_item_but->view_item);
        }
        break;
      }
      default:
        is_push = -1;
        break;
    }
  }

  if ((but->drawflag & UI_BUT_CHECKBOX_INVERT) && (is_push != -1)) {
    is_push = !((bool)is_push);
  }
  return is_push;
}

template<typename T> int ui_but_is_pushed(uiBut *but)
{
  T value;
  return ui_but_is_pushed_ex(but, &value);
}

template<typename T> static void ui_but_update_select_flag(uiBut *but, T *value)
{
  switch (ui_but_is_pushed_ex(but, value)) {
    case true:
      but->flag |= UI_SELECT;
      break;
    case false:
      but->flag &= ~UI_SELECT;
      break;
  }
}

static void ui_but_string_free_internal(uiBut *but)
{
  if (but->str) {
    if (but->str != but->strdata) {
      delete but->str;
    }
    /* must call 'ui_but_string_set_internal' after */
    but->str = nullptr;
  }
}

/* just the assignment/free part */
static void ui_but_string_set_internal(uiBut *but, const char *str, size_t str_len)
{
  KLI_assert(str_len == strlen(str));
  KLI_assert(but->str == nullptr);
  str_len += 1;

  if (str_len > UI_MAX_NAME_STR) {
    but->str = new char[str_len]();
  } else {
    but->str = but->strdata;
  }
  memcpy(but->str, str, str_len);
}

bool ui_but_is_float(const uiBut *but)
{
  if (but->pointype == UI_BUT_POIN_FLOAT && but->poin) {
    return true;
  }

  if (but->stageprop && (but->stageprop->type == PROP_FLOAT)) {
    return true;
  }

  return false;
}

static float ui_but_get_float_step_size(uiBut *but)
{
  if (but->type == UI_BTYPE_NUM) {
    return ((uiButNumber *)but)->step_size;
  }

  return but->a1;
}

int UI_but_unit_type_get(const uiBut *but)
{
  const int ownUnit = (int)but->unit_type;

  /* own unit define always takes precedence over RNA provided, allowing for overriding
   * default value provided in RNA in a few special cases (i.e. Active Keyframe in Graph Edit)
   */
  /* XXX: this doesn't allow clearing unit completely, though the same could be said for icons */
  if ((ownUnit != 0) || (but->stageprop == nullptr)) {
    return ownUnit << 16;
  }
  return ((but->stageprop->subtype) & 0x00FF0000);
}

static bool ui_but_hide_fraction(uiBut *but, double value)
{
  /* Hide the fraction if both the value and the step are exact integers. */
  if (floor(value) == value) {
    const float step = ui_but_get_float_step_size(but) * UI_PRECISION_FLOAT_SCALE;

    if (floorf(step) == step) {
      /* Don't hide if it has any unit except frame count. */
      switch (UI_but_unit_type_get(but)) {
        case PROP_UNIT_NONE:
        case PROP_UNIT_TIME:
          return true;

        default:
          return false;
      }
    }
  }

  return false;
}

static float ui_but_get_float_precision(uiBut *but)
{
  if (but->type == UI_BTYPE_NUM) {
    return ((uiButNumber *)but)->precision;
  }

  return but->a2;
}

static bool ui_but_is_unit_radians_ex(UnitSettings *unit, const int unit_type)
{
  return (unit->system_rotation == USER_UNIT_ROT_RADIANS && unit_type == PROP_UNIT_ROTATION);
}

static bool ui_but_is_unit_radians(const uiBut *but)
{
  UnitSettings *unit = but->block->unit;
  const int unit_type = UI_but_unit_type_get(but);

  return ui_but_is_unit_radians_ex(unit, unit_type);
}

int UI_calc_float_precision(int prec, double value)
{
  static const double pow10_neg[UI_PRECISION_FLOAT_MAX + 1] =
    {1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6};
  static const double max_pow = 10000000.0; /* pow(10, UI_PRECISION_FLOAT_MAX) */

  KLI_assert(prec <= UI_PRECISION_FLOAT_MAX);
  KLI_assert(fabs(pow10_neg[prec] - pow(10, -prec)) < 1e-16);

  /* Check on the number of decimal places need to display the number,
   * this is so 0.00001 is not displayed as 0.00,
   * _but_, this is only for small values as 10.0001 will not get the same treatment.
   */
  value = fabs(value);
  if ((value < pow10_neg[prec]) && (value > (1.0 / max_pow))) {
    int value_i = (int)lround(value * max_pow);
    if (value_i != 0) {
      const int prec_span = 3; /* show: 0.01001, 5 would allow 0.0100001 for eg. */
      int test_prec;
      int prec_min = -1;
      int dec_flag = 0;
      int i = UI_PRECISION_FLOAT_MAX;
      while (i && value_i) {
        if (value_i % 10) {
          dec_flag |= 1 << i;
          prec_min = i;
        }
        value_i /= 10;
        i--;
      }

      /* even though its a small value, if the second last digit is not 0, use it */
      test_prec = prec_min;

      dec_flag = (dec_flag >> (prec_min + 1)) & ((1 << prec_span) - 1);

      while (dec_flag) {
        test_prec++;
        dec_flag = dec_flag >> 1;
      }

      if (test_prec > prec) {
        prec = test_prec;
      }
    }
  }

  CLAMP(prec, 0, UI_PRECISION_FLOAT_MAX);

  return prec;
}

static int ui_but_calc_float_precision(uiBut *but, double value)
{
  if (ui_but_hide_fraction(but, value)) {
    return 0;
  }

  int prec = (int)ui_but_get_float_precision(but);

  /* first check for various special cases:
   * * If button is radians, we want additional precision (see T39861).
   * * If prec is not set, we fallback to a simple default */
  if (ui_but_is_unit_radians(but) && prec < 5) {
    prec = 5;
  } else if (prec == -1) {
    prec = (but->hardmax < 10.001f) ? 3 : 2;
  } else {
    CLAMP(prec, 0, UI_PRECISION_FLOAT_MAX);
  }

  return UI_calc_float_precision(prec, value);
}

static void ui_but_build_drawstr_float(uiBut *but, double value)
{
  size_t slen = 0;
  STR_CONCAT(but->drawstr, slen, but->str);

  PropertySubType subtype;
  if (but->stageprop) {
    subtype = but->stageprop->subtype;
  }

  /* Change negative zero to regular zero, without altering anything else. */
  value += +0.0f;

  if (value == (double)FLT_MAX) {
    STR_CONCAT(but->drawstr, slen, "inf");
  } else if (value == (double)-FLT_MAX) {
    STR_CONCAT(but->drawstr, slen, "-inf");
  } else if (subtype == PROP_PERCENTAGE) {
    const int prec = ui_but_calc_float_precision(but, value);
    STR_CONCATF(but->drawstr, slen, "%.*f%%", prec, value);
  } else if (subtype == PROP_PIXEL) {
    const int prec = ui_but_calc_float_precision(but, value);
    STR_CONCATF(but->drawstr, slen, "%.*f px", prec, value);
  } else if (subtype == PROP_FACTOR) {
    const int precision = ui_but_calc_float_precision(but, value);

    if (UI_FACTOR_DISPLAY_TYPE == USER_FACTOR_AS_FACTOR) {
      STR_CONCATF(but->drawstr, slen, "%.*f", precision, value);
    } else {
      STR_CONCATF(but->drawstr, slen, "%.*f%%", MAX2(0, precision - 2), value * 100);
    }
  } else if (ui_but_is_unit(but)) {
    char new_str[sizeof(but->drawstr)];
    ui_get_but_string_unit(but, new_str, sizeof(new_str), value, true, -1);
    STR_CONCAT(but->drawstr, slen, new_str);
  } else {
    const int prec = ui_but_calc_float_precision(but, value);
    STR_CONCATF(but->drawstr, slen, "%.*f", prec, value);
  }
}

/**
 * \param but: Button to update.
 * \param validate: When set, this function may change the button value.
 * Otherwise treat the button value as read-only.
 */
template<typename T> static void ui_but_update_ex(uiBut *but, const bool validate)
{
  /* if something changed in the button */
  T value;

  ui_but_update_select_flag(but, &value);

  /* only update soft range while not editing */
  if (!UI_but_is_editing(but)) {
    if ((but->stageprop != nullptr) || (but->poin && (but->pointype & UI_BUT_POIN_TYPES))) {
      ui_but_range_set_soft(but);
    }
  }

  /* test for min and max, icon sliders, etc */
  switch (but->type) {
    case UI_BTYPE_NUM:
    case UI_BTYPE_SCROLL:
    case UI_BTYPE_NUM_SLIDER:
      if (validate) {
        UI_GET_BUT_VALUE_INIT(but, T, value);
        UI_but_value_set(but, but->hardmax);

        /* max must never be smaller than min! Both being equal is allowed though */
        KLI_assert(but->softmin <= but->softmax && but->hardmin <= but->hardmax);
      }
      break;

    case UI_BTYPE_ICON_TOGGLE:
    case UI_BTYPE_ICON_TOGGLE_N:
      if ((but->stageprop == nullptr) || (but->stageprop->flag & PROP_ICONS_CONSECUTIVE)) {
        if (but->stageprop && (but->stageprop->flag & PROP_ICONS_REVERSE)) {
          but->drawflag |= UI_BUT_ICON_REVERSE;
        }

        but->iconadd = (but->flag & UI_SELECT) ? 1 : 0;
      }
      break;

      /* quiet warnings for unhandled types */
    default:
      break;
  }

  /* safety is 4 to enable small number buttons (like 'users') */
  // okwidth = -4 + (KLI_rcti_size_x(&but->rect)); /* UNUSED */

  /* name: */
  switch (but->type) {

    case UI_BTYPE_MENU:
      if (KLI_rctf_size_x(&but->rect) >= (UI_UNIT_X * 2)) {
        /* only needed for menus in popup blocks that don't recreate buttons on redraw */
        if (but->block->flag & UI_BLOCK_LOOP) {
          if (but->stageprop && (but->stageprop->type == PROP_ENUM)) {
            const TfToken value_enum = FormFactory(
              but->stagepoin.GetAttribute(but->stageprop->name));

            // EnumPropertyItem item;
            // if (RNA_property_enum_item_from_value_gettexted(
            //       static_cast<kContext *>(but->block->evil_C),
            //       &but->stagepoin,
            //       but->stageprop,
            //       value_enum,
            //       &item)) {
            const size_t slen = strlen(but->stageprop->GetDisplayName());
            ui_but_string_free_internal(but);
            ui_but_string_set_internal(but, item.name, slen);
            but->icon = (BIFIconID)item.icon;
            // }
          }
        }
        KLI_strncpy(but->drawstr, but->str, sizeof(but->drawstr));
      }
      break;

    case UI_BTYPE_NUM:
    case UI_BTYPE_NUM_SLIDER:
      if (but->editstr) {
        break;
      }
      UI_GET_BUT_VALUE_INIT(but, T, value);
      if (ui_but_is_float(but)) {
        ui_but_build_drawstr_float(but, value);
      } else {
        ui_but_build_drawstr_int(but, (int)value);
      }
      break;

    case UI_BTYPE_LABEL:
      if (ui_but_is_float(but)) {
        UI_GET_BUT_VALUE_INIT(but, T, value);
        const int prec = ui_but_calc_float_precision(but, value);
        KLI_snprintf(but->drawstr, sizeof(but->drawstr), "%s%.*f", but->str, prec, value);
      } else {
        KLI_strncpy(but->drawstr, but->str, UI_MAX_DRAW_STR);
      }

      break;

    case UI_BTYPE_TEXT:
    case UI_BTYPE_SEARCH_MENU:
      if (!but->editstr) {
        char str[UI_MAX_DRAW_STR];

        UI_but_string_get(but, str, UI_MAX_DRAW_STR);
        KLI_snprintf(but->drawstr, sizeof(but->drawstr), "%s%s", but->str, str);
      }
      break;

    case UI_BTYPE_KEY_EVENT: {
      const char *str;
      if (but->flag & UI_SELECT) {
        str = "Press a key";
      } else {
        UI_GET_BUT_VALUE_INIT(but, T, value);
        str = WM_key_event_string((short)value, false);
      }
      KLI_snprintf(but->drawstr, UI_MAX_DRAW_STR, "%s%s", but->str, str);
      break;
    }
    case UI_BTYPE_HOTKEY_EVENT:
      if (but->flag & UI_SELECT) {
        const uiButHotkeyEvent *hotkey_but = (uiButHotkeyEvent *)but;

        if (hotkey_but->modifier_key) {
          char *str = but->drawstr;
          but->drawstr[0] = '\0';

          if (hotkey_but->modifier_key & KM_SHIFT) {
            str += KLI_strcpy_rlen(str, "Shift ");
          }
          if (hotkey_but->modifier_key & KM_CTRL) {
            str += KLI_strcpy_rlen(str, "Ctrl ");
          }
          if (hotkey_but->modifier_key & KM_ALT) {
            str += KLI_strcpy_rlen(str, "Alt ");
          }
          if (hotkey_but->modifier_key & KM_OSKEY) {
            str += KLI_strcpy_rlen(str, "Cmd ");
          }

          (void)str; /* UNUSED */
        } else {
          KLI_strncpy(but->drawstr, "Press a key", UI_MAX_DRAW_STR);
        }
      } else {
        KLI_strncpy(but->drawstr, but->str, UI_MAX_DRAW_STR);
      }

      break;

    case UI_BTYPE_HSVCUBE:
    case UI_BTYPE_HSVCIRCLE:
      break;
    default:
      KLI_strncpy(but->drawstr, but->str, UI_MAX_DRAW_STR);
      break;
  }

  /* if we are doing text editing, this will override the drawstr */
  if (but->editstr) {
    but->drawstr[0] = '\0';
  }

  /* text clipping moved to widget drawing code itself */
}

template<typename T> void UI_but_update(uiBut *but)
{
  ui_but_update_ex<T>(but, false);
}

template<typename T> void UI_but_update_edited(uiBut *but)
{
  ui_but_update_ex<T>(but, true);
}

template<typename T> T UI_but_value_get(uiBut *but)
{
  T value;

  if (but->editval) {
    return *(but->editval);
  }
  if (but->poin == nullptr && but->stagepoin.data == nullptr) {
    return value;
  }

  if (but->stageprop) {
    KrakenPROP *prop = but->stageprop;

    KLI_assert(but->stageindex != -1);

    switch (prop->type) {
      case PROP_BOOLEAN:
        value = FormFactory<bool>(&but->stagepoin.GetAttribute(prop->name));
        break;
      case PROP_INT:
        value = FormFactory<int>(&but->stagepoin.GetAttribute(prop->name));
        break;
      case PROP_FLOAT:
        value = FormFactory<float>(&but->stagepoin.GetAttribute(prop->name));
        break;
      case PROP_ENUM:
        value = FormFactory<wabi::TfToken>(&but->stagepoin.GetAttribute(prop->name));
        break;
      default:
        value = 0.0;
        break;
    }
  } else if (but->pointype == UI_BUT_POIN_CHAR) {
    value = *(char *)but->poin;
  } else if (but->pointype == UI_BUT_POIN_SHORT) {
    value = *(short *)but->poin;
  } else if (but->pointype == UI_BUT_POIN_INT) {
    value = *(int *)but->poin;
  } else if (but->pointype == UI_BUT_POIN_FLOAT) {
    value = *(float *)but->poin;
  }

  return value;
}

bool ui_but_is_unit(const uiBut *but)
{
  UnitSettings *unit = but->block->unit;
  const int unit_type = UI_but_unit_type_get(but);

  if (unit_type == PROP_UNIT_NONE) {
    return false;
  }

#if 1 /* removed so angle buttons get correct snapping */
  if (ui_but_is_unit_radians_ex(unit, unit_type)) {
    return false;
  }
#endif

  /* for now disable time unit conversion */
  if (unit_type == PROP_UNIT_TIME) {
    return false;
  }

  if (unit->system == USER_UNIT_NONE) {
    if (unit_type != PROP_UNIT_ROTATION) {
      return false;
    }
  }

  return true;
}

static double ui_get_but_scale_unit(uiBut *but, double value)
{
  UnitSettings *unit = but->block->unit;
  const int unit_type = UI_but_unit_type_get(but);

  /* WARNING: using evil_C :| */
  Scene *scene = CTX_data_scene(static_cast<const kContext *>(but->block->evil_C));

  /* Time unit is a bit special, not handled by KKE_scene_unit_scale() for now. */
  if (unit_type == PROP_UNIT_TIME) {
    return FRA2TIME(value);
  }

  return KKE_scene_unit_scale(unit, ((unit_type) >> 16), value, scene->stage);
}

template<typename T> void UI_but_value_set(uiBut *but, T value)
{
  /* Value is a HSV value: convert to RGB. */
  if (but->stageprop) {
    KrakenPROP *prop = but->stageprop;

    if (prop && (!prop->IsHidden() && prop->IsValid())) {
      switch (prop->type) {
        case PROP_BOOLEAN:
          CreationFactory::BOOL::Set(&but->stagepoin, prop->GetName(), value);
          break;
        case PROP_INT:
          CreationFactory::INT::Set(&but->stagepoin, prop->GetName(), value);
          break;
        case PROP_FLOAT:
          CreationFactory::FLOAT::Set(&but->stagepoin, prop->GetName(), value);
          break;
        case PROP_ENUM:
          CreationFactory::TOKEN::Set(&but->stagepoin, prop->GetName(), value);
          break;
        default:
          break;
      }
    }
  } else if (but->pointype == 0) {
    /* pass */
  } else {
    /* first do rounding */
    if (but->pointype == UI_BUT_POIN_CHAR) {
      value = round_db_to_uchar_clamp(value);
    } else if (but->pointype == UI_BUT_POIN_SHORT) {
      value = round_db_to_short_clamp(value);
    } else if (but->pointype == UI_BUT_POIN_INT) {
      value = round_db_to_int_clamp(value);
    } else if (but->pointype == UI_BUT_POIN_FLOAT) {
      float fval = (float)value;
      if (fval >= -0.00001f && fval <= 0.00001f) {
        /* prevent negative zero */
        fval = 0.0f;
      }
      value = fval;
    }

    /* then set value with possible edit override */
    if (but->editval) {
      value = *but->editval = value;
    } else if (but->pointype == UI_BUT_POIN_CHAR) {
      value = *((char *)but->poin) = (char)value;
    } else if (but->pointype == UI_BUT_POIN_SHORT) {
      value = *((short *)but->poin) = (short)value;
    } else if (but->pointype == UI_BUT_POIN_INT) {
      value = *((int *)but->poin) = (int)value;
    } else if (but->pointype == UI_BUT_POIN_FLOAT) {
      value = *((float *)but->poin) = (float)value;
    }
  }

  ui_but_update_select_flag(but, &value);
}

bool UI_but_is_utf8(const uiBut *but)
{
  if (but->stageprop) {
    const int subtype = but->stageprop->subtype;
    return !(ELEM(subtype, PROP_FILEPATH, PROP_DIRPATH, PROP_FILENAME, PROP_BYTESTRING));
  }
  return !(but->flag & UI_BUT_NO_UTF8);
}

/**
 * \param float_precision: Override the button precision.
 */
static void ui_get_but_string_unit(uiBut *but,
                                   char *str,
                                   int len_max,
                                   double value,
                                   bool pad,
                                   int float_precision)
{
  UnitSettings *unit = but->block->unit;
  const int unit_type = UI_but_unit_type_get(but);
  int precision;

  if (unit->scale_length < 0.0001f) {
    unit->scale_length = 1.0f; /* XXX do_versions */
  }

  /* Use precision override? */
  if (float_precision == -1) {
    /* Sanity checks */
    precision = (int)ui_but_get_float_precision(but);
    if (precision > UI_PRECISION_FLOAT_MAX) {
      precision = UI_PRECISION_FLOAT_MAX;
    } else if (precision == -1) {
      precision = 2;
    }
  } else {
    precision = float_precision;
  }

  KKE_unit_value_as_string(str,
                           len_max,
                           ui_get_but_scale_unit(but, value),
                           precision,
                           ((unit_type) >> 16),
                           unit,
                           pad);
}

void UI_but_string_get_ex(uiBut *but,
                          char *str,
                          const size_t maxlen,
                          const int float_precision,
                          const bool use_exp_float,
                          bool *r_use_exp_float)
{
  if (r_use_exp_float) {
    *r_use_exp_float = false;
  }

  if (but->stageprop && ELEM(but->type, UI_BTYPE_TEXT, UI_BTYPE_SEARCH_MENU, UI_BTYPE_TAB)) {
    const PropertyType type = but->stageprop->type;

    size_t buf_len;
    const char *buf = nullptr;
    if ((but->type == UI_BTYPE_TAB) && (but->custom_data)) {
      KrakenPRIM *ptr_type = &CreationFactory::PTR::Set(&but->stagepoin,
                                                        but->stageprop->GetName(),
                                                        but->stageprop);

      /* uiBut.custom_data points to data this tab represents (e.g. workspace).
       * uiBut.stagepoin/prop store an active value (e.g. active workspace). */
      LUXO_pointer_create(ptr_type, but->custom_data, &but->stagepoin);

      buf_len = but->stagepoin.GetParent().GetName().GetString().length();
      if (buf_len + 1 < maxlen) {
        buf = str;
      } else {
        buf = new char[buf_len + 1]();
      }

      buf = KLI_strncpy(str, but->stagepoin.GetParent().GetName().GetText(), buf_len);

    } else if (type == PROP_STRING) {
      buf_len = but->stageprop->GetName().GetString().length();
      if (buf_len + 1 < maxlen) {
        buf = str;
      } else {
        buf = new char[buf_len + 1]();
      }

      buf = KLI_strncpy(str, but->stageprop->GetName().GetText(), buf_len);

    } else if (type == PROP_ENUM) {
      /* RNA enum */
      const TfToken value = FormFactory(but->stagepoin.GetAttribute(but->stageprop->GetName()));
      // if (RNA_property_enum_name(static_cast<kContext *>(but->block->evil_C),
      //                            &but->stagepoin,
      //                            but->stageprop,
      //                            value,
      //                            &buf)) {
      buf = KLI_strncpy(str, value.GetText(), maxlen);
      // }
    } else if (type == PROP_POINTER) {
      /* RNA pointer */
      KrakenPRIM ptr = but->stageprop->GetPrim();

      buf_len = ptr.GetName().GetString().length();
      if (buf_len + 1 < maxlen) {
        buf = str;
      } else {
        buf = new char[buf_len + 1]();
      }

      buf = KLI_strncpy(str, ptr.GetName().GetText(), buf_len);
    } else {
      KLI_assert(0);
    }

    if (buf == nullptr) {
      str[0] = '\0';
    } else if (buf != str) {
      KLI_assert(maxlen <= buf_len + 1);
      /* string was too long, we have to truncate */
      if (UI_but_is_utf8(but)) {
        KLI_strncpy_utf8(str, buf, maxlen);
      } else {
        KLI_strncpy(str, buf, maxlen);
      }
      delete buf;
    }
  } else if (ELEM(but->type, UI_BTYPE_TEXT, UI_BTYPE_SEARCH_MENU)) {
    /* string */
    KLI_strncpy(str, but->poin, maxlen);
    return;
    // } else if (ui_but_anim_expression_get(but, str, maxlen)) {
    /* driver expression */
  } else {
    /* number editing */
    const double value = UI_but_value_get<double>(but);

    PropertySubType subtype = PROP_NONE;
    if (but->stageprop) {
      subtype = but->stageprop->subtype;
    }

    if (ui_but_is_float(but)) {
      int prec = float_precision;

      if (float_precision == -1) {
        prec = ui_but_calc_float_precision(but, value);
      } else if (!use_exp_float && ui_but_hide_fraction(but, value)) {
        prec = 0;
      }

      if (ui_but_is_unit(but)) {
        ui_get_but_string_unit(but, str, maxlen, value, false, prec);
      } else if (subtype == PROP_FACTOR) {
        if (UI_FACTOR_DISPLAY_TYPE == USER_FACTOR_AS_FACTOR) {
          KLI_snprintf(str, maxlen, "%.*f", prec, value);
        } else {
          KLI_snprintf(str, maxlen, "%.*f", MAX2(0, prec - 2), value * 100);
        }
      } else {
        const int int_digits_num = integer_digits_f(value);
        if (use_exp_float) {
          if (int_digits_num < -6 || int_digits_num > 12) {
            KLI_snprintf(str, maxlen, "%.*g", prec, value);
            if (r_use_exp_float) {
              *r_use_exp_float = true;
            }
          } else {
            prec -= int_digits_num;
            CLAMP(prec, 0, UI_PRECISION_FLOAT_MAX);
            KLI_snprintf(str, maxlen, "%.*f", prec, value);
          }
        } else {
          prec -= int_digits_num;
          CLAMP(prec, 0, UI_PRECISION_FLOAT_MAX);
          KLI_snprintf(str, maxlen, "%.*f", prec, value);
        }
      }
    } else {
      KLI_snprintf(str, maxlen, "%d", (int)value);
    }
  }
}

void UI_but_string_get(uiBut *but, char *str, const size_t maxlen)
{
  UI_but_string_get_ex(but, str, maxlen, -1, false, nullptr);
}

KRAKEN_NAMESPACE_END