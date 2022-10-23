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

#include "MEM_guardedalloc.h"

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
#include "USD_userdef_types.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_listbase.h"
#include "KLI_assert.h"
#include "KLI_math.h"

#include "WM_event_system.h"
#include "WM_window.hh"
#include "WM_inline_tools.h"

#include "ED_defines.h"
#include "ED_screen.h"

#include "UI_interface.h"

struct RegionTypeAlignInfo
{
  struct
  {
    short alignment;
    bool hidden;
  } by_type[RGN_TYPE_NUM];
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
  for (int index = 0; index < RGN_TYPE_NUM; index++) {
    r_align_info->by_type[index].alignment = -1;
    r_align_info->by_type[index].hidden = true;
  }

  LISTBASE_FOREACH(ARegion *, region, &area->regions)
  {
    const int index = region->regiontype;
    if ((uint)index < RGN_TYPE_NUM) {
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

void ED_area_newspace(kContext *C, ScrArea *area, TfToken type, bool skip_region_exit)
{
  wmWindow *win = CTX_wm_window(C);

  TfToken spacetype = FormFactory(area->spacetype);

  if (spacetype != type) {
    kSpaceLink *slold = MEM_new<kSpaceLink>("kSpaceLink");
    slold->link_flag = area->spacedata.front()->link_flag;
    // slold->regionbase = area->spacedata.front()->regionbase;
    slold->spacetype = area->spacedata.front()->spacetype;


    // void *area_exit = area->type ? area->type->exit : NULL;

    bool sync_header_alignment = false;
    RegionTypeAlignInfo region_align_info[RGN_TYPE_NUM];
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
    SpaceProperties *sl = POINTER_ZERO;
    UNIVERSE_FOR_ALL (sl_iter, area->spacedata) {
      if (sl_iter->spacetype == type.Hash()) {
        sl = sl_iter;
        break;
      }
    }

    if (sl && KLI_listbase_is_empty(&sl->regionbase)) {
      // st->free(sl);
      // area->spacedata.erase(sl);
      // if (slold == sl) {
      // slold = NULL;
      // }
      sl = NULL;
    }

    if (sl) {
      /* swap regions */
      // slold->regions = area->regions;
      // area->regions = sl->regions;
      // sl->regions.clear();
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
        kScene *scene = CTX_data_scene(C);
        // sl = st->create(area, scene);
        area->spacedata.insert(area->spacedata.begin(), sl);

        /* swap regions */
        if (slold) {
          // slold->regions = area->regions;
        }
        // area->regions = sl->regions;
        // sl->regions.clear();
      }
    }

    /* Sync header alignment. */
    if (sync_header_alignment) {
      // region_align_info_to_area(area, region_align_info);
    }

    ED_area_init(CTX_wm_manager(C), win, area);

    /* tell WM to refresh, cursor types etc */
    WM_event_add_mousemove(win);

    /* send space change notifier */
    WM_event_add_notifier(C, NC_SPACE | ND_SPACE_CHANGED, area);

    ED_area_tag_refresh(area);
  }

  /* also redraw when re-used */
  // ED_area_tag_redraw(area);
}

int ED_area_headersize(void)
{
  /* Accommodate widget and padding. */
  return U.widget_unit + (int)(UI_DPI_FAC * HEADER_PADDING_Y);
}

bool ED_area_is_global(const ScrArea *area)
{
  return area->global != NULL;
}


int ED_area_global_size_y(const ScrArea *area)
{
  KLI_assert(ED_area_is_global(area));
  return round_fl_to_int(area->global->cur_fixed_height * U.dpi_fac);
}

void ED_area_tag_refresh(ScrArea *area)
{
  if (area) {
    area->do_refresh = true;
  }
}

void ED_region_tag_redraw(ARegion *region)
{
  /* don't tag redraw while drawing, it shouldn't happen normally
   * but python scripts can cause this to happen indirectly */
  if (region && !(region->do_draw & RGN_DRAWING)) {
    /* zero region means full region redraw */
    region->do_draw &= ~(RGN_DRAW_PARTIAL | RGN_DRAW_NO_REBUILD | RGN_DRAW_EDITOR_OVERLAYS);
    region->do_draw |= RGN_DRAW;
    memset(&region->drawrct, 0, sizeof(region->drawrct));
  }
}

void ED_region_do_msg_notify_tag_redraw(
  /* Follow wmMsgNotifyFn spec */
  kContext *UNUSED(C),
  wmMsgSubscribeKey *UNUSED(msg_key),
  wmMsgSubscribeValue *msg_val)
{
  ARegion *region = (ARegion *)msg_val->owner;
  ED_region_tag_redraw(region);

  /* This avoids _many_ situations where header/properties control display settings.
   * the common case is space properties in the header */
  if (ELEM(region->regiontype, RGN_TYPE_HEADER, RGN_TYPE_TOOL_HEADER, RGN_TYPE_UI)) {
    while (region && region->prev) {
      region = region->prev;
    }
    for (; region; region = region->next) {
      if (ELEM(region->regiontype, RGN_TYPE_WINDOW, RGN_TYPE_CHANNELS)) {
        ED_region_tag_redraw(region);
      }
    }
  }
}

void ED_area_do_msg_notify_tag_refresh(
  /* Follow wmMsgNotifyFn spec */
  kContext *UNUSED(C),
  wmMsgSubscribeKey *UNUSED(msg_key),
  wmMsgSubscribeValue *msg_val)
{
  ScrArea *area = (ScrArea *)msg_val->user_data;
  ED_area_tag_refresh(area);
}

ScrArea *ED_screen_areas_iter_first(const wmWindow *win, const kScreen *screen)
{
  ScrArea *global_area = (ScrArea *)win->global_areas.areas.first;

  if (!global_area) {
    return (ScrArea *)screen->areas.first;
  }
  if ((global_area->global->flag & GLOBAL_AREA_IS_HIDDEN) == 0) {
    return global_area;
  }
  /* Find next visible area. */
  return ED_screen_areas_iter_next(screen, global_area);
}
ScrArea *ED_screen_areas_iter_next(const kScreen *screen, const ScrArea *area)
{
  if (area->global == nullptr) {
    return area->next;
  }

  for (ScrArea *area_iter = area->next; area_iter; area_iter = area_iter->next) {
    if ((area_iter->global->flag & GLOBAL_AREA_IS_HIDDEN) == 0) {
      return area_iter;
    }
  }
  /* No visible next global area found, start iterating over layout areas. */
  return (ScrArea *)screen->areas.first;
}

static void area_calc_totrct(ScrArea *area, const rcti *window_rect)
{
  short px = (short)U.pixelsize;

  area->totrct.xmin = (int)(area->v1->vec[0].bits());
  area->totrct.xmax = (int)(area->v4->vec[0].bits());
  area->totrct.ymin = (int)(area->v1->vec[1].bits());
  area->totrct.ymax = (int)(area->v2->vec[1].bits());

  /* scale down totrct by 1 pixel on all sides not matching window borders */
  if (area->totrct.xmin > window_rect->xmin) {
    area->totrct.xmin += px;
  }
  if (area->totrct.xmax < (window_rect->xmax - 1)) {
    area->totrct.xmax -= px;
  }
  if (area->totrct.ymin > window_rect->ymin) {
    area->totrct.ymin += px;
  }
  if (area->totrct.ymax < (window_rect->ymax - 1)) {
    area->totrct.ymax -= px;
  }
  /* Although the following asserts are correct they lead to a very unstable Blender.
   * And the asserts would fail even in 2.7x
   * (they were added in 2.8x as part of the top-bar commit).
   * For more details see T54864. */
#if 0
  KLI_assert(area->totrct.xmin >= 0);
  KLI_assert(area->totrct.xmax >= 0);
  KLI_assert(area->totrct.ymin >= 0);
  KLI_assert(area->totrct.ymax >= 0);
#endif

  /* for speedup */
  FormFactory(area->size,
              GfVec2f(KLI_rcti_size_x(&area->totrct) + 1, KLI_rcti_size_y(&area->totrct) + 1));
}

void ED_area_init(wmWindowManager *wm, wmWindow *win, ScrArea *area)
{
  WorkSpace *workspace = WM_window_get_active_workspace(win);
  const kScreen *screen = KKE_workspace_active_screen_get(win->workspace_hook);
  const kScene *scene = WM_window_get_active_scene(win);
  // ViewLayer *view_layer = WM_window_get_active_view_layer(win);

  if (ED_area_is_global(area) && (area->global->flag & GLOBAL_AREA_IS_HIDDEN)) {
    return;
  }

  wabi::GfRect2i window_rect;
  WM_window_rect_calc(win, &window_rect);

  /* Set type-definitions. */
  wabi::TfToken st = FormFactory(area->spacetype);
  area->type = KKE_spacetype_from_id(WM_spacetype_enum_from_token(st));

  if (area->type == nullptr) {
    FormFactory(area->spacetype, UsdUITokens->spaceView3D);
    area->type = KKE_spacetype_from_id(WM_spacetype_enum_from_token(UsdUITokens->spaceView3D));
  }

  LISTBASE_FOREACH(ARegion *, region, &area->regions)
  {
    region->type = KKE_regiontype_from_id_or_first(area->type, region->regiontype);
  }

  /* area sizes */
  rcti gf_torcti;
  gf_torcti.xmin = window_rect.GetMinX();
  gf_torcti.xmax = window_rect.GetMaxX();
  gf_torcti.ymin = window_rect.GetMinY();
  gf_torcti.ymax = window_rect.GetMaxY();
  area_calc_totrct(area, &gf_torcti);

  /* region rect sizes */
  rcti rect = area->totrct;
  rcti overlap_rect = rect;
  // region_rect_recursive(area, area->regionbase.first, &rect, &overlap_rect, 0);
  // area->flag &= ~AREA_FLAG_REGION_SIZE_UPDATE;

  /* default area handlers */
  // ed_default_handlers(wm, area, NULL, &area->handlers, area->type->keymapflag);
  /* checks spacedata, adds own handlers */
  if (area->type->init) {
    area->type->init(wm, area);
  }

  /* clear all azones, add the area triangle widgets */
  // area_azone_init(win, screen, area);

  /* region windows, default and own handlers */
  LISTBASE_FOREACH(ARegion *, region, &area->regions)
  {
    // region_subwindow(region);

    if (region->visible) {
      /* default region handlers */
      // ed_default_handlers(wm, area, region, &region->handlers, region->type->keymapflag);
      /* own handlers */
      if (region->type->init) {
        region->type->init(wm, region);
      }
    } else {
      /* prevent uiblocks to run */
      UI_blocklist_free(NULL, region);
    }

    /* Some AZones use View2D data which is only updated in region init, so call that first! */
    // region_azones_add(screen, area, region);
  }

  /* Avoid re-initializing tools while resizing the window. */
  // if ((G.moving & G_TRANSFORM_WM) == 0) {
  //   TfToken st = FormFactory(area->spacetype);
  //   if ((1 << WM_spacetype_enum_from_token(st)) & WM_TOOLSYSTEM_SPACE_MASK) {
  //     // WM_toolsystem_refresh_screen_area(workspace, scene, view_layer, area);
  //     // area->flag |= AREA_FLAG_ACTIVE_TOOL_UPDATE;
  //   } else {
  //     // area->runtime.tool = NULL;
  //     // area->runtime.is_tool_set = true;
  //   }
  // }
}
