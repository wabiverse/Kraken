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
 * @file\
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_area.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_window.h"

#include "KKE_context.h"

#include "WM_draw.h"
#include "WM_window.hh"



void WM_draw_update(kContext *C)
{
  Main *kmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  for (auto &win : wm->windows) {
    CTX_wm_window_set(C, VALUE(win));
    WM_window_swap_buffers(VALUE(win));
  }
}

void wm_draw_region_clear(wmWindow *win, ARegion *UNUSED(region))
{
  kScreen *screen = WM_window_get_active_screen(win);
  screen->do_draw = true;
}


