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
#include "wabi/usd/usdGeom/nodeGraph.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomNodeGraph, TfType::Bases<UsdTyped>>();

  // Register the usd prim typename as an alias under UsdSchemaBase. This
  // enables one to call
  // TfType::Find<UsdSchemaBase>().FindDerivedByName("GeomNodeGraph")
  // to find TfType<UsdGeomNodeGraph>, which is how IsA queries are
  // answered.
  TfType::AddAlias<UsdSchemaBase, UsdGeomNodeGraph>("GeomNodeGraph");
}

/* virtual */
UsdGeomNodeGraph::~UsdGeomNodeGraph()
{}

/* static */
UsdGeomNodeGraph UsdGeomNodeGraph::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomNodeGraph();
  }
  return UsdGeomNodeGraph(stage->GetPrimAtPath(path));
}

/* static */
UsdGeomNodeGraph UsdGeomNodeGraph::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("GeomNodeGraph");
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomNodeGraph();
  }
  return UsdGeomNodeGraph(stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdGeomNodeGraph::_GetSchemaKind() const
{
  return UsdGeomNodeGraph::schemaKind;
}

/* virtual */
UsdSchemaKind UsdGeomNodeGraph::_GetSchemaType() const
{
  return UsdGeomNodeGraph::schemaType;
}

/* static */
const TfType &UsdGeomNodeGraph::_GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdGeomNodeGraph>();
  return tfType;
}

