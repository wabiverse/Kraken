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
#ifndef USDRI_GENERATED_PXRRAMPLIGHTFILTER_H
#define USDRI_GENERATED_PXRRAMPLIGHTFILTER_H

/// \file usdRi/pxrRampLightFilter.h

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdLux/lightFilter.h"
#include "wabi/usd/usdRi/api.h"
#include "wabi/usd/usdRi/tokens.h"
#include "wabi/wabi.h"

#include "wabi/usd/usdRi/splineAPI.h"

#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// PXRRAMPLIGHTFILTER                                                         //
// -------------------------------------------------------------------------- //

/// \class UsdRiPxrRampLightFilter
///
///
/// \deprecated This schema will be replaced in a future release.
/// A ramp to modulate how a light falls off with distance.
///
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdRiTokens.
/// So to set an attribute to the value "rightHanded", use UsdRiTokens->rightHanded
/// as the value.
///
class UsdRiPxrRampLightFilter : public UsdLuxLightFilter
{
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;



  /// Construct a UsdRiPxrRampLightFilter on UsdPrim \p prim .
  /// Equivalent to UsdRiPxrRampLightFilter::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdRiPxrRampLightFilter(const UsdPrim &prim = UsdPrim())
    : UsdLuxLightFilter(prim)
  {}

  /// Construct a UsdRiPxrRampLightFilter on the prim held by \p schemaObj .
  /// Should be preferred over UsdRiPxrRampLightFilter(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdRiPxrRampLightFilter(const UsdSchemaBase &schemaObj)
    : UsdLuxLightFilter(schemaObj)
  {}

