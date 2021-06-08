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

#include "wabi/usd/usdGeom/nodeDefAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomNodeDefAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (GeomNodeDefAPI));

/* virtual */
UsdGeomNodeDefAPI::~UsdGeomNodeDefAPI()
{}

/* static */
UsdGeomNodeDefAPI UsdGeomNodeDefAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomNodeDefAPI();
  }
  return UsdGeomNodeDefAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdGeomNodeDefAPI::_GetSchemaKind() const
{
  return UsdGeomNodeDefAPI::schemaKind;
}

/* virtual */
UsdSchemaKind UsdGeomNodeDefAPI::_GetSchemaType() const
{
  return UsdGeomNodeDefAPI::schemaType;
}

/* static */
UsdGeomNodeDefAPI UsdGeomNodeDefAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdGeomNodeDefAPI>()) {
    return UsdGeomNodeDefAPI(prim);
  }
  return UsdGeomNodeDefAPI();
}

/* static */
const TfType &UsdGeomNodeDefAPI::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdGeomNodeDefAPI>();
  return tfType;
}

/* static */
bool UsdGeomNodeDefAPI::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdGeomNodeDefAPI::_GetTfType() const
{
  return _GetStaticTfType();
}

UsdAttribute UsdGeomNodeDefAPI::GetImplementationSourceAttr() const
{
  return GetPrim().GetAttribute(UsdGeomTokens->infoImplementationSource);
}

UsdAttribute UsdGeomNodeDefAPI::CreateImplementationSourceAttr(VtValue const &defaultValue,
                                                               bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdGeomTokens->infoImplementationSource,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdGeomNodeDefAPI::GetIdAttr() const
{
  return GetPrim().GetAttribute(UsdGeomTokens->infoId);
}

UsdAttribute UsdGeomNodeDefAPI::CreateIdAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdGeomTokens->infoId,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

namespace {
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
const TfTokenVector &UsdGeomNodeDefAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
      UsdGeomTokens->infoImplementationSource,
      UsdGeomTokens->infoId,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(
      UsdAPISchemaBase::GetSchemaAttributeNames(true), localNames);

  if (includeInherited)
    return allNames;
  else
    return localNames;
}

WABI_NAMESPACE_END

/**
 * =====================================================================
 *  Feel free to add custom code below this line. It will be preserved
 *  by the code generator.
 *
 *
 *  Just remember to wrap code in the appropriate delimiters:
 *  'WABI_NAMESPACE_BEGIN', 'WABI_NAMESPACE_END'.
 * =====================================================================
 * --(BEGIN CUSTOM CODE)-- */

#include "wabi/usd/gdr/registry.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens,
                         (info)((infoSourceAsset, "info:sourceAsset"))((
                             infoSubIdentifier,
                             "info:sourceAsset:subIdentifier"))((infoSourceCode,
                                                                 "info:sourceCode")));

TfToken UsdGeomNodeDefAPI::GetImplementationSource() const
{
  TfToken implSource;
  GetImplementationSourceAttr().Get(&implSource);

  if (implSource == UsdGeomTokens->id || implSource == UsdGeomTokens->sourceAsset ||
      implSource == UsdGeomTokens->sourceCode) {
    return implSource;
  }
  else {
    TF_WARN(
        "Found invalid info:implementationSource value '%s' on geom "
        "at path <%s>. Falling back to 'id'.",
        implSource.GetText(),
        GetPath().GetText());
    return UsdGeomTokens->id;
  }
}

bool UsdGeomNodeDefAPI::SetGeomId(const TfToken &id) const
{
  return CreateImplementationSourceAttr(VtValue(UsdGeomTokens->id), /*writeSparsely*/ true) &&
         GetIdAttr().Set(id);
}

bool UsdGeomNodeDefAPI::GetGeomId(TfToken *id) const
{
  TfToken implSource = GetImplementationSource();
  if (implSource == UsdGeomTokens->id) {
    return GetIdAttr().Get(id);
  }
  return false;
}

static TfToken _GetSourceAssetAttrName(const TfToken &sourceType)
{
  if (sourceType == UsdGeomTokens->universalSourceType) {
    return _tokens->infoSourceAsset;
  }
  return TfToken(SdfPath::JoinIdentifier(
      TfTokenVector{_tokens->info, sourceType, UsdGeomTokens->sourceAsset}));
}

bool UsdGeomNodeDefAPI::SetSourceAsset(const SdfAssetPath &sourceAsset,
                                       const TfToken &sourceType) const
{
  TfToken sourceAssetAttrName = _GetSourceAssetAttrName(sourceType);
  return CreateImplementationSourceAttr(VtValue(UsdGeomTokens->sourceAsset)) &&
         UsdSchemaBase::_CreateAttr(sourceAssetAttrName,
                                    SdfValueTypeNames->Asset,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    VtValue(sourceAsset),
                                    /* writeSparsely */ false);
}

bool UsdGeomNodeDefAPI::GetSourceAsset(SdfAssetPath *sourceAsset, const TfToken &sourceType) const
{
  TfToken implSource = GetImplementationSource();
  if (implSource != UsdGeomTokens->sourceAsset) {
    return false;
  }

  TfToken sourceAssetAttrName  = _GetSourceAssetAttrName(sourceType);
  UsdAttribute sourceAssetAttr = GetPrim().GetAttribute(sourceAssetAttrName);
  if (sourceAssetAttr) {
    return sourceAssetAttr.Get(sourceAsset);
  }

  if (sourceType != UsdGeomTokens->universalSourceType) {
    UsdAttribute univSourceAssetAttr = GetPrim().GetAttribute(
        _GetSourceAssetAttrName(UsdGeomTokens->universalSourceType));
    if (univSourceAssetAttr) {
      return univSourceAssetAttr.Get(sourceAsset);
    }
  }

  return false;
}

