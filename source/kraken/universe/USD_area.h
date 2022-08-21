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
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"

#include "KLI_icons.h"

#include "KKE_context.h"
#include "KKE_screen.h"

#include <wabi/usd/usdUI/area.h>

WABI_NAMESPACE_BEGIN


enum GlobalAreaAlign
{
  GLOBAL_AREA_ALIGN_TOP = 0,
  GLOBAL_AREA_ALIGN_BOTTOM = 1,
};

enum GlobalAreaFlag
{
  GLOBAL_AREA_IS_HIDDEN = (1 << 0),
};

struct ScrGlobalAreaData
{
  short cur_fixed_height;
  short size_min, size_max;
  GlobalAreaAlign align;
  GlobalAreaFlag flag;

  ScrGlobalAreaData()
    : cur_fixed_height(VALUE_ZERO),
      size_min(VALUE_ZERO),
      size_max(VALUE_ZERO),
      align(GLOBAL_AREA_ALIGN_TOP),
      flag(GLOBAL_AREA_IS_HIDDEN)
  {}
};


struct ScrArea : UsdUIArea
{
  int areaid;
  SdfPath path;

  ScrVert *v1, *v2, *v3, *v4;

  std::vector<SpaceLink *> spacedata;

  UsdAttribute name;
  UsdAttribute spacetype;
  UsdAttribute icon;
  UsdAttribute pos;
  UsdAttribute size;

  struct SpaceType *type;
  ScrGlobalAreaData *global;

  /** Private, for spacetype refresh callback. */
  bool do_refresh;

  /**
   * Index of last used region of 'RGN_TYPE_WINDOW'
   * runtime variable, updated by executing operators. */
  short region_active_win;

  std::vector<ARegion *> regions;

  inline ScrArea(kContext *C, kScreen *prim, const SdfPath &stagepath);
};

ScrArea::ScrArea(kContext *C, kScreen *prim, const SdfPath &stagepath)
  : UsdUIArea(KRAKEN_STAGE_CREATE_CHILD(C)),
    areaid(VALUE_ZERO),
    path(GetPath()),
    v1(POINTER_ZERO),
    v2(POINTER_ZERO),
    v3(POINTER_ZERO),
    v4(POINTER_ZERO),
    name(CreateNameAttr(DEFAULT_TOKEN("Area"))),
    spacetype(CreateSpacetypeAttr(DEFAULT_VALUE(UsdUITokens->spaceEmpty))),
    icon(CreateIconAttr(DEFAULT_ASSET(KLI_icon(ICON_KRAKEN)))),
    pos(CreatePosAttr(DEFAULT_VEC2F(0, 0))),
    size(CreateSizeAttr(DEFAULT_VEC2F(1, 1))),
    type(POINTER_ZERO),
    global(POINTER_ZERO),
    do_refresh(VALUE_ZERO),
    region_active_win(VALUE_ZERO),
    regions(EMPTY)
{}

WABI_NAMESPACE_END