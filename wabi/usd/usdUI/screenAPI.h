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

#ifndef USDUI_GENERATED_SCREENAPI_H
#define USDUI_GENERATED_SCREENAPI_H

/**
 * @file usdUI/screenAPI.h */

#include "wabi/wabi.h"

#include "wabi/usd/usdUI/api.h"
#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usd/apiSchemaBase.h"
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
 * SCREENAPI                                                                 
 * --------------------------------------------------------------------------
 * 
 * @class UsdUIScreenAPI
 * 
 * A screen defines the layout of areas within a window. Typically a single
 * screen wil contain a layout of areas within a window - controlling each
 * area's individual size, and additional UI components such as statusbars
 * or whether an area should go fullscreen, hiding all other areas.
 * 
 * For any described attribute @em Fallback @em Value or @em Allowed
 * @em Values below that are text/tokens, the actual token is published
 * and defined in @ref UsdUITokens. So to set an attribute
 * to the value "rightHanded", use UsdUITokens->rightHanded
 * as the value.
 */

class UsdUIScreenAPI : public UsdAPISchemaBase {
 public:
  /**
   * Compile time constant representing what kind of schema this class is.
   *
   * @sa UsdSchemaKind */
  static const UsdSchemaKind schemaKind = UsdSchemaKind::SingleApplyAPI;

  /**
   * @deprecated
   * Same as schemaKind, provided to maintain temporary backward
   * compatibility with older generated schemas. */
  static const UsdSchemaKind schemaType = UsdSchemaKind::SingleApplyAPI;

  /**
   * Construct a UsdUIScreenAPI on UsdPrim @p prim . Equivalent to
   * UsdUIScreenAPI::Get(prim.GetStage(), prim.GetPath()) for a @em
   * valid @p prim, but will not immediately throw an error for an invalid
   * @p prim. */
  explicit UsdUIScreenAPI(const UsdPrim &prim = UsdPrim())
      : UsdAPISchemaBase(prim)
  {}

  /**
   * Construct a UsdUIScreenAPI on the prim held by @p schemaObj .
   * Should be preferred over UsdUIScreenAPI(schemaObj.GetPrim()),
   * as it preserves SchemaBase state. */
  explicit UsdUIScreenAPI(const UsdSchemaBase &schemaObj)
      : UsdAPISchemaBase(schemaObj)
  {}

  /**
   * Destructor. */
  USDUI_API
  virtual ~UsdUIScreenAPI();
 
  /**
   * Return a vector of names of all pre-declared attributes for this schema
   * class and all its ancestor classes.  Does not include attributes that
   * may be authored by custom/extended methods of the schemas involved. */
  USDUI_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /**
   * Return a UsdUIScreenAPI holding the prim adhering to this
   * schema at @p path on @p stage. If no prim exists at @p path on @p
   * stage, or if the prim at that path does not adhere to this schema
 
   * return an invalid schema object.  This is shorthand for the following:
   *
   * @code
   * UsdUIScreenAPI(stage->GetPrimAtPath(path));
   * @endcode */
  USDUI_API
  static UsdUIScreenAPI Get(const UsdStagePtr &stage, const SdfPath &path);


  /**
   * Applies this <b>single-apply</b> API schema to the given @p prim.
   * This information is stored by adding "ScreenAPI" to the
   * token-valued, listOp metadata @em apiSchemas on the prim.
   *
   * @return A valid UsdUIScreenAPI object is returned upon success.
   * An invalid (or empty) UsdUIScreenAPI object is returned upon
   * failure. See @ref UsdPrim::ApplyAPI() for conditions
   * resulting in failure.
   *
   * @sa UsdPrim::GetAppliedSchemas()
   * @sa UsdPrim::HasAPI()
   * @sa UsdPrim::ApplyAPI()
   * @sa UsdPrim::RemoveAPI() */
  USDUI_API
  static UsdUIScreenAPI Apply(const UsdPrim &prim);
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
   * POS
   * ---------------------------------------------------------------------
   * 
   * Position for a area in a screen. X is the window relative vertical
   * location of the area. Y is the window relative horizontal location
   * of the area.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float2 ui:screen:area:pos` |
   *
   * | C++ Type | GfVec2f |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float2 |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetPosAttr() const;

  /**
   * See GetPosAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreatePosAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * SIZE
   * ---------------------------------------------------------------------
   * 
   * Size for a area in a screen. X is the width. Y is the height.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float2 ui:screen:area:size` |
   *
   * | C++ Type | GfVec2f |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float2 |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetSizeAttr() const;

