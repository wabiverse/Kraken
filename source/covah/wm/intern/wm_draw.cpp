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
 * @file\
 * Window Manager.
 * Making GUI Fly.
 */

#include "UNI_area.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_window.h"

#include "CKE_context.h"

#include "WM_draw.h"
#include "WM_window.h"

WABI_NAMESPACE_BEGIN

void WM_draw_update(const cContext &C)
{
  Main cmain = CTX_data_main(C);
  wmWindowManager wm = CTX_wm_manager(C);

  TF_FOR_ALL (win, wm->windows)
  {
    CTX_wm_window_set(C, win->second);
    WM_window_swap_buffers(win->second);
  }
}

WABI_NAMESPACE_END