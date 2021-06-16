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

/* clang-format off */

#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/usdUI/screenAPI.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN
 
/**
 * Register the schema with the TfType system. */
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdUIScreenAPI, TfType::Bases<UsdAPISchemaBase>>();
}

/* clang-format off */
TF_DEFINE_PRIVATE_TOKENS(
  _schemaTokens,
  (ScreenAPI)
);

/* virtual */
UsdUIScreenAPI::~UsdUIScreenAPI()
{}

/* static */
UsdUIScreenAPI UsdUIScreenAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdUIScreenAPI();
  }
  return UsdUIScreenAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdUIScreenAPI::_GetSchemaKind() const
{
  return UsdUIScreenAPI::schemaKind;
}

/* virtual */
UsdSchemaKind UsdUIScreenAPI::_GetSchemaType() const
{
  return UsdUIScreenAPI::schemaType;
}

/* static */
UsdUIScreenAPI
UsdUIScreenAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdUIScreenAPI>()) {
    return UsdUIScreenAPI(prim);
  }
  return UsdUIScreenAPI();
}

/* static */
const TfType &UsdUIScreenAPI::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdUIScreenAPI>();
  return tfType;
}

/* static */
bool UsdUIScreenAPI::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdUIScreenAPI::_GetTfType() const
{
  return _GetStaticTfType();
}

UsdAttribute UsdUIScreenAPI::GetPosAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaPos);
}

UsdAttribute UsdUIScreenAPI::CreatePosAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaPos,
    SdfValueTypeNames->Float2,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetSizeAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaSize);
}

UsdAttribute UsdUIScreenAPI::CreateSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaSize,
    SdfValueTypeNames->Float2,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetIconAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaIcon);
}

UsdAttribute UsdUIScreenAPI::CreateIconAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaIcon,
    SdfValueTypeNames->Asset,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetTypeAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaType);
}

UsdAttribute UsdUIScreenAPI::CreateTypeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaType,
    SdfValueTypeNames->Token,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetNameAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaName);
}

UsdAttribute UsdUIScreenAPI::CreateNameAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaName,
    SdfValueTypeNames->Token,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetShowMenusAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaShowMenus);
}

UsdAttribute UsdUIScreenAPI::CreateShowMenusAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaShowMenus,
    SdfValueTypeNames->Bool,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetPurposeAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaPurpose);
}

UsdAttribute UsdUIScreenAPI::CreatePurposeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaPurpose,
    SdfValueTypeNames->Token,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUIScreenAPI::GetLayoutAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiScreenAreaLayout);
}

UsdAttribute UsdUIScreenAPI::CreateLayoutAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiScreenAreaLayout,
    SdfValueTypeNames->Token,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdRelationship UsdUIScreenAPI::GetUiScreenAreaRegionRel() const
{
  return GetPrim().GetRelationship(
    UsdUITokens->uiScreenAreaRegion);
}

UsdRelationship UsdUIScreenAPI::CreateUiScreenAreaRegionRel() const
{
  return GetPrim().CreateRelationship(
    UsdUITokens->uiScreenAreaRegion,
    /* custom = */ false);
}

UsdRelationship UsdUIScreenAPI::GetWorkspaceRel() const
{
  return GetPrim().GetRelationship(
    UsdUITokens->uiScreenAreaWorkspace);
}

UsdRelationship UsdUIScreenAPI::CreateWorkspaceRel() const
{
  return GetPrim().CreateRelationship(
    UsdUITokens->uiScreenAreaWorkspace,
    /* custom = */ false);
}

namespace {
static inline TfTokenVector _ConcatenateAttributeNames(const TfTokenVector& left,
                           const TfTokenVector& right)
{
  TfTokenVector result;
  result.reserve(left.size() + right.size());
  result.insert(result.end(), left.begin(), left.end());
  result.insert(result.end(), right.begin(), right.end());
  return result;
}
}  /* anonymous */

/*static*/
const TfTokenVector& UsdUIScreenAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdUITokens->uiScreenAreaPos,
    UsdUITokens->uiScreenAreaSize,
    UsdUITokens->uiScreenAreaIcon,
    UsdUITokens->uiScreenAreaType,
    UsdUITokens->uiScreenAreaName,
    UsdUITokens->uiScreenAreaShowMenus,
    UsdUITokens->uiScreenAreaPurpose,
    UsdUITokens->uiScreenAreaLayout,
  };
  static TfTokenVector allNames =
    _ConcatenateAttributeNames(UsdAPISchemaBase::GetSchemaAttributeNames(true), localNames);

  if (includeInherited)
    return allNames;
  else
    return localNames;
}

WABI_NAMESPACE_END

/* clang-format off */

  /**
   * ======================================================================
   *   Feel free to add custom code below this line. It will be preserved
   *   by the code generator.
   *
   *   Just remember to wrap code in the appropriate delimiters:
   *     - 'WABI_NAMESPACE_BEGIN', 'WABI_NAMESPACE_END'.
   * ======================================================================
   * --(BEGIN CUSTOM CODE)-- */
