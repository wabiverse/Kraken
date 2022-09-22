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

#include "USD_context.h"
#include "USD_screen.h"

#include "KKE_context.h"
#include "KKE_screen.h"

#include <wabi/usd/usdUI/area.h>

KRAKEN_NAMESPACE_BEGIN

enum eRegionType
{
  RGN_TYPE_WINDOW = 0,
  RGN_TYPE_HEADER = 1,
  RGN_TYPE_CHANNELS = 2,
  RGN_TYPE_TEMPORARY = 3,
  RGN_TYPE_UI = 4,
  RGN_TYPE_TOOLS = 5,
  RGN_TYPE_TOOL_PROPS = 6,
  RGN_TYPE_PREVIEW = 7,
  RGN_TYPE_HUD = 8,
  /* Region to navigate the main region from (RGN_TYPE_WINDOW). */
  RGN_TYPE_NAV_BAR = 9,
  /* A place for buttons to trigger execution of something that was set up in other regions. */
  RGN_TYPE_EXECUTE = 10,
  RGN_TYPE_FOOTER = 11,
  RGN_TYPE_TOOL_HEADER = 12,

#define RGN_TYPE_LEN (RGN_TYPE_TOOL_HEADER + 1)
};

#define RGN_TYPE_IS_HEADER_ANY(regiontype) \
  (((1 << (regiontype)) &                  \
    ((1 << RGN_TYPE_HEADER) | 1 << (RGN_TYPE_TOOL_HEADER) | (1 << RGN_TYPE_FOOTER))) != 0)


#define RGN_ALIGN_ENUM_FROM_MASK(align) ((align) & ((1 << 4) - 1))
#define RGN_ALIGN_FLAG_FROM_MASK(align) ((align) & ~((1 << 4) - 1))

enum
{
  RGN_FLAG_HIDDEN = (1 << 0),
  RGN_FLAG_TOO_SMALL = (1 << 1),
  RGN_FLAG_DYNAMIC_SIZE = (1 << 2),
  RGN_FLAG_TEMP_REGIONDATA = (1 << 3),
  RGN_FLAG_PREFSIZE_OR_HIDDEN = (1 << 4),
  RGN_FLAG_SIZE_CLAMP_X = (1 << 5),
  RGN_FLAG_SIZE_CLAMP_Y = (1 << 6),
  RGN_FLAG_HIDDEN_BY_USER = (1 << 7),
  RGN_FLAG_SEARCH_FILTER_ACTIVE = (1 << 8),
  RGN_FLAG_SEARCH_FILTER_UPDATE = (1 << 9),
};

struct ARegion_Runtime
{
  /* Panel category to use between 'layout' and 'draw'. */
  const char *category;

  /**
   * The visible part of the region, use with region overlap not to draw
   * on top of the overlapping regions.
   *
   * Lazy initialize, zero'd when unset, relative to #ARegion.winrct x/y min. */
  wabi::GfVec4i visible_rect;

  /* The offset needed to not overlap with window scroll-bars. Only used by HUD regions for now. */
  int offset_x, offset_y;

  /* Maps uiBlock->name to uiBlock for faster lookups. */
  struct RHash *block_name_map;
};

struct ARegion : public UsdUIArea
{
  SdfPath path;

  UsdAttribute name;
  UsdAttribute spacetype;
  UsdAttribute icon;
  UsdAttribute coords;
  UsdAttribute pos;
  UsdAttribute size;

  eRegionType regiontype;
  short flag;
  short alignment;

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

  std::vector<struct uiBlock *> uiblocks;

  ARegion_Runtime runtime;

  inline ARegion(kContext *C, kScreen *prim, const SdfPath &stagepath);
};

ARegion::ARegion(kContext *C, kScreen *prim, const SdfPath &stagepath)
  : UsdUIArea(KRAKEN_STAGE_CREATE_CHILD(C)),
    path(GetPath()),
    name(CreateNameAttr()),
    spacetype(CreateSpacetypeAttr()),
    icon(CreateIconAttr()),
    coords(CreatePosAttr(DEFAULT_VEC4I(0, 0, 0, 0))),
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

KRAKEN_NAMESPACE_END