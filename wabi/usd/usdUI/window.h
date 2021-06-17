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

#ifndef USDUI_GENERATED_WINDOW_H
#define USDUI_GENERATED_WINDOW_H

/**
 * @file usdUI/window.h */

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
 * WINDOW                                                                    
 * --------------------------------------------------------------------------
 * 
 * @class UsdUIWindow
 * 
 * Provides a window for the purpose of displaying an application's GUI.
 * 
 * For any described attribute @em Fallback @em Value or @em Allowed
 * @em Values below that are text/tokens, the actual token is published
 * and defined in @ref UsdUITokens. So to set an attribute
 * to the value "rightHanded", use UsdUITokens->rightHanded
 * as the value.
 */

class UsdUIWindow : public UsdTyped {
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
   * Construct a UsdUIWindow on UsdPrim @p prim . Equivalent to
   * UsdUIWindow::Get(prim.GetStage(), prim.GetPath()) for a @em
   * valid @p prim, but will not immediately throw an error for an invalid
   * @p prim. */
  explicit UsdUIWindow(const UsdPrim &prim = UsdPrim())
      : UsdTyped(prim)
  {}

  /**
   * Construct a UsdUIWindow on the prim held by @p schemaObj .
   * Should be preferred over UsdUIWindow(schemaObj.GetPrim()),
   * as it preserves SchemaBase state. */
  explicit UsdUIWindow(const UsdSchemaBase &schemaObj)
      : UsdTyped(schemaObj)
  {}

  /**
   * Destructor. */
  USDUI_API
  virtual ~UsdUIWindow();
 
  /**
   * Return a vector of names of all pre-declared attributes for this schema
   * class and all its ancestor classes.  Does not include attributes that
   * may be authored by custom/extended methods of the schemas involved. */
  USDUI_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /**
   * Return a UsdUIWindow holding the prim adhering to this
   * schema at @p path on @p stage. If no prim exists at @p path on @p
   * stage, or if the prim at that path does not adhere to this schema
 
   * return an invalid schema object.  This is shorthand for the following:
   *
   * @code
   * UsdUIWindow(stage->GetPrimAtPath(path));
   * @endcode */
  USDUI_API
  static UsdUIWindow Get(const UsdStagePtr &stage, const SdfPath &path);

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
  static UsdUIWindow
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
   * TITLE
   * ---------------------------------------------------------------------
   * 
   * The window title that is displayed on the top of the window. This
   * window title gives context for what this window is used for.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:window:title` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetTitleAttr() const;

  /**
   * See GetTitleAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateTitleAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * ICON
   * ---------------------------------------------------------------------
   * 
   * This points to an image that should be displayed on the window's,
   * top most region where the title is.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform asset ui:window:icon` |
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
   * The window type will determine whether it is apart of the main
   * central window instance, or whether it is a detached, seperate
   * window instance, or if it is a temporary dialog box.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:window:type = "normal"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | normal, detached, dialog |
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
   * STATE
   * ---------------------------------------------------------------------
   * 
   * The current window state as it pertains to
   * normal, maximized, minimized, fullscreen, or
   * embedded.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:window:state = "normal"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | normal, maximized, minimized, fullscreen, embedded |
   */
  USDUI_API
  UsdAttribute GetStateAttr() const;

  /**
   * See GetStateAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateStateAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * DPI
   * ---------------------------------------------------------------------
   * 
   * The current window DPI value
   * for better interface drawing.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:dpi = 1` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetDpiAttr() const;

  /**
   * See GetDpiAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateDpiAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * DPIFAC
   * ---------------------------------------------------------------------
   * 
   * The user interface dpi multiplier
   * to scale UI elements based on the
   * DPI.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:dpifac = 1` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetDpifacAttr() const;

  /**
   * See GetDpifacAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateDpifacAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * SCALE
   * ---------------------------------------------------------------------
   * 
   * The user interface scale, scaling
   * all UI components to this value.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:scale = 1` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetScaleAttr() const;

  /**
   * See GetScaleAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateScaleAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * PIXELSZ
   * ---------------------------------------------------------------------
   * 
   * The user interface pixel size,
   * scaling the pixel size for all
   * UI components in the window.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:pixelsz = 1` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetPixelszAttr() const;

  /**
   * See GetPixelszAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreatePixelszAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * WIDGETUNIT
   * ---------------------------------------------------------------------
   * 
   * Additional adjustments to optimize
   * widget displays based on DPI settings.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:widgetunit = 20` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetWidgetunitAttr() const;

  /**
   * See GetWidgetunitAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateWidgetunitAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * LINEWIDTH
   * ---------------------------------------------------------------------
   * 
   * The user interface line width,
   * scaling the width of borders.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float ui:window:linewidth = 1` |
   *
   * | C++ Type | float |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   */
  USDUI_API
  UsdAttribute GetLinewidthAttr() const;

  /**
   * See GetLinewidthAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateLinewidthAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * CURSOR
   * ---------------------------------------------------------------------
   * 
   * The current window cursor type.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:window:cursor = "default"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | default, textEdit, wait, stop, edit, copy, hand, cross, paint, dot, knife, vloop, paintBrush, eraser, eyedropper, swapArea, xMove, yMove, hSplit, vSplit, zoomIn, zoomOut |
   */
  USDUI_API
  UsdAttribute GetCursorAttr() const;

  /**
   * See GetCursorAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateCursorAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * ALIGNMENT
   * ---------------------------------------------------------------------
   * 
   * Alignment for a window in a monitor. To control it's
   * position within the display, whether it be absolutley
   * positioned, centered, or centered to it's parent window.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform token ui:window:alignment = "alignAbsolute"` |
   *
   * | C++ Type | TfToken |
   *
   * | @ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
   *
   * | @ref SdfVariability "Variability" | SdfVariabilityUniform |
   *
   * | @ref UsdUITokens "Allowed Values" | alignAbsolute, alignCenter, alignParent |
   */
  USDUI_API
  UsdAttribute GetAlignmentAttr() const;

  /**
   * See GetAlignmentAttr(), and also @ref
   * Usd_Create_Or_Get_Property for when to use Get vs Create.
   * If specified, author @p defaultValue as the attribute's
   * default, sparsely (when it makes sense to do so) if @p
   * writeSparsely is @c true, the default for @p writeSparsely
   * is @c false. */
  USDUI_API
  UsdAttribute CreateAlignmentAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  /**
   * ---------------------------------------------------------------------
   * POS
   * ---------------------------------------------------------------------
   * 
   * Position for a window in a monitor. X is the monitor's relative
   * vertical location of the window. Y is the monitor's relative
   * horizontal location of the window.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float2 ui:window:pos` |
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
   * Size for a window in a monitor. X is the width. Y is the height.
   * 
   *
   *
   * | ||
   * | -- | -- |
   *
   * | Declaration | `uniform float2 ui:window:size` |
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
   * UIWINDOWWORKSPACE
   * ---------------------------------------------------------------------
   * 
   * The active workspace to be presented within this window.
   * 
   */
  USDUI_API
  UsdRelationship GetUiWindowWorkspaceRel() const;

  /**
   * See GetUiWindowWorkspaceRel(), and also
   * @ref Usd_Create_Or_Get_Property for when to use
   * Get vs Create. */
  USDUI_API
  UsdRelationship CreateUiWindowWorkspaceRel() const;

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
