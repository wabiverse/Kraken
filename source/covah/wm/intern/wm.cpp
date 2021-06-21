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

#include "CKE_context.h"

#include "WM_draw.h"
#include "WM_window.h"

WABI_NAMESPACE_BEGIN

void WM_main(const cContext &C)
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

void WM_check(const cContext &C)
{
  Main cmain = CTX_data_main(C);
  wmWindowManager wm = CTX_wm_manager(C);

  /* WM context. */
  if (wm == NULL)
  {
    wm = TfCreateRefPtr(new CovahWindowManager());
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
  //   ED_screens_init(bmain, wm);
  //   wm->initialized |= WM_WINDOW_IS_INIT;
  // }
}

WABI_NAMESPACE_END