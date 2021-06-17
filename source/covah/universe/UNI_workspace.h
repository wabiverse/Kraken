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
#include "UNI_object.h"

#include "CKE_context.h"

#include <wabi/usd/usdUI/workspace.h>

WABI_NAMESPACE_BEGIN

struct CovahWorkspace : public UsdUIWorkspace, public CovahObject {

  SdfPath path;

  UsdAttribute name;
  UsdRelationship screen;

  inline CovahWorkspace(cContext C, const SdfPath &stagepath, const TfToken &title = TfToken("Workspace"));
};

CovahWorkspace::CovahWorkspace(cContext C, const SdfPath &stagepath, const TfToken &title)
  : UsdUIWorkspace(COVAH_UNIVERSE_CREATE(C)),
    path(stagepath),
    name(CreateNameAttr(VtValue(title))),
    screen(CreateScreenRel())
{}

WABI_NAMESPACE_END