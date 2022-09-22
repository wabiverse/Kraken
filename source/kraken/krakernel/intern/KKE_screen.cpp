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

#include "KKE_screen.h"

#include <wabi/usd/usd/tokens.h>

KRAKEN_NAMESPACE_BEGIN


/* keep global; this has to be accessible outside of windowmanager */
static std::vector<SpaceType *> spacetypes;


SdfPath make_screenpath(const char *layout_name, int id)
{
  SdfPath sdf_layout(SdfPath(layout_name + STRINGALL("Screen") + STRINGALL(id)));
  return SdfPath(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES)).AppendPath(sdf_layout);
}


int find_free_screenid(kContext *C)
{
  int id = 1;

  Main *kmain = CTX_data_main(C);
  UNIVERSE_FOR_ALL (screen, kmain->screens) {
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
  UNIVERSE_FOR_ALL (st, spacetypes) {
    if (st->spaceid == spaceid) {
      return st;
    }
  }
  return NULL;
}


ScrArea *KKE_screen_find_big_area(kScreen *screen, const int spacetype, const short min)
{
  ScrArea *big = nullptr;
  int maxsize = 0;

  for (auto &area : screen->areas) {
    if ((spacetype == SPACE_TYPE_ANY) &&  area->spacetype.IsValid()) {
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
    UNIVERSE_FOR_ALL (region, area->regions) {
      if (region->regiontype == region_type) {
        return region;
      }
    }
  }

  return NULL;
}

ARegion *KKE_area_find_region_active_win(ScrArea *area)
{
  if (area == NULL) {
    return NULL;
  }

  ARegion *region = area->regions.at(area->region_active_win);
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

ARegionType *KKE_regiontype_from_id(const SpaceType *st, int regionid)
{
  for (auto &art : st->regiontypes) {
    if (art->regionid == regionid) {
      return art;
    }
  }
  return NULL;
}

void KKE_area_region_free(SpaceType *st, ARegion *region)
{
  if (st) {
    ARegionType *art = KKE_regiontype_from_id(st, region->regiontype);

    if (art && art->free) {
      art->free(region);
    }

    if (region->regiondata) {
      printf("regiondata free error\n");
    }
  }
  else if (region->type && region->type->free) {
    region->type->free(region);
  }

  // KKE_area_region_panels_free(&region->panels);

  // for (auto &uilst : region->ui_lists) {
  //   if (uilst->dyn_data && uilst->dyn_data->free_runtime_data_fn) {
  //     uilst->dyn_data->free_runtime_data_fn(uilst);
  //   }
  //   if (uilst->properties) {
  //     IDP_FreeProperty(uilst->properties);
  //   }
  //   MEM_SAFE_FREE(uilst->dyn_data);
  // }

  // if (region->gizmo_map != NULL) {
  //   region_free_gizmomap_callback(region->gizmo_map);
  // }

  // if (region->runtime.block_name_map != NULL) {
  //   KKE_rhash_free(region->runtime.block_name_map, NULL, NULL);
  //   region->runtime.block_name_map = NULL;
  // }

  // KLI_freelistN(&region->ui_lists);
  // KLI_freelistN(&region->ui_previews);
  // KLI_freelistN(&region->panels_category);
  // KLI_freelistN(&region->panels_category_active);
}

KRAKEN_NAMESPACE_END