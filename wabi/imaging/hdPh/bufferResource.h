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
#ifndef WABI_IMAGING_HD_ST_BUFFER_RESOURCE_H
#define WABI_IMAGING_HD_ST_BUFFER_RESOURCE_H

#include "wabi/imaging/hd/bufferResource.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hgi/buffer.h"

#include "wabi/base/tf/token.h"

#include <memory>
#include <utility>
#include <vector>

WABI_NAMESPACE_BEGIN

using HdPhBufferResourceSharedPtr = std::shared_ptr<class HdPhBufferResource>;

using HdPhBufferResourceNamedPair = std::pair<TfToken, HdPhBufferResourceSharedPtr>;
using HdPhBufferResourceNamedList = std::vector<HdPhBufferResourceNamedPair>;

/// \class HdPhBufferResource
///
/// A specific type of HdBufferResource (GPU resource) representing
/// an HgiBufferHandle.
///
class HdPhBufferResource final : public HdBufferResource
{
 public:
  HDPH_API
  HdPhBufferResource(TfToken const &role, HdTupleType tupleType, int offset, int stride);
  HDPH_API
  ~HdPhBufferResource();

  /// Sets the HgiBufferHandle for this resource and its size.
  /// also caches the gpu address of the buffer.
  HDPH_API
  void SetAllocation(HgiBufferHandle const &handle, size_t size);

  /// Returns the HgiBufferHandle for this GPU resource
  HgiBufferHandle &GetHandle()
  {
    return _handle;
  }

  /// Returns the gpu address (if available. otherwise returns 0).
  uint64_t GetGPUAddress() const
  {
    return _gpuAddr;
  }

 private:
  uint64_t _gpuAddr;
  HgiBufferHandle _handle;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_BUFFER_RESOURCE_H
