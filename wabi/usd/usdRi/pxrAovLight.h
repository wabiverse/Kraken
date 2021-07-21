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
#ifndef USDRI_GENERATED_PXRAOVLIGHT_H
#define USDRI_GENERATED_PXRAOVLIGHT_H

/// \file usdRi/pxrAovLight.h

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdLux/light.h"
#include "wabi/usd/usdRi/api.h"
#include "wabi/usd/usdRi/tokens.h"
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
// PXRAOVLIGHT                                                                //
// -------------------------------------------------------------------------- //

/// \class UsdRiPxrAovLight
///
/// \deprecated This schema will be replaced in a future release.
///
class UsdRiPxrAovLight : public UsdLuxLight
{
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;



  /// Construct a UsdRiPxrAovLight on UsdPrim \p prim .
  /// Equivalent to UsdRiPxrAovLight::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdRiPxrAovLight(const UsdPrim &prim = UsdPrim())
    : UsdLuxLight(prim)
  {}

  /// Construct a UsdRiPxrAovLight on the prim held by \p schemaObj .
  /// Should be preferred over UsdRiPxrAovLight(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdRiPxrAovLight(const UsdSchemaBase &schemaObj)
    : UsdLuxLight(schemaObj)
  {}

  /// Destructor.
  USDRI_API
  virtual ~UsdRiPxrAovLight();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDRI_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdRiPxrAovLight holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdRiPxrAovLight(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDRI_API
  static UsdRiPxrAovLight Get(const UsdStagePtr &stage, const SdfPath &path);

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
  static UsdRiPxrAovLight Define(const UsdStagePtr &stage, const SdfPath &path);

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
  // AOVNAME
  // --------------------------------------------------------------------- //
  /// The name of the AOV to write to.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `string aovName = ""` |
  /// | C++ Type | std::string |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->String |
  USDRI_API
  UsdAttribute GetAovNameAttr() const;

  /// See GetAovNameAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateAovNameAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // INPRIMARYHIT
  // --------------------------------------------------------------------- //
  /// If this is on, the usual mask of the illuminated objects
  /// is generated. If this is off, you can get a mask of only in the
  /// refraction or reflection.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool inPrimaryHit = 1` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetInPrimaryHitAttr() const;

  /// See GetInPrimaryHitAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateInPrimaryHitAttr(VtValue const &defaultValue = VtValue(),
                                      bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // INREFLECTION
  // --------------------------------------------------------------------- //
  /// If this is on, the rays are traced through the specular
  /// reflections to get  the masking signal.  Warning: this will
  /// require some amount of samples to get a clean mask.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool inReflection = 0` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetInReflectionAttr() const;

  /// See GetInReflectionAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateInReflectionAttr(VtValue const &defaultValue = VtValue(),
                                      bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // INREFRACTION
  // --------------------------------------------------------------------- //
  /// If this is on, the rays are traced through the glass
  /// refractions  to get the masking signal.  Warning: this will
  /// require some amount of samples to get a clean mask.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool inRefraction = 0` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetInRefractionAttr() const;

  /// See GetInRefractionAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateInRefractionAttr(VtValue const &defaultValue = VtValue(),
                                      bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // INVERT
  // --------------------------------------------------------------------- //
  /// If this is on, it inverts the signal for the AOV.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool invert = 0` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetInvertAttr() const;

  /// See GetInvertAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateInvertAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // ONVOLUMEBOUNDARIES
  // --------------------------------------------------------------------- //
  /// If this is on, the bounding box or shape of volumes will
  /// appear in the mask. Since this is not always desirable, this can
  /// be turned off.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool onVolumeBoundaries = 1` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetOnVolumeBoundariesAttr() const;

  /// See GetOnVolumeBoundariesAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateOnVolumeBoundariesAttr(VtValue const &defaultValue = VtValue(),
                                            bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // USECOLOR
  // --------------------------------------------------------------------- //
  /// If this is on, it outputs a RGB color image instead of a
  /// float image for the AOV.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool useColor = 0` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetUseColorAttr() const;

  /// See GetUseColorAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateUseColorAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // USETHROUGHPUT
  // --------------------------------------------------------------------- //
  /// If this is on, the values in the mask for the reflected
  /// or refracted rays will be affected by the strength of the reflection
  /// or refraction. This can lead to values below and above 1.0. Turn
  /// this off if you want a more solid mask.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `bool useThroughput = 1` |
  /// | C++ Type | bool |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
  USDRI_API
  UsdAttribute GetUseThroughputAttr() const;

  /// See GetUseThroughputAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDRI_API
  UsdAttribute CreateUseThroughputAttr(VtValue const &defaultValue = VtValue(),
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
