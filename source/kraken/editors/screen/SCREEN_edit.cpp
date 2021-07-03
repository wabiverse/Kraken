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
 * Editors.
 * Tools for Artists.
 */

#include "UNI_area.h"
#include "UNI_context.h"
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
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_assert.h"
#include "KLI_math_inline.h"

#include "WM_window.h"

#include "ED_defines.h"
#include "ED_screen.h"

#include <wabi/base/gf/rect2i.h>

WABI_NAMESPACE_BEGIN


bool ED_screen_change(cContext *C, cScreen *screen)
{
  Main *cmain = CTX_data_main(C);
  wmWindow *win = CTX_wm_window(C);
  WorkSpace *workspace = KKE_workspace_active_get(win->workspace_hook);
  WorkSpaceLayout *layout = KKE_workspace_layout_find(workspace, screen);
  cScreen *screen_old = CTX_wm_screen(C);

  /* Get the actual layout/screen to be activated (guaranteed to be unused, even if that means
   * having to duplicate an existing one). */
  WorkSpaceLayout *layout_new = ED_workspace_screen_change_ensure_unused_layout(cmain,
                                                                                workspace,
                                                                                layout,
                                                                                layout,
                                                                                win);
  cScreen *screen_new = KKE_workspace_layout_screen_get(layout_new);

  // screen_change_prepare(screen_old, screen_new, cmain, C, win);

  if (screen_old != screen_new)
  {
    // WM_window_set_active_screen(win, workspace, screen_new);
    // screen_change_update(C, win, screen_new);

    return true;
  }

  return false;
}


void ED_region_exit(cContext *C, ARegion *region)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  ARegion *prevar = CTX_wm_region(C);

  if (region->type && region->type->exit)
  {
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


void ED_area_exit(cContext *C, ScrArea *area)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  ScrArea *prevsa = CTX_wm_area(C);

  // if (area->type && area->type->exit) {
  // area->type->exit(wm, area);
  // }

  CTX_wm_area_set(C, area);

  UNIVERSE_FOR_ALL(region, area->regions)
  {
    ED_region_exit(C, region);
  }

  // WM_event_remove_handlers(C, &area->handlers);
  // WM_event_modal_handler_area_replace(win, area, NULL);

  CTX_wm_area_set(C, prevsa);
}


static ScrArea *screen_addarea_ex(cContext *C,
                                  cScreen *screen,
                                  ScrAreaMap *area_map,
                                  ScrVert *bottom_left,
                                  ScrVert *top_left,
                                  ScrVert *top_right,
                                  ScrVert *bottom_right,
                                  const TfToken &spacetype)
{
  ScrArea *area = new ScrArea(C, screen, SdfPath("Area"));

  area->v1 = bottom_left;
  area->v2 = top_left;
  area->v3 = top_right;
  area->v4 = bottom_right;
  FormFactory(area->spacetype, spacetype);

  area_map->areas.push_back(area);

  return area;
}


static ScrArea *screen_addarea(cContext *C,
                               cScreen *screen,
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


cScreen *screen_add(cContext *C, const char *name, const GfRect2i *rect)
{
  SdfPath path(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES));
  cScreen *screen = new cScreen(C, path.AppendPath(SdfPath(name)));

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