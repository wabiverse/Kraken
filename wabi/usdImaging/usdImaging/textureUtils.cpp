//
// Copyright 2018 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "wabi/usdImaging/usdImaging/textureUtils.h"

#include "wabi/base/tf/fileUtils.h"

#include "wabi/usd/ar/resolver.h"
#include "wabi/usd/ar/resolverScopedCache.h"

#include "wabi/usd/sdf/layerUtils.h"

WABI_NAMESPACE_BEGIN

std::vector<std::tuple<int, TfToken>> UsdImaging_GetUdimTiles(std::string const &basePath,
                                                              int tileLimit,
                                                              SdfLayerHandle const &layerHandle)
{
  const std::string::size_type pos = basePath.find("<UDIM>");
  if (pos == std::string::npos)
  {
    return {};
  }
  std::string formatString = basePath;
  formatString.replace(pos, 6, "%i");

  ArResolverScopedCache resolverCache;
  ArResolver &resolver = ArGetResolver();

  constexpr int startTile = 1001;
  const int endTile = startTile + tileLimit;
  std::vector<std::tuple<int, TfToken>> ret;
  ret.reserve(tileLimit);
  for (int t = startTile; t <= endTile; ++t)
  {
    const std::string path = layerHandle ? SdfComputeAssetPathRelativeToLayer(
                                             layerHandle, TfStringPrintf(formatString.c_str(), t)) :
                                           TfStringPrintf(formatString.c_str(), t);
    if (!resolver.Resolve(path).empty())
    {
      ret.emplace_back(t - startTile, TfToken(path));
    }
  }
  ret.shrink_to_fit();
  return ret;
}

WABI_NAMESPACE_END
