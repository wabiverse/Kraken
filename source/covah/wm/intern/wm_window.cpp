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
#include "ANCHOR_event_consumer.h"
#include "ANCHOR_system.h"

#include "CKE_context.h"

#include "CLI_time.h"

/* handle to anchor system. */
static ANCHOR_SystemHandle anchor_system;

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

  if (!anchor_system) {
    /** The only raw ANCHOR_xxx calls */
    anchor_system = ANCHOR_CreateSystem();
  }

  if (C != NULL) {
    /**
     * ANCHOR:: access from here on out. */
    ANCHOR::AddEventConsumer(anchor_system, consumer);
  }
}

void WM_window_process_events(const cContext *C)
{
  bool has_event = ANCHOR::ProcessEvents(anchor_system, false);

  if (has_event) {
    ANCHOR::DispatchEvents(anchor_system);
  }

  if ((has_event == false)) {
    printf("Quick sleep: No Events on Stack\n");
    PIL_sleep_ms(5);
  }
}

void WM_window_swap_buffers(wmWindow *win)
{
  ANCHOR::SwapChain((ANCHOR_SystemWindowHandle)win->anchorwin);
}