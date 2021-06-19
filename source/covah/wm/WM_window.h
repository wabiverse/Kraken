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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#pragma once

#include "WM_api.h"

#include "CKE_context.h"

WABI_NAMESPACE_BEGIN

typedef void (*wmGenericUserDataFreeFn)(void *data);

struct wmGenericUserData
{
  void *data;
  wmGenericUserDataFreeFn free_fn;
  bool use_free;
};

typedef void (*wmGenericCallbackFn)(const cContext &C, void *user_data);

struct wmGenericCallback
{
  wmGenericCallbackFn exec;
  void *user_data;
  wmGenericUserDataFreeFn free_user_data;
};

wmWindow WM_window_open(const cContext &C,
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

void WM_anchor_init(cContext C);
void WM_anchor_exit(void);
void WM_window_process_events(const cContext &C);
void WM_window_swap_buffers(wmWindow win);

/** Poll Callback, Context Checks. */
bool WM_operator_winactive(const cContext &C);

/** Cleanup. */
void wm_exit_schedule_delayed(const cContext &C);


/** Window Operators */
int wm_window_close_exec(const cContext &C, UsdAttribute *op);
int wm_window_fullscreen_toggle_exec(const cContext &C, UsdAttribute *op);
void wm_quit_with_optional_confirmation_prompt(const cContext &C, const wmWindow &win) ATTR_NONNULL();

int wm_window_new_exec(const cContext &C, UsdAttribute *op);
int wm_window_new_main_exec(const cContext &C, UsdAttribute *op);


void WM_OT_window_close(wmOperatorType *ot);
void WM_OT_window_new(wmOperatorType *ot);
void WM_OT_window_new_main(wmOperatorType *ot);
void WM_OT_window_fullscreen_toggle(wmOperatorType *ot);
void WM_OT_quit_covah(wmOperatorType *ot);

int wm_exit_covah_exec(const cContext &C, UsdAttribute *UNUSED(op));
int wm_exit_covah_invoke(const cContext &C, UsdAttribute *UNUSED(op), const wmEvent *UNUSED(event));

WABI_NAMESPACE_END