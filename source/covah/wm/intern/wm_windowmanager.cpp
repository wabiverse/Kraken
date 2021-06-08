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

#include "ANCHOR_api.h"

#include "WM_init_exit.h"
#include "WM_windowmanager.h"

#include "CKE_main.h"
#include "CKE_space.h"

#include "CLI_icons.h"

#include "UNI_context.h"

#include "ED_code.h"
#include "ED_view3d.h"

WABI_NAMESPACE_USING

wmWindowManager::wmWindowManager()
{}

wmWindowManager::~wmWindowManager()
{}

void wmWindowManager::DisplaySpaceWindows()
{
  static wmWindowFlags flags;

  flags |= SPACE_WINDOW_VIEW3D;
  flags |= SPACE_WINDOW_CODE;
  flags |= SPACE_WINDOW_SPLASH;

  if (flags & SPACE_WINDOW_VIEW3D) {
    static bool reset = true;

    ED_view3d_init_engine(UNI_stage_root(), reset);
    ED_view3d_run();
  }

  if (flags & SPACE_WINDOW_CODE) {
    ED_code_run();
  }

  if (flags & SPACE_WINDOW_SPLASH) {
  }
}
