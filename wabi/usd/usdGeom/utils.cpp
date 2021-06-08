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
#include "wabi/usd/usdGeom/utils.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/usd/usdGeom/connectableAPI.h"
#include "wabi/usd/usdGeom/input.h"
#include "wabi/usd/usdGeom/output.h"
#include "wabi/usd/usdGeom/tokens.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/stringUtils.h"

#include <string>

WABI_NAMESPACE_BEGIN

using std::string;
using std::vector;

/* static */
string UsdGeomUtils::GetPrefixForAttributeType(UsdGeomAttributeType sourceType)
{
  switch (sourceType) {
    case UsdGeomAttributeType::Input:
      return UsdGeomTokens->inputs.GetString();
    case UsdGeomAttributeType::Output:
      return UsdGeomTokens->outputs.GetString();
    default:
      return string();
  }
}

/* static */
std::pair<TfToken, UsdGeomAttributeType> UsdGeomUtils::GetBaseNameAndType(const TfToken &fullName)
{
  std::pair<std::string, bool> res = SdfPath::StripPrefixNamespace(fullName,
                                                                   UsdGeomTokens->inputs);
  if (res.second) {
    return std::make_pair(TfToken(res.first), UsdGeomAttributeType::Input);
  }

  res = SdfPath::StripPrefixNamespace(fullName, UsdGeomTokens->outputs);
  if (res.second) {
    return std::make_pair(TfToken(res.first), UsdGeomAttributeType::Output);
  }

  return std::make_pair(fullName, UsdGeomAttributeType::Invalid);
}

/* static */
UsdGeomAttributeType UsdGeomUtils::GetType(const TfToken &fullName)
{
  std::pair<std::string, bool> res = SdfPath::StripPrefixNamespace(fullName,
                                                                   UsdGeomTokens->inputs);
  if (res.second) {
    return UsdGeomAttributeType::Input;
  }

  res = SdfPath::StripPrefixNamespace(fullName, UsdGeomTokens->outputs);
  if (res.second) {
    return UsdGeomAttributeType::Output;
  }

  return UsdGeomAttributeType::Invalid;
}

/* static */
TfToken UsdGeomUtils::GetFullName(const TfToken &baseName, const UsdGeomAttributeType type)
{
  return TfToken(UsdGeomUtils::GetPrefixForAttributeType(type) + baseName.GetString());
}

// Note: to avoid getting stuck in an infinite loop when following connections,
// we need to check if we've visited an attribute before, so that we can break
// the cycle and return an invalid result.
// We expect most connections chains to be very small with most of them having
// 0 or 1 connection in the chain. Few will include multiple hops. That is why
// we are going with a vector and not a set to check for previous attributes.
// To avoid the cost of allocating memory on the heap at each invocation, we
// use a TfSmallVector to keep the first couple of entries on the stack.
constexpr unsigned int N = 5;
typedef TfSmallVector<SdfPath, N> _SmallSdfPathVector;

template<typename UsdGeomInOutput>
bool _GetValueProducingAttributesRecursive(UsdGeomInOutput const &inoutput,
                                           _SmallSdfPathVector *foundAttributes,
                                           UsdGeomAttributeVector &attrs,
                                           bool geomOutputsOnly);

bool _FollowConnectionSourceRecursive(UsdGeomConnectionSourceInfo const &sourceInfo,
                                      _SmallSdfPathVector *foundAttributes,
                                      UsdGeomAttributeVector &attrs,
                                      bool geomOutputsOnly)
{
  if (sourceInfo.sourceType == UsdGeomAttributeType::Output) {
    UsdGeomOutput connectedOutput = sourceInfo.source.GetOutput(sourceInfo.sourceName);
    if (!sourceInfo.source.IsContainer()) {
      attrs.push_back(connectedOutput.GetAttr());
      return true;
    }
    else {
      return _GetValueProducingAttributesRecursive(
          connectedOutput, foundAttributes, attrs, geomOutputsOnly);
    }
  }
  else {  // sourceType == UsdGeomAttributeType::Input
    UsdGeomInput connectedInput = sourceInfo.source.GetInput(sourceInfo.sourceName);
    if (!sourceInfo.source.IsContainer()) {
      // Note, this is an invalid situation for a connected
      // chain. Since we started on an input to either a
      // Geom or a container we cannot legally connect to an
      // input on a non-container.
    }
    else {
      return _GetValueProducingAttributesRecursive(
          connectedInput, foundAttributes, attrs, geomOutputsOnly);
    }
  }

  return false;
}

