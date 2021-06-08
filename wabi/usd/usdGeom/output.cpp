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
#include "wabi/usd/usdGeom/output.h"
#include "wabi/wabi.h"

#include "wabi/usd/usdGeom/connectableAPI.h"
#include "wabi/usd/usdGeom/input.h"
#include "wabi/usd/usdGeom/utils.h"

#include "wabi/usd/sdf/schema.h"

#include "wabi/usd/usdGeom/connectableAPI.h"

#include <algorithm>
#include <stdlib.h>

WABI_NAMESPACE_BEGIN

using std::string;
using std::vector;

TF_DEFINE_PRIVATE_TOKENS(_tokens, (primitiveType));

UsdGeomOutput::UsdGeomOutput(const UsdAttribute &attr) : _attr(attr)
{}

TfToken UsdGeomOutput::GetBaseName() const
{
  return TfToken(SdfPath::StripPrefixNamespace(GetFullName(), UsdGeomTokens->outputs).first);
}

SdfValueTypeName UsdGeomOutput::GetTypeName() const
{
  return _attr.GetTypeName();
}

static TfToken _GetOutputAttrName(const TfToken outputName)
{
  return TfToken(UsdGeomTokens->outputs.GetString() + outputName.GetString());
}

UsdGeomOutput::UsdGeomOutput(UsdPrim prim, TfToken const &name, SdfValueTypeName const &typeName)
{
  // XXX what do we do if the type name doesn't match and it exists already?
  TfToken attrName = _GetOutputAttrName(name);
  _attr            = prim.GetAttribute(attrName);
  if (!_attr) {
    _attr = prim.CreateAttribute(attrName, typeName, /* custom = */ false);
  }
}

bool UsdGeomOutput::Set(const VtValue &value, UsdTimeCode time) const
{
  if (UsdAttribute attr = GetAttr()) {
    return attr.Set(value, time);
  }
  return false;
}

bool UsdGeomOutput::SetPrimitiveType(TfToken const &primitiveType) const
{
  return _attr.SetMetadata(_tokens->primitiveType, primitiveType);
}

TfToken UsdGeomOutput::GetPrimitiveType() const
{
  TfToken primitiveType;
  _attr.GetMetadata(_tokens->primitiveType, &primitiveType);
  return primitiveType;
}

bool UsdGeomOutput::HasPrimitiveType() const
{
  return _attr.HasMetadata(_tokens->primitiveType);
}

NdrTokenMap UsdGeomOutput::GetGdrMetadata() const
{
  NdrTokenMap result;

  VtDictionary gdrMetadata;
  if (GetAttr().GetMetadata(UsdGeomTokens->gdrMetadata, &gdrMetadata)) {
    for (const auto &it : gdrMetadata) {
      result[TfToken(it.first)] = TfStringify(it.second);
    }
  }

  return result;
}

std::string UsdGeomOutput::GetGdrMetadataByKey(const TfToken &key) const
{
  VtValue val;
  GetAttr().GetMetadataByDictKey(UsdGeomTokens->gdrMetadata, key, &val);
  return TfStringify(val);
}

void UsdGeomOutput::SetGdrMetadata(const NdrTokenMap &gdrMetadata) const
{
  for (auto &i : gdrMetadata) {
    SetGdrMetadataByKey(i.first, i.second);
  }
}

void UsdGeomOutput::SetGdrMetadataByKey(const TfToken &key, const std::string &value) const
{
  GetAttr().SetMetadataByDictKey(UsdGeomTokens->gdrMetadata, key, value);
}

bool UsdGeomOutput::HasGdrMetadata() const
{
  return GetAttr().HasMetadata(UsdGeomTokens->gdrMetadata);
}

bool UsdGeomOutput::HasGdrMetadataByKey(const TfToken &key) const
{
  return GetAttr().HasMetadataDictKey(UsdGeomTokens->gdrMetadata, key);
}

void UsdGeomOutput::ClearGdrMetadata() const
{
  GetAttr().ClearMetadata(UsdGeomTokens->gdrMetadata);
}

void UsdGeomOutput::ClearGdrMetadataByKey(const TfToken &key) const
{
  GetAttr().ClearMetadataByDictKey(UsdGeomTokens->gdrMetadata, key);
}

/* static */
bool UsdGeomOutput::IsOutput(const UsdAttribute &attr)
{
  return TfStringStartsWith(attr.GetName().GetString(), UsdGeomTokens->outputs);
}

bool UsdGeomOutput::CanConnect(const UsdAttribute &source) const
{
  return UsdGeomConnectableAPI::CanConnect(*this, source);
}

bool UsdGeomOutput::CanConnect(const UsdGeomInput &sourceInput) const
{
  return CanConnect(sourceInput.GetAttr());
}

bool UsdGeomOutput::CanConnect(const UsdGeomOutput &sourceOutput) const
{
  return CanConnect(sourceOutput.GetAttr());
}

bool UsdGeomOutput::ConnectToSource(UsdGeomConnectionSourceInfo const &source,
                                    ConnectionModification const mod) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, source, mod);
}

bool UsdGeomOutput::ConnectToSource(UsdGeomConnectableAPI const &source,
                                    TfToken const &sourceName,
                                    UsdGeomAttributeType const sourceType,
                                    SdfValueTypeName typeName) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, source, sourceName, sourceType, typeName);
}

bool UsdGeomOutput::ConnectToSource(SdfPath const &sourcePath) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourcePath);
}

bool UsdGeomOutput::ConnectToSource(UsdGeomInput const &sourceInput) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourceInput);
}

bool UsdGeomOutput::ConnectToSource(UsdGeomOutput const &sourceOutput) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourceOutput);
}

bool UsdGeomOutput::SetConnectedSources(
    std::vector<UsdGeomConnectionSourceInfo> const &sourceInfos) const
{
  return UsdGeomConnectableAPI::SetConnectedSources(*this, sourceInfos);
}

UsdGeomOutput::SourceInfoVector UsdGeomOutput::GetConnectedSources(
    SdfPathVector *invalidSourcePaths) const
{
  return UsdGeomConnectableAPI::GetConnectedSources(*this, invalidSourcePaths);
}

bool UsdGeomOutput::GetConnectedSource(UsdGeomConnectableAPI *source,
                                       TfToken *sourceName,
                                       UsdGeomAttributeType *sourceType) const
{
  return UsdGeomConnectableAPI::GetConnectedSource(*this, source, sourceName, sourceType);
}

bool UsdGeomOutput::GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const
{
  return UsdGeomConnectableAPI::GetRawConnectedSourcePaths(*this, sourcePaths);
}

bool UsdGeomOutput::HasConnectedSource() const
{
  return UsdGeomConnectableAPI::HasConnectedSource(*this);
}

bool UsdGeomOutput::IsSourceConnectionFromBaseMaterial() const
{
  return UsdGeomConnectableAPI::IsSourceConnectionFromBaseMaterial(*this);
}

bool UsdGeomOutput::DisconnectSource(UsdAttribute const &sourceAttr) const
{
  return UsdGeomConnectableAPI::DisconnectSource(*this, sourceAttr);
}

bool UsdGeomOutput::ClearSources() const
{
  return UsdGeomConnectableAPI::ClearSources(*this);
}

bool UsdGeomOutput::ClearSource() const
{
  return UsdGeomConnectableAPI::ClearSources(*this);
}

UsdGeomAttributeVector UsdGeomOutput::GetValueProducingAttributes(bool geomOutputsOnly) const
{
  return UsdGeomUtils::GetValueProducingAttributes(*this, geomOutputsOnly);
}

WABI_NAMESPACE_END
