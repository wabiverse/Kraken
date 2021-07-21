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
#include "wabi/usd/usdRi/lightAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdRiLightAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (RiLightAPI));

/* virtual */
UsdRiLightAPI::~UsdRiLightAPI()
{}

/* static */
UsdRiLightAPI UsdRiLightAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdRiLightAPI();
  }
  return UsdRiLightAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdRiLightAPI::GetSchemaKind() const
{
  return UsdRiLightAPI::schemaKind;
}

/* static */
UsdRiLightAPI UsdRiLightAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdRiLightAPI>())
  {
    return UsdRiLightAPI(prim);
  }
  return UsdRiLightAPI();
}

/* static */
const TfType &UsdRiLightAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdRiLightAPI>();
  return tfType;
}

/* static */
bool UsdRiLightAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdRiLightAPI::GetTfType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdRiLightAPI::GetRiSamplingFixedSampleCountAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riSamplingFixedSampleCount);
}

UsdAttribute UsdRiLightAPI::CreateRiSamplingFixedSampleCountAttr(VtValue const &defaultValue,
                                                                 bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riSamplingFixedSampleCount,
                                    SdfValueTypeNames->Int,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiLightAPI::GetRiSamplingImportanceMultiplierAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riSamplingImportanceMultiplier);
}

UsdAttribute UsdRiLightAPI::CreateRiSamplingImportanceMultiplierAttr(VtValue const &defaultValue,
                                                                     bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riSamplingImportanceMultiplier,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiLightAPI::GetRiIntensityNearDistAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riIntensityNearDist);
}

UsdAttribute UsdRiLightAPI::CreateRiIntensityNearDistAttr(VtValue const &defaultValue,
                                                          bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riIntensityNearDist,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiLightAPI::GetRiLightGroupAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riLightGroup);
}

UsdAttribute UsdRiLightAPI::CreateRiLightGroupAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riLightGroup,
                                    SdfValueTypeNames->String,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiLightAPI::GetRiShadowThinShadowAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riShadowThinShadow);
}

UsdAttribute UsdRiLightAPI::CreateRiShadowThinShadowAttr(VtValue const &defaultValue,
                                                         bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riShadowThinShadow,
                                    SdfValueTypeNames->Bool,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiLightAPI::GetRiTraceLightPathsAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->riTraceLightPaths);
}

UsdAttribute UsdRiLightAPI::CreateRiTraceLightPathsAttr(VtValue const &defaultValue,
                                                        bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->riTraceLightPaths,
                                    SdfValueTypeNames->Bool,
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
const TfTokenVector &UsdRiLightAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdRiTokens->riSamplingFixedSampleCount,
    UsdRiTokens->riSamplingImportanceMultiplier,
    UsdRiTokens->riIntensityNearDist,
    UsdRiTokens->riLightGroup,
    UsdRiTokens->riShadowThinShadow,
    UsdRiTokens->riTraceLightPaths,
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
