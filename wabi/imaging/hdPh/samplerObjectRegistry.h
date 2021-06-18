//
// Copyright 2020 Pixar
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
#ifndef WABI_IMAGING_HD_ST_SAMPLER_OBJECT_REGISTRY_H
#define WABI_IMAGING_HD_ST_SAMPLER_OBJECT_REGISTRY_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include <memory>
#include <vector>

WABI_NAMESPACE_BEGIN

class HdSamplerParameters;
using HdPhTextureObjectSharedPtr = std::shared_ptr<class HdPhTextureObject>;
using HdPhSamplerObjectSharedPtr = std::shared_ptr<class HdPhSamplerObject>;
class HdPhResourceRegistry;

/// \class HdPh_SamplerObjectRegistry
///
/// A simple registry for GPU samplers and GL texture sampler handles
/// (for bindless textures).
///
/// The registry makes no attempt at de-duplication. But construction
/// is dispatched by texture type returing a matching sampler (e.g.,
/// HdPhFieldSamplerObject for a HdPhFieldTextureObject or
/// HdPhPtexSamplerObject for the (not yet existing)
/// HdPhPtexTextureObject). Also, it keeps a shared pointer to a sampler
/// around until garbage collection so that clients can safely drop their
/// shared pointers from different threads.
///
class HdPh_SamplerObjectRegistry final
{
 public:
  HDPH_API
  explicit HdPh_SamplerObjectRegistry(HdPhResourceRegistry *registry);

  HDPH_API
  ~HdPh_SamplerObjectRegistry();

  /// Create new sampler object matching the given texture object.
  ///
  /// If createBindlessHandle, also creates a texture sampler handle
  /// (for bindless textures). The associated GPU resource is
  /// created immediately and the call is not thread-safe.
  HDPH_API
  HdPhSamplerObjectSharedPtr AllocateSampler(HdPhTextureObjectSharedPtr const &texture,
                                             HdSamplerParameters const &samplerParameters,
                                             bool createBindlessHandle);

  /// Delete samplers no longer used by a client.
  HDPH_API
  void GarbageCollect();

  HDPH_API
  void MarkGarbageCollectionNeeded();

  /// Get resource registry
  ///
  HDPH_API
  HdPhResourceRegistry *GetResourceRegistry() const;

 private:
  std::vector<HdPhSamplerObjectSharedPtr> _samplerObjects;

  bool _garbageCollectionNeeded;
  HdPhResourceRegistry *_resourceRegistry;
};

WABI_NAMESPACE_END

#endif
