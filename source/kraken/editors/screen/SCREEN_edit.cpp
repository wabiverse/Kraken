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
 * Editors.
 * Tools for Artists.
 */

#include "USD_area.h"
#include "USD_context.h"
#include "USD_factory.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_assert.h"
#include "KLI_math_inline.h"

#include "WM_window.h"

#include "ED_defines.h"
#include "ED_screen.h"

#include <wabi/base/gf/rect2i.h>
#include <wabi/usd/usdUI/tokens.h>

WABI_NAMESPACE_BEGIN


bool ED_screen_change(kContext *C, kScreen *screen)
{
  Main *kmain = CTX_data_main(C);
  wmWindow *win = CTX_wm_window(C);
  WorkSpace *workspace = KKE_workspace_active_get(win->workspace_hook);
  WorkSpaceLayout *layout = KKE_workspace_layout_find(workspace, screen);
  kScreen *screen_old = CTX_wm_screen(C);

  /* Get the actual layout/screen to be activated (guaranteed to be unused, even if that means
   * having to duplicate an existing one). */
  WorkSpaceLayout *layout_new =
    ED_workspace_screen_change_ensure_unused_layout(kmain, workspace, layout, layout, win);
  kScreen *screen_new = KKE_workspace_layout_screen_get(layout_new);

  // screen_change_prepare(screen_old, screen_new, kmain, C, win);

  if (screen_old != screen_new) {
    // WM_window_set_active_screen(win, workspace, screen_new);
    // screen_change_update(C, win, screen_new);

    return true;
  }

  return false;
}


void ED_region_exit(kContext *C, ARegion *region)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  ARegion *prevar = CTX_wm_region(C);

  if (region->type && region->type->exit) {
    region->type->exit(wm, region);
  }

  CTX_wm_region_set(C, region);

  // WM_event_remove_handlers(C, &region->handlers);
  // WM_event_modal_handler_region_replace(win, region, NULL);
  // WM_draw_region_free(region, true);

  // if (region->headerstr) {
  //   MEM_freeN(region->headerstr);
  //   region->headerstr = NULL;
  // }

  // if (region->regiontimer) {
  //   WM_event_remove_timer(wm, win, region->regiontimer);
  //   region->regiontimer = NULL;
  // }

  // WM_msgbus_clear_by_owner(wm->message_bus, region);

  CTX_wm_region_set(C, prevar);
}


void ED_area_exit(kContext *C, ScrArea *area)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  ScrArea *prevsa = CTX_wm_area(C);

  // if (area->type && area->type->exit) {
  // area->type->exit(wm, area);
  // }

  CTX_wm_area_set(C, area);

  UNIVERSE_FOR_ALL (region, area->regions) {
    ED_region_exit(C, region);
  }

  // WM_event_remove_handlers(C, &area->handlers);
  // WM_event_modal_handler_area_replace(win, area, NULL);

  CTX_wm_area_set(C, prevsa);
}

void ED_screen_exit(kContext *C, wmWindow *window, kScreen *screen)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *prevwin = CTX_wm_window(C);

  CTX_wm_window_set(C, window);

  screen->active_region = NULL;

  UNIVERSE_FOR_ALL (region, screen->regions) {
    ED_region_exit(C, region);
  }

  UNIVERSE_FOR_ALL (area, screen->areas) {
    ED_area_exit(C, area);
  }

  UNIVERSE_FOR_ALL (area, window->global_areas.areas) {
    ED_area_exit(C, area);
  }

  /* mark it available for use for other windows */
  screen->winid = 0;

  if (!WM_window_is_temp_screen(prevwin)) {
    /* use previous window if possible */
    CTX_wm_window_set(C, prevwin);
  } else {
    /* none otherwise */
    CTX_wm_window_set(C, NULL);
  }
}

static SdfPath make_areapath(kScreen *screen, int id)
{
  return SdfPath(screen->path.GetName() + "Area" + STRINGALL(id));
}


static int find_free_areaid(kContext *C)
{
  int id = 1;

  Main *kmain = CTX_data_main(C);

  /**
   * Depending on the number of screens we have,
   * each with their own areas, we may want to
   * optimize this. But for now keeping it simple,
   * and safe for ensuring id's are unique, will
   * revisit this if it becomes a problem -- which,
   * realistically speaking, I don't think will be
   * of much concern, users are not likely to have
   * any more than a MAX of about 8 or so areas per
   * screen.
   *
   * Though of course, users should be allowed to
   * create any number of screen areas without them
   * noticing a performance impact. */
  UNIVERSE_FOR_ALL (screen, kmain->screens) {
    UNIVERSE_FOR_ALL (area, screen->areas) {
      if (id <= area->areaid) {
        id = area->areaid + 1;
      }
    }
  }
  return id;
}

static ScrArea *screen_addarea_ex(kContext *C,
                                  kScreen *screen,
                                  ScrAreaMap *area_map,
                                  ScrVert *bottom_left,
                                  ScrVert *top_left,
                                  ScrVert *top_right,
                                  ScrVert *bottom_right,
                                  const TfToken &spacetype)
{
  int id = find_free_areaid(C);
  ScrArea *area = new ScrArea(C, screen, make_areapath(screen, id));
  area->areaid = id;

  area->v1 = bottom_left;
  area->v2 = top_left;
  area->v3 = top_right;
  area->v4 = bottom_right;
  FormFactory(area->spacetype, spacetype);

  area_map->areas.push_back(area);

  FormFactory(screen->areas_rel, area->path);

  return area;
}


static ScrArea *screen_addarea(kContext *C,
                               kScreen *screen,
                               ScrVert *left_bottom,
                               ScrVert *left_top,
                               ScrVert *right_top,
                               ScrVert *right_bottom,
                               const TfToken &spacetype)
{
  return screen_addarea_ex(C,
                           screen,
                           AREAMAP_FROM_SCREEN(screen),
                           left_bottom,
                           left_top,
                           right_top,
                           right_bottom,
                           spacetype);
}


kScreen *screen_add(kContext *C, const char *name, const GfRect2i *rect)
{
  int id = find_free_screenid(C);
  SdfPath path(make_screenpath(name, id));
  kScreen *screen = new kScreen(C, path);
  screen->path = path;
  screen->winid = id;

  screen->do_refresh = true;
  screen->redraws_flag = TIME_ALL_3D_WIN | TIME_ALL_ANIM_WIN;

  ScrVert *sv1 = screen_geom_vertex_add(screen, rect->GetMinX(), rect->GetMinY());
  ScrVert *sv2 = screen_geom_vertex_add(screen, rect->GetMinX(), rect->GetMaxY() - 1);
  ScrVert *sv3 = screen_geom_vertex_add(screen, rect->GetMaxX() - 1, rect->GetMaxY() - 1);
  ScrVert *sv4 = screen_geom_vertex_add(screen, rect->GetMaxX() - 1, rect->GetMinY());

  screen_geom_edge_add(screen, sv1, sv2);
  screen_geom_edge_add(screen, sv2, sv3);
  screen_geom_edge_add(screen, sv3, sv4);
  screen_geom_edge_add(screen, sv4, sv1);

  screen_addarea(C, screen, sv1, sv2, sv3, sv4, UsdUITokens->spaceEmpty);

  return screen;
}


WABI_NAMESPACE_END