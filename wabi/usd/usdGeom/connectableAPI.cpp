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
#include "wabi/usd/usdGeom/connectableAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomConnectableAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (GeomConnectableAPI));

/* virtual */
UsdGeomConnectableAPI::~UsdGeomConnectableAPI()
{}

/* static */
UsdGeomConnectableAPI UsdGeomConnectableAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomConnectableAPI();
  }
  return UsdGeomConnectableAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdGeomConnectableAPI::_GetSchemaKind() const
{
  return UsdGeomConnectableAPI::schemaKind;
}

/* virtual */
UsdSchemaKind UsdGeomConnectableAPI::_GetSchemaType() const
{
  return UsdGeomConnectableAPI::schemaType;
}

/* static */
const TfType &UsdGeomConnectableAPI::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdGeomConnectableAPI>();
  return tfType;
}

/* static */
bool UsdGeomConnectableAPI::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdGeomConnectableAPI::_GetTfType() const
{
  return _GetStaticTfType();
}

/*static*/
const TfTokenVector &UsdGeomConnectableAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames;
  static TfTokenVector allNames = UsdAPISchemaBase::GetSchemaAttributeNames(true);

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

#include "wabi/usd/pcp/layerStack.h"
#include "wabi/usd/pcp/node.h"
#include "wabi/usd/pcp/primIndex.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/usd/sdf/attributeSpec.h"
#include "wabi/usd/sdf/propertySpec.h"
#include "wabi/usd/sdf/relationshipSpec.h"
#include "wabi/usd/usdGeom/tokens.h"
#include "wabi/usd/usdGeom/utils.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens, (outputName)(outputs));

static UsdAttribute _GetOrCreateSourceAttr(UsdGeomConnectionSourceInfo const &sourceInfo,
                                           SdfValueTypeName fallbackTypeName)
{
  // Note, the validity of sourceInfo has been checked in ConnectToSource and
  // SetConnectedSources, which includes a check of source, sourceType and
  // sourceName
  UsdPrim sourcePrim = sourceInfo.source.GetPrim();

  std::string prefix = UsdGeomUtils::GetPrefixForAttributeType(sourceInfo.sourceType);
  TfToken sourceAttrName(prefix + sourceInfo.sourceName.GetString());

  UsdAttribute sourceAttr = sourcePrim.GetAttribute(sourceAttrName);

  // If a source attribute doesn't exist on the sourcePrim we create one with
  // the proper type
  if (!sourceAttr) {
    sourceAttr = sourcePrim.CreateAttribute(sourceAttrName,
                                            // If typeName isn't valid use
                                            // the fallback
                                            sourceInfo.typeName ? sourceInfo.typeName :
                                                                  fallbackTypeName,
                                            /* custom = */ false);
  }

  return sourceAttr;
}

/* static */
bool UsdGeomConnectableAPI::ConnectToSource(UsdAttribute const &shadingAttr,
                                            UsdGeomConnectionSourceInfo const &source,
                                            ConnectionModification const mod)
{
  if (!source) {
    TF_CODING_ERROR(
        "Failed connecting shading attribute <%s> to "
        "attribute %s%s on prim %s. The given source "
        "information is not valid",
        shadingAttr.GetPath().GetText(),
        UsdGeomUtils::GetPrefixForAttributeType(source.sourceType).c_str(),
        source.sourceName.GetText(),
        source.source.GetPath().GetText());
    return false;
  }

  UsdAttribute sourceAttr = _GetOrCreateSourceAttr(source, shadingAttr.GetTypeName());
  if (!sourceAttr) {
    // _GetOrCreateSourceAttr can only fail if CreateAttribute fails, which
    // will issue an appropriate error
    return false;
  }

  if (mod == ConnectionModification::Replace) {
    return shadingAttr.SetConnections(SdfPathVector{sourceAttr.GetPath()});
  }
  else if (mod == ConnectionModification::Prepend) {
    return shadingAttr.AddConnection(sourceAttr.GetPath(), UsdListPositionFrontOfPrependList);
  }
  else if (mod == ConnectionModification::Append) {
    return shadingAttr.AddConnection(sourceAttr.GetPath(), UsdListPositionBackOfAppendList);
  }

  return false;
}

/* static */
bool UsdGeomConnectableAPI::ConnectToSource(UsdAttribute const &shadingAttr,
                                            UsdGeomConnectableAPI const &source,
                                            TfToken const &sourceName,
                                            UsdGeomAttributeType const sourceType,
                                            SdfValueTypeName typeName)
{
  return ConnectToSource(shadingAttr,
                         UsdGeomConnectionSourceInfo(source, sourceName, sourceType, typeName));
}

