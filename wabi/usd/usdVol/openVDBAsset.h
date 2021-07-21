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
#ifndef USDVOL_GENERATED_OPENVDBASSET_H
#define USDVOL_GENERATED_OPENVDBASSET_H

/// \file usdVol/openVDBAsset.h

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdVol/api.h"
#include "wabi/usd/usdVol/fieldAsset.h"
#include "wabi/usd/usdVol/tokens.h"
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
// OPENVDBASSET                                                               //
// -------------------------------------------------------------------------- //

/// \class UsdVolOpenVDBAsset
///
/// OpenVDB field primitive. The FieldAsset filePath attribute must
/// specify a file in the OpenVDB format on disk.
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdVolTokens.
/// So to set an attribute to the value "rightHanded", use UsdVolTokens->rightHanded
/// as the value.
///
class UsdVolOpenVDBAsset : public UsdVolFieldAsset
{
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;


  /// Construct a UsdVolOpenVDBAsset on UsdPrim \p prim .
  /// Equivalent to UsdVolOpenVDBAsset::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdVolOpenVDBAsset(const UsdPrim &prim = UsdPrim())
    : UsdVolFieldAsset(prim)
  {}

  /// Construct a UsdVolOpenVDBAsset on the prim held by \p schemaObj .
  /// Should be preferred over UsdVolOpenVDBAsset(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdVolOpenVDBAsset(const UsdSchemaBase &schemaObj)
    : UsdVolFieldAsset(schemaObj)
  {}

  /// Destructor.
  USDVOL_API
  virtual ~UsdVolOpenVDBAsset();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDVOL_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdVolOpenVDBAsset holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdVolOpenVDBAsset(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDVOL_API
  static UsdVolOpenVDBAsset Get(const UsdStagePtr &stage, const SdfPath &path);

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
  USDVOL_API
  static UsdVolOpenVDBAsset Define(const UsdStagePtr &stage, const SdfPath &path);

 protected:
  /// Returns the kind of schema this class belongs to.
  ///
  /// \sa UsdSchemaKind
  USDVOL_API
  UsdSchemaKind GetSchemaKind() const override;

 private:
  // needs to invoke GetStaticTfType.
  friend class UsdSchemaRegistry;
  USDVOL_API
  static const TfType &GetStaticTfType();

  static bool IsTypedSchema();

  // override SchemaBase virtuals.
  USDVOL_API
  const TfType &GetTfType() const override;

 public:
  // --------------------------------------------------------------------- //
  // FIELDDATATYPE
  // --------------------------------------------------------------------- //
  /// Token which is used to indicate the data type of an
  /// individual field. Authors use this to tell consumers more
  /// about the field without opening the file on disk. The list of
  /// allowed tokens reflects the available choices for OpenVDB
  /// volumes.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `token fieldDataType` |
  /// | C++ Type | TfToken |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
  /// | \ref UsdVolTokens "Allowed Values" | half, float, double, int, uint, int64, half2, float2,
  /// double2, int2, half3, float3, double3, int3, matrix3d, matrix4d, quatd, bool, mask, string |
  USDVOL_API
  UsdAttribute GetFieldDataTypeAttr() const;

  /// See GetFieldDataTypeAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDVOL_API
  UsdAttribute CreateFieldDataTypeAttr(VtValue const &defaultValue = VtValue(),
                                       bool writeSparsely = false) const;

 public:
  // --------------------------------------------------------------------- //
  // FIELDCLASS
  // --------------------------------------------------------------------- //
  /// Optional token which can be used to indicate the class of
  /// an individual grid. This is a mapping to openvdb::GridClass
  /// where the values are GRID_LEVEL_SET, GRID_FOG_VOLUME,
  /// GRID_STAGGERED, and GRID_UNKNOWN.
  ///
  /// | ||
  /// | -- | -- |
  /// | Declaration | `token fieldClass` |
  /// | C++ Type | TfToken |
  /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
  /// | \ref UsdVolTokens "Allowed Values" | levelSet, fogVolume, staggered, unknown |
  USDVOL_API
  UsdAttribute GetFieldClassAttr() const;

  /// See GetFieldClassAttr(), and also
  /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
  /// If specified, author \p defaultValue as the attribute's default,
  /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
  /// the default for \p writeSparsely is \c false.
  USDVOL_API
  UsdAttribute CreateFieldClassAttr(VtValue const &defaultValue = VtValue(),
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
