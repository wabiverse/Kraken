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

#include "USD_area.h"
#include "USD_operator.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_wm_types.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KLI_rect.h"

#include "KKE_context.h"

#include "interface_intern.h"
#include "interface_regions_intern.h"

KRAKEN_NAMESPACE_BEGIN

#define UI_TIP_PAD_FAC 1.3f
#define UI_TIP_PADDING (int)(UI_TIP_PAD_FAC * UI_UNIT_Y)
#define UI_TIP_MAXWIDTH 600

#define UI_TIP_STR_MAX 1024

struct uiTooltipFormat {
  enum class Style : int8_t {
    Normal,
    Header,
    Mono,
  };
  enum class ColorID : int8_t {
    /** Primary Text. */
    Main = 0,
    /** The value of buttons (also shortcuts). */
    Value = 1,
    /** Titles of active enum values. */
    Active = 2,
    /** Regular text. */
    Normal = 3,
    /** Python snippet. */
    Python = 4,
    /** Description of why an operator can't run. */
    Alert = 5,
  };
  Style style;
  ColorID color_id;
  bool is_pad;
};

struct uiTooltipField {
  char *text;
  char *text_suffix;
  struct {
    /** X cursor position at the end of the last line. */
    uint x_pos;
    /** Number of lines, 1 or more with word-wrap. */
    uint lines;
  } geom;
  uiTooltipFormat format;
};

struct uiTooltipData {
  wabi::GfVec4i bbox;
  uiTooltipField *fields;
  uint fields_len;
  uiFontStyle fstyle;
  int wrap_width;
  int toth, lineh;
};

ARegion *UI_tooltip_create_from_button_or_extra_icon(kContext *C, ARegion *butregion, uiBut *but, uiButExtraOpIcon *extra_icon, bool is_label)
{
  wmWindow *win = CTX_wm_window(C);
  /* Aspect values that shrink text are likely unreadable. */
  const float aspect = min_ff(1.0f, but->block->aspect);
  float init_position[2];

  if (but->drawflag & UI_BUT_NO_TOOLTIP) {
    return nullptr;
  }
  uiTooltipData *data = nullptr;

  if (data == nullptr) {
    // data = ui_tooltip_data_from_tool(C, but, is_label);
  }

  if (data == nullptr) {
    // data = ui_tooltip_data_from_button_or_extra_icon(C, but, extra_icon);
  }

  if (data == nullptr) {
    // data = ui_tooltip_data_from_button_or_extra_icon(C, but, nullptr);
  }

  if (data == nullptr) {
    return nullptr;
  }

  const bool is_no_overlap = UI_but_has_tooltip_label(but) || UI_but_is_tool(but);
  wabi::GfVec4i init_rect;
  if (is_no_overlap) {
    wabi::GfVec4f overlap_rect_fl;
    init_position[0] = KLI_rctf_cent_x(but->rect);
    init_position[1] = KLI_rctf_cent_y(but->rect);
    if (butregion) {
      ui_block_to_window_fl(butregion, but->block, &init_position[0], &init_position[1]);
      ui_block_to_window_rctf(butregion, but->block, &overlap_rect_fl, &but->rect);
    }
    else {
      overlap_rect_fl = but->rect;
    }
    KLI_rcti_rctf_copy_round(&init_rect, overlap_rect_fl);
  }
  else {
    init_position[0] = KLI_rctf_cent_x(but->rect);
    init_position[1] = but->rect.ymin;
    if (butregion) {
      ui_block_to_window_fl(butregion, but->block, &init_position[0], &init_position[1]);
      init_position[0] = win->eventstate->xy[0];
    }
    init_position[1] -= (UI_POPUP_MARGIN / 2);
  }

  ARegion *region = ui_tooltip_create_with_data(
      C, data, init_position, is_no_overlap ? &init_rect : nullptr, aspect);

  return region;
}

void UI_tooltip_free(kContext *C, kScreen *screen, ARegion *region)
{
  ui_region_temp_remove(C, screen, region);
}

KRAKEN_NAMESPACE_END