/* static */
bool UsdGeomNodeGraph::_IsTypedSchema()
{
  static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdGeomNodeGraph::_GetTfType() const
{
  return _GetStaticTfType();
}

/*static*/
const TfTokenVector &UsdGeomNodeGraph::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames;
  static TfTokenVector allNames = UsdTyped::GetSchemaAttributeNames(true);

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

#include "wabi/usd/usd/primRange.h"
#include "wabi/usd/usdGeom/connectableAPI.h"
#include "wabi/usd/usdGeom/utils.h"

WABI_NAMESPACE_BEGIN

UsdGeomNodeGraph::UsdGeomNodeGraph(const UsdGeomConnectableAPI &connectable)
    : UsdGeomNodeGraph(connectable.GetPrim())
{}

UsdGeomConnectableAPI UsdGeomNodeGraph::ConnectableAPI() const
{
  return UsdGeomConnectableAPI(GetPrim());
}

UsdGeomOutput UsdGeomNodeGraph::CreateOutput(const TfToken &name,
                                             const SdfValueTypeName &typeName) const
{
  return UsdGeomConnectableAPI(GetPrim()).CreateOutput(name, typeName);
}

UsdGeomOutput UsdGeomNodeGraph::GetOutput(const TfToken &name) const
{
  return UsdGeomConnectableAPI(GetPrim()).GetOutput(name);
}

std::vector<UsdGeomOutput> UsdGeomNodeGraph::GetOutputs(bool onlyAuthored) const
{
  return UsdGeomConnectableAPI(GetPrim()).GetOutputs(onlyAuthored);
}

UsdGeomImageable UsdGeomNodeGraph::ComputeOutputSource(const TfToken &outputName,
                                                       TfToken *sourceName,
                                                       UsdGeomAttributeType *sourceType) const
{
  // Check that we have a legit output
  UsdGeomOutput output = GetOutput(outputName);
  if (!output) {
    return UsdGeomImageable();
  }

  UsdGeomAttributeVector valueAttrs = UsdGeomUtils::GetValueProducingAttributes(output);

  if (valueAttrs.empty()) {
    return UsdGeomImageable();
  }

  if (valueAttrs.size() > 1) {
    TF_WARN(
        "Found multiple upstream attributes for output %s on GeomNodeGraph "
        "%s. ComputeOutputSource will only report the first upsteam "
        "UsdGeomImageable. Please use GetValueProducingAttributes to "
        "retrieve all.",
        outputName.GetText(),
        GetPath().GetText());
  }

  UsdAttribute attr                  = valueAttrs[0];
  std::tie(*sourceName, *sourceType) = UsdGeomUtils::GetBaseNameAndType(attr.GetName());

  UsdGeomImageable geom(attr.GetPrim());

  if (*sourceType != UsdGeomAttributeType::Output || !geom) {
    return UsdGeomImageable();
  }

  return geom;
}

UsdGeomInput UsdGeomNodeGraph::CreateInput(const TfToken &name,
                                           const SdfValueTypeName &typeName) const
{
  return UsdGeomConnectableAPI(GetPrim()).CreateInput(name, typeName);
}

UsdGeomInput UsdGeomNodeGraph::GetInput(const TfToken &name) const
{
  return UsdGeomConnectableAPI(GetPrim()).GetInput(name);
}

std::vector<UsdGeomInput> UsdGeomNodeGraph::GetInputs(bool onlyAuthored) const
{
  return UsdGeomConnectableAPI(GetPrim()).GetInputs(onlyAuthored);
}

std::vector<UsdGeomInput> UsdGeomNodeGraph::GetInterfaceInputs() const
{
  return GetInputs();
}

static bool _IsValidInput(UsdGeomConnectableAPI const &source,
                          UsdGeomAttributeType const sourceType)
{
  return (sourceType == UsdGeomAttributeType::Input);
}

static UsdGeomNodeGraph::InterfaceInputConsumersMap _ComputeNonTransitiveInputConsumersMap(
    const UsdGeomNodeGraph &nodeGraph)
{
  UsdGeomNodeGraph::InterfaceInputConsumersMap result;

  for (const auto &input : nodeGraph.GetInputs()) {
    result[input] = {};
  }

  // XXX: This traversal isn't instancing aware. We must update this
  // once we have instancing aware USD objects. See http://bug/126053
  for (UsdPrim prim : nodeGraph.GetPrim().GetDescendants()) {

    UsdGeomConnectableAPI connectable(prim);
    if (!connectable)
      continue;

    std::vector<UsdGeomInput> internalInputs = connectable.GetInputs();
    for (const auto &internalInput : internalInputs) {
      UsdGeomConnectableAPI source;
      TfToken sourceName;
      UsdGeomAttributeType sourceType;
      if (UsdGeomConnectableAPI::GetConnectedSource(
              internalInput, &source, &sourceName, &sourceType)) {
        if (source.GetPrim() == nodeGraph.GetPrim() && _IsValidInput(source, sourceType)) {
          result[nodeGraph.GetInput(sourceName)].push_back(internalInput);
        }
      }
    }
  }

  return result;
}

static void _RecursiveComputeNodeGraphInterfaceInputConsumers(
    const UsdGeomNodeGraph::InterfaceInputConsumersMap &inputConsumersMap,
    UsdGeomNodeGraph::NodeGraphInputConsumersMap *nodeGraphInputConsumers)
{
  for (const auto &inputAndConsumers : inputConsumersMap) {
    const std::vector<UsdGeomInput> &consumers = inputAndConsumers.second;
    for (const UsdGeomInput &consumer : consumers) {
      UsdGeomConnectableAPI connectable(consumer.GetAttr().GetPrim());
      if (connectable.GetPrim().IsA<UsdGeomNodeGraph>()) {
        if (!nodeGraphInputConsumers->count(connectable)) {

          const auto &irMap = _ComputeNonTransitiveInputConsumersMap(
              UsdGeomNodeGraph(connectable));
          (*nodeGraphInputConsumers)[connectable] = irMap;

          _RecursiveComputeNodeGraphInterfaceInputConsumers(irMap, nodeGraphInputConsumers);
        }
      }
    }
  }
}

static void _ResolveConsumers(
    const UsdGeomInput &consumer,
    const UsdGeomNodeGraph::NodeGraphInputConsumersMap &nodeGraphInputConsumers,
    std::vector<UsdGeomInput> *resolvedConsumers)
{
  UsdGeomNodeGraph consumerNodeGraph(consumer.GetAttr().GetPrim());
  if (!consumerNodeGraph) {
    resolvedConsumers->push_back(consumer);
    return;
  }

  const auto &nodeGraphIt = nodeGraphInputConsumers.find(consumerNodeGraph);
  if (nodeGraphIt != nodeGraphInputConsumers.end()) {
    const UsdGeomNodeGraph::InterfaceInputConsumersMap &inputConsumers = nodeGraphIt->second;

    const auto &inputIt = inputConsumers.find(consumer);
    if (inputIt != inputConsumers.end()) {
      const auto &consumers = inputIt->second;
      if (!consumers.empty()) {
        for (const auto &nestedConsumer : consumers) {
          _ResolveConsumers(nestedConsumer, nodeGraphInputConsumers, resolvedConsumers);
        }
      }
      else {
        // If the node-graph input has no consumers, then add it to
        // the list of resolved consumers.
        resolvedConsumers->push_back(consumer);
      }
    }
  }
  else {
    resolvedConsumers->push_back(consumer);
  }
}

UsdGeomNodeGraph::InterfaceInputConsumersMap UsdGeomNodeGraph::ComputeInterfaceInputConsumersMap(
    bool computeTransitiveConsumers) const
{
  InterfaceInputConsumersMap result = _ComputeNonTransitiveInputConsumersMap(*this);

  if (!computeTransitiveConsumers)
    return result;

  // Collect all node-graphs for which we must compute the input-consumers map.
  NodeGraphInputConsumersMap nodeGraphInputConsumers;
  _RecursiveComputeNodeGraphInterfaceInputConsumers(result, &nodeGraphInputConsumers);

  // If the are no consumers belonging to node-graphs, we're done.
  if (nodeGraphInputConsumers.empty())
    return result;

  InterfaceInputConsumersMap resolved;
  for (const auto &inputAndConsumers : result) {
    const std::vector<UsdGeomInput> &consumers = inputAndConsumers.second;

    std::vector<UsdGeomInput> resolvedConsumers;
    for (const UsdGeomInput &consumer : consumers) {
      std::vector<UsdGeomInput> nestedConsumers;
      _ResolveConsumers(consumer, nodeGraphInputConsumers, &nestedConsumers);

      resolvedConsumers.insert(
          resolvedConsumers.end(), nestedConsumers.begin(), nestedConsumers.end());
    }

    resolved[inputAndConsumers.first] = resolvedConsumers;
  }

  return resolved;
}

bool UsdGeomNodeGraph::ConnectableAPIBehavior::CanConnectOutputToSource(
    const UsdGeomOutput &output,
    const UsdAttribute &source,
    std::string *reason)
{
  return UsdGeomConnectableAPIBehavior::_CanConnectOutputToSource(output, source, reason);
}

bool UsdGeomNodeGraph::ConnectableAPIBehavior::IsContainer() const
{
  // NodeGraph does act as a namespace container for connected nodes
  return true;
}

TF_REGISTRY_FUNCTION(UsdGeomConnectableAPI)
{
  UsdGeomRegisterConnectableAPIBehavior<UsdGeomNodeGraph,
                                        UsdGeomNodeGraph::ConnectableAPIBehavior>();
}

WABI_NAMESPACE_END
