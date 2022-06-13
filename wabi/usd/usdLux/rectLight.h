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
#ifndef USDLUX_GENERATED_RECTLIGHT_H
#define USDLUX_GENERATED_RECTLIGHT_H

/// \file usdLux/rectLight.h

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdLux/api.h"
#include "wabi/usd/usdLux/light.h"
#include "wabi/usd/usdLux/tokens.h"
#include "wabi/wabi.h"

#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// RECTLIGHT                                                                  //
// -------------------------------------------------------------------------- //

/// \class UsdLuxRectLight
///
/// Light emitted from one side of a rectangle.
/// The rectangle is centered in the XY plane and emits light along the -Z axis.
/// The rectangle is 1 unit in length in the X and Y axis.  In the default
/// position, a texture file's min coordinates should be at (+X, +Y) and
/// max coordinates at (-X, -Y).
///
class UsdLuxRectLight : public UsdLuxLight
{
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;


  /// Construct a UsdLuxRectLight on UsdPrim \p prim .
  /// Equivalent to UsdLuxRectLight::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdLuxRectLight(const UsdPrim &prim = UsdPrim())
    : UsdLuxLight(prim)
  {}

  /// Construct a UsdLuxRectLight on the prim held by \p schemaObj .
  /// Should be preferred over UsdLuxRectLight(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdLuxRectLight(const UsdSchemaBase &schemaObj)
    : UsdLuxLight(schemaObj)
  {}

  /// Destructor.
  USDLUX_API
  virtual ~UsdLuxRectLight();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDLUX_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdLuxRectLight holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdLuxRectLight(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDLUX_API
  static UsdLuxRectLight Get(const UsdStagePtr &stage, const SdfPath &path);

  /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
  /// is defined (according to UsdPrim::IsDefined()) on this stage.
  ///
  /// If a prim adhering to this schema at \p path is already defined on this
  /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
  /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
  /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
  /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
  /// current EditTarget for any nonexistent, or existing but not \a Defined
  /// ancestors.
  ///
  /// The given \a path must be an absolute prim path that does not contain
  /// any variant selections.
  ///
  /// If it is impossible to author any of the necessary PrimSpecs, (for
  /// example, in case \a path cannot map to the current UsdEditTarget's
  /// namespace) issue an error and return an invalid \a UsdPrim.
  ///
  /// Note that this method may return a defined prim whose typeName does not
  /// specify this schema class, in case a stronger typeName opinion overrides
  /// the opinion at the current EditTarget.
  ///
  USDLUX_API
  static UsdLuxRectLight Define(const UsdStagePtr &stage, const SdfPath &path);

 protected:
  /// Returns the kind of schema this class belongs to.
  ///
  /// \sa UsdSchemaKind
  USDLUX_API
  UsdSchemaKind _GetSchemaKind() const override;

 private:
  // needs to invoke GetStaticTfType.
  friend class UsdSchemaRegistry;
  USDLUX_API
  static const TfType &_GetStaticTfType();

  static bool _IsTypedSchema();

  // override SchemaBase virtuals.
  USDLUX_API
  const TfType &_GetTfType() const override;;

 public:
  // --------------------------------------------------------------------- //
  // WIDTH
  // --------------------------------------------------------------------- //
  /// Width of the rectangle, in the local X axis.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float inputs:width = 1` |
  /// | C++ Type | float |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
  USDLUX_API
  UsdAttribute GetWidthAttr() const;

  /// See GetWidthAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDLUX_API
  UsdAttribute CreateWidthAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // HEIGHT
  // --------------------------------------------------------------------- //
  /// Height of the rectangle, in the local Y axis.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float inputs:height = 1` |
  /// | C++ Type | float |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
  USDLUX_API
  UsdAttribute GetHeightAttr() const;

  /// See GetHeightAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDLUX_API
  UsdAttribute CreateHeightAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // TEXTUREFILE
  // --------------------------------------------------------------------- //
  /// A color texture to use on the rectangle.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `asset inputs:texture:file` |
  /// | C++ Type | SdfAssetPath |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Asset |
  USDLUX_API
  UsdAttribute GetTextureFileAttr() const;

  /// See GetTextureFileAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDLUX_API
  UsdAttribute CreateTextureFileAttr(VtValue const &defaultValue = VtValue(),
                                     bool writeSparsely = false) const;

 public:
  // ===================================================================== //
  // Feel free to add custom code below this line, it will be preserved by
  // the code generator.
  //
  // Just remember to:
  //  - Close the class declaration with };
  //  - Close the namespace with WABI_NAMESPACE_END
  //  - Close the include guard with #endif
  // ===================================================================== //
  // --(BEGIN CUSTOM CODE)--
};

WABI_NAMESPACE_END

#endif
