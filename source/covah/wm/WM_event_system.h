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

WABI_NAMESPACE_BEGIN

void WM_event_add_anchorevent(const wmWindowManager &wm, const wmWindow &win, int type, void *customdata);

int WM_operator_name_call(const cContext &C, const TfToken &optoken, short context, UsdAttributeVector *properties);

void WM_event_do_refresh_wm(const cContext &C);

WABI_NAMESPACE_END