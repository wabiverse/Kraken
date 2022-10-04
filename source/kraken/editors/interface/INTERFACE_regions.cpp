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
#include "USD_wm_types.h"

#include "WM_draw.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KKE_context.h"
#include "KKE_screen.h"

#include "interface_intern.h"
#include "interface_regions_intern.h"

void ui_region_temp_remove(kContext *C, kScreen *screen, ARegion *region)
{
  wmWindow *win = CTX_wm_window(C);

  KLI_assert(region->regiontype == RGN_TYPE_TEMPORARY);
  KLI_assert(std::find(screen->regions.begin(), screen->regions.end(), region) !=
             screen->regions.end());
  if (win) {
    WM_draw_region_clear(win, region);
  }

  ED_region_exit(C, region);

  /* nullptr: no space-type. */
  KKE_area_region_free(nullptr, region);

  screen->regions.erase(std::remove(screen->regions.begin(), screen->regions.end(), region),
                        screen->regions.end());
  delete region;
}
