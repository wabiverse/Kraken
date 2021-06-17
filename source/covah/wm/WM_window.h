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

wmWindow WM_window_open(cContext &C,
                        const char *title,
                        const char *icon,
                        int x,
                        int y,
                        int sizex,
                        int sizey,
                        int space_type,
                        bool dialog,
                        bool temp);

void WM_anchor_init(cContext &C);
void WM_anchor_exit(void);
void WM_window_process_events(const cContext C);
void WM_window_swap_buffers(wmWindow win);

WABI_NAMESPACE_END