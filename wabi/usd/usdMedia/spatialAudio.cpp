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
#include "wabi/usd/usdMedia/spatialAudio.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdMediaSpatialAudio, TfType::Bases<UsdGeomXformable>>();

  // Register the usd prim typename as an alias under UsdSchemaBase. This
  // enables one to call
  // TfType::Find<UsdSchemaBase>().FindDerivedByName("SpatialAudio")
  // to find TfType<UsdMediaSpatialAudio>, which is how IsA queries are
  // answered.
  TfType::AddAlias<UsdSchemaBase, UsdMediaSpatialAudio>("SpatialAudio");
}

/* virtual */
UsdMediaSpatialAudio::~UsdMediaSpatialAudio() {}

/* static */
UsdMediaSpatialAudio UsdMediaSpatialAudio::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdMediaSpatialAudio();
  }
  return UsdMediaSpatialAudio(stage->GetPrimAtPath(path));
}

/* static */
UsdMediaSpatialAudio UsdMediaSpatialAudio::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("SpatialAudio");
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdMediaSpatialAudio();
  }
  return UsdMediaSpatialAudio(stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdMediaSpatialAudio::_GetSchemaKind() const
{
  return UsdMediaSpatialAudio::schemaKind;
}

/* static */
const TfType &UsdMediaSpatialAudio::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdMediaSpatialAudio>();
  return tfType;
}

/* static */
bool UsdMediaSpatialAudio::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdMediaSpatialAudio::_GetTfType() const
{
  return _GetStaticTfType();
}

UsdAttribute UsdMediaSpatialAudio::GetFilePathAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->filePath);
}

UsdAttribute UsdMediaSpatialAudio::CreateFilePathAttr(VtValue const &defaultValue,
                                                      bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->filePath,
                                    SdfValueTypeNames->Asset,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetAuralModeAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->auralMode);
}

UsdAttribute UsdMediaSpatialAudio::CreateAuralModeAttr(VtValue const &defaultValue,
                                                       bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->auralMode,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetPlaybackModeAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->playbackMode);
}

UsdAttribute UsdMediaSpatialAudio::CreatePlaybackModeAttr(VtValue const &defaultValue,
                                                          bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->playbackMode,
                                    SdfValueTypeNames->Token,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetStartTimeAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->startTime);
}

UsdAttribute UsdMediaSpatialAudio::CreateStartTimeAttr(VtValue const &defaultValue,
                                                       bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->startTime,
                                    SdfValueTypeNames->TimeCode,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetEndTimeAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->endTime);
}

UsdAttribute UsdMediaSpatialAudio::CreateEndTimeAttr(VtValue const &defaultValue,
                                                     bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->endTime,
                                    SdfValueTypeNames->TimeCode,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetMediaOffsetAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->mediaOffset);
}

UsdAttribute UsdMediaSpatialAudio::CreateMediaOffsetAttr(VtValue const &defaultValue,
                                                         bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->mediaOffset,
                                    SdfValueTypeNames->Double,
                                    /* custom = */ false,
                                    SdfVariabilityUniform,
                                    defaultValue,
                                    writeSparsely);
}

UsdAttribute UsdMediaSpatialAudio::GetGainAttr() const
{
  return GetPrim().GetAttribute(UsdMediaTokens->gain);
}

UsdAttribute UsdMediaSpatialAudio::CreateGainAttr(VtValue const &defaultValue,
                                                  bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdMediaTokens->gain,
                                    SdfValueTypeNames->Double,
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
const TfTokenVector &UsdMediaSpatialAudio::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdMediaTokens->filePath,
    UsdMediaTokens->auralMode,
    UsdMediaTokens->playbackMode,
    UsdMediaTokens->startTime,
    UsdMediaTokens->endTime,
    UsdMediaTokens->mediaOffset,
    UsdMediaTokens->gain,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(
    UsdGeomXformable::GetSchemaAttributeNames(true),
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
