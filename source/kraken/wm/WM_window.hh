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

#pragma once

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_wm_types.h"
#include "USD_workspace.h"
#include "USD_vec_types.h"

#include "WM_api.h"

#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_tooltip.h"

#include "KKE_context.h"

#include <wabi/base/gf/rect2i.h>


typedef void (*wmGenericUserDataFreeFn)(void *data);

struct wmGenericUserData
{
  void *data;
  wmGenericUserDataFreeFn free_fn;
  bool use_free;
};

typedef void (*wmGenericCallbackFn)(kContext *C, void *user_data);

struct wmGenericCallback
{
  wmGenericCallbackFn exec;
  void *user_data;
  wmGenericUserDataFreeFn free_user_data;
};

wmGenericCallback *WM_generic_callback_steal(wmGenericCallback *callback);
void WM_generic_callback_free(wmGenericCallback *callback);

wmWindow *wm_window_new(kContext *C, wmWindowManager *wm, wmWindow *parent, bool dialog);
wmWindow *WM_window_open(kContext *C,
                         const char *title,
                         const char *icon,
                         int x,
                         int y,
                         int sizex,
                         int sizey,
                         TfToken space_type,
                         TfToken alignment,
                         bool dialog,
                         bool temp);
void wm_window_close(kContext *C, wmWindowManager *wm, wmWindow *win);
void WM_window_make_drawable(wmWindowManager *wm, wmWindow *win);
bool WM_window_is_temp_screen(const wmWindow *win);

void WM_anchor_init(kContext *C);
void WM_anchor_exit(void);

void WM_tooltip_timer_init(kContext *C,
                           wmWindow *win,
                           ScrArea *area,
                           ARegion *region,
                           wmTooltipInitFn init);
void WM_tooltip_timer_init_ex(kContext *C,
                              wmWindow *win,
                              ScrArea *area,
                              ARegion *region,
                              wmTooltipInitFn init,
                              double delay);

struct PanelType *WM_paneltype_find(const char *idname, bool quiet);

void WM_window_process_events(kContext *C);
void WM_window_swap_buffers(wmWindow *win);

void WM_window_anchorwindows_remove_invalid(kContext *C, wmWindowManager *wm);
void WM_window_anchorwindows_ensure(wmWindowManager *wm);

wmTimer *WM_event_add_timer(wmWindowManager *wm, wmWindow *win, int event_type, double timestep);
void WM_event_remove_timer(wmWindowManager *wm, wmWindow *win, wmTimer *timer);

void WM_tooltip_clear(kContext *C, wmWindow *win);

/** Operators :: Register */
void WM_window_operators_register();

/** Utils. */
int WM_window_pixels_x(const wmWindow *win);
int WM_window_pixels_y(const wmWindow *win);
wmWindow *WM_window_find_under_cursor(wmWindow *win, const int mval[2], int r_mval[2]);
void WM_window_screen_rect_calc(const wmWindow *win, wabi::GfRect2i *r_rect);
void WM_window_rect_calc(const wmWindow *win, wabi::GfRect2i *r_rect);
void WM_cursor_position_to_anchor_screen_coords(wmWindow *win, int *x, int *y);
void WM_cursor_position_to_anchor_client_coords(wmWindow *win, int *x, int *y);
void WM_clipboard_text_set(const char *buf, bool selection);

kScene *WM_window_get_active_scene(const wmWindow *win);
WorkSpace *WM_window_get_active_workspace(const wmWindow *win);
kScreen *WM_window_get_active_screen(const wmWindow *win);
WorkSpaceLayout *WM_window_get_active_layout(const wmWindow *win);

void WM_window_set_dpi(const wmWindow *win);

void WM_init_state_size_set(int stax, int stay, int sizx, int sizy);
void WM_init_state_fullscreen_set(void);
void WM_init_state_normal_set(void);
void WM_init_state_maximized_set(void);
void WM_init_window_focus_set(bool do_it);
void WM_init_native_pixels(bool do_it);

void WM_window_set_active_layout(wmWindow *win, WorkSpace *workspace, WorkSpaceLayout *layout);

/** Cleanup. */
void WM_exit_schedule_delayed(const kContext *C);
void WM_quit_with_optional_confirmation_prompt(kContext *C, wmWindow *win);

void wmOrtho2(float x1, float x2, float y1, float y2);
static void wmOrtho2_offset(const float x, const float y, const float ofs);
void wmOrtho2_region_pixelspace(const ARegion *region);
void wmGetProjectionMatrix(float mat[4][4], const rcti *winrct);