/* static */
bool UsdGeomConnectableAPI::ConnectToSource(UsdAttribute const &shadingAttr,
                                            SdfPath const &sourcePath)
{
  return ConnectToSource(shadingAttr,
                         UsdGeomConnectionSourceInfo(shadingAttr.GetStage(), sourcePath));
}

/* static */
bool UsdGeomConnectableAPI::ConnectToSource(UsdAttribute const &shadingAttr,
                                            UsdGeomInput const &sourceInput)
{
  return ConnectToSource(shadingAttr,
                         UsdGeomConnectableAPI(sourceInput.GetPrim()),
                         sourceInput.GetBaseName(),
                         UsdGeomAttributeType::Input,
                         sourceInput.GetTypeName());
}

/* static */
bool UsdGeomConnectableAPI::ConnectToSource(UsdAttribute const &shadingAttr,
                                            UsdGeomOutput const &sourceOutput)
{
  return ConnectToSource(shadingAttr,
                         UsdGeomConnectableAPI(sourceOutput.GetPrim()),
                         sourceOutput.GetBaseName(),
                         UsdGeomAttributeType::Output,
                         sourceOutput.GetTypeName());
}

/* static */
bool UsdGeomConnectableAPI::SetConnectedSources(
    UsdAttribute const &shadingAttr,
    std::vector<UsdGeomConnectionSourceInfo> const &sourceInfos)
{
  SdfPathVector sourcePaths;
  sourcePaths.reserve(sourceInfos.size());

  for (UsdGeomConnectionSourceInfo const &sourceInfo : sourceInfos) {
    if (!sourceInfo) {
      TF_CODING_ERROR(
          "Failed connecting shading attribute <%s> to "
          "attribute %s%s on prim %s. The given information "
          "in `sourceInfos` in is not valid",
          shadingAttr.GetPath().GetText(),
          UsdGeomUtils::GetPrefixForAttributeType(sourceInfo.sourceType).c_str(),
          sourceInfo.sourceName.GetText(),
          sourceInfo.source.GetPath().GetText());
      return false;
    }

    UsdAttribute sourceAttr = _GetOrCreateSourceAttr(sourceInfo, shadingAttr.GetTypeName());
    if (!sourceAttr) {
      // _GetOrCreateSourceAttr can only fail if CreateAttribute fails,
      // which will issue an appropriate error
      return false;
    }

    sourcePaths.push_back(sourceAttr.GetPath());
  }

  return shadingAttr.SetConnections(sourcePaths);
}

/* static */
bool UsdGeomConnectableAPI::GetConnectedSource(UsdAttribute const &shadingAttr,
                                               UsdGeomConnectableAPI *source,
                                               TfToken *sourceName,
                                               UsdGeomAttributeType *sourceType)
{
  TRACE_SCOPE("UsdGeomConnectableAPI::GetConnectedSource");

  if (!(source && sourceName && sourceType)) {
    TF_CODING_ERROR(
        "GetConnectedSource() requires non-NULL "
        "output-parameters.");
    return false;
  }

  UsdGeomSourceInfoVector sourceInfos = GetConnectedSources(shadingAttr);
  if (sourceInfos.empty()) {
    return false;
  }

  if (sourceInfos.size() > 1u) {
    TF_WARN(
        "More than one connection for shading attribute %s. "
        "GetConnectedSource will only report the first one. "
        "Please use GetConnectedSources to retrieve all.",
        shadingAttr.GetPath().GetText());
  }

  UsdGeomConnectionSourceInfo const &sourceInfo = sourceInfos[0];

  *source     = sourceInfo.source;
  *sourceName = sourceInfo.sourceName;
  *sourceType = sourceInfo.sourceType;

  return true;
}

