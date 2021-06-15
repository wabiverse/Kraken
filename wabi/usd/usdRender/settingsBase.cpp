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
#include "wabi/usd/usdRender/settingsBase.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdRenderSettingsBase, TfType::Bases<UsdTyped>>();
}

/* virtual */
UsdRenderSettingsBase::~UsdRenderSettingsBase()
{}

/* static */
UsdRenderSettingsBase UsdRenderSettingsBase::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdRenderSettingsBase();
  }
  return UsdRenderSettingsBase(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdRenderSettingsBase::_GetSchemaKind() const
{
  return UsdRenderSettingsBase::schemaKind;
}

/* virtual */
UsdSchemaKind UsdRenderSettingsBase::_GetSchemaType() const
{
  return UsdRenderSettingsBase::schemaType;
}

/* static */
const TfType &UsdRenderSettingsBase::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdRenderSettingsBase>();
  return tfType;
}

/* static */
bool UsdRenderSettingsBase::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdRenderSettingsBase::_GetTfType() const
{
  return _GetStaticTfType();
}

UsdAttribute UsdRenderSettingsBase::GetResolutionAttr() const
{
  return GetPrim().GetAttribute(UsdRenderTokens->resolution);
}

UsdAttribute UsdRenderSettingsBase::CreateResolutionAttr(VtValue const &defaultValue,
                                                         bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRenderTokens->resolution,
                                    SdfValueTypeNames->Int2,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRenderSettingsBase::GetPixelAspectRatioAttr() const
{
  return GetPrim().GetAttribute(UsdRenderTokens->pixelAspectRatio);
}

UsdAttribute UsdRenderSettingsBase::CreatePixelAspectRatioAttr(VtValue const &defaultValue,
                                                               bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRenderTokens->pixelAspectRatio,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRenderSettingsBase::GetAspectRatioConformPolicyAttr() const
{
  return GetPrim().GetAttribute(UsdRenderTokens->aspectRatioConformPolicy);
}

UsdAttribute UsdRenderSettingsBase::CreateAspectRatioConformPolicyAttr(VtValue const &defaultValue,
                                                                       bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRenderTokens->aspectRatioConformPolicy,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRenderSettingsBase::GetDataWindowNDCAttr() const
{
  return GetPrim().GetAttribute(UsdRenderTokens->dataWindowNDC);
}

UsdAttribute UsdRenderSettingsBase::CreateDataWindowNDCAttr(VtValue const &defaultValue,
                                                            bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRenderTokens->dataWindowNDC,
                                    SdfValueTypeNames->Float4,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRenderSettingsBase::GetInstantaneousShutterAttr() const
{
  return GetPrim().GetAttribute(UsdRenderTokens->instantaneousShutter);
}

UsdAttribute UsdRenderSettingsBase::CreateInstantaneousShutterAttr(VtValue const &defaultValue,
                                                                   bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRenderTokens->instantaneousShutter,
                                    SdfValueTypeNames->Bool,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdRelationship UsdRenderSettingsBase::GetCameraRel() const
{
  return GetPrim().GetRelationship(UsdRenderTokens->camera);
}

UsdRelationship UsdRenderSettingsBase::CreateCameraRel() const
{
  return GetPrim().CreateRelationship(UsdRenderTokens->camera,
                                      /* custom = */ false);
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
const TfTokenVector &UsdRenderSettingsBase::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdRenderTokens->resolution,
    UsdRenderTokens->pixelAspectRatio,
    UsdRenderTokens->aspectRatioConformPolicy,
    UsdRenderTokens->dataWindowNDC,
    UsdRenderTokens->instantaneousShutter,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdTyped::GetSchemaAttributeNames(true),
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
