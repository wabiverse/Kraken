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

#include "UNI_context.h"
#include "UNI_object.h"
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

WABI_NAMESPACE_BEGIN

/* handle to anchor system. */
static ANCHOR_SystemHandle anchor_system;

/**
 * This is called by anchor, and this is where
 * we handle events for windows or send them to
 * the event system. */
static int anchor_event_proc(ANCHOR_EventHandle evt, ANCHOR_UserPtr C_void_ptr)
{
  cContext C = TfCreateRefPtr((CovahContext *)C_void_ptr);
  wmWindowManager wm = CTX_wm_manager(C);
  eAnchorEventType type = ANCHOR::GetEventType(evt);

  if (type == ANCHOR_EventTypeQuitRequest) {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    wmWindow win;
    if (anchorwin && ANCHOR::ValidWindow(anchor_system, anchorwin)) {
      win = TfCreateRefPtr((CovahWindow *)ANCHOR::GetWindowUserData(anchorwin));
    }
  }
  else {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    ANCHOR_EventDataPtr data = ANCHOR::GetEventData(evt);
  }

  return COVAH_SUCCESS;
}

static void wm_window_set_dpi(const wmWindow win)
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

static void wm_window_anchorwindow_add(wmWindowManager wm, wmWindow win, bool is_dialog)
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

static void wm_window_anchorwindow_ensure(wmWindowManager wm, wmWindow win, bool is_dialog)
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

static void wm_get_screensize(int *r_width, int *r_height)
{
  unsigned int uiwidth;
  unsigned int uiheight;

  ANCHOR::GetMainDisplayDimensions(anchor_system, &uiwidth, &uiheight);
  *r_width = uiwidth;
  *r_height = uiheight;
}

/* keeps size within monitor bounds */
static void wm_window_check_size(GfVec4i *rect)
{
  int width, height;
  wm_get_screensize(&width, &height);

  int xmin = rect->GetArray()[0];
  int ymin = rect->GetArray()[1];
  int xmax = rect->GetArray()[2];
  int ymax = rect->GetArray()[3];

  int sizex = (xmax - xmin);
  int sizey = (ymax - ymin);

  if (sizex > width) {
    int centx = (xmin + xmax) / 2;
    xmin = centx - (width / 2);
    xmax = xmin + width;
  }

  if (sizey > height) {
    int centy = (ymin + ymax) / 2;
    ymin = centy - (height / 2);
    ymax = ymin + height;
  }

  rect->Set(xmin, ymin, xmax, ymax);
}

/**
 * @param space_type: SPACE_VIEW3D, SPACE_INFO, ... (eSpace_Type)
 * @param dialog: whether this should be made as a dialog-style window
 * @param temp: whether this is considered a short-lived window
 * @param alignment: how this window is positioned relative to its parent
 * @return the window or NULL in case of failure. */
wmWindow WM_window_open(cContext &C,
                        const char *title,
                        const char *icon,
                        int x,
                        int y,
                        int sizex,
                        int sizey,
                        int space_type,
                        bool dialog,
                        bool temp)
{
  Main cmain = CTX_data_main(C);
  wmWindowManager wm = CTX_wm_manager(C);
  wmWindow win_prev = CTX_wm_window(C);
  Scene scene = CTX_data_scene(C);
  GfVec4i rect;

  GfVec2f pos;
  win_prev->pos.Get(&pos);

  GfVec2f size;
  win_prev->pos.Get(&size);

  TfToken alignment;
  win_prev->pos.Get(&alignment);

  const float native_pixel_size = ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win_prev->anchorwin);
  /* convert to native OS window coordinates */
  rect[0] = pos[0] + (x / native_pixel_size);
  rect[1] = pos[1] + (y / native_pixel_size);
  sizex /= native_pixel_size;
  sizey /= native_pixel_size;

  if (alignment == UsdUITokens->alignCenter) {
    /* Window centered around x,y location. */
    rect[0] -= sizex / 2;
    rect[1] -= sizey / 2;
  }
  else if (alignment == UsdUITokens->alignParent) {
    /* Centered within parent. X,Y as offsets from there. */
    rect[0] += (size[0] - sizex) / 2;
    rect[1] += (size[1] - sizey) / 2;
  }
  else {
    /* Positioned absolutely within parent bounds. */
  }

  rect[2] = rect[0] + sizex;
  rect[3] = rect[1] + sizey;

  /* changes rect to fit within desktop */
  wm_window_check_size(&rect);

  /* ----- */

  /**
   * Create Window. */
  wmWindow win = TfCreateRefPtr(new CovahWindow(C, win_prev, SdfPath("Child")));
  wm->windows.insert(std::make_pair(win->path, win));

  /**
   * Dialogs may have a child window as parent.
   * Otherwise, a child must not be a parent too. */
  win->parent = (!dialog && win_prev && win_prev->parent) ? win_prev->parent : win_prev;

  /* ----- */

  win->pos.Set(GfVec2f(rect[0], rect[1]));
  win->size.Set(GfVec2f(rect[2] - rect[0], rect[3] - rect[1]));

  if (!win->prims.workspace->GetPrim().IsValid()) {
    win->prims.workspace = win_prev->prims.workspace;
  }

  if (!win->prims.screen->areas_rel.HasAuthoredTargets()) {
    /* add new screen layout */
  }

  CTX_wm_window_set(C, win);
  const bool new_window = (win->anchorwin == NULL);
  if (new_window) {
    wm_window_anchorwindow_ensure(wm, win, dialog);
  }

  return win;
}

void WM_anchor_init(cContext &C)
{
  /* Event handle of anchor stack. */
  ANCHOR_EventConsumerHandle consumer;

  if (C != NULL) {
    consumer = ANCHOR_CreateEventConsumer(anchor_event_proc, &C);
  }

  if (!anchor_system) {
    anchor_system = ANCHOR_CreateSystem();
  }

  if (C != NULL) {
    ANCHOR::AddEventConsumer(anchor_system, consumer);
  }
}

void WM_window_process_events(const cContext C)
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

void WM_window_swap_buffers(wmWindow win)
{
  ANCHOR::SwapChain((ANCHOR_SystemWindowHandle)win->anchorwin);
}

WABI_NAMESPACE_END