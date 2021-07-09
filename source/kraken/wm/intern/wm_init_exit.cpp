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

#include "WM_init_exit.h" /* Own include. */
#include "WM_cursors.h"
#include "WM_event_system.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_tokens.h"
#include "WM_window.h"

#include "ANCHOR_api.h"
#include "ANCHOR_system_paths.h"

#include "UNI_context.h"

#include "KLI_icons.h"

#include "KKE_context.h"
#include "KKE_main.h"

#include "ED_debug_codes.h"

#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_BEGIN


void WM_init(kContext *C, int argc, const char **argv)
{
  WM_anchor_init(C);
  WM_init_cursor_data();

  WM_operators_init(C);
  WM_operators_register(C);

  Main *kmain = CTX_data_main(C);
  wm_add_default(kmain, C);
}


WABI_NAMESPACE_END