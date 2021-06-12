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
#include "WM_windowmanager.h"

#include "ANCHOR_api.h"

#include "UNI_context.h"

#include "CLI_icons.h"

#include "CKE_main.h"

#include "ED_debug_codes.h"

#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_USING

static void ShowFileMenu()
{
  if (ANCHOR::BeginMenu("File")) {
    if (ANCHOR::BeginMenu("New")) {
      ANCHOR::MenuItem("General", "Ctrl+N");
      ANCHOR::MenuItem("2D Animation");
      ANCHOR::MenuItem("Sculpting");
      ANCHOR::MenuItem("VFX");
      ANCHOR::MenuItem("Video Editing");
      ANCHOR::EndMenu();
    }
    if (ANCHOR::MenuItem("Open...", "Ctrl+O")) {
    }
    if (ANCHOR::BeginMenu("Open Recent")) {
      ANCHOR::MenuItem("Cigarettes");
      ANCHOR::MenuItem("Glasses");
      ANCHOR::MenuItem("Cardigans");
      if (ANCHOR::BeginMenu("More...")) {
        ANCHOR::MenuItem("Fedoras");
        ANCHOR::MenuItem("Cocktails");
        ANCHOR::EndMenu();
      }
      ANCHOR::EndMenu();
    }
    if (ANCHOR::MenuItem("Save", "Ctrl+S")) {
    }
    if (ANCHOR::MenuItem("Save As...", "Shift+Ctrl+S")) {
    }
    if (ANCHOR::MenuItem("Quit", "Ctrl+Q")) {
      // CKE_has_kill_signal(COVAH_SUCCESS);
    }
    ANCHOR::EndMenu();
  }
}

static void ShowMainMenuBar()
{
  if (ANCHOR::BeginMainMenuBar()) {
    ShowFileMenu();
    ANCHOR::EndMainMenuBar();
  }
}

void WM_covah_runtime(wmWindowManager *manager)
{
  static std::once_flag setupStyles;
  std::call_once(setupStyles, []() { WM_init_default_styles(); });

  ShowMainMenuBar();
  manager->DisplaySpaceWindows();
}
