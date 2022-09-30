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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_area.h"
#include "USD_operator.h"
#include "USD_screen.h"
#include "USD_types.h"
#include "USD_object.h"
#include "USD_userpref.h"

#include "WM_window.h"

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

ARegion *ui_region_temp_add(kScreen *screen)
{
  wabi::UsdStageWeakPtr stage = screen->GetPrim().GetStage();

  ARegion *region = MEM_new<ARegion>(__func__, stage, screen, SdfPath("RegionTemp"));
  screen->regions.push_back(region);

  region->regiontype = RGN_TYPE_TEMPORARY;
  region->alignment = RGN_ALIGN_FLOAT;

  return region;
}

void UI_tooltip_free(kContext *C, kScreen *screen, ARegion *region)
{
  ui_region_temp_remove(C, screen, region);
}

KRAKEN_NAMESPACE_END