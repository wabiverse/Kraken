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
#include "UNI_screen.h"

#include "CKE_context.h"

#include <wabi/usd/usdUI/area.h>

WABI_NAMESPACE_BEGIN

struct CovahArea : public UsdUIArea, public CovahObject {

  SdfPath path;

  UsdAttribute name;
  UsdAttribute icon;
  UsdAttribute pos;
  UsdAttribute size;

  inline CovahArea(cContext &C, cScreen &prim, SdfPath const &stagepath);
};

CovahArea::CovahArea(cContext &C, cScreen &prim, SdfPath const &stagepath)
  : UsdUIArea(COVAH_UNIVERSE_CREATE_CHILD(C)),
    path(GetPath()),
    name(CreateNameAttr()),
    icon(CreateIconAttr()),
    pos(CreatePosAttr()),
    size(CreateSizeAttr())
{}

WABI_NAMESPACE_END