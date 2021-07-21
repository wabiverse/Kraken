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
#include "wabi/usd/usdRi/textureAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdRiTextureAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (RiTextureAPI));

/* virtual */
UsdRiTextureAPI::~UsdRiTextureAPI()
{}

/* static */
UsdRiTextureAPI UsdRiTextureAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdRiTextureAPI();
  }
  return UsdRiTextureAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdRiTextureAPI::GetSchemaKind() const
{
  return UsdRiTextureAPI::schemaKind;
}

/* static */
UsdRiTextureAPI UsdRiTextureAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdRiTextureAPI>())
  {
    return UsdRiTextureAPI(prim);
  }
  return UsdRiTextureAPI();
}

/* static */
const TfType &UsdRiTextureAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdRiTextureAPI>();
  return tfType;
}

/* static */
bool UsdRiTextureAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdRiTextureAPI::GetType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdRiTextureAPI::GetRiTextureGammaAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riTextureGamma);
}

UsdAttribute UsdRiTextureAPI::CreateRiTextureGammaAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riTextureGamma,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiTextureAPI::GetRiTextureSaturationAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riTextureSaturation);
}

UsdAttribute UsdRiTextureAPI::CreateRiTextureSaturationAttr(VtValue const &defaultValue,
                                                            bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riTextureSaturation,
                                    SdfValueTypeNames->Float,
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
const TfTokenVector &UsdRiTextureAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdRiTokens->riTextureGamma,
    UsdRiTokens->riTextureSaturation,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdAPISchemaBase::GetSchemaAttributeNames(true),
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
