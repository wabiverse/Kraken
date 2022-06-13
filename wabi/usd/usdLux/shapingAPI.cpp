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
#include "wabi/usd/usdLux/shapingAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdLuxShapingAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (ShapingAPI));

/* virtual */
UsdLuxShapingAPI::~UsdLuxShapingAPI() {}

/* static */
UsdLuxShapingAPI UsdLuxShapingAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdLuxShapingAPI();
  }
  return UsdLuxShapingAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdLuxShapingAPI::GetSchemaKind() const
{
  return UsdLuxShapingAPI::schemaKind;
}

/* static */
UsdLuxShapingAPI UsdLuxShapingAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdLuxShapingAPI>()) {
    return UsdLuxShapingAPI(prim);
  }
  return UsdLuxShapingAPI();
}

/* static */
const TfType &UsdLuxShapingAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdLuxShapingAPI>();
  return tfType;
}

/* static */
bool UsdLuxShapingAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdLuxShapingAPI::GetTfType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdLuxShapingAPI::GetShapingFocusAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingFocus);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingFocusAttr(VtValue const &defaultValue,
                                                      bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingFocus,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingFocusTintAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingFocusTint);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingFocusTintAttr(VtValue const &defaultValue,
                                                          bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingFocusTint,
                                    SdfValueTypeNames->Color3f,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingConeAngleAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingConeAngle);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingConeAngleAttr(VtValue const &defaultValue,
                                                          bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingConeAngle,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingConeSoftnessAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingConeSoftness);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingConeSoftnessAttr(VtValue const &defaultValue,
                                                             bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingConeSoftness,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingIesFileAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingIesFile);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingIesFileAttr(VtValue const &defaultValue,
                                                        bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingIesFile,
                                    SdfValueTypeNames->Asset,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingIesAngleScaleAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingIesAngleScale);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingIesAngleScaleAttr(VtValue const &defaultValue,
                                                              bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingIesAngleScale,
                                    SdfValueTypeNames->Float,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdLuxShapingAPI::GetShapingIesNormalizeAttr() const
{
  return GetPrim().GetAttribute(UsdLuxTokens->inputsShapingIesNormalize);
}

UsdAttribute UsdLuxShapingAPI::CreateShapingIesNormalizeAttr(VtValue const &defaultValue,
                                                             bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdLuxTokens->inputsShapingIesNormalize,
                                    SdfValueTypeNames->Bool,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

namespace
{
  static inline TfTokenVector _ConcatenateAttributeNames(const TfTokenVector &left,
                                                         const TfTokenVector &right)
  {
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
  }
}  // namespace

/*static*/
const TfTokenVector &UsdLuxShapingAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdLuxTokens->inputsShapingFocus,
    UsdLuxTokens->inputsShapingFocusTint,
    UsdLuxTokens->inputsShapingConeAngle,
    UsdLuxTokens->inputsShapingConeSoftness,
    UsdLuxTokens->inputsShapingIesFile,
    UsdLuxTokens->inputsShapingIesAngleScale,
    UsdLuxTokens->inputsShapingIesNormalize,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(
    UsdAPISchemaBase::GetSchemaAttributeNames(true),
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

#include "wabi/usd/usdShade/connectableAPI.h"

WABI_NAMESPACE_BEGIN

UsdLuxShapingAPI::UsdLuxShapingAPI(const UsdShadeConnectableAPI &connectable)
  : UsdLuxShapingAPI(connectable.GetPrim())
{}

UsdShadeConnectableAPI UsdLuxShapingAPI::ConnectableAPI() const
{
  return UsdShadeConnectableAPI(GetPrim());
}

UsdShadeOutput UsdLuxShapingAPI::CreateOutput(const TfToken &name,
                                              const SdfValueTypeName &typeName)
{
  return UsdShadeConnectableAPI(GetPrim()).CreateOutput(name, typeName);
}

UsdShadeOutput UsdLuxShapingAPI::GetOutput(const TfToken &name) const
{
  return UsdShadeConnectableAPI(GetPrim()).GetOutput(name);
}

std::vector<UsdShadeOutput> UsdLuxShapingAPI::GetOutputs(bool onlyAuthored) const
{
  return UsdShadeConnectableAPI(GetPrim()).GetOutputs(onlyAuthored);
}

UsdShadeInput UsdLuxShapingAPI::CreateInput(const TfToken &name, const SdfValueTypeName &typeName)
{
  return UsdShadeConnectableAPI(GetPrim()).CreateInput(name, typeName);
}

UsdShadeInput UsdLuxShapingAPI::GetInput(const TfToken &name) const
{
  return UsdShadeConnectableAPI(GetPrim()).GetInput(name);
}

std::vector<UsdShadeInput> UsdLuxShapingAPI::GetInputs(bool onlyAuthored) const
{
  return UsdShadeConnectableAPI(GetPrim()).GetInputs(onlyAuthored);
}

WABI_NAMESPACE_END
