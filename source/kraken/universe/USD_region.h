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

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "KLI_rhash.h"

#include "USD_space_types.h"

#ifdef __cplusplus
#  include "USD_context.h"
#  include "USD_screen.h"
#  include "USD_view2d.h"

#  include "KKE_context.h"
#  include "KKE_screen.h"
#  include <wabi/usd/usdUI/area.h>
#endif /* __cplusplus */

struct RHash;

struct ARegion_Runtime
{
  /* Panel category to use between 'layout' and 'draw'. */
  const char *category;

  /**
   * The visible part of the region, use with region overlap not to draw
   * on top of the overlapping regions.
   *
   * Lazy initialize, zero'd when unset, relative to #ARegion.winrct x/y min. */
  // GfVec4i visible_rect;

  /* The offset needed to not overlap with window scroll-bars. Only used by HUD regions for now. */
  int offset_x, offset_y;

  /* Maps uiBlock->name to uiBlock for faster lookups. */
  RHash *block_name_map;
};

#ifdef __cplusplus

struct ARegion : public wabi::UsdUIArea
{
  struct ARegion *next, *prev;

  wabi::SdfPath path;

  wabi::UsdAttribute name;
  wabi::UsdAttribute spacetype;
  wabi::UsdAttribute icon;
  wabi::UsdAttribute coords;
  wabi::UsdAttribute pos;
  wabi::UsdAttribute size;

  /** 2D-View scrolling/zoom info (most regions are 2d anyways). */
  View2D v2d;

  eRegionType regiontype;
  short flag;
  short alignment;

  /** Region is currently visible on screen. */
  short visible;

  struct ARegionType *type;

  void *regiondata;

  /** Private, cached notifier events. */
  short do_draw;
  /** Private, cached notifier events. */
  short do_draw_paintcursor;
  /** Private, set for indicate drawing overlapped. */
  short overlap;
  /** Temporary copy of flag settings for clean full-screen. */
  short flagfullscreen;

  /** Runtime for partial redraw, same or smaller than coords. */
  wabi::GfVec4i drawrct;

  ListBase panels;
  ListBase panels_category;

  std::vector<struct uiBlock *> uiblocks;
  ListBase handlers;

  ARegion_Runtime runtime;

  inline ARegion(kContext *C, kScreen *prim, const wabi::SdfPath &stagepath);
  inline ARegion(const wabi::UsdStageWeakPtr &S, kScreen *prim, const wabi::SdfPath &stagepath);
};

ARegion::ARegion(const wabi::UsdStageWeakPtr &S, kScreen *prim, const wabi::SdfPath &stagepath)
  : wabi::UsdUIArea(KRAKEN_KSTAGE_CREATE_CHILD(S)),
    path(GetPath()),
    name(CreateNameAttr()),
    spacetype(CreateSpacetypeAttr()),
    icon(CreateIconAttr()),
    coords(CreateCoordsAttr(DEFAULT_VEC4I(0, 0, 0, 0))),
    pos(CreatePosAttr()),
    size(CreateSizeAttr()),
    regiontype(RGN_TYPE_WINDOW),
    flag(VALUE_ZERO),
    alignment(VALUE_ZERO),
    type(POINTER_ZERO),
    regiondata(POINTER_ZERO),
    do_draw(0),
    do_draw_paintcursor(0),
    overlap(0),
    flagfullscreen(0),
    drawrct(0, 0, 0, 0)
{}

ARegion::ARegion(kContext *C, kScreen *prim, const wabi::SdfPath &stagepath)
  : wabi::UsdUIArea(KRAKEN_STAGE_CREATE_CHILD(C)),
    path(GetPath()),
    name(CreateNameAttr()),
    spacetype(CreateSpacetypeAttr()),
    icon(CreateIconAttr()),
    coords(CreateCoordsAttr(DEFAULT_VEC4I(0, 0, 0, 0))),
    pos(CreatePosAttr()),
    size(CreateSizeAttr()),
    regiontype(RGN_TYPE_WINDOW),
    flag(VALUE_ZERO),
    alignment(VALUE_ZERO),
    type(POINTER_ZERO),
    regiondata(POINTER_ZERO),
    do_draw(0),
    do_draw_paintcursor(0),
    overlap(0),
    flagfullscreen(0),
    drawrct(0, 0, 0, 0)
{}

#endif /* __cplusplus */