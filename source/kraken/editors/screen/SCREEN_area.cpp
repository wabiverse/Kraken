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
#include "KLI_math.h"

#include "WM_event_system.h"
#include "WM_window.h"

#include "ED_defines.h"
#include "ED_screen.h"

KRAKEN_NAMESPACE_BEGIN


struct RegionTypeAlignInfo
{
  struct
  {
    short alignment;
    bool hidden;
  } by_type[RGN_TYPE_LEN];
};

void ED_area_do_refresh(kContext *C, ScrArea *area)
{
  if (area->type && area->type->refresh) {
    area->type->refresh(C, area);
  }
  area->do_refresh = false;
}

static void region_align_info_from_area(ScrArea *area, RegionTypeAlignInfo *r_align_info)
{
  for (int index = 0; index < RGN_TYPE_LEN; index++) {
    r_align_info->by_type[index].alignment = -1;
    r_align_info->by_type[index].hidden = true;
  }

  UNIVERSE_FOR_ALL (region, area->regions) {
    const int index = region->regiontype;
    if ((uint)index < RGN_TYPE_LEN) {
      r_align_info->by_type[index].alignment = RGN_ALIGN_ENUM_FROM_MASK(region->alignment);
      r_align_info->by_type[index].hidden = (region->flag & RGN_FLAG_HIDDEN) != 0;
    }
  }
}

/* -------------------------------------------------------------------- */
/** \name Region Tag Redraws
 * \{ */

void ED_region_tag_redraw_no_rebuild(ARegion *region)
{
  if (region && !(region->do_draw & (RGN_DRAWING | RGN_DRAW))) {
    region->do_draw &= ~(RGN_DRAW_PARTIAL | RGN_DRAW_EDITOR_OVERLAYS);
    region->do_draw |= RGN_DRAW_NO_REBUILD;
    memset(&region->drawrct, 0, sizeof(region->drawrct));
  }
}

void ED_region_tag_refresh_ui(ARegion *region)
{
  if (region) {
    region->do_draw |= RGN_REFRESH_UI;
  }
}

/** \} */

void ED_area_newspace(kContext *C, ScrArea *area, const TfToken &type, const bool skip_region_exit)
{
  wmWindow *win = CTX_wm_window(C);

  TfToken spacetype = FormFactory(area->spacetype);

  if (spacetype != type) {
    SpaceLink *slold = area->spacedata.at(0);

    // void *area_exit = area->type ? area->type->exit : NULL;

    bool sync_header_alignment = false;
    RegionTypeAlignInfo region_align_info[RGN_TYPE_LEN];
    if ((slold != NULL) && (slold->link_flag & SPACE_FLAG_TYPE_TEMPORARY) == 0) {
      region_align_info_from_area(area, region_align_info);
      sync_header_alignment = true;
    }

    /* in some cases (opening temp space) we don't want to
     * call area exit callback, so we temporarily unset it */
    if (skip_region_exit && area->type) {
      area->type->exit = NULL;
    }

    ED_area_exit(C, area);

    /* restore old area exit callback */
    if (skip_region_exit && area->type) {
      // area->type->exit = area_exit;
    }

    SpaceType *st = KKE_spacetype_from_id((int)type.Hash());

    FormFactory(area->spacetype, type);
    area->type = st;

    /* If st->create may be called, don't use context until then. The
     * area->type->context() callback has changed but data may be invalid
     * (e.g. with properties editor) until space-data is properly created */

    /* check previously stored space */
    SpaceLink *sl = POINTER_ZERO;
    UNIVERSE_FOR_ALL (sl_iter, area->spacedata) {
      if (sl_iter->spacetype == type.Hash()) {
        sl = sl_iter;
        break;
      }
    }

    if (sl && sl->regions.empty()) {
      st->free(sl);
      // area->spacedata.erase(sl);
      if (slold == sl) {
        slold = NULL;
      }
      sl = NULL;
    }

    if (sl) {
      /* swap regions */
      slold->regions = area->regions;
      area->regions = sl->regions;
      sl->regions.clear();
      /* SPACE_FLAG_TYPE_WAS_ACTIVE is only used to go back to a previously active space that is
       * overlapped by temporary ones. It's now properly activated, so the flag should be cleared
       * at this point. */
      sl->link_flag &= ~SPACE_FLAG_TYPE_WAS_ACTIVE;

      /* put in front of list */

      // area->spacedata.erase(sl);
      area->spacedata.insert(area->spacedata.begin(), sl);
    } else {
      /* new space */
      if (st) {
        /* Don't get scene from context here which may depend on space-data. */
        Scene *scene = CTX_data_scene(C);
        sl = st->create(area, scene);
        area->spacedata.insert(area->spacedata.begin(), sl);

        /* swap regions */
        if (slold) {
          slold->regions = area->regions;
        }
        area->regions = sl->regions;
        sl->regions.clear();
      }
    }

    /* Sync header alignment. */
    if (sync_header_alignment) {
      // region_align_info_to_area(area, region_align_info);
    }

    // ED_area_init(CTX_wm_manager(C), win, area);

    /* tell WM to refresh, cursor types etc */
    WM_event_add_mousemove(win);

    /* send space change notifier */
    WM_event_add_notifier(C, NC_SPACE | ND_SPACE_CHANGED, area);

    // ED_area_tag_refresh(area);
  }

  /* also redraw when re-used */
  // ED_area_tag_redraw(area);
}


bool ED_area_is_global(const ScrArea *area)
{
  return area->global != NULL;
}


int ED_area_global_size_y(const ScrArea *area)
{
  KLI_assert(ED_area_is_global(area));
  return round_fl_to_int(area->global->cur_fixed_height * UI_DPI_FAC);
}


KRAKEN_NAMESPACE_END