template<typename UsdGeomInOutput>
bool _GetValueProducingAttributesRecursive(UsdGeomInOutput const &inoutput,
                                           _SmallSdfPathVector *foundAttributes,
                                           UsdGeomAttributeVector &attrs,
                                           bool geomOutputsOnly)
{
  if (!inoutput) {
    return false;
  }

  // Check if we've visited this attribute before and if so abort with an
  // error, since this means we have a loop in the chain
  const SdfPath &thisAttrPath = inoutput.GetAttr().GetPath();
  if (!foundAttributes->empty() &&
      std::find(foundAttributes->begin(), foundAttributes->end(), thisAttrPath) !=
          foundAttributes->end()) {
    TF_WARN("GetValueProducingAttributes: Found cycle with attribute %s", thisAttrPath.GetText());
    return false;
  }

  // Retrieve all valid connections
  UsdGeomSourceInfoVector sourceInfos = UsdGeomConnectableAPI::GetConnectedSources(inoutput);

  if (!sourceInfos.empty()) {
    // Remember the path of this attribute, so that we do not visit it again
    // Since this a cycle protection we only need to do this if we have
    // valid connections
    foundAttributes->push_back(thisAttrPath);
  }

  bool foundValidAttr = false;

  if (sourceInfos.size() > 1) {
    // Follow each connection until we reach an output attribute on an
    // actual geom node or an input attribute with a value
    for (const UsdGeomConnectionSourceInfo &sourceInfo : sourceInfos) {
      // To handle cycle detection in the case of multiple connection we
      // have to copy the found attributes vector (multiple connections
      // leading to the same attribute would trigger the cycle detection).
      // Since we want to avoid that copy we only do it in case of
      // multiple connections.
      _SmallSdfPathVector localFoundAttrs = *foundAttributes;

      foundValidAttr |= _FollowConnectionSourceRecursive(
          sourceInfo, &localFoundAttrs, attrs, geomOutputsOnly);
    }
  }
  else if (!sourceInfos.empty()) {
    // Follow the one connection it until we reach an output attribute on an
    // actual geom node or an input attribute with a value
    foundValidAttr = _FollowConnectionSourceRecursive(
        sourceInfos[0], foundAttributes, attrs, geomOutputsOnly);
  }

  // If our trace should accept attributes with authored values, check if this
  // input or output doesn't have any valid attributes from connections, but
  // has an authored value. Return this attribute.
  if (!geomOutputsOnly && !foundValidAttr) {
    // N.B. Checking whether an attribute has an authored value is a
    // non-trivial operation and should not be done unless required
    if (inoutput.GetAttr().HasAuthoredValue()) {
      VtValue val;
      inoutput.GetAttr().Get(&val);
      attrs.push_back(inoutput.GetAttr());
      foundValidAttr = true;
    }
  }

  return foundValidAttr;
}

/* static */
UsdGeomAttributeVector UsdGeomUtils::GetValueProducingAttributes(UsdGeomInput const &input,
                                                                 bool geomOutputsOnly)
{
  TRACE_FUNCTION_SCOPE("INPUT");

  // We track which attributes we've visited so far to avoid getting caught
  // in an infinite loop, if the network contains a cycle.
  _SmallSdfPathVector foundAttributes;

  UsdGeomAttributeVector valueAttributes;
  _GetValueProducingAttributesRecursive(input, &foundAttributes, valueAttributes, geomOutputsOnly);

  return valueAttributes;
}

/* static */
UsdGeomAttributeVector UsdGeomUtils::GetValueProducingAttributes(UsdGeomOutput const &output,
                                                                 bool geomOutputsOnly)
{
  TRACE_FUNCTION_SCOPE("OUTPUT");

  // We track which attributes we've visited so far to avoid getting caught
  // in an infinite loop, if the network contains a cycle.
  _SmallSdfPathVector foundAttributes;

  UsdGeomAttributeVector valueAttributes;
  _GetValueProducingAttributesRecursive(
      output, &foundAttributes, valueAttributes, geomOutputsOnly);

  return valueAttributes;
}

WABI_NAMESPACE_END