/* static */
UsdGeomSourceInfoVector UsdGeomConnectableAPI::GetConnectedSources(
    UsdAttribute const &shadingAttr,
    SdfPathVector *invalidSourcePaths)
{
  TRACE_SCOPE("UsdGeomConnectableAPI::GetConnectedSources");

  SdfPathVector sourcePaths;
  shadingAttr.GetConnections(&sourcePaths);

  UsdGeomSourceInfoVector sourceInfos;
  if (sourcePaths.empty()) {
    return sourceInfos;
  }

  UsdStagePtr stage = shadingAttr.GetStage();

  sourceInfos.reserve(sourcePaths.size());
  for (SdfPath const &sourcePath : sourcePaths) {

    // Make sure the source attribute exists
    UsdAttribute sourceAttr = stage->GetAttributeAtPath(sourcePath);
    if (!sourceAttr) {
      if (invalidSourcePaths) {
        invalidSourcePaths->push_back(sourcePath);
      }
      continue;
    }

    // Check that the attribute has a legal prefix
    TfToken sourceName;
    UsdGeomAttributeType sourceType;
    std::tie(sourceName, sourceType) = UsdGeomUtils::GetBaseNameAndType(sourcePath.GetNameToken());
    if (sourceType == UsdGeomAttributeType::Invalid) {
      if (invalidSourcePaths) {
        invalidSourcePaths->push_back(sourcePath);
      }
      continue;
    }

    // We do not check whether the UsdGeomConnectableAPI is valid. We
    // implicitly know the prim is valid, since we got a valid attribute.
    // That is the only requirement.
    UsdGeomConnectableAPI source(sourceAttr.GetPrim());

    sourceInfos.emplace_back(source, sourceName, sourceType, sourceAttr.GetTypeName());
  }

  return sourceInfos;
}

// N.B. The implementation of these static methods is in the cpp file, since the
// UsdGeomSourceInfoVector type is not fully defined at the corresponding point
// in the header.

/* static */
UsdGeomSourceInfoVector UsdGeomConnectableAPI::GetConnectedSources(
    UsdGeomInput const &input,
    SdfPathVector *invalidSourcePaths)
{
  return GetConnectedSources(input.GetAttr(), invalidSourcePaths);
}

/* static */
UsdGeomSourceInfoVector UsdGeomConnectableAPI::GetConnectedSources(
    UsdGeomOutput const &output,
    SdfPathVector *invalidSourcePaths)
{
  return GetConnectedSources(output.GetAttr(), invalidSourcePaths);
}

/* static  */
bool UsdGeomConnectableAPI::GetRawConnectedSourcePaths(UsdAttribute const &shadingAttr,
                                                       SdfPathVector *sourcePaths)
{
  return shadingAttr.GetConnections(sourcePaths);
}

/* static */
bool UsdGeomConnectableAPI::HasConnectedSource(const UsdAttribute &shadingAttr)
{
  // This MUST have the same semantics as GetConnectedSources().
  // XXX someday we might make this more efficient through careful
  // refactoring, but safest to just call the exact same code.
  return !GetConnectedSources(shadingAttr).empty();
}

// This tests if a given node represents a "live" base material,
// i.e. once that hasn't been "flattened out" due to being
// pulled across a reference to a library.
static bool _NodeRepresentsLiveBaseMaterial(const PcpNodeRef &node)
{
  bool isLiveBaseMaterial = false;
  for (PcpNodeRef n = node; n;  // 0, or false, means we are at the root node
       n            = n.GetOriginNode()) {
    switch (n.GetArcType()) {
      case PcpArcTypeSpecialize:
        isLiveBaseMaterial = true;
        break;
      // dakrunch: specializes across references are actually still valid.
      //
      // case PcpArcTypeReference:
      //     if (isLiveBaseMaterial) {
      //         // Node is within a base material, but that is in turn
      //         // across a reference. That means this is a library
      //         // material, so it is not live and we should flatten it
      //         // out.  Continue iterating, however, since this
      //         // might be referenced into some other live base material
      //         // downstream.
      //         isLiveBaseMaterial = false;
      //     }
      //     break;
      default:
        break;
    }
  }
  return isLiveBaseMaterial;
}

/* static */
bool UsdGeomConnectableAPI::IsSourceConnectionFromBaseMaterial(const UsdAttribute &shadingAttr)
{
  // USD core doesn't provide a UsdResolveInfo style API for asking where
  // connections are authored, so we do it here ourselves.
  // Find the strongest opinion about connections.
  SdfAttributeSpecHandle strongestAttrSpecWithConnections;
  SdfPropertySpecHandleVector propStack = shadingAttr.GetPropertyStack();
  for (const SdfPropertySpecHandle &prop : propStack) {
    if (SdfAttributeSpecHandle attrSpec = TfDynamic_cast<SdfAttributeSpecHandle>(prop)) {
      if (attrSpec->HasConnectionPaths()) {
        strongestAttrSpecWithConnections = attrSpec;
        break;
      }
    }
  }

  // Find which prim node introduced that opinion.
  if (strongestAttrSpecWithConnections) {
    for (const PcpNodeRef &node : shadingAttr.GetPrim().GetPrimIndex().GetNodeRange()) {
      if (node.GetPath() == strongestAttrSpecWithConnections->GetPath().GetPrimPath() &&
          node.GetLayerStack()->HasLayer(strongestAttrSpecWithConnections->GetLayer())) {
        return _NodeRepresentsLiveBaseMaterial(node);
      }
    }
  }

  return false;
}

