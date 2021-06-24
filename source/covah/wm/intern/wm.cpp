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

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_default_tables.h"
#include "UNI_factory.h"
#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_pixar_utils.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

#include "CKE_context.h"
#include "CKE_workspace.h"

#include "WM_draw.h"
#include "WM_window.h"

#include "ED_screen.h"

WABI_NAMESPACE_BEGIN

void WM_main(cContext *C)
{
  while (1)
  {

    /**
     * Process events from anchor, handle window events. */
    WM_window_process_events(C);

    /**
     * Per window, all events to the window, screen, area and region handlers. */
    // wm_event_do_handlers(C);

    /**
     *  Events have left notes about changes, we handle and cache it. */
    // wm_event_do_notifiers(C);

    /**
     *  Execute cached changes and draw. */
    WM_draw_update(C);
  }
}


void wm_add_default(Main *cmain, cContext *C)
{
  wmWindowManager *wm = new wmWindowManager();
  CTX_wm_manager_set(C, wm);

  WorkSpace *ws = ED_workspace_add(C, "Layout");
  cmain->workspaces.push_back(ws);

  wmWindow *win = wm_window_new(C, wm, NULL, false);
  cScreen *screen = ED_workspace_layout_add(C, ws, win, "Layout")->screen;

  WorkSpace *workspace;
  WorkSpaceLayout *layout = CKE_workspace_layout_find_global(cmain, screen, &workspace);
  CKE_workspace_active_set(win->workspace_hook, workspace);
  CKE_workspace_active_layout_set(win->workspace_hook, win->winid, workspace, layout);
  screen->winid = win->winid;

  UserDef *uprefs = new UserDef(C);

  CTX_wm_window_set(C, win);
  CTX_data_prefs_set(C, uprefs);

  UNI_default_table_main_window(C);
  UNI_default_table_user_prefs(C);
  UNI_default_table_scene_data(C);

  WM_check(C);

  wm->winactive = win;
  wm->file_saved = true;
  wm_window_make_drawable(C, wm, win);
}


void WM_check(cContext *C)
{
  Main *cmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  /* WM context. */
  if (wm == NULL)
  {
    wm = new wmWindowManager();
    CTX_wm_manager_set(C, wm);
  }

  if (wm == NULL || wm->windows.empty())
  {
    return;
  }

  /* Run before loading the keyconfig. */
  // if (wm->message_bus == NULL) {
  //   wm->message_bus = WM_msgbus_create();
  // }

  // if (!G.background) {
  /* Case: fileread. */
  // if ((wm->initialized & WM_WINDOW_IS_INIT) == 0) {
  //   WM_keyconfig_init(C);
  //   WM_autosave_init(wm);
  // }

  /* Case: no open windows at all, for old file reads. */
  WM_window_anchorwindows_ensure(C, wm);
  // }

  /* Case: fileread. */
  /* Note: this runs in background mode to set the screen context cb. */
  // if ((wm->initialized & WM_WINDOW_IS_INIT) == 0) {
  //   ED_screens_init(cmain, wm);
  //   wm->initialized |= WM_WINDOW_IS_INIT;
  // }
}

WABI_NAMESPACE_END