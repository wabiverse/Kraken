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

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_region.h"

#include "KKE_context.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include <wabi/base/gf/vec2i.h>
#include <wabi/usd/usdUI/screen.h>

WABI_NAMESPACE_BEGIN

#define AREAMAP_FROM_SCREEN(screen) ((ScrAreaMap *)&(screen)->verts)

enum eScreenRedrawsFlag
{
  TIME_REGION = (1 << 0),
  TIME_ALL_3D_WIN = (1 << 1),
  TIME_ALL_ANIM_WIN = (1 << 2),
  TIME_ALL_BUTS_WIN = (1 << 3),
  TIME_SEQ = (1 << 4),
  TIME_ALL_IMAGE_WIN = (1 << 5),
  TIME_NODES = (1 << 6),
  TIME_CLIPS = (1 << 7),

  TIME_FOLLOW = (1 << 15),
};

struct ScrVert
{
  GfVec2h vec;
  /* first one used internally, second one for tools */
  short flag, editflag;

  ScrVert()
    : vec(VALUE_ZERO),
      flag(VALUE_ZERO),
      editflag(VALUE_ZERO)
  {}
};

struct ScrEdge
{
  ScrVert *v1, *v2;
  /** 1 when at edge of screen. */
  short border;
  short flag;

  ScrEdge()
    : v1(POINTER_ZERO),
      v2(POINTER_ZERO),
      border(VALUE_ZERO),
      flag(VALUE_ZERO)
  {}
};

struct cScreen : public UsdUIScreen, public UniverseObject
{
  SdfPath path;

  UsdAttribute align;
  UsdRelationship areas_rel;

  std::vector<ScrVert *> verts;
  std::vector<ScrEdge *> edges;
  std::vector<ScrArea *> areas;

  ARegion *active_region;

  /** Runtime */
  short redraws_flag;

  char temp;
  int winid;
  char do_refresh;


  inline cScreen(cContext *C, const SdfPath &stagepath);
};

cScreen::cScreen(cContext *C, const SdfPath &stagepath)
  : UsdUIScreen(KRAKEN_UNIVERSE_CREATE(C)),
    path(stagepath),
    align(CreateAlignmentAttr(DEFAULT_VALUE(UsdUITokens->none))),
    areas_rel(CreateAreasRel()),
    verts(EMPTY),
    areas(EMPTY),
    active_region(POINTER_ZERO),
    redraws_flag(VALUE_ZERO),
    temp(VALUE_ZERO),
    winid(VALUE_ZERO),
    do_refresh(VALUE_ZERO)
{}


struct wmRegionMessageSubscribeParams
{
  const struct cContext *context;
  struct wmMsgBus *message_bus;
  struct WorkSpace *workspace;
  struct Scene *scene;
  struct cScreen *screen;
  struct ScrArea *area;
  struct ARegion *region;

  wmRegionMessageSubscribeParams()
    : context(POINTER_ZERO),
      message_bus(POINTER_ZERO),
      workspace(POINTER_ZERO),
      scene(POINTER_ZERO),
      screen(POINTER_ZERO),
      area(POINTER_ZERO),
      region(POINTER_ZERO)
  {}
};


WABI_NAMESPACE_END