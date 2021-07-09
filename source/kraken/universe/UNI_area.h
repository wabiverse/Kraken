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

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "UNI_context.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"

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


struct ScrArea : public UsdUIArea, public UniverseObject
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

  std::vector<ARegion *> regions;

  inline ScrArea(cContext *C, cScreen *prim, const SdfPath &stagepath);
};

ScrArea::ScrArea(cContext *C, cScreen *prim, const SdfPath &stagepath)
  : UsdUIArea(KRAKEN_UNIVERSE_CREATE_CHILD(C)),
    areaid(VALUE_ZERO),
    path(UsdUIArea::GetPath()),
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
    global(POINTER_ZERO)
{}

WABI_NAMESPACE_END