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


ScrVert *screen_geom_vertex_add_ex(ScrAreaMap *area_map, short x, short y)
{
  ScrVert *sv = new ScrVert();
  SET_VEC2(sv->vec, x, y);

  area_map->verts.push_back(sv);
  return sv;
}


ScrVert *screen_geom_vertex_add(cScreen *screen, short x, short y)
{
  return screen_geom_vertex_add_ex(AREAMAP_FROM_SCREEN(screen), x, y);
}


ScrEdge *screen_geom_edge_add_ex(ScrAreaMap *area_map, ScrVert *v1, ScrVert *v2)
{
  ScrEdge *se = new ScrEdge();

  KKE_screen_sort_scrvert(&v1, &v2);
  se->v1 = v1;
  se->v2 = v2;

  area_map->edges.push_back(se);
  return se;
}


ScrEdge *screen_geom_edge_add(cScreen *screen, ScrVert *v1, ScrVert *v2)
{
  return screen_geom_edge_add_ex(AREAMAP_FROM_SCREEN(screen), v1, v2);
}


WABI_NAMESPACE_END