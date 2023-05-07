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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "USD_area.h"
#include "USD_context.h"
#include "USD_factory.h"
#include "USD_listBase.h"
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

#include "KLI_listbase.h"
#include "KKE_idprop.h"
#include "KKE_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "WM_inline_tools.h"

#include <wabi/usd/usdUI/area.h>
#include <wabi/usd/usdUI/screen.h>
#include <wabi/usd/usd/tokens.h>

using namespace wabi;

/* keep global; this has to be accessible outside of windowmanager */
static ListBase spacetypes = {NULL, NULL};

SdfPath make_screenpath(const char *layout_name, int id)
{
  SdfPath sdf_layout(SdfPath(layout_name + STRINGALL("Screen") + STRINGALL(id)));
  return SdfPath(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES)).AppendPath(sdf_layout);
}


int find_free_screenid(kContext *C)
{
  int id = 1;

  Main *kmain = CTX_data_main(C);
  LISTBASE_FOREACH(kScreen *, screen, &kmain->screens)
  {
    if (id <= screen->winid) {
      id = screen->winid + 1;
    }
  }
  return id;
}


bool KKE_screen_is_used(const kScreen *screen)
{
  return (screen->winid != 0);
}


SpaceType *KKE_spacetype_from_id(int spaceid)
{
  LISTBASE_FOREACH(SpaceType *, st, &spacetypes)
  {
    if (st->spaceid == spaceid) {
      return st;
    }
  }
  return nullptr;
}

ScrArea *KKE_screen_find_big_area(kScreen *screen, const int spacetype, const short min)
{
  ScrArea *big = nullptr;
  int maxsize = 0;

  LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
  {
    TfToken st = FormFactory(area->spacetype);
    if (ELEM(spacetype, SPACE_TYPE_ANY, WM_spacetype_enum_from_token(st))) {
      GfVec2f winsize = FormFactory(area->size);
      if (min <= GET_X(winsize) && min <= GET_Y(winsize)) {
        int size = GET_X(winsize) * GET_Y(winsize);
        if (size > maxsize) {
          maxsize = size;
          big = area;
        }
      }
    }
  }

  return big;
}

ARegion *KKE_area_find_region_type(const ScrArea *area, int region_type)
{
  if (area) {
    LISTBASE_FOREACH(ARegion *, region, &area->regions)
    {
      if (region->regiontype == region_type) {
        return region;
      }
    }
  }

  return nullptr;
}

ARegion *KKE_area_find_region_active_win(ScrArea *area)
{
  if (area == nullptr) {
    return nullptr;
  }

  ARegion *region = (ARegion *)KLI_findlink(&area->regions, area->region_active_win);
  if (region && (region->regiontype == RGN_TYPE_WINDOW)) {
    return region;
  }

  /* fallback to any */
  return KKE_area_find_region_type(area, RGN_TYPE_WINDOW);
}

void KKE_screen_sort_scrvert(ScrVert **v1, ScrVert **v2)
{
  if (*v1 > *v2) {
    ScrVert *tmp = *v1;
    *v1 = *v2;
    *v2 = tmp;
  }
}

static void area_region_panels_free_recursive(Panel *panel)
{
  MEM_SAFE_FREE(panel->activedata);

  LISTBASE_FOREACH_MUTABLE(Panel *, child_panel, &panel->children)
  {
    area_region_panels_free_recursive(child_panel);
  }

  MEM_delete(panel);
}

void KKE_area_region_panels_free(ListBase *panels)
{
  LISTBASE_FOREACH_MUTABLE(Panel *, panel, panels)
  {
    /* Free custom data just for parent panels to avoid a double free. */
    MEM_SAFE_FREE(panel->runtime.custom_data_ptr);
    area_region_panels_free_recursive(panel);
  }
  KLI_listbase_clear(panels);
}

void KKE_screen_area_map_free(ScrAreaMap *area_map)
{
  LISTBASE_FOREACH_MUTABLE(ScrArea *, area, &area_map->areas)
  {
    KKE_screen_area_free(area);
  }

  KLI_freelistN(&area_map->verts);
  KLI_freelistN(&area_map->edges);
  KLI_freelistN(&area_map->areas);
}

void KKE_area_region_free(SpaceType *st, ARegion *region)
{
  if (st) {
    struct ARegionType *art = KKE_regiontype_from_id(st, region->regiontype);

    if (art && art->free) {
      art->free(region);
    }

    if (region->regiondata) {
      printf("regiondata free error\n");
    }
  } else if (region->type && region->type->free) {
    region->type->free(region);
  }

  KKE_area_region_panels_free(&region->panels);

  LISTBASE_FOREACH(uiList *, uilst, &region->ui_lists)
  {
    if (uilst->dyn_data && uilst->dyn_data->free_runtime_data_fn) {
      uilst->dyn_data->free_runtime_data_fn(uilst);
    }
    if (uilst->properties) {
      IDP_FreeProperty(uilst->properties);
    }
    MEM_SAFE_FREE(uilst->dyn_data);
  }

  // if (region->gizmo_map != NULL) {
  //   region_free_gizmomap_callback(region->gizmo_map);
  // }

  if (region->runtime.block_name_map != NULL) {
    KLI_rhash_free(region->runtime.block_name_map, NULL, NULL);
    region->runtime.block_name_map = NULL;
  }

  KLI_freelistN(&region->ui_lists);
  KLI_freelistN(&region->ui_previews);
  KLI_freelistN(&region->panels_category);
  KLI_freelistN(&region->panels_category_active);
}

void KKE_screen_area_free(ScrArea *area)
{
  TfToken space_type = FormFactory(area->spacetype);
  SpaceType *st = KKE_spacetype_from_id(WM_spacetype_enum_from_token(space_type));

  LISTBASE_FOREACH (ARegion *, region, &area->regions) {
    KKE_area_region_free(st, region);
  }

  MEM_SAFE_FREE(area->global);
  KLI_freelistN(&area->regions);

  //KKE_spacedata_freelist(&area->spacedata);

  //KLI_freelistN(&area->actionzones);
}

ARegionType *KKE_regiontype_from_id_or_first(SpaceType *st, int regionid)
{
  LISTBASE_FOREACH(ARegionType *, art, &st->regiontypes)
  {
    if (art->regionid == regionid) {
      return art;
    }
  }

  printf("Error, region type %d missing in - name:\"%s\", id:%d\n",
         regionid,
         st->name.data(),
         st->spaceid);
  return (ARegionType *)st->regiontypes.first;
}

ARegionType *KKE_regiontype_from_id(SpaceType *st, int regionid)
{
  LISTBASE_FOREACH(ARegionType *, art, &st->regiontypes)
  {
    if (art->regionid == regionid) {
      return art;
    }
  }
  return nullptr;
}


/* ***************** Screen edges & verts ***************** */

ScrEdge *KKE_screen_find_edge(const kScreen *screen, ScrVert *v1, ScrVert *v2)
{
  KKE_screen_sort_scrvert(&v1, &v2);
  LISTBASE_FOREACH(ScrEdge *, se, &screen->edges)
  {
    if (se->v1 == v1 && se->v2 == v2) {
      return se;
    }
  }

  return NULL;
}
