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
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_area.h"
#include "USD_context.h"
#include "USD_factory.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "WM_window.h"

#include "KLI_time.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "UI_interface.h"

KRAKEN_NAMESPACE_BEGIN

static double g_tooltip_time_closed;
double WM_tooltip_time_closed(void)
{
  return g_tooltip_time_closed;
}

void WM_tooltip_timer_clear(kContext *C, wmWindow *win)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  kScreen *screen = WM_window_get_active_screen(win);
  if (screen->tool_tip != NULL) {
    if (screen->tool_tip->timer != NULL) {
      WM_event_remove_timer(wm, win, screen->tool_tip->timer);
      screen->tool_tip->timer = NULL;
    }
  }
}

void WM_tooltip_clear(kContext *C, wmWindow *win)
{
  WM_tooltip_timer_clear(C, win);
  kScreen *screen = WM_window_get_active_screen(win);
  if (screen->tool_tip != NULL) {
    if (screen->tool_tip->region) {
      UI_tooltip_free(C, screen, screen->tool_tip->region);
      screen->tool_tip->region = NULL;
      g_tooltip_time_closed = PIL_check_seconds_timer();
    }
    delete screen->tool_tip;
    screen->tool_tip = NULL;
  }
}

void WM_tooltip_timer_init_ex(kContext *C, wmWindow *win, ScrArea *area, ARegion *region, wmTooltipInitFn init, double delay)
{
  WM_tooltip_timer_clear(C, win);

  kScreen *screen = WM_window_get_active_screen(win);
  wmWindowManager *wm = CTX_wm_manager(C);
  if (screen->tool_tip == NULL) {
    screen->tool_tip = new wmTooltipState();
  }
  screen->tool_tip->area_from = area;
  screen->tool_tip->region_from = region;
  screen->tool_tip->timer = WM_event_add_timer(wm, win, TIMER, delay);
  screen->tool_tip->init = init;
}

void WM_tooltip_timer_init(kContext *C, wmWindow *win, ScrArea *area, ARegion *region, wmTooltipInitFn init)
{
  WM_tooltip_timer_init_ex(C, win, area, region, init, UI_TOOLTIP_DELAY);
}

KRAKEN_NAMESPACE_END