  /**
   * See GetSizeAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateSizeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * ICON
   * ---------------------------------------------------------------------
   * 
   * This points to an image that should be displayed on the area, or to
   * be shown next to the area name within a dropdown. It is intended to
   * be useful for summary visual classification of areas.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform asset ui:screen:area:icon` |
   *
   * | C++ Type | SdfAssetPath |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Asset |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetIconAttr() const;

  /**
   * See GetIconAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateIconAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * TYPE
   * ---------------------------------------------------------------------
   * 
   * The type classification is used to distinguish this area from other
   * areas within the DCC. As well as store various layouts and widgets
   * within this type.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:screen:area:type = "Empty"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | Empty, View3D, ImageEditor, NodeGraph, SequenceEditor, MovieEditor, DopeSheetEditor, TextEditor, Console, Info, TopBar, StatusBar, Outliner, Properties, FileBrowser, Spreadsheet, Preferences |
   */
  USDUI_API
  UsdAttribute GetTypeAttr() const;

  /**
   * See GetTypeAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateTypeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * NAME
   * ---------------------------------------------------------------------
   * 
   * The name used to label this area, displayed in headers or to simply
   * give a UI label to when selecting from a dropdown list.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:screen:area:name` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetNameAttr() const;

  /**
   * See GetNameAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateNameAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * SHOWMENUS
   * ---------------------------------------------------------------------
   * 
   * Whether or not to show clickable menus in the header.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform bool ui:screen:area:showMenus = 0` |
   *
   * | C++ Type | bool |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetShowMenusAttr() const;

  /**
   * See GetShowMenusAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateShowMenusAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * PURPOSE
   * ---------------------------------------------------------------------
   * 
   * Determine whether the area fills all available space, or is split
   * by an authored alignment to share it's own space with another area.  
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:screen:area:purpose = "area"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | area, region |
   */
  USDUI_API
  UsdAttribute GetPurposeAttr() const;

  /**
   * See GetPurposeAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreatePurposeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * LAYOUT
   * ---------------------------------------------------------------------
   * 
   * This authored attribute only has an effect if the area's purpose is
   * region. If it is set to region, it will split the area into a subdivided
   * area pertaining to this alignment.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:screen:area:layout = "none"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | none, top, bottom, left, right, horizontalSplit, verticalSplit |
   */
  USDUI_API
  UsdAttribute GetLayoutAttr() const;

  /**
   * See GetLayoutAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateLayoutAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * UISCREENAREAREGION
   * ---------------------------------------------------------------------
   * 
   * A region for this area, to have subdivided inner areas, for areas
   * whose purpose is region.
   * 
   */
  USDUI_API
  UsdRelationship GetUiScreenAreaRegionRel() const;

  /**
   * See GetUiScreenAreaRegionRel(), and also
   * @ref Usd_Create_Or_Get_Property for when to use
   * Get vs Create. */
  USDUI_API
  UsdRelationship CreateUiScreenAreaRegionRel() const;

 public:
  /**
   * ---------------------------------------------------------------------
   * WORKSPACE
   * ---------------------------------------------------------------------
   * 
   * A relationship between an area and a workspace, marking that this
   * area is apart of an workspace.
   * 
   */
  USDUI_API
  UsdRelationship GetWorkspaceRel() const;

  /**
   * See GetWorkspaceRel(), and also
   * @ref Usd_Create_Or_Get_Property for when to use
   * Get vs Create. */
  USDUI_API
  UsdRelationship CreateWorkspaceRel() const;

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