  /// Destructor.
  USDRI_API
  virtual ~UsdRiPxrRampLightFilter();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDRI_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdRiPxrRampLightFilter holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdRiPxrRampLightFilter(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDRI_API
  static UsdRiPxrRampLightFilter Get(const UsdStagePtr &stage, const SdfPath &path);

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
  USDRI_API
  static UsdRiPxrRampLightFilter Define(const UsdStagePtr &stage, const SdfPath &path);

 protected:
  /// Returns the kind of schema this class belongs to.
  ///
  /// \sa UsdSchemaKind
  USDRI_API
  UsdSchemaKind GetSchemaKind() const override;

 private:
  // needs to invoke GetStaticTfType.
  friend class UsdSchemaRegistry;
  USDRI_API
  static const TfType &GetStaticTfType();

  static bool IsTypedSchema();

  // override SchemaBase virtuals.
  USDRI_API
  const TfType &GetTfType() const override;

 public:
  // --------------------------------------------------------------------- //
  // RAMPMODE
  // --------------------------------------------------------------------- //
  /// Specifies the direction in which the ramp is applied
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `token rampMode = "distanceToLight"` |
  /// | C++ Type | TfToken |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
  /// | \ref UsdRiTokens "Allowed Values" | distanceToLight, linear, spherical, radial |
  USDRI_API
  UsdAttribute GetRampModeAttr() const;

  /// See GetRampModeAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateRampModeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // BEGINDISTANCE
  // --------------------------------------------------------------------- //
  /// Distance where the ramp starts.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float beginDistance = 0` |
  /// | C++ Type | float |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
  USDRI_API
  UsdAttribute GetBeginDistanceAttr() const;

  /// See GetBeginDistanceAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateBeginDistanceAttr(VtValue const &defaultValue = VtValue(),
                                       bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // ENDDISTANCE
  // --------------------------------------------------------------------- //
  /// Distance where the ramp ends.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float endDistance = 10` |
  /// | C++ Type | float |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
  USDRI_API
  UsdAttribute GetEndDistanceAttr() const;

  /// See GetEndDistanceAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateEndDistanceAttr(VtValue const &defaultValue = VtValue(),
                                     bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // FALLOFF
  // --------------------------------------------------------------------- //
  /// Controls the transition from the core to the edge.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `int falloff = 4` |
  /// | C++ Type | int |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Int |
  USDRI_API
  UsdAttribute GetFalloffAttr() const;

  /// See GetFalloffAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateFalloffAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // FALLOFFKNOTS
  // --------------------------------------------------------------------- //
  /// Knots of the falloff spline.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float[] falloff:knots = [0, 0, 1, 1]` |
  /// | C++ Type | VtArray<float> |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->FloatArray |
  USDRI_API
  UsdAttribute GetFalloffKnotsAttr() const;

  /// See GetFalloffKnotsAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateFalloffKnotsAttr(VtValue const &defaultValue = VtValue(),
                                      bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // FALLOFFFLOATS
  // --------------------------------------------------------------------- //
  /// Float values of the falloff spline.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float[] falloff:floats = [0, 0, 1, 1]` |
  /// | C++ Type | VtArray<float> |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->FloatArray |
  USDRI_API
  UsdAttribute GetFalloffFloatsAttr() const;

  /// See GetFalloffFloatsAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateFalloffFloatsAttr(VtValue const &defaultValue = VtValue(),
                                       bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // FALLOFFINTERPOLATION
  // --------------------------------------------------------------------- //
  /// Falloff spline type.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `token falloff:interpolation = "linear"` |
  /// | C++ Type | TfToken |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
  /// | \ref UsdRiTokens "Allowed Values" | linear, catmull-rom, bspline, constant |
  USDRI_API
  UsdAttribute GetFalloffInterpolationAttr() const;

  /// See GetFalloffInterpolationAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateFalloffInterpolationAttr(VtValue const &defaultValue = VtValue(),
                                              bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // COLORRAMP
  // --------------------------------------------------------------------- //
  /// Controls the color gradient for the transition.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `int colorRamp = 4` |
  /// | C++ Type | int |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Int |
  USDRI_API
  UsdAttribute GetColorRampAttr() const;

  /// See GetColorRampAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateColorRampAttr(VtValue const &defaultValue = VtValue(),
                                   bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // COLORRAMPKNOTS
  // --------------------------------------------------------------------- //
  /// Knots of the colorRamp spline.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `float[] colorRamp:knots = [0, 0, 1, 1]` |
  /// | C++ Type | VtArray<float> |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->FloatArray |
  USDRI_API
  UsdAttribute GetColorRampKnotsAttr() const;

  /// See GetColorRampKnotsAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateColorRampKnotsAttr(VtValue const &defaultValue = VtValue(),
                                        bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // COLORRAMPCOLORS
  // --------------------------------------------------------------------- //
  /// Color values of the colorRamp spline.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `color3f[] colorRamp:colors = [(1, 1, 1), (1, 1, 1), (1, 1, 1), (1, 1, 1)]` |
  /// | C++ Type | VtArray<GfVec3f> |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Color3fArray |
  USDRI_API
  UsdAttribute GetColorRampColorsAttr() const;

  /// See GetColorRampColorsAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateColorRampColorsAttr(VtValue const &defaultValue = VtValue(),
                                         bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // COLORRAMPINTERPOLATION
  // --------------------------------------------------------------------- //
  /// ColorRamp spline type.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `token colorRamp:interpolation = "linear"` |
  /// | C++ Type | TfToken |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
  /// | \ref UsdRiTokens "Allowed Values" | linear, catmull-rom, bspline, constant |
  USDRI_API
  UsdAttribute GetColorRampInterpolationAttr() const;

  /// See GetColorRampInterpolationAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateColorRampInterpolationAttr(VtValue const &defaultValue = VtValue(),
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

  /// Return the UsdRiSplineAPI interface used for examining and modifying
  /// the falloff ramp.
  USDRI_API
  UsdRiSplineAPI GetFalloffRampAPI() const;

  /// Return the UsdRiSplineAPI interface used for examining and modifying
  /// the color ramp.
  USDRI_API
  UsdRiSplineAPI GetColorRampAPI() const;
};

WABI_NAMESPACE_END

#endif
