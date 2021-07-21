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
#include "wabi/usd/usdRi/pxrRampLightFilter.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdRiPxrRampLightFilter, TfType::Bases<UsdLuxLightFilter>>();

  // Register the usd prim typename as an alias under UsdSchemaBase. This
  // enables one to call
  // TfType::Find<UsdSchemaBase>().FindDerivedByName("wabiRampLightFilter")
  // to find TfType<UsdRiPxrRampLightFilter>, which is how IsA queries are
  // answered.
  TfType::AddAlias<UsdSchemaBase, UsdRiPxrRampLightFilter>("wabiRampLightFilter");
}

/* virtual */
UsdRiPxrRampLightFilter::~UsdRiPxrRampLightFilter()
{}

/* static */
UsdRiPxrRampLightFilter UsdRiPxrRampLightFilter::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdRiPxrRampLightFilter();
  }
  return UsdRiPxrRampLightFilter(stage->GetPrimAtPath(path));
}

/* static */
UsdRiPxrRampLightFilter UsdRiPxrRampLightFilter::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("wabiRampLightFilter");
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdRiPxrRampLightFilter();
  }
  return UsdRiPxrRampLightFilter(stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdRiPxrRampLightFilter::GetSchemaKind() const
{
  return UsdRiPxrRampLightFilter::schemaKind;
}

/* static */
const TfType &UsdRiPxrRampLightFilter::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdRiPxrRampLightFilter>();
  return tfType;
}

/* static */
bool UsdRiPxrRampLightFilter::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdRiPxrRampLightFilter::GetTfType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdRiPxrRampLightFilter::GetRampModeAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->rampMode);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateRampModeAttr(VtValue const &defaultValue,
                                                         bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->rampMode,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetBeginDistanceAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->beginDistance);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateBeginDistanceAttr(VtValue const &defaultValue,
                                                              bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->beginDistance,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetEndDistanceAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->endDistance);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateEndDistanceAttr(VtValue const &defaultValue,
                                                            bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->endDistance,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetFalloffAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->falloff);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateFalloffAttr(VtValue const &defaultValue,
                                                        bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->falloff,
                                    SdfValueTypeNames->Int,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetFalloffKnotsAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->falloffKnots);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateFalloffKnotsAttr(VtValue const &defaultValue,
                                                             bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->falloffKnots,
                                    SdfValueTypeNames->FloatArray,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetFalloffFloatsAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->falloffFloats);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateFalloffFloatsAttr(VtValue const &defaultValue,
                                                              bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->falloffFloats,
                                    SdfValueTypeNames->FloatArray,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetFalloffInterpolationAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->falloffInterpolation);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateFalloffInterpolationAttr(VtValue const &defaultValue,
                                                                     bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->falloffInterpolation,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetColorRampAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->colorRamp);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateColorRampAttr(VtValue const &defaultValue,
                                                          bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->colorRamp,
                                    SdfValueTypeNames->Int,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetColorRampKnotsAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->colorRampKnots);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateColorRampKnotsAttr(VtValue const &defaultValue,
                                                               bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->colorRampKnots,
                                    SdfValueTypeNames->FloatArray,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetColorRampColorsAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->colorRampColors);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateColorRampColorsAttr(VtValue const &defaultValue,
                                                                bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->colorRampColors,
                                    SdfValueTypeNames->Color3fArray,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdRiPxrRampLightFilter::GetColorRampInterpolationAttr() const
{
  return GetPrim().GetAttribute(UsdRiTokens->colorRampInterpolation);
}

UsdAttribute UsdRiPxrRampLightFilter::CreateColorRampInterpolationAttr(VtValue const &defaultValue,
                                                                       bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdRiTokens->colorRampInterpolation,
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
const TfTokenVector &UsdRiPxrRampLightFilter::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdRiTokens->rampMode,
    UsdRiTokens->beginDistance,
    UsdRiTokens->endDistance,
    UsdRiTokens->falloff,
    UsdRiTokens->falloffKnots,
    UsdRiTokens->falloffFloats,
    UsdRiTokens->falloffInterpolation,
    UsdRiTokens->colorRamp,
    UsdRiTokens->colorRampKnots,
    UsdRiTokens->colorRampColors,
    UsdRiTokens->colorRampInterpolation,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(
    UsdLuxLightFilter::GetSchemaAttributeNames(true), localNames);

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

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens, (falloffRamp)(colorRamp));

UsdRiSplineAPI UsdRiPxrRampLightFilter::GetFalloffRampAPI() const
{
  return UsdRiSplineAPI(*this,
                        _tokens->falloffRamp,
                        SdfValueTypeNames->FloatArray,
                        /* duplicate */ true);
}

UsdRiSplineAPI UsdRiPxrRampLightFilter::GetColorRampAPI() const
{
  return UsdRiSplineAPI(*this,
                        _tokens->colorRamp,
                        SdfValueTypeNames->Color3fArray,
                        /* duplicate */ true);
}

WABI_NAMESPACE_END
