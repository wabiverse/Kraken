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
#include "wabi/usd/usdHydra/discoveryPlugin.h"

#include "wabi/base/plug/plugin.h"
#include "wabi/base/plug/thisPlugin.h"

#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/fileUtils.h"
#include "wabi/base/tf/stringUtils.h"

#include "wabi/usd/ar/resolver.h"
#include "wabi/usd/ar/resolverContext.h"
#include "wabi/usd/ar/resolverContextBinder.h"

#include "wabi/usd/usd/attribute.h"
#include "wabi/usd/usd/stage.h"

#include "wabi/usd/usdShade/shader.h"
#include "wabi/usd/usdShade/shaderDefUtils.h"
#include "wabi/usd/usdShade/tokens.h"

WABI_NAMESPACE_BEGIN

static std::string _GetShaderResourcePath(char const *resourceName = "")
{
  static PlugPluginPtr plugin = PLUG_THIS_PLUGIN;
  const std::string path = PlugFindPluginResource(plugin, TfStringCatPaths("shaders", resourceName));

  TF_VERIFY(!path.empty(), "Could not find shader resource: %s\n", resourceName);

  return path;
}

const NdrStringVec &UsdHydraDiscoveryPlugin::GetSearchURIs() const
{
  static const NdrStringVec searchPaths{_GetShaderResourcePath()};
  return searchPaths;
}

NdrNodeDiscoveryResultVec UsdHydraDiscoveryPlugin::DiscoverNodes(const Context &context)
{
  NdrNodeDiscoveryResultVec result;

  static std::string shaderDefsFile = _GetShaderResourcePath("shaderDefs.usda");
  if (shaderDefsFile.empty())
    return result;

  auto resolverContext = ArGetResolver().CreateDefaultContextForAsset(shaderDefsFile);

  const UsdStageRefPtr stage = UsdStage::Open(shaderDefsFile, resolverContext);

  if (!stage) {
    TF_RUNTIME_ERROR("Could not open file '%s' on a USD stage.", shaderDefsFile.c_str());
    return result;
  }

  ArResolverContextBinder binder(resolverContext);
  const TfToken discoveryType(ArGetResolver().GetExtension(shaderDefsFile));

  auto rootPrims = stage->GetPseudoRoot().GetChildren();
  for (const auto &shaderDef : rootPrims) {
    UsdShadeShader shader(shaderDef);
    if (!shader) {
      continue;
    }

    auto discoveryResults = UsdShadeShaderDefUtils::GetNodeDiscoveryResults(shader, shaderDefsFile);

    result.insert(result.end(), discoveryResults.begin(), discoveryResults.end());

    if (discoveryResults.empty()) {
      TF_RUNTIME_ERROR(
        "Found shader definition <%s> with no valid "
        "discovery results. This is likely because there are no "
        "resolvable info:sourceAsset values.",
        shaderDef.GetPath().GetText());
    }
  }

  return result;
}

NDR_REGISTER_DISCOVERY_PLUGIN(UsdHydraDiscoveryPlugin);

WABI_NAMESPACE_END
