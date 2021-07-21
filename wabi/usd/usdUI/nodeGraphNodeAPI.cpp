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

#include "wabi/usd/usdUI/nodeGraphNodeAPI.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN
 
/**
 * Register the schema with the TfType system. */
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdUINodeGraphNodeAPI, TfType::Bases<UsdAPISchemaBase>>();
}

/* clang-format off */
TF_DEFINE_PRIVATE_TOKENS(
  _schemaTokens,
  (NodeGraphNodeAPI)
);

/* virtual */
UsdUINodeGraphNodeAPI::~UsdUINodeGraphNodeAPI()
{}

/* static */
UsdUINodeGraphNodeAPI UsdUINodeGraphNodeAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdUINodeGraphNodeAPI();
  }
  return UsdUINodeGraphNodeAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdUINodeGraphNodeAPI::GetSchemaKind() const
{
  return UsdUINodeGraphNodeAPI::schemaKind;
}

/* static */
UsdUINodeGraphNodeAPI
UsdUINodeGraphNodeAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdUINodeGraphNodeAPI>()) {
    return UsdUINodeGraphNodeAPI(prim);
  }
  return UsdUINodeGraphNodeAPI();
}

/* static */
const TfType &UsdUINodeGraphNodeAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdUINodeGraphNodeAPI>();
  return tfType;
}

/* static */
bool UsdUINodeGraphNodeAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdUINodeGraphNodeAPI::GetType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdUINodeGraphNodeAPI::GetPosAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodePos);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreatePosAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodePos,
    SdfValueTypeNames->Float2,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUINodeGraphNodeAPI::GetStackingOrderAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodeStackingOrder);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreateStackingOrderAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodeStackingOrder,
    SdfValueTypeNames->Int,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUINodeGraphNodeAPI::GetDisplayColorAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodeDisplayColor);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreateDisplayColorAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodeDisplayColor,
    SdfValueTypeNames->Color3f,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUINodeGraphNodeAPI::GetIconAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodeIcon);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreateIconAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodeIcon,
    SdfValueTypeNames->Asset,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUINodeGraphNodeAPI::GetExpansionStateAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodeExpansionState);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreateExpansionStateAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodeExpansionState,
    SdfValueTypeNames->Token,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
}

UsdAttribute UsdUINodeGraphNodeAPI::GetSizeAttr() const
{
  return GetPrim().GetAttribute(UsdUITokens->uiNodegraphNodeSize);
}

UsdAttribute UsdUINodeGraphNodeAPI::CreateSizeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(
    UsdUITokens->uiNodegraphNodeSize,
    SdfValueTypeNames->Float2,
    /* custom = */ false,
    SdfVariabilityUniform,
    defaultValue,
    writeSparsely);
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
const TfTokenVector& UsdUINodeGraphNodeAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdUITokens->uiNodegraphNodePos,
    UsdUITokens->uiNodegraphNodeStackingOrder,
    UsdUITokens->uiNodegraphNodeDisplayColor,
    UsdUITokens->uiNodegraphNodeIcon,
    UsdUITokens->uiNodegraphNodeExpansionState,
    UsdUITokens->uiNodegraphNodeSize,
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

/* clang-format on */
