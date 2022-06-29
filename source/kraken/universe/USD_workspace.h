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

#include "USD_context.h"
#include "USD_object.h"
#include "USD_screen.h"
#include "USD_window.h"

#include "KKE_context.h"

// #include <wabi/usd/usdUI/workspace.h>

WABI_NAMESPACE_BEGIN

struct WorkSpaceLayout
{
  kScreen *screen;
  TfToken name;

  WorkSpaceLayout() : screen(POINTER_ZERO), name(EMPTY) {}
};

typedef std::vector<WorkSpaceLayout *> WorkSpaceLayoutVector;

struct WorkSpaceInstanceHook
{
  WorkSpace *active;
  WorkSpaceLayout *act_layout;

  WorkSpace *temp_workspace_store;
  WorkSpaceLayout *temp_layout_store;

  WorkSpaceInstanceHook()
    : active(POINTER_ZERO),
      act_layout(POINTER_ZERO),
      temp_workspace_store(POINTER_ZERO),
      temp_layout_store(POINTER_ZERO)
  {}
};

struct WorkSpaceDataRelation
{
  WorkSpaceInstanceHook *parent;
  WorkSpaceLayout *value;

  int parentid;

  WorkSpaceDataRelation() : parent(POINTER_ZERO), value(POINTER_ZERO), parentid(VALUE_ZERO) {}
};

typedef std::vector<WorkSpaceDataRelation *> WorkSpaceDataRelationVector;

struct WorkSpace : public UsdPrim
{
  SdfPath path;

  UsdAttribute name;
  UsdRelationship screen_rel;

  WorkSpaceLayoutVector layouts;

  WorkSpaceDataRelationVector hook_layout_relations;

  inline WorkSpace(kContext *C, const SdfPath &stagepath);
};

WorkSpace::WorkSpace(kContext *C, const SdfPath &stagepath)
  : UsdPrim(),
    path(stagepath),
    name(EMPTY /*CreateNameAttr(DEFAULT_TOKEN("Workspace"))*/),
    screen_rel(EMPTY /*CreateScreenRel()*/),
    layouts(EMPTY),
    hook_layout_relations(EMPTY)
{}

WABI_NAMESPACE_END