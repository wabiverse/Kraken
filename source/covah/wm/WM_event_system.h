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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

#include <wabi/usd/usd/attribute.h>
#include <wabi/wabi.h>

#include "CKE_context.h"

#include "UNI_object.h"

#include "WM_operators.h"

WABI_NAMESPACE_BEGIN

struct wmEvent;

void WM_event_add_anchorevent(wmWindowManager *wm, wmWindow *win, int type, void *customdata);
wmEvent *wm_event_add(wmWindow *win, wmEvent *event_to_add);
void WM_main_add_notifier(cContext *C, unsigned int type, void *reference);
void WM_event_add_notifier_ex(wmWindowManager *wm, wmWindow *win, uint type, void *reference);
void WM_event_add_notifier(cContext *C, uint type, void *reference);

int WM_operator_name_call_ptr(cContext *C,
                              wmOperatorType *ot,
                              short context,
                              PointerUNI *properties);

void WM_event_init_from_window(wmWindow *win, wmEvent *event);

int WM_operator_name_call(cContext *C, const TfToken &optoken, short context, PointerUNI *properties);

void WM_event_do_refresh_wm(cContext *C);

WABI_NAMESPACE_END