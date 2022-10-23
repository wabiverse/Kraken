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
#include "KLI_listbase.h"
#include "KLI_math.h"

#include "WM_window.hh"

#include "ED_defines.h"
#include "ED_screen.h"

#include <wabi/base/gf/rect2i.h>

ScrVert *screen_geom_vertex_add_ex(ScrAreaMap *area_map, short x, short y)
{
  ScrVert *sv = MEM_new<ScrVert>("addscrvert");
  sv->vec[0] = x;
  sv->vec[1] = y;

  KLI_addtail(&area_map->verts, sv);
  return sv;
}


ScrVert *screen_geom_vertex_add(kScreen *screen, short x, short y)
{
  return screen_geom_vertex_add_ex(AREAMAP_FROM_SCREEN(screen), x, y);
}


ScrEdge *screen_geom_edge_add_ex(ScrAreaMap *area_map, ScrVert *v1, ScrVert *v2)
{
  ScrEdge *se = MEM_new<ScrEdge>("addscredge");

  KKE_screen_sort_scrvert(&v1, &v2);
  se->v1 = v1;
  se->v2 = v2;

  KLI_addtail(&area_map->edges, se);
  return se;
}


ScrEdge *screen_geom_edge_add(kScreen *screen, ScrVert *v1, ScrVert *v2)
{
  return screen_geom_edge_add_ex(AREAMAP_FROM_SCREEN(screen), v1, v2);
}

void screen_geom_select_connected_edge(const wmWindow *win, ScrEdge *edge)
{
  kScreen *screen = WM_window_get_active_screen(win);

  /* 'dir_axis' is the direction of EDGE */
  eScreenAxis dir_axis;
  if (edge->v1->vec[0] == edge->v2->vec[0]) {
    dir_axis = SCREEN_AXIS_V;
  } else {
    dir_axis = SCREEN_AXIS_H;
  }

  ED_screen_verts_iter(win, screen, sv)
  {
    sv->flag = 0;
  }

  edge->v1->flag = 1;
  edge->v2->flag = 1;

  /* select connected, only in the right direction */
  bool oneselected = true;
  while (oneselected) {
    oneselected = false;
    LISTBASE_FOREACH(ScrEdge *, se, &screen->edges)
    {
      if (se->v1->flag + se->v2->flag == 1) {
        if (dir_axis == SCREEN_AXIS_H) {
          if (se->v1->vec[1] == se->v2->vec[1]) {
            se->v1->flag = se->v2->flag = 1;
            oneselected = true;
          }
        } else if (dir_axis == SCREEN_AXIS_V) {
          if (se->v1->vec[0] == se->v2->vec[0]) {
            se->v1->flag = se->v2->flag = 1;
            oneselected = true;
          }
        }
      }
    }
  }
}
