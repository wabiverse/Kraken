/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */

#include "wabi/usd/usdGeom/grid.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

/**
 * Register the schema with the TfType system. */
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomGrid, TfType::Bases<UsdGeomMesh>>();

  /**
   * Register the usd prim typename as an alias under UsdSchemaBase. This
   * enables one to call
   * TfType::Find<UsdSchemaBase>().FindDerivedByName("Grid")
   * to find TfType<UsdGeomGrid>, which is how IsA queries are
   * answered. */
  TfType::AddAlias<UsdSchemaBase, UsdGeomGrid>("Grid");
}

/* static */
UsdGeomGrid UsdGeomGrid::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomGrid();
  }
  return UsdGeomGrid(stage->GetPrimAtPath(path));
}

/* static */
UsdGeomGrid UsdGeomGrid::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("Grid");
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomGrid();
  }
  return UsdGeomGrid(stage->DefinePrim(path, usdPrimTypeName));
}

const static VtIntArray vertex_counts({1, 4});

const static VtIntArray vertex_indices({0, 1, 2, 3});

const static VtVec3fArray normals({GfVec3f(0, 0, 1)});

const static VtVec3fArray points(
    {GfVec3f(1, -1, 0), GfVec3f(1, 1, 0), GfVec3f(-1, 1, 0), GfVec3f(-1, -1, 0)});

UsdGeomGrid::UsdGeomGrid(const UsdPrim &prim) : UsdGeomMesh(prim)
{
  CreateFaceVertexCountsAttr(VtValue(vertex_counts));
  CreateFaceVertexIndicesAttr(VtValue(vertex_indices));
  CreateNormalsAttr(VtValue(normals));
  CreatePointsAttr(VtValue(points));
}

UsdGeomGrid::UsdGeomGrid(const UsdSchemaBase &schemaObj) : UsdGeomMesh(schemaObj)
{
  CreateFaceVertexCountsAttr(VtValue(vertex_counts));
  CreateFaceVertexIndicesAttr(VtValue(vertex_indices));
  CreateNormalsAttr(VtValue(normals));
  CreatePointsAttr(VtValue(points));
}

/* virtual */
UsdGeomGrid::~UsdGeomGrid()
{}

WABI_NAMESPACE_END
