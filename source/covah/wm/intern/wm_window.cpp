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

#include "WM_window.h"

#include "UNI_window.h"

#include "ANCHOR_api.h"

#include "CKE_context.h"

typedef std::pair<ANCHOR_SystemHandle *, ANCHOR_SurfaceHandle *> AnchorSysGPU;

/* handle to anchor system. */
static AnchorSysGPU anchor_backend;

/**
 * This is called by anchor, and this is where
 * we handle events for windows or send them to
 * the event system. */
static int anchor_event_proc(ANCHOR_EventHandle event, ANCHOR_UserPtr C_context)
{}

void WM_anchor_init(cContext *C)
{
  /* Event handle of anchor stack. */
  ANCHOR_EventConsumerHandle consumer;

  if (C != NULL) {
    /** The only raw ANCHOR_xxx calls */
    consumer = ANCHOR_CreateEventConsumer(anchor_event_proc, C);
  }

  if (!anchor_backend.first) {
    /** The only raw ANCHOR_xxx calls */
    anchor_backend = ANCHOR_CreateSystem(ANCHOR_SDL | ANCHOR_VULKAN);
  }

  if (C != NULL) {
    /**
     * ANCHOR:: access from here on out. */
    ANCHOR::AddEventConsumer(consumer);
  }
}

void WM_window_process_events(cContext *C)
{
  bool has_event = ANCHOR::ProcessEvents(anchor_backend.first, anchor_backend.second);

  if (has_event) {
    // ANCHOR::DispatchEvents();
  }
}

void WM_window_swap_buffers(wmWindow *win)
{
  /**
   * TODO: Once we implement GPU Library. */
  TF_UNUSED(win);

  ANCHOR::SwapChain(anchor_backend.second);
}