/* static */
bool UsdGeomConnectableAPI::DisconnectSource(UsdAttribute const &shadingAttr,
                                             UsdAttribute const &sourceAttr)
{
  if (sourceAttr) {
    return shadingAttr.RemoveConnection(sourceAttr.GetPath());
  }
  else {
    return shadingAttr.SetConnections({});
  }
}

/* static */
bool UsdGeomConnectableAPI::ClearSources(UsdAttribute const &shadingAttr)
{
  return shadingAttr.ClearConnections();
}

UsdGeomOutput UsdGeomConnectableAPI::CreateOutput(const TfToken &name,
                                                  const SdfValueTypeName &typeName) const
{
  return UsdGeomOutput(GetPrim(), name, typeName);
}

UsdGeomOutput UsdGeomConnectableAPI::GetOutput(const TfToken &name) const
{
  TfToken outputAttrName(UsdGeomTokens->outputs.GetString() + name.GetString());
  if (GetPrim().HasAttribute(outputAttrName)) {
    return UsdGeomOutput(GetPrim().GetAttribute(outputAttrName));
  }

  return UsdGeomOutput();
}

std::vector<UsdGeomOutput> UsdGeomConnectableAPI::GetOutputs(bool onlyAuthored) const
{
  std::vector<UsdProperty> props;
  if (onlyAuthored) {
    props = GetPrim().GetAuthoredPropertiesInNamespace(UsdGeomTokens->outputs);
  }
  else {
    props = GetPrim().GetPropertiesInNamespace(UsdGeomTokens->outputs);
  }

  // Filter for attributes and convert them to ouputs
  std::vector<UsdGeomOutput> outputs;
  outputs.reserve(props.size());
  for (UsdProperty const &prop : props) {
    if (UsdAttribute attr = prop.As<UsdAttribute>()) {
      outputs.push_back(UsdGeomOutput(attr));
    }
  }
  return outputs;
}

UsdGeomInput UsdGeomConnectableAPI::CreateInput(const TfToken &name,
                                                const SdfValueTypeName &typeName) const
{
  return UsdGeomInput(GetPrim(), name, typeName);
}

UsdGeomInput UsdGeomConnectableAPI::GetInput(const TfToken &name) const
{
  TfToken inputAttrName(UsdGeomTokens->inputs.GetString() + name.GetString());

  if (GetPrim().HasAttribute(inputAttrName)) {
    return UsdGeomInput(GetPrim().GetAttribute(inputAttrName));
  }

  return UsdGeomInput();
}

std::vector<UsdGeomInput> UsdGeomConnectableAPI::GetInputs(bool onlyAuthored) const
{
  std::vector<UsdProperty> props;
  if (onlyAuthored) {
    props = GetPrim().GetAuthoredPropertiesInNamespace(UsdGeomTokens->inputs);
  }
  else {
    props = GetPrim().GetPropertiesInNamespace(UsdGeomTokens->inputs);
  }

  // Filter for attributes and convert them to inputs
  std::vector<UsdGeomInput> inputs;
  inputs.reserve(props.size());
  for (UsdProperty const &prop : props) {
    if (UsdAttribute attr = prop.As<UsdAttribute>()) {
      inputs.push_back(UsdGeomInput(attr));
    }
  }
  return inputs;
}

UsdGeomConnectionSourceInfo::UsdGeomConnectionSourceInfo(UsdStagePtr const &stage,
                                                         SdfPath const &sourcePath)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return;
  }

  if (!sourcePath.IsPropertyPath()) {
    return;
  }

  std::tie(sourceName, sourceType) = UsdGeomUtils::GetBaseNameAndType(sourcePath.GetNameToken());

  // Check if the prim can be found on the stage and is a
  // UsdGeomConnectableAPI compatible prim
  source = UsdGeomConnectableAPI::Get(stage, sourcePath.GetPrimPath());

  // Note, initialization of typeName is optional, since the target attribute
  // might not exist (yet)
  // XXX try to get attribute from source.GetPrim()?
  UsdAttribute sourceAttr = stage->GetAttributeAtPath(sourcePath);
  if (sourceAttr) {
    typeName = sourceAttr.GetTypeName();
  }
}

WABI_NAMESPACE_END
