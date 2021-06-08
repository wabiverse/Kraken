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
#include "wabi/usd/usdGeom/input.h"
#include "wabi/wabi.h"

#include "wabi/usd/usdGeom/connectableAPI.h"
#include "wabi/usd/usdGeom/output.h"
#include "wabi/usd/usdGeom/utils.h"

#include "wabi/usd/sdf/schema.h"
#include "wabi/usd/usd/relationship.h"

#include "wabi/usd/usdGeom/connectableAPI.h"

#include "wabi/base/tf/smallVector.h"

#include <algorithm>
#include <stdlib.h>

WABI_NAMESPACE_BEGIN

using std::string;
using std::vector;

TF_DEFINE_PRIVATE_TOKENS(_tokens, (connectability)(primitiveType));

UsdGeomInput::UsdGeomInput(const UsdAttribute &attr) : _attr(attr)
{}

TfToken UsdGeomInput::GetBaseName() const
{
  string name = GetFullName();
  if (TfStringStartsWith(name, UsdGeomTokens->inputs)) {
    return TfToken(name.substr(UsdGeomTokens->inputs.GetString().size()));
  }

  return GetFullName();
}

SdfValueTypeName UsdGeomInput::GetTypeName() const
{
  return _attr.GetTypeName();
}

static TfToken _GetInputAttrName(const TfToken inputName)
{
  return TfToken(UsdGeomTokens->inputs.GetString() + inputName.GetString());
}

UsdGeomInput::UsdGeomInput(UsdPrim prim, TfToken const &name, SdfValueTypeName const &typeName)
{
  // XXX what do we do if the type name doesn't match and it exists already?
  TfToken inputAttrName = _GetInputAttrName(name);
  if (prim.HasAttribute(inputAttrName)) {
    _attr = prim.GetAttribute(inputAttrName);
  }

  if (!_attr) {
    _attr = prim.CreateAttribute(inputAttrName,
                                 typeName,
                                 /* custom = */ false);
  }
}

bool UsdGeomInput::Get(VtValue *value, UsdTimeCode time) const
{
  if (!_attr) {
    return false;
  }

  return _attr.Get(value, time);
}

bool UsdGeomInput::Set(const VtValue &value, UsdTimeCode time) const
{
  return _attr.Set(value, time);
}

bool UsdGeomInput::SetPrimitiveType(TfToken const &primitiveType) const
{
  return _attr.SetMetadata(_tokens->primitiveType, primitiveType);
}

TfToken UsdGeomInput::GetPrimitiveType() const
{
  TfToken primitiveType;
  _attr.GetMetadata(_tokens->primitiveType, &primitiveType);
  return primitiveType;
}

bool UsdGeomInput::HasPrimitiveType() const
{
  return _attr.HasMetadata(_tokens->primitiveType);
}

NdrTokenMap UsdGeomInput::GetGdrMetadata() const
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

std::string UsdGeomInput::GetGdrMetadataByKey(const TfToken &key) const
{
  VtValue val;
  GetAttr().GetMetadataByDictKey(UsdGeomTokens->gdrMetadata, key, &val);
  return TfStringify(val);
}

void UsdGeomInput::SetGdrMetadata(const NdrTokenMap &gdrMetadata) const
{
  for (auto &i : gdrMetadata) {
    SetGdrMetadataByKey(i.first, i.second);
  }
}

void UsdGeomInput::SetGdrMetadataByKey(const TfToken &key, const std::string &value) const
{
  GetAttr().SetMetadataByDictKey(UsdGeomTokens->gdrMetadata, key, value);
}

bool UsdGeomInput::HasGdrMetadata() const
{
  return GetAttr().HasMetadata(UsdGeomTokens->gdrMetadata);
}

bool UsdGeomInput::HasGdrMetadataByKey(const TfToken &key) const
{
  return GetAttr().HasMetadataDictKey(UsdGeomTokens->gdrMetadata, key);
}

void UsdGeomInput::ClearGdrMetadata() const
{
  GetAttr().ClearMetadata(UsdGeomTokens->gdrMetadata);
}

void UsdGeomInput::ClearGdrMetadataByKey(const TfToken &key) const
{
  GetAttr().ClearMetadataByDictKey(UsdGeomTokens->gdrMetadata, key);
}

/* static */
bool UsdGeomInput::IsInput(const UsdAttribute &attr)
{
  return attr && attr.IsDefined() &&
         TfStringStartsWith(attr.GetName().GetString(), UsdGeomTokens->inputs);
}

/* static */
bool UsdGeomInput::IsInterfaceInputName(const std::string &name)
{
  if (TfStringStartsWith(name, UsdGeomTokens->inputs)) {
    return true;
  }

  return false;
}

bool UsdGeomInput::SetDocumentation(const std::string &docs) const
{
  if (!_attr) {
    return false;
  }

  return _attr.SetDocumentation(docs);
}

std::string UsdGeomInput::GetDocumentation() const
{
  if (!_attr) {
    return "";
  }

  return _attr.GetDocumentation();
}

bool UsdGeomInput::SetDisplayGroup(const std::string &docs) const
{
  if (!_attr) {
    return false;
  }

  return _attr.SetDisplayGroup(docs);
}

