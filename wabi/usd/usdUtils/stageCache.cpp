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
#include "wabi/usd/usdUtils/stageCache.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/layer.h"
#include "wabi/usd/sdf/primSpec.h"

#include "wabi/base/tf/hashmap.h"
#include <algorithm>
#include <mutex>

WABI_NAMESPACE_BEGIN

namespace {

// Cache of string keys (currently representing variant selections) to session
// layers.
typedef TfHashMap<std::string, SdfLayerRefPtr, TfHash> _SessionLayerMap;

_SessionLayerMap &GetSessionLayerMap()
{
  // Heap-allocate and deliberately leak this static cache to avoid
  // problems with static destruction order.
  static _SessionLayerMap *sessionLayerMap = new _SessionLayerMap();
  return *sessionLayerMap;
}

}  // namespace

UsdStageCache &UsdUtilsStageCache::Get()
{
  // Heap-allocate and deliberately leak this static cache to avoid
  // problems with static destruction order.
  static UsdStageCache *theCache = new UsdStageCache();
  return *theCache;
}

SdfLayerRefPtr UsdUtilsStageCache::GetSessionLayerForVariantSelections(
    const TfToken &modelName,
    const std::vector<std::pair<std::string, std::string>> &variantSelections)
{
  // Sort so that the key is deterministic.
  std::vector<std::pair<std::string, std::string>> variantSelectionsSorted(
      variantSelections.begin(), variantSelections.end());
  std::sort(variantSelectionsSorted.begin(), variantSelectionsSorted.end());

  std::string sessionKey = modelName;
  TF_FOR_ALL(itr, variantSelectionsSorted)
  {
    sessionKey += ":" + itr->first + "=" + itr->second;
  }

  SdfLayerRefPtr ret;
  {
    static std::mutex sessionLayerMapLock;
    std::lock_guard<std::mutex> lock(sessionLayerMapLock);

    _SessionLayerMap &sessionLayerMap = GetSessionLayerMap();
    _SessionLayerMap::iterator itr    = sessionLayerMap.find(sessionKey);
    if (itr == sessionLayerMap.end()) {
      SdfLayerRefPtr layer = SdfLayer::CreateAnonymous();
      if (!variantSelections.empty()) {
        SdfPrimSpecHandle over = SdfPrimSpec::New(layer, modelName, SdfSpecifierOver);
        TF_FOR_ALL(varSelItr, variantSelections)
        {
          // Construct the variant opinion for the session layer.
          over->GetVariantSelections()[varSelItr->first] = varSelItr->second;
        }
      }
      sessionLayerMap[sessionKey] = layer;
      ret                         = layer;
    }
    else {
      ret = itr->second;
    }
  }
  return ret;
}

WABI_NAMESPACE_END
