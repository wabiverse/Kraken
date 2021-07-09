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

#include "KKE_context.h"
#include "KKE_workspace.h"

#include "WM_draw.h"
#include "WM_event_system.h"
#include "WM_tokens.h"
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
  /**
   * Create a new WindowManager. */
  wmWindowManager *wm = new wmWindowManager();
  CTX_wm_manager_set(C, wm);

  /**
   * Create a new 'Layout' Workspace. */
  WorkSpace *ws = ED_workspace_add(C, "Layout");
  cmain->workspaces.push_back(ws);

  /**
   * Create a new MainWindow. */
  wmWindow *win = wm_window_new(C, wm, NULL, false);

  /**
   * Create a new Screen and assign the
   * new 'Layout' Workspace to it. */
  cScreen *screen = ED_workspace_layout_add(C, ws, win, "Layout")->screen;

  /**
   * This workspace should effectively point back the workspace
   * created above, but globally searched to ensure it is not a
   * duplicate. */
  WorkSpace *workspace;
  WorkSpaceLayout *layout = KKE_workspace_layout_find_global(cmain, screen, &workspace);

  /**
   * Set this workspace as active, additionally creating
   * it's accompanying workspace layout, so that this
   * workspace datablock will present a valid default gui
   * layout upon startup. */
  KKE_workspace_active_set(win->workspace_hook, workspace);
  KKE_workspace_active_layout_set(win->workspace_hook, win->winid, workspace, layout);
  screen->winid = win->winid;

  /**
   * Create default user preferences. */
  UserDef *uprefs = new UserDef(C);
  CTX_data_prefs_set(C, uprefs);
  UNI_default_table_user_prefs(C);

  /**
   * Create default window, some of
   * these fields may be incorrect,
   * ##WM_check() -> below, will
   * ensure anything incorrect,
   * gets corrected. */
  CTX_wm_window_set(C, win);
  UNI_default_table_main_window(C);

  /**
   * Create default cube scene. */
  UNI_default_table_scene_data(C);

  /* ----- */

  WM_check(C);

  /**
   * Now that window is properly initialized,
   * activate it, and signify that the changes
   * have been saved ##WM_window_anchorwindows_ensure(),
   * called within the ##WM_check() seen above,
   * verifies a title, icon, and various other
   * required fields are properly setup and are
   * not empty. Additionally, it allocates a GPU
   * surface, backend window, as well as backend
   * system to handle event processing and GUI
   * display. */

  wm->winactive = win;
  wm->file_saved = true;

  /* ----- */

  /**
   * Save DPI factor, which ANCHOR properly
   * sets up from the system backend, back
   * to the Pixar Stage. */
  FormFactory(uprefs->dpifac, UI_DPI_FAC);

  /**
   * Add window's workspace relationship, by adding
   * the workspace created above as a target to our
   * window's workspace relationships. */
  FormFactory(win->workspace_rel, workspace->path);
  FormFactory(workspace->screen_rel, screen->path);

  /**
   * Save defaults, all should now be
   * properly setup, and verfied for
   * correctness, this is now a valid
   * startup Kraken project file. */
  UNI_save_stage(C);
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
  WM_window_anchorwindows_ensure(wm);
  // }

  /* Case: fileread. */
  /* Note: this runs in background mode to set the screen context cb. */
  // if ((wm->initialized & WM_WINDOW_IS_INIT) == 0) {
  //   ED_screens_init(cmain, wm);
  //   wm->initialized |= WM_WINDOW_IS_INIT;
  // }
}

WABI_NAMESPACE_END