std::string UsdGeomInput::GetDisplayGroup() const
{
  if (!_attr) {
    return "";
  }

  return _attr.GetDisplayGroup();
}

bool UsdGeomInput::CanConnect(const UsdAttribute &source) const
{
  return UsdGeomConnectableAPI::CanConnect(*this, source);
}

bool UsdGeomInput::CanConnect(const UsdGeomInput &sourceInput) const
{
  return CanConnect(sourceInput.GetAttr());
}

bool UsdGeomInput::CanConnect(const UsdGeomOutput &sourceOutput) const
{
  return CanConnect(sourceOutput.GetAttr());
}

bool UsdGeomInput::ConnectToSource(UsdGeomConnectionSourceInfo const &source,
                                   ConnectionModification const mod) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, source, mod);
}

bool UsdGeomInput::ConnectToSource(UsdGeomConnectableAPI const &source,
                                   TfToken const &sourceName,
                                   UsdGeomAttributeType const sourceType,
                                   SdfValueTypeName typeName) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, source, sourceName, sourceType, typeName);
}

bool UsdGeomInput::ConnectToSource(SdfPath const &sourcePath) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourcePath);
}

bool UsdGeomInput::ConnectToSource(UsdGeomInput const &sourceInput) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourceInput);
}

bool UsdGeomInput::ConnectToSource(UsdGeomOutput const &sourceOutput) const
{
  return UsdGeomConnectableAPI::ConnectToSource(*this, sourceOutput);
}

bool UsdGeomInput::SetConnectedSources(
    std::vector<UsdGeomConnectionSourceInfo> const &sourceInfos) const
{
  return UsdGeomConnectableAPI::SetConnectedSources(*this, sourceInfos);
}

UsdGeomInput::SourceInfoVector UsdGeomInput::GetConnectedSources(
    SdfPathVector *invalidSourcePaths) const
{
  return UsdGeomConnectableAPI::GetConnectedSources(*this, invalidSourcePaths);
}

bool UsdGeomInput::GetConnectedSource(UsdGeomConnectableAPI *source,
                                      TfToken *sourceName,
                                      UsdGeomAttributeType *sourceType) const
{
  return UsdGeomConnectableAPI::GetConnectedSource(*this, source, sourceName, sourceType);
}

bool UsdGeomInput::GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const
{
  return UsdGeomConnectableAPI::GetRawConnectedSourcePaths(*this, sourcePaths);
}

bool UsdGeomInput::HasConnectedSource() const
{
  return UsdGeomConnectableAPI::HasConnectedSource(*this);
}

bool UsdGeomInput::IsSourceConnectionFromBaseMaterial() const
{
  return UsdGeomConnectableAPI::IsSourceConnectionFromBaseMaterial(*this);
}

bool UsdGeomInput::DisconnectSource(UsdAttribute const &sourceAttr) const
{
  return UsdGeomConnectableAPI::DisconnectSource(*this, sourceAttr);
}

bool UsdGeomInput::ClearSources() const
{
  return UsdGeomConnectableAPI::ClearSources(*this);
}

bool UsdGeomInput::ClearSource() const
{
  return UsdGeomConnectableAPI::ClearSources(*this);
}

bool UsdGeomInput::SetConnectability(const TfToken &connectability) const
{
  return _attr.SetMetadata(_tokens->connectability, connectability);
}

TfToken UsdGeomInput::GetConnectability() const
{
  TfToken connectability;
  _attr.GetMetadata(_tokens->connectability, &connectability);

  // If there's an authored non-empty connectability value, then return it.
  // If not, return "full".
  if (!connectability.IsEmpty()) {
    return connectability;
  }

  return UsdGeomTokens->full;
}

bool UsdGeomInput::ClearConnectability() const
{
  return _attr.ClearMetadata(_tokens->connectability);
}

UsdGeomAttributeVector UsdGeomInput::GetValueProducingAttributes(bool geomOutputsOnly) const
{
  return UsdGeomUtils::GetValueProducingAttributes(*this, geomOutputsOnly);
}

UsdAttribute UsdGeomInput::GetValueProducingAttribute(UsdGeomAttributeType *attrType) const
{
  // Call the multi-connection aware version
  UsdGeomAttributeVector valueAttrs = UsdGeomUtils::GetValueProducingAttributes(*this);

  if (valueAttrs.empty()) {
    if (attrType) {
      *attrType = UsdGeomAttributeType::Invalid;
    }
    return UsdAttribute();
  }
  else {
    // If we have valid connections extract the first one
    if (valueAttrs.size() > 1) {
      TF_WARN(
          "More than one value producing attribute for shading input "
          "%s. GetValueProducingAttribute will only report the first "
          "one. Please use GetValueProducingAttributes to retrieve "
          "all.",
          GetAttr().GetPath().GetText());
    }

    UsdAttribute attr = valueAttrs[0];
    if (attrType) {
      *attrType = UsdGeomUtils::GetType(attr.GetName());
    }

    return attr;
  }
}

WABI_NAMESPACE_END
