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
#include "CKE_main.h"

#include "CLI_icons.h"
#include "CLI_math_inline.h"
#include "CLI_time.h"

#include <wabi/base/gf/vec2f.h>

WABI_NAMESPACE_USING

/* handle to anchor system. */
static ANCHOR_SystemHandle anchor_system;

/**
 * This is called by anchor, and this is where
 * we handle events for windows or send them to
 * the event system. */
static int anchor_event_proc(ANCHOR_EventHandle evt, ANCHOR_UserPtr C_void_ptr)
{
  cContext *C = (cContext *)C_void_ptr;
  wmWindowManager *wm = CTX_wm_manager(C);
  eAnchorEventType type = ANCHOR::GetEventType(evt);

  if (type == ANCHOR_EventTypeQuitRequest) {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    wmWindow *win;
    if (anchorwin && ANCHOR::ValidWindow(anchor_system, anchorwin)) {
      win = (wmWindow *)ANCHOR::GetWindowUserData(anchorwin);
    }
  }
  else {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    ANCHOR_EventDataPtr data = ANCHOR::GetEventData(evt);
  }

  return COVAH_SUCCESS;
}

static void wm_window_set_dpi(const wmWindow *win)
{
  float uscale;
  win->scale.Get(&uscale);

  float ulinewidth;
  win->linewidth.Get(&ulinewidth);

  float auto_dpi = ANCHOR::GetDPIHint((ANCHOR_SystemWindowHandle)win->anchorwin);

  auto_dpi = max_ff(auto_dpi, 96.0f);
  auto_dpi *= ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win->anchorwin);
  int dpi = auto_dpi * uscale * (72.0 / 96.0f);

  int pixelsize = max_ii(1, (int)(dpi / 64));
  pixelsize = max_ii(1, pixelsize + ulinewidth);

  float dpiadj = dpi / pixelsize;
  float dpifac = (pixelsize * (float)(dpiadj)) / 72.0f;
  float wunit = (pixelsize * (dpiadj / pixelsize) * 20 + 36) / 72;

  /* ----- */

  /**
   * Set prefs on
   * Pixar Stage. */

  win->pixelsz.Set(pixelsize);
  win->dpi.Set(dpiadj);
  win->dpifac.Set(dpifac);
  win->widgetunit.Set(wunit += 2 * ((int)pixelsize - (int)dpifac));

  /* ----- */

  /* update font drawing */
  ANCHOR::GetIO().FontGlobalScale = pixelsize * dpiadj;
}

static void wm_window_anchorwindow_add(wmWindowManager *wm, wmWindow *win, bool is_dialog)
{

  /* ----- */

  /**
   * This comes direct
   * from Pixar Stage. */

  TfToken title;
  win->title.Get(&title);

  SdfAssetPath icon;
  win->icon.Get(&icon);

  GfVec2f pos;
  win->pos.Get(&pos);

  GfVec2f size;
  win->size.Get(&size);

  /* ----- */

  ANCHOR_SystemWindowHandle anchorwin = ANCHOR::CreateWindow(anchor_system,
                                                             NULL,
                                                             title.GetText(),
                                                             icon.GetAssetPath().c_str(),
                                                             pos[0],
                                                             pos[1],
                                                             size[0],
                                                             size[1],
                                                             ANCHOR_WindowStateNormal,
                                                             is_dialog,
                                                             ANCHOR_DrawingContextTypeVulkan,
                                                             0);
  if (anchorwin) {
    win->anchorwin = anchorwin;
  }
}

static void wm_window_anchorwindow_ensure(wmWindowManager *wm, wmWindow *win, bool is_dialog)
{
  if (win->anchorwin == NULL) {

    /* ----- */

    /**
     * This comes direct
     * from Pixar Stage. */

    GfVec2f pos;
    win->pos.Get(&pos);

    GfVec2f size;
    win->size.Get(&size);

    TfToken title;
    win->title.Get(&title);

    SdfAssetPath icon;
    win->icon.Get(&icon);

    TfToken cursor;
    win->cursor.Get(&cursor);

    /* ----- */

    if ((size[0] == 0)) {
      win->pos.Set(GfVec2f(0, 0));
      win->size.Set(GfVec2f(1920, 1080));

      if (cursor.IsEmpty()) {
        win->cursor.Set(UsdUITokens->default_);
      }

      if (title.IsEmpty()) {
        win->title.Set("Covah");
      }

      if (icon.GetAssetPath().empty()) {
        win->icon.Set(CLI_icon(ICON_COVAH));
      }

      wm_window_anchorwindow_add(wm, win, is_dialog);
      wm_window_set_dpi(win);
    }
  }
}

void WM_anchor_init(cContext *C)
{
  /* Event handle of anchor stack. */
  ANCHOR_EventConsumerHandle consumer;

  if (C != NULL) {
    consumer = ANCHOR_CreateEventConsumer(anchor_event_proc, C);
  }

  if (!anchor_system) {
    anchor_system = ANCHOR_CreateSystem();
  }

  if (C != NULL) {
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