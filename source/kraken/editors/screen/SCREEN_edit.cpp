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

#include "USD_wm_types.h"
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
#include "USD_workspace.h"

#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_assert.h"
#include "KLI_listbase.h"
#include "KLI_math.h"

#include "WM_event_system.h"
#include "WM_inline_tools.h"
#include "WM_window.hh"
#include "WM_window.h"

#include "ED_defines.h"
#include "ED_screen.h"

#include <wabi/base/gf/rect2i.h>
#include <wabi/usd/usdUI/tokens.h>

#include "UI_interface.h"

static int screen_geom_area_height(const ScrArea *area)
{
  return area->v2->vec[1] - area->v1->vec[1] + 1;
}
static int screen_geom_area_width(const ScrArea *area)
{
  return area->v4->vec[0] - area->v1->vec[0] + 1;
}

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

  if (area->type && area->type->exit) {
    area->type->exit(wm, area);
  }

  CTX_wm_area_set(C, area);

  LISTBASE_FOREACH (ARegion *, region, &area->regions) {
    ED_region_exit(C, region);
  }

  WM_event_remove_handlers(C, &area->handlers);
  // WM_event_modal_handler_area_replace(win, area, NULL);

  CTX_wm_area_set(C, prevsa);
}

void ED_screen_exit(kContext *C, wmWindow *window, kScreen *screen)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *prevwin = CTX_wm_window(C);

  CTX_wm_window_set(C, window);

  // if (screen->animtimer) {
  //   WM_event_remove_timer(wm, window, screen->animtimer);

  //   Hydra *hydra = CTX_data_hydra_pointer(C);
  //   Scene *scene = WM_window_get_active_scene(prevwin);
  //   Scene *scene_eval = (Scene *)Hydra_get_evaluated_id(hydra, &scene->id);
  //   KKE_sound_stop_scene(scene_eval);
  // }
  // screen->animtimer = nullptr;
  // screen->scrubbing = false;

  screen->active_region = nullptr;

  LISTBASE_FOREACH (ARegion *, region, &screen->regions) {
    ED_region_exit(C, region);
  }
  LISTBASE_FOREACH (ScrArea *, area, &screen->areas) {
    ED_area_exit(C, area);
  }
  /* Don't use ED_screen_areas_iter here, it skips hidden areas. */
  LISTBASE_FOREACH (ScrArea *, area, &window->global_areas.areas) {
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
  LISTBASE_FOREACH(kScreen *, screen, &kmain->screens)
  {
    LISTBASE_FOREACH (ScrArea *, area, &screen->areas) {
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

  KLI_addtail(&area_map->areas, area);

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


kScreen *screen_add(kContext *C, const char *name, const rcti *rect)
{
  int id = find_free_screenid(C);
  SdfPath path(make_screenpath(name, id));
  kScreen *screen = new kScreen(C, path);
  screen->path = path;
  screen->winid = id;

  screen->do_refresh = true;
  screen->redraws_flag = TIME_ALL_3D_WIN | TIME_ALL_ANIM_WIN;

  ScrVert *sv1 = screen_geom_vertex_add(screen, rect->xmin, rect->ymin);
  ScrVert *sv2 = screen_geom_vertex_add(screen, rect->xmin, rect->ymax - 1);
  ScrVert *sv3 = screen_geom_vertex_add(screen, rect->xmax - 1, rect->ymax - 1);
  ScrVert *sv4 = screen_geom_vertex_add(screen, rect->xmax - 1, rect->ymin);

  screen_geom_edge_add(screen, sv1, sv2);
  screen_geom_edge_add(screen, sv2, sv3);
  screen_geom_edge_add(screen, sv3, sv4);
  screen_geom_edge_add(screen, sv4, sv1);

  screen_addarea(C, screen, sv1, sv2, sv3, sv4, UsdUITokens->spaceEmpty);

  return screen;
}

static bool screen_geom_vertices_scale_pass(const wmWindow *win,
                                            const kScreen *screen,
                                            const rcti *screen_rect)
{

  const int screen_size_x = KLI_rcti_size_x(screen_rect);
  const int screen_size_y = KLI_rcti_size_y(screen_rect);
  bool needs_another_pass = false;

  /* calculate size */
  float min[2] = {20000.0f, 20000.0f};
  float max[2] = {0.0f, 0.0f};

  LISTBASE_FOREACH(ScrVert *, sv, &screen->verts)
  {
    const float fv[2] = {(float)sv->vec[0], (float)sv->vec[1]};
    minmax_v2v2_v2(min, max, fv);
  }

  int screen_size_x_prev = (max[0] - min[0]) + 1;
  int screen_size_y_prev = (max[1] - min[1]) + 1;

  if (screen_size_x_prev != screen_size_x || screen_size_y_prev != screen_size_y) {
    const float facx = ((float)screen_size_x - 1) / ((float)screen_size_x_prev - 1);
    const float facy = ((float)screen_size_y - 1) / ((float)screen_size_y_prev - 1);

    /* make sure it fits! */
    LISTBASE_FOREACH(ScrVert *, sv, &screen->verts)
    {
      sv->vec[0] = screen_rect->xmin + round_fl_to_short((sv->vec[0] - min[0]) * facx);
      CLAMP(sv->vec[0], screen_rect->xmin, screen_rect->xmax - 1);

      sv->vec[1] = screen_rect->ymin + round_fl_to_short((sv->vec[1] - min[1]) * facy);
      CLAMP(sv->vec[1], screen_rect->ymin, screen_rect->ymax - 1);
    }

    /* test for collapsed areas. This could happen in some blender version... */
    /* ton: removed option now, it needs Context... */

    int headery = ED_area_headersize() + (U.pixelsize * 2);

    if (facy > 1) {
      /* Keep timeline small in video edit workspace. */
      LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
      {
        TfToken st = FormFactory(area->spacetype);
        if (WM_spacetype_enum_from_token(st) == SPACE_ACTION &&
            area->v1->vec[1] == screen_rect->ymin &&
            screen_geom_area_height(area) <= headery * facy + 1) {
          ScrEdge *se = KKE_screen_find_edge(screen, area->v2, area->v3);
          if (se) {
            const int yval = area->v1->vec[1] + headery - 1;

            screen_geom_select_connected_edge(win, se);

            /* all selected vertices get the right offset */
            LISTBASE_FOREACH(ScrVert *, sv, &screen->verts)
            {
              /* if is a collapsed area */
              if (!ELEM(sv, area->v1, area->v4)) {
                if (sv->flag) {
                  sv->vec[1] = yval;
                  /* Changed size of a area. Run another pass to ensure everything still fits. */
                  needs_another_pass = true;
                }
              }
            }
          }
        }
      }
    }
    if (facy < 1) {
      /* make each window at least ED_area_headersize() high */
      LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
      {
        if (screen_geom_area_height(area) < headery) {
          /* lower edge */
          ScrEdge *se = KKE_screen_find_edge(screen, area->v4, area->v1);
          if (se && area->v1 != area->v2) {
            const int yval = area->v2->vec[1] - headery + 1;

            screen_geom_select_connected_edge(win, se);

            /* all selected vertices get the right offset */
            LISTBASE_FOREACH(ScrVert *, sv, &screen->verts)
            {
              /* if is not a collapsed area */
              if (!ELEM(sv, area->v2, area->v3)) {
                if (sv->flag) {
                  sv->vec[1] = yval;
                  /* Changed size of a area. Run another pass to ensure everything still fits. */
                  needs_another_pass = true;
                }
              }
            }
          }
        }
      }
    }
  }

  return needs_another_pass;
}

static void screen_geom_vertices_scale(const wmWindow *win, kScreen *screen)
{
  wabi::GfRect2i window_rect, screen_rect;
  WM_window_rect_calc(win, &window_rect);
  WM_window_screen_rect_calc(win, &screen_rect);

  rcti gf_screen_rcti;
  gf_screen_rcti.xmin = screen_rect.GetMinX();
  gf_screen_rcti.xmax = screen_rect.GetMaxX();
  gf_screen_rcti.ymin = screen_rect.GetMinY();
  gf_screen_rcti.ymax = screen_rect.GetMaxY();

  bool needs_another_pass;
  int max_passes_left = 10; /* Avoids endless loop. Number is rather arbitrary. */
  do {
    needs_another_pass = screen_geom_vertices_scale_pass(win, screen, &gf_screen_rcti);
    max_passes_left--;
  } while (needs_another_pass && (max_passes_left > 0));

  screen_rect.SetMinX(gf_screen_rcti.xmin);
  screen_rect.SetMaxX(gf_screen_rcti.xmax);
  screen_rect.SetMinY(gf_screen_rcti.ymin);
  screen_rect.SetMaxY(gf_screen_rcti.ymax);

  /* Global areas have a fixed size that only changes with the DPI.
   * Here we ensure that exactly this size is set. */
  LISTBASE_FOREACH(ScrArea *, area, &win->global_areas.areas)
  {
    if (area->global->flag & GLOBAL_AREA_IS_HIDDEN) {
      continue;
    }

    int height = ED_area_global_size_y(area) - 1;

    if (area->v1->vec[1] > window_rect.GetMinY()) {
      height += U.pixelsize;
    }
    if (area->v2->vec[1] < (window_rect.GetMaxY() - 1)) {
      height += U.pixelsize;
    }

    /* width */
    area->v1->vec[0] = area->v2->vec[0] = window_rect.GetMinX();
    area->v3->vec[0] = area->v4->vec[0] = window_rect.GetMaxX() - 1;
    /* height */
    area->v1->vec[1] = area->v4->vec[1] = window_rect.GetMinY();
    area->v2->vec[1] = area->v3->vec[1] = window_rect.GetMaxY() - 1;

    switch (area->global->align) {
      case GLOBAL_AREA_ALIGN_TOP:
        area->v1->vec[1] = area->v4->vec[1] = area->v2->vec[1] - height;
        break;
      case GLOBAL_AREA_ALIGN_BOTTOM:
        area->v2->vec[1] = area->v3->vec[1] = area->v1->vec[1] + height;
        break;
    }
  }
}

void ED_screen_global_areas_refresh(wmWindow *win)
{
  /* Don't create global area for child and temporary windows. */
  kScreen *screen = KKE_workspace_active_screen_get(win->workspace_hook);
  if ((win->parent != NULL) || screen->temp) {
    if (win->global_areas.areas.first) {
      screen->do_refresh = true;
      KKE_screen_area_map_free(&win->global_areas);
    }
    return;
  }

  // screen_global_topbar_area_refresh(win, screen);
  // screen_global_statusbar_area_refresh(win, screen);
}

void ED_screen_refresh(wmWindowManager *wm, wmWindow *win)
{
  kScreen *screen = WM_window_get_active_screen(win);

  /* Exception for background mode, we only need the screen context. */
  if (!G.background) {

    /* Called even when creating the ghost window fails in #WM_window_open. */
    if (win->anchorwin) {
      /* Header size depends on DPI, let's verify. */
      WM_window_set_dpi(win);
    }

    ED_screen_global_areas_refresh(win);

    screen_geom_vertices_scale(win, screen);

    ED_screen_areas_iter(win, screen, area)
    {
      /* Set space-type and region callbacks, calls init() */
      /* Sets sub-windows for regions, adds handlers. */
      ED_area_init(wm, win, area);
    }

    /* wake up animtimer */
    // if (screen->animtimer) {
    //   WM_event_timer_sleep(wm, win, screen->animtimer, false);
    // }
  }

  if (G.debug & G_DEBUG_EVENTS) {
    printf("%s: set screen\n", __func__);
  }
  screen->do_refresh = false;
  /* prevent multiwin errors */
  screen->winid = win->winid;

  // screen->context = ed_screen_context;
}

void ED_screen_ensure_updated(wmWindowManager *wm, wmWindow *win, kScreen *screen)
{
  if (screen->do_refresh) {
    ED_screen_refresh(wm, win);
  }
}
