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

/* clang-format off */

#ifndef USDUI_GENERATED_WORKSPACE_H
#define USDUI_GENERATED_WORKSPACE_H

/**
 * @file usdUI/workspace.h */

#include "wabi/wabi.h"

#include "wabi/usd/usdUI/api.h"
#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usdUI/tokens.h"
#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

/**
 * --------------------------------------------------------------------------
 * WORKSPACE                                                                 
 * --------------------------------------------------------------------------
 * 
 * @class UsdUIWorkspace
 * 
 * A workspace acts like a predefined window layout. The purpose of a
 * workspace is to have a layout created around your needs, whether that
 * purpose might be modeling, animating, scripting, or any other purpose
 * you might find helpful for any specific task.
 * 
 * It is often useful to have many different kinds of workspaces so you
 * can quickly switch between layouts based on your needs and contextual
 * task at hand.
 * 
 * For any described attribute @em Fallback @em Value or @em Allowed
 * @em Values below that are text/tokens, the actual token is published
 * and defined in @ref UsdUITokens. So to set an attribute
 * to the value "rightHanded", use UsdUITokens->rightHanded
 * as the value.
 */

class UsdUIWorkspace : public UsdTyped {
 public:
  /**
   * Compile time constant representing what kind of schema this class is.
   *
   * @sa UsdSchemaKind */
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

  /**
   * @deprecated
   * Same as schemaKind, provided to maintain temporary backward
   * compatibility with older generated schemas. */
  static const UsdSchemaKind schemaType = UsdSchemaKind::ConcreteTyped;

  /**
   * Construct a UsdUIWorkspace on UsdPrim @p prim . Equivalent to
   * UsdUIWorkspace::Get(prim.GetStage(), prim.GetPath()) for a @em
   * valid @p prim, but will not immediately throw an error for an invalid
   * @p prim. */
  explicit UsdUIWorkspace(const UsdPrim &prim = UsdPrim())
      : UsdTyped(prim)
  {}

  /**
   * Construct a UsdUIWorkspace on the prim held by @p schemaObj .
   * Should be preferred over UsdUIWorkspace(schemaObj.GetPrim()),
   * as it preserves SchemaBase state. */
  explicit UsdUIWorkspace(const UsdSchemaBase &schemaObj)
      : UsdTyped(schemaObj)
  {}

  /**
   * Destructor. */
  USDUI_API
  virtual ~UsdUIWorkspace();
 
  /**
   * Return a vector of names of all pre-declared attributes for this schema
   * class and all its ancestor classes.  Does not include attributes that
   * may be authored by custom/extended methods of the schemas involved. */
  USDUI_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /**
   * Return a UsdUIWorkspace holding the prim adhering to this
   * schema at @p path on @p stage. If no prim exists at @p path on @p
   * stage, or if the prim at that path does not adhere to this schema
 
   * return an invalid schema object.  This is shorthand for the following:
   *
   * @code
   * UsdUIWorkspace(stage->GetPrimAtPath(path));
   * @endcode */
  USDUI_API
  static UsdUIWorkspace Get(const UsdStagePtr &stage, const SdfPath &path);

  /**
   * Attempt to ensure a @a UsdPrim adhering to this schema at @p
   * path is defined (according to UsdPrim::IsDefined()) on this
   * stage.
   *
   * If a prim adhering to this schema at @p path is already defined
   * on this stage, return that prim. Otherwise author an @a SdfPrimSpec
   * with @a specifier == @a SdfSpecifierDef and this schema's prim type
   * name for the prim at @p path at the current EditTarget. Author @a
   * SdfPrimSpec with @p specifier == @a SdfSpecifierDef and empty typeName
   * at the current EditTarget for any nonexistent, or existing but not @a
   * Defined ancestors.
   *
   * The given @a path must be an absolute prim path that does not contain
   * any variant selections.
   *
   * If it is impossible to author any of the necessary PrimSpecs,
   * (for example, in case @a path cannot map to the current UsdEditTarget's
   * namespace) issue an error and return an invalid @a UsdPrim.
   *
   * Note that this method may return a defined prim whose typeName does not
   * specify this schema class, in case a stronger typeName opinion overrides
   * the opinion at the current EditTarget. */
  USDUI_API
  static UsdUIWorkspace
  Define(const UsdStagePtr &stage, const SdfPath &path);
 protected:
  /**
   * Returns the kind of schema this class belongs to.
   *
   * @sa UsdSchemaKind */
  USDUI_API
  UsdSchemaKind _GetSchemaKind() const override;

  /**
   * @deprecated
   * Same as _GetSchemaKind, provided to maintain temporary backward
   * compatibility with older generated schemas. */
  USDUI_API
  UsdSchemaKind _GetSchemaType() const override;

 private:
  /* needs to invoke _GetStaticTfType. */
  friend class UsdSchemaRegistry;

  USDUI_API
  static const TfType &_GetStaticTfType();

  static bool _IsTypedSchema();

  /* override SchemaBase virtuals. */
  USDUI_API
  const TfType &_GetTfType() const override;

 public:
  /**
   * ---------------------------------------------------------------------
   * WORKSPACENAME
   * ---------------------------------------------------------------------
   * 
   * The workspace name that is displayed in a application's GUI.
   * Usually laid out within a quickly accessible tab selection
   * grouped with other existing workspaces, to quickly switch 
   * around.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:workspaceName` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetWorkspaceNameAttr() const;

  /**
   * See GetWorkspaceNameAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateWorkspaceNameAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ======================================================================
   *   Feel free to add custom code below this line. It will be preserved
   *   by the code generator.
   *
   *   Just remember to:
   *     - Close the class declaration with };
   *
   *     - Close the namespace with WABI_NAMESPACE_END
   *
   *     - Close the include guard with #endif
   * ======================================================================
   * --(BEGIN CUSTOM CODE)-- */
};

WABI_NAMESPACE_END

#endif
