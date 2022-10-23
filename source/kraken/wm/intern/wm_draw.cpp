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
 * @file\
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_area.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_window.h"

#include "MEM_guardedalloc.h"

#include "KLI_kraklib.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"
#include "KKE_scene.h"
#include "KKE_screen.h"

#include "ANCHOR_api.h"

// #include "ED_node.h"
#include "ED_screen.h"
#include "ED_view3d.h"

#include "GPU_batch_presets.h"
#include "GPU_context.h"
#include "GPU_debug.h"
#include "GPU_framebuffer.h"
#include "GPU_immediate.h"
#include "GPU_matrix.h"
#include "GPU_state.h"
#include "GPU_texture.h"
#include "GPU_viewport.h"

#include "WM_draw.h"
#include "wm_event_system.h"
#include "WM_window.h"
#include "WM_window.hh"

#include "UI_resources.h"

/* -------------------------------------------------------------------- */
/** \name Main Update Call
 * \{ */

/* quick test to prevent changing window drawable */
static bool wm_draw_update_test_window(Main *kmain, kContext *C, wmWindow *win)
{
  const wmWindowManager *wm = CTX_wm_manager(C);
  kScene *scene = WM_window_get_active_scene(win);
  // ViewLayer *view_layer = WM_window_get_active_view_layer(win);
  // struct Hydra *depsgraph = KKE_scene_ensure_hydra(kmain, scene, view_layer);
  kScreen *screen = WM_window_get_active_screen(win);
  bool do_draw = false;

  LISTBASE_FOREACH(ARegion *, region, &screen->regions)
  {
    if (region->do_draw_paintcursor) {
      screen->do_draw_paintcursor = true;
      region->do_draw_paintcursor = false;
    }
    if (region->visible && region->do_draw) {
      do_draw = true;
    }
  }

  ED_screen_areas_iter(win, screen, area)
  {
    LISTBASE_FOREACH(ARegion *, region, &area->regions)
    {
      // wm_region_test_gizmo_do_draw(C, area, region, true);
      // wm_region_test_render_do_draw(scene, hydra, area, region);
      // #ifdef WITH_XR_OPENXR
      //       wm_region_test_xr_do_draw(wm, area, region);
      // #endif

      if (region->visible && region->do_draw) {
        do_draw = true;
      }
    }
  }

  if (do_draw) {
    return true;
  }

  if (screen->do_refresh) {
    return true;
  }
  if (screen->do_draw) {
    return true;
  }
  if (screen->do_draw_gesture) {
    return true;
  }
  if (screen->do_draw_paintcursor) {
    return true;
  }
  if (screen->do_draw_drag) {
    return true;
  }

  // if (wm_software_cursor_needed()) {
  //   struct GrabState grab_state;
  //   if (wm_software_cursor_needed_for_window(win, &grab_state)) {
  //     if (wm_software_cursor_motion_test(win)) {
  //       return true;
  //     }
  //   }
  //   else {
  //     /* Detect the edge case when the previous draw used the software cursor but this one
  //     doesn't,
  //      * it's important to redraw otherwise the software cursor will remain displayed. */
  //     if (g_software_cursor.winid != -1) {
  //       return true;
  //     }
  //   }
  // }

  // #ifndef WITH_XR_OPENXR
  //   UNUSED_VARS(wm);
  // #endif

  return false;
}


void WM_draw_update(kContext *C)
{
  Main *kmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  GPU_context_main_lock();

  GPU_render_begin();
  GPU_render_step();

  // KKE_image_free_unused_gpu_textures();

  LISTBASE_FOREACH(wmWindow *, win, &wm->windows)
  {
#ifdef WIN32
    eAnchorWindowState state = ANCHOR::GetWindowState(win->anchorwin);

    if (state == AnchorWindowStateMinimized) {
      /* do not update minimized windows, gives issues on Intel (see T33223)
       * and AMD (see T50856). it seems logical to skip update for invisible
       * window anyway.
       */
      continue;
    }
#endif
    CTX_wm_window_set(C, win);

    if (wm_draw_update_test_window(kmain, C, win)) {
      kScreen *screen = WM_window_get_active_screen(win);

      /* sets context window+screen */
      WM_window_make_drawable(wm, win);

      /* notifiers for screen redraw */
      ED_screen_ensure_updated(wm, win, screen);

      // wm_draw_window(C, win);
      // wm_draw_update_clear_window(C, win);

      WM_window_swap_buffers(win);
    }
  }

  CTX_wm_window_set(C, NULL);

  /* Draw non-windows (surfaces) */
  // wm_surfaces_iter(C, wm_draw_surface);

  GPU_render_end();
  GPU_context_main_unlock();
}

void WM_draw_region_clear(wmWindow *win, ARegion *UNUSED(region))
{
  kScreen *screen = WM_window_get_active_screen(win);
  screen->do_draw = true;
}