static TfToken _GetSourceAssetSubIdentifierAttrName(const TfToken &sourceType)
{
  if (sourceType == UsdGeomTokens->universalSourceType) {
    return _tokens->infoSubIdentifier;
  }
  return TfToken(SdfPath::JoinIdentifier(TfTokenVector{
      _tokens->info, sourceType, UsdGeomTokens->sourceAsset, UsdGeomTokens->subIdentifier}));
}

bool UsdGeomNodeDefAPI::SetSourceAssetSubIdentifier(const TfToken &subIdentifier,
                                                    const TfToken &sourceType) const
{
  TfToken subIdentifierAttrName = _GetSourceAssetSubIdentifierAttrName(sourceType);
  return CreateImplementationSourceAttr(VtValue(UsdGeomTokens->sourceAsset)) &&
         UsdSchemaBase::_CreateAttr(subIdentifierAttrName,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    VtValue(subIdentifier),
                                    /* writeSparsely */ false);
}

bool UsdGeomNodeDefAPI::GetSourceAssetSubIdentifier(TfToken *subIdentifier,
                                                    const TfToken &sourceType) const
{
  TfToken implSource = GetImplementationSource();
  if (implSource != UsdGeomTokens->sourceAsset) {
    return false;
  }

  TfToken subIdentifierAttrName  = _GetSourceAssetSubIdentifierAttrName(sourceType);
  UsdAttribute subIdentifierAttr = GetPrim().GetAttribute(subIdentifierAttrName);
  if (subIdentifierAttr) {
    return subIdentifierAttr.Get(subIdentifier);
  }

  if (sourceType != UsdGeomTokens->universalSourceType) {
    UsdAttribute univSubIdentifierAttr = GetPrim().GetAttribute(
        _GetSourceAssetSubIdentifierAttrName(UsdGeomTokens->universalSourceType));
    if (univSubIdentifierAttr) {
      return univSubIdentifierAttr.Get(subIdentifier);
    }
  }

  return false;
}

static TfToken _GetSourceCodeAttrName(const TfToken &sourceType)
{
  if (sourceType == UsdGeomTokens->universalSourceType) {
    return _tokens->infoSourceCode;
  }
  return TfToken(SdfPath::JoinIdentifier(
      TfTokenVector{_tokens->info, sourceType, UsdGeomTokens->sourceCode}));
}

bool UsdGeomNodeDefAPI::SetSourceCode(const std::string &sourceCode,
                                      const TfToken &sourceType) const
{
  TfToken sourceCodeAttrName = _GetSourceCodeAttrName(sourceType);
  return CreateImplementationSourceAttr(VtValue(UsdGeomTokens->sourceCode)) &&
         UsdSchemaBase::_CreateAttr(sourceCodeAttrName,
                                    SdfValueTypeNames->String,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    VtValue(sourceCode),
                                    /* writeSparsely */ false);
}

bool UsdGeomNodeDefAPI::GetSourceCode(std::string *sourceCode, const TfToken &sourceType) const
{
  TfToken implSource = GetImplementationSource();
  if (implSource != UsdGeomTokens->sourceCode) {
    return false;
  }

  TfToken sourceCodeAttrName  = _GetSourceCodeAttrName(sourceType);
  UsdAttribute sourceCodeAttr = GetPrim().GetAttribute(sourceCodeAttrName);
  if (sourceCodeAttr) {
    return sourceCodeAttr.Get(sourceCode);
  }

  if (sourceType != UsdGeomTokens->universalSourceType) {
    UsdAttribute univSourceCodeAttr = GetPrim().GetAttribute(
        _GetSourceCodeAttrName(UsdGeomTokens->universalSourceType));
    if (univSourceCodeAttr) {
      return univSourceCodeAttr.Get(sourceCode);
    }
  }

  return false;
}

static NdrTokenMap _GetGdrMetadata(UsdPrim const &prim)
{
  NdrTokenMap result;

  VtDictionary gdrMetadata;
  if (prim.GetMetadata(UsdGeomTokens->gdrMetadata, &gdrMetadata)) {
    for (const auto &it : gdrMetadata) {
      result[TfToken(it.first)] = TfStringify(it.second);
    }
  }

  return result;
}

GdrGeomNodeConstPtr UsdGeomNodeDefAPI::GetGeomNodeForSourceType(const TfToken &sourceType) const
{
  TfToken implSource = GetImplementationSource();
  if (implSource == UsdGeomTokens->id) {
    TfToken geomId;
    if (GetGeomId(&geomId)) {
      return GdrRegistry::GetInstance().GetGeomNodeByIdentifierAndType(geomId, sourceType);
    }
  }
  else if (implSource == UsdGeomTokens->sourceAsset) {
    SdfAssetPath sourceAsset;
    if (GetSourceAsset(&sourceAsset, sourceType)) {
      TfToken subIdentifier;
      GetSourceAssetSubIdentifier(&subIdentifier, sourceType);
      return GdrRegistry::GetInstance().GetGeomNodeFromAsset(
          sourceAsset, _GetGdrMetadata(GetPrim()), subIdentifier, sourceType);
    }
  }
  else if (implSource == UsdGeomTokens->sourceCode) {
    std::string sourceCode;
    if (GetSourceCode(&sourceCode, sourceType)) {
      return GdrRegistry::GetInstance().GetGeomNodeFromSourceCode(
          sourceCode, sourceType, _GetGdrMetadata(GetPrim()));
    }
  }

  return nullptr;
}

WABI_NAMESPACE_END
