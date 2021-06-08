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

#include "wabi/usd/gdr/registry.h"
#include "wabi/base/tf/instantiateSingleton.h"

#include "wabi/base/trace/trace.h"

#include <algorithm>

WABI_NAMESPACE_BEGIN

TF_INSTANTIATE_SINGLETON(GdrRegistry);

namespace {
GdrGeomNodeConstPtr NdrNodeToGeomNode(NdrNodeConstPtr node)
{
  return dynamic_cast<GdrGeomNodeConstPtr>(node);
}

GdrGeomNodePtrVec NdrNodeVecToGeomNodeVec(NdrNodeConstPtrVec nodeVec)
{
  GdrGeomNodePtrVec gdrNodes;

  std::transform(nodeVec.begin(),
                 nodeVec.end(),
                 std::back_inserter(gdrNodes),
                 [](NdrNodeConstPtr baseNode) { return GdrGeomNodeConstPtr(baseNode); });

  return gdrNodes;
}
}  // namespace

GdrRegistry::GdrRegistry() : NdrRegistry()
{
  /**
   * Track plugin discovery cost of base class. */
  TRACE_FUNCTION();
}

GdrRegistry::~GdrRegistry()
{}

GdrRegistry &GdrRegistry::GetInstance()
{
  return TfSingleton<GdrRegistry>::GetInstance();
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeByIdentifier(const NdrIdentifier &identifier,
                                                         const NdrTokenVec &typePriority)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(GetInstance().GetNodeByIdentifier(identifier, typePriority));
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeByIdentifierAndType(const NdrIdentifier &identifier,
                                                                const TfToken &nodeType)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(GetInstance().GetNodeByIdentifierAndType(identifier, nodeType));
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeFromAsset(const SdfAssetPath &geomAsset,
                                                      const NdrTokenMap &metadata,
                                                      const TfToken &subIdentifier,
                                                      const TfToken &sourceType)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(
      GetInstance().GetNodeFromAsset(geomAsset, metadata, subIdentifier, sourceType));
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeFromSourceCode(const std::string &sourceCode,
                                                           const TfToken &sourceType,
                                                           const NdrTokenMap &metadata)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(GetInstance().GetNodeFromSourceCode(sourceCode, sourceType, metadata));
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeByName(const std::string &name,
                                                   const NdrTokenVec &typePriority,
                                                   NdrVersionFilter filter)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(GetInstance().GetNodeByName(name, typePriority, filter));
}

GdrGeomNodeConstPtr GdrRegistry::GetGeomNodeByNameAndType(const std::string &name,
                                                          const TfToken &nodeType,
                                                          NdrVersionFilter filter)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeToGeomNode(GetInstance().GetNodeByNameAndType(name, nodeType, filter));
}

GdrGeomNodePtrVec GdrRegistry::GetGeomNodesByIdentifier(const NdrIdentifier &identifier)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeVecToGeomNodeVec(GetInstance().GetNodesByIdentifier(identifier));
}

GdrGeomNodePtrVec GdrRegistry::GetGeomNodesByName(const std::string &name, NdrVersionFilter filter)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeVecToGeomNodeVec(GetInstance().GetNodesByName(name, filter));
}

GdrGeomNodePtrVec GdrRegistry::GetGeomNodesByFamily(const TfToken &family, NdrVersionFilter filter)
{
  /**
   * XXX Remove trace function when function performance has improved. */
  TRACE_FUNCTION();

  return NdrNodeVecToGeomNodeVec(GetInstance().GetNodesByFamily(family, filter));
}

WABI_NAMESPACE_END
