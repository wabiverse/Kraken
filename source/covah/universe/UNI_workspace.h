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
#include "UNI_screen.h"
#include "UNI_window.h"

#include "CKE_context.h"

#include <wabi/usd/usdUI/workspace.h>

WABI_NAMESPACE_BEGIN

struct WorkSpaceLayout
{
  cScreen *screen;
  TfToken name;
};

typedef std::vector<WorkSpaceLayout *> WorkSpaceLayoutVector;

struct WorkSpaceInstanceHook
{
  WorkSpace *active;
  WorkSpaceLayout *act_layout;

  WorkSpace *temp_workspace_store;
  WorkSpaceLayout *temp_layout_store;
};

struct WorkSpaceDataRelation
{
  WorkSpaceInstanceHook *parent;
  WorkSpaceLayout *value;

  SdfPath parentid;
};

typedef std::vector<WorkSpaceDataRelation *> WorkSpaceDataRelationVector;

struct WorkSpace : public UsdUIWorkspace, public UniverseObject
{

  SdfPath path;

  UsdAttribute name;
  UsdRelationship screen_rel;

  WorkSpaceLayoutVector layouts;

  WorkSpaceDataRelationVector hook_layout_relations;

  inline WorkSpace(cContext *C,
                   const SdfPath &stagepath,
                   const TfToken &title = TfToken("Workspace"));
};

WorkSpace::WorkSpace(cContext *C, const SdfPath &stagepath, const TfToken &title)
  : UsdUIWorkspace(COVAH_UNIVERSE_CREATE(C)),
    path(stagepath),
    name(CreateNameAttr(VtValue(title))),
    screen_rel(CreateScreenRel())
{}

WABI_NAMESPACE_END