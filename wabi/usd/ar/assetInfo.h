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
#ifndef WABI_USD_AR_ASSET_INFO_H
#define WABI_USD_AR_ASSET_INFO_H

/// \file ar/assetInfo.h

#include "wabi/base/vt/value.h"
#include "wabi/usd/ar/api.h"
#include "wabi/wabi.h"
#include <string>

WABI_NAMESPACE_BEGIN

/// \class ArAssetInfo
///
/// Contains information about a resolved asset.
///
class ArAssetInfo {
 public:
  /// Version of the resolved asset, if any.
  std::string version;

  /// The name of the asset represented by the resolved
  /// asset, if any.
  std::string assetName;

  /// \deprecated
  /// The repository path corresponding to the resolved asset.
  std::string repoPath;

  /// Additional information specific to the active plugin
  /// asset resolver implementation.
  VtValue resolverInfo;
};

/// \relates ArAssetInfo
inline void swap(ArAssetInfo &lhs, ArAssetInfo &rhs)
{
  lhs.version.swap(rhs.version);
  lhs.assetName.swap(rhs.assetName);
  lhs.repoPath.swap(rhs.repoPath);
  lhs.resolverInfo.Swap(rhs.resolverInfo);
}

/// \relates ArAssetInfo
AR_API
bool operator==(const ArAssetInfo &lhs, const ArAssetInfo &rhs);

/// \relates ArAssetInfo
AR_API
bool operator!=(const ArAssetInfo &lhs, const ArAssetInfo &rhs);

WABI_NAMESPACE_END

#endif  // WABI_USD_AR_ASSET_INFO_H
