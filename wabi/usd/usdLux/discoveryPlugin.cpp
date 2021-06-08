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
#include "wabi/usd/usdLux/discoveryPlugin.h"

#include "wabi/usd/usdLux/light.h"
#include "wabi/usd/usdLux/lightDefParser.h"
#include "wabi/usd/usdLux/lightFilter.h"

#include "wabi/base/plug/registry.h"

#include "wabi/usd/usd/schemaRegistry.h"

WABI_NAMESPACE_BEGIN

const NdrStringVec &UsdLux_DiscoveryPlugin::GetSearchURIs() const
{
  static const NdrStringVec empty;
  return empty;
}

NdrNodeDiscoveryResultVec UsdLux_DiscoveryPlugin::DiscoverNodes(const Context &context)
{
  NdrNodeDiscoveryResultVec result;

  // We want to discover nodes for all concrete schema types that derive from
  // UsdLuxLight and UsdLuxLightFilter.
  static const TfType lightType       = TfType::Find<UsdLuxLight>();
  static const TfType lightFilterType = TfType::Find<UsdLuxLightFilter>();
  // LightFilter is a concrete type and is legit to instantiate while Light
  // is abstract and cannot be instantiated. However since the loop below
  // filters out abstract types, there's no harm in including Light in
  // addition to LightFilter. If Light were to be changed to be a concrete
  // type at any point for any reason, this code would not have to change.
  std::set<TfType> types({lightType, lightFilterType});
  PlugRegistry::GetAllDerivedTypes(lightType, &types);
  PlugRegistry::GetAllDerivedTypes(lightFilterType, &types);

  for (const TfType &type : types) {
    const TfToken name = UsdSchemaRegistry::GetConcreteSchemaTypeName(type);
    // The type name from the schema registry will be empty if the type is
    // not concrete (i.e. abstract); we skip abstract types.
    if (!name.IsEmpty()) {
      // The schema type name is the name and identifier. The URIs are
      // left empty as these nodes can be populated from the schema
      // registry prim definitions.
      result.emplace_back(
          /*identifier=*/name,
          NdrVersion().GetAsDefault(),
          name,
          /*family=*/TfToken(),
          UsdLux_LightDefParserPlugin::_GetDiscoveryType(),
          UsdLux_LightDefParserPlugin::_GetSourceType(),
          /*uri=*/"",
          /*resolvedUri=*/"");
    }
  }

  return result;
}

NDR_REGISTER_DISCOVERY_PLUGIN(UsdLux_DiscoveryPlugin);

WABI_NAMESPACE_END
