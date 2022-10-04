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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_wm_types.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_default_tables.h"
#include "USD_factory.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_userdef_types.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "KLI_string.h"

#include "KKE_context.h"
#include "KKE_workspace.h"

#include "WM_draw.h"
#include "WM_event_system.h"
#include "WM_tokens.h"
#include "WM_window.h"

#include "ED_screen.h"



void WM_main(kContext *C)
{
  /**
   * Single refresh before handling events. */
  WM_event_do_refresh_wm(C);

  while (1) {

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


void WM_init_manager(kContext *C)
{
  wmWindowManager *wm = new wmWindowManager();
  CTX_wm_manager_set(C, wm);
}


void wm_add_default(Main *kmain, kContext *C)
{
  wmWindow *win;
  wmWindowManager *wm = CTX_wm_manager(C);
  kScreen *screen = CTX_wm_screen(C);
  WorkSpace *workspace;
  WorkSpaceLayout *layout = KKE_workspace_layout_find_global(kmain, screen, &workspace);

  win = wm_window_new(C, wm, NULL, false);
  win->scene = CTX_data_scene(C);

  if (!workspace) {
    workspace = new WorkSpace(
      C,
      SdfPath(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES)).AppendPath(SdfPath("Layout")));

    if (!layout) {
      layout = new WorkSpaceLayout();
      layout->name = TfToken("Layout");
    }

    if (!screen) {
      screen = new kScreen(C, workspace->path.AppendPath(SdfPath("Screen")));
    }

    layout->screen = screen;
  }
  KKE_workspace_active_set(win->workspace_hook, workspace);
  KKE_workspace_active_layout_set(win->workspace_hook, win->winid, workspace, layout);
  screen->winid = win->winid;

  wm->winactive = win;
  wm->file_saved = 1;

  /* ----- */

  CTX_wm_window_set(C, win);
  USD_default_table_main_window(C);

  /**
   * Now that window is properly initialized,
   * activate it, and signify that the changes
   * have been saved ##WM_window_anchorwindows_ensure(),
   * called within the ##WM_check() seen below,
   * verifies a title, icon, and various other
   * required fields are properly setup and are
   * not empty. Additionally, it allocates a GPU
   * surface, backend window, as well as backend
   * system to handle event processing and GUI
   * display. */

  WM_check(C);

  /* ----- */

  wm_window_make_drawable(wm, win);

  /**
   * Create default user preferences. */
  kUserDef *uprefs = new kUserDef(C);
  CTX_data_prefs_set(C, uprefs);
  USD_default_table_user_prefs(C);

  /**
   * Create default cube scene. */
  USD_default_table_scene_data(C);

  /**
   * Save DPI factor, which ANCHOR properly
   * sets up from the system backend, back
   * to the Pixar Stage. */
  FormFactory(uprefs->dpifac, U.dpi_fac);

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
  USD_save_stage(C);
}


void WM_check(kContext *C)
{
  Main *kmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  /* WM context. */
  if (wm == NULL) {
    wm = new wmWindowManager();
    CTX_wm_manager_set(C, wm);
  }

  if (wm == NULL || wm->windows.empty()) {
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
  //   ED_screens_init(kmain, wm);
  //   wm->initialized |= WM_WINDOW_IS_INIT;
  // }
}

/* wait until every job ended */
void WM_jobs_kill_all(wmWindowManager *wm) {}

