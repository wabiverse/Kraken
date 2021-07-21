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
#include "wabi/usd/usdVol/field3DAsset.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdVolField3DAsset, TfType::Bases<UsdVolFieldAsset>>();

  // Register the usd prim typename as an alias under UsdSchemaBase. This
  // enables one to call
  // TfType::Find<UsdSchemaBase>().FindDerivedByName("Field3DAsset")
  // to find TfType<UsdVolField3DAsset>, which is how IsA queries are
  // answered.
  TfType::AddAlias<UsdSchemaBase, UsdVolField3DAsset>("Field3DAsset");
}

/* virtual */
UsdVolField3DAsset::~UsdVolField3DAsset()
{}

/* static */
UsdVolField3DAsset UsdVolField3DAsset::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdVolField3DAsset();
  }
  return UsdVolField3DAsset(stage->GetPrimAtPath(path));
}

/* static */
UsdVolField3DAsset UsdVolField3DAsset::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("Field3DAsset");
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdVolField3DAsset();
  }
  return UsdVolField3DAsset(stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdVolField3DAsset::GetSchemaKind() const
{
  return UsdVolField3DAsset::schemaKind;
}

/* static */
const TfType &UsdVolField3DAsset::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdVolField3DAsset>();
  return tfType;
}

/* static */
bool UsdVolField3DAsset::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdVolField3DAsset::GetType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdVolField3DAsset::GetFieldDataTypeAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->fieldDataType);
}

UsdAttribute UsdVolField3DAsset::CreateFieldDataTypeAttr(VtValue const &defaultValue,
                                                         bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->fieldDataType,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdVolField3DAsset::GetFieldPurposeAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->fieldPurpose);
}

UsdAttribute UsdVolField3DAsset::CreateFieldPurposeAttr(VtValue const &defaultValue,
                                                        bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->fieldPurpose,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

namespace
{
static inline TfTokenVector _ConcatenateAttributeNames(const TfTokenVector &left, const TfTokenVector &right)
{
  TfTokenVector result;
  result.reserve(left.size() + right.size());
  result.insert(result.end(), left.begin(), left.end());
  result.insert(result.end(), right.begin(), right.end());
  return result;
}
}  // namespace

/*static*/
const TfTokenVector &UsdVolField3DAsset::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdVolTokens->fieldDataType,
    UsdVolTokens->fieldPurpose,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdVolFieldAsset::GetSchemaAttributeNames(true),
                                                             localNames);

  if (includeInherited)
    return allNames;
  else
    return localNames;
}

WABI_NAMESPACE_END

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'WABI_NAMESPACE_BEGIN', 'WABI_NAMESPACE_END'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
