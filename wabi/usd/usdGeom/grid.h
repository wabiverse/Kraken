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

#ifndef USDGEOM_GRID_H
#define USDGEOM_GRID_H

/**
 * @file usdGeom/grid.h */

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/usd/usdGeom/mesh.h"
#include "wabi/usd/usdGeom/tokens.h"
#include "wabi/wabi.h"

#include "wabi/usd/usd/timeCode.h"

#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

class UsdGeomGrid : public UsdGeomMesh {
 public:
  /**
   * Compile time constant representing what kind of schema this class is.
   *
   * @sa UsdSchemaKind */
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

  /**
   * Construct a UsdGeomGrid on UsdPrim @p prim.
   * Equivalent to UsdGeomGrid::Get(prim.GetStage(), prim.GetPath())
   * for a @em valid @p prim, but will not immediately throw an error for
   * an invalid @p prim. */
  USDGEOM_API
  explicit UsdGeomGrid(const UsdPrim &prim = UsdPrim());

  /**
   * Construct a UsdGeomGrid on the prim held by @p schemaObj.
   * Should be preferred over UsdGeomGrid(schemaObj.GetPrim()),
   * as it preserves SchemaBase state. */
  USDGEOM_API
  explicit UsdGeomGrid(const UsdSchemaBase &schemaObj);

  /**
   * Return a UsdGeomGrid holding the prim adhering to this
   * schema at @p path on @p stage. If no prim exists at @p path on
   * @p stage, or if the prim at that path does not adhere to this schema,
   * return an invalid schema object. This is shorthand for the following:
   *
   * @code
   * UsdGeomGrid(stage->GetPrimAtPath(path));
   * @endcode */
  USDGEOM_API
  static UsdGeomGrid Get(const UsdStagePtr &stage, const SdfPath &path);

  /**
   * Attempt to ensure a @a UsdPrim adhering to this schema at @p path
   * is defined (according to UsdPrim::IsDefined()) on this stage.
   *
   * If a prim adhering to this schema at @p path is already defined on this
   * stage, return that prim.  Otherwise author an @a SdfPrimSpec with
   * @a specifier == @a SdfSpecifierDef and this schema's prim type name for
   * the prim at @p path at the current EditTarget. Author @a SdfPrimSpec with
   * @p specifier == @a SdfSpecifierDef and empty typeName at the current
   * EditTarget for any nonexistent, or existing but not @a Defined ancestors.
   *
   * The given @a path must be an absolute prim path that does not contain
   * any variant selections.
   *
   * If it is impossible to author any of the necessary PrimSpecs, (for
   * example, in case @a path cannot map to the current UsdEditTarget's
   * namespace) issue an error and return an invalid @a UsdPrim.
   *
   * Note that this method may return a defined prim whose typeName does not
   * specify this schema class, in case a stronger typeName opinion overrides
   * the opinion at the current EditTarget. */
  USDGEOM_API
  static UsdGeomGrid Define(const UsdStagePtr &stage, const SdfPath &path);

  /**
   * Destructor. */
  USDGEOM_API
  virtual ~UsdGeomGrid();
};

WABI_NAMESPACE_END

#endif /* USDGEOM_GRID_H */
