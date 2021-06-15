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
#include "wabi/usd/usdVol/fieldAsset.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdVolFieldAsset, TfType::Bases<UsdVolFieldBase>>();
}

/* virtual */
UsdVolFieldAsset::~UsdVolFieldAsset()
{}

/* static */
UsdVolFieldAsset UsdVolFieldAsset::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdVolFieldAsset();
  }
  return UsdVolFieldAsset(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdVolFieldAsset::_GetSchemaKind() const
{
  return UsdVolFieldAsset::schemaKind;
}

/* virtual */
UsdSchemaKind UsdVolFieldAsset::_GetSchemaType() const
{
  return UsdVolFieldAsset::schemaType;
}

/* static */
const TfType &UsdVolFieldAsset::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdVolFieldAsset>();
  return tfType;
}

/* static */
bool UsdVolFieldAsset::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdVolFieldAsset::_GetTfType() const
{
  return _GetStaticTfType();
}

UsdAttribute UsdVolFieldAsset::GetFilePathAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->filePath);
}

UsdAttribute UsdVolFieldAsset::CreateFilePathAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->filePath,
                                    SdfValueTypeNames->Asset,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdVolFieldAsset::GetFieldNameAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->fieldName);
}

UsdAttribute UsdVolFieldAsset::CreateFieldNameAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->fieldName,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdVolFieldAsset::GetFieldIndexAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->fieldIndex);
}

UsdAttribute UsdVolFieldAsset::CreateFieldIndexAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->fieldIndex,
                                    SdfValueTypeNames->Int,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdVolFieldAsset::GetFieldDataTypeAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->fieldDataType);
}

UsdAttribute UsdVolFieldAsset::CreateFieldDataTypeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->fieldDataType,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdVolFieldAsset::GetVectorDataRoleHintAttr() const
{
  return GetPrim().GetAttribute(UsdVolTokens->vectorDataRoleHint);
}

UsdAttribute UsdVolFieldAsset::CreateVectorDataRoleHintAttr(VtValue const &defaultValue,
                                                            bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdVolTokens->vectorDataRoleHint,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

namespace {
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
const TfTokenVector &UsdVolFieldAsset::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdVolTokens->filePath,
    UsdVolTokens->fieldName,
    UsdVolTokens->fieldIndex,
    UsdVolTokens->fieldDataType,
    UsdVolTokens->vectorDataRoleHint,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdVolFieldBase::GetSchemaAttributeNames(true),
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
