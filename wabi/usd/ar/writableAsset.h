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
#ifndef WABI_USD_AR_WRITABLE_ASSET_H
#define WABI_USD_AR_WRITABLE_ASSET_H

/// \file ar/writableAsset.h

#include "wabi/wabi.h"

#include "wabi/usd/ar/api.h"

#include <cstdio>

WABI_NAMESPACE_BEGIN

/// \class ArWritableAsset
///
/// Interface for writing data to an asset.
///
/// \see ArResolver::OpenAssetForWrite for how to retrieve instances of
/// this object.
class ArWritableAsset
{
 public:
  AR_API
  virtual ~ArWritableAsset();

  ArWritableAsset(const ArWritableAsset &) = delete;
  ArWritableAsset &operator=(const ArWritableAsset &) = delete;

  /// Close this asset, performing any necessary finalization or commits
  /// of data that was previously written. Returns true on success, false
  /// otherwise.
  ///
  /// If successful, reads to the written asset in the same process should
  /// reflect the fully written state by the time this function returns.
  /// Also, further calls to any functions on this interface are invalid.
  virtual bool Close() = 0;

  /// Writes \p count bytes from \p buffer at \p offset from the beginning
  /// of the asset. Returns number of bytes written, or 0 on error.
  virtual size_t Write(const void *buffer, size_t count, size_t offset) = 0;

 protected:
  AR_API
  ArWritableAsset();
};

WABI_NAMESPACE_END

#endif
