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
#ifndef WABI_IMAGING_HD_ST_TEXTURE_HANDLE_REGISTRY_H
#define WABI_IMAGING_HD_ST_TEXTURE_HANDLE_REGISTRY_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hdPh/textureObject.h"

#include "wabi/imaging/hd/enums.h"

#include <tbb/concurrent_vector.h>

#include <memory>
#include <set>

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
class HdPhTextureIdentifier;
class HdSamplerParameters;
class HdPh_SamplerObjectRegistry;

using HdPhTextureHandlePtr = std::weak_ptr<class HdPhTextureHandle>;
using HdPhTextureHandleSharedPtr = std::shared_ptr<class HdPhTextureHandle>;
using HdPhTextureObjectPtr = std::weak_ptr<class HdPhTextureObject>;
using HdPhTextureObjectSharedPtr = std::shared_ptr<class HdPhTextureObject>;
using HdPhSamplerObjectSharedPtr = std::shared_ptr<class HdPhSamplerObject>;
using HdPhShaderCodePtr = std::weak_ptr<class HdPhShaderCode>;
using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;

/// \class HdPh_TextureHandleRegistry
///
/// Keeps track of texture handles and allocates the textures and
/// samplers using the HdPh_TextureObjectRegistry, respectively,
/// HdPh_SamplerObjectRegistry.  Its responsibilities including
/// tracking what texture handles are associated to a texture,
/// computing the target memory of a texture from the memory requests
/// in the texture handles, triggering sampler and texture garbage
/// collection, and determining what HdPhShaderCode instances are
/// affecting by (re-)committing a texture.
///
class HdPh_TextureHandleRegistry final
{
 public:

  HDPH_API
  explicit HdPh_TextureHandleRegistry(HdPhResourceRegistry *registry);

  HDPH_API
  ~HdPh_TextureHandleRegistry();

  /// Allocate texture handle (thread-safe).
  ///
  /// See HdPhResourceRegistry::AllocateTextureHandle for details.
  ///
  HDPH_API
  HdPhTextureHandleSharedPtr AllocateTextureHandle(const HdPhTextureIdentifier &textureId,
                                                   HdTextureType textureType,
                                                   const HdSamplerParameters &samplerParams,
                                                   /// memoryRequest in bytes.
                                                   size_t memoryRequest,
                                                   bool createBindlessHandle,
                                                   HdPhShaderCodePtr const &shaderCode);

  /// Mark texture dirty (thread-safe).
  ///
  /// If set, the target memory of the texture will be recomputed
  /// during commit and the data structure tracking the associated
  /// handles will be updated potentially triggering texture garbage
  /// collection.
  ///
  HDPH_API
  void MarkDirty(HdPhTextureObjectPtr const &texture);

  /// Mark shader dirty (thread-safe).
  ///
  /// If set, the shader is scheduled to be updated (i.e., have its
  /// AddResourcesFromTextures called) on the next commit.
  ///
  HDPH_API
  void MarkDirty(HdPhShaderCodePtr const &shader);

  /// Mark that sampler garbage collection needs to happen during
  /// next commit (thead-safe).
  ///
  HDPH_API
  void MarkSamplerGarbageCollectionNeeded();

  /// Get texture object registry.
  ///
  HdPh_TextureObjectRegistry *GetTextureObjectRegistry() const
  {
    return _textureObjectRegistry.get();
  }

  /// Get sampler object registry.
  ///
  HdPh_SamplerObjectRegistry *GetSamplerObjectRegistry() const
  {
    return _samplerObjectRegistry.get();
  }

  /// Commit textures. Return shader code instances that
  /// depend on the (re-)loaded textures so that they can add
  /// buffer sources based on the texture meta-data.
  ///
  /// Also garbage collect textures and samplers if necessary.
  ///
  HDPH_API
  std::set<HdPhShaderCodeSharedPtr> Commit();

  /// Sets how much memory a single texture can consume in bytes by
  /// texture type.
  ///
  /// Only has an effect if non-zero and only applies to textures if
  /// no texture handle referencing the texture has a memory
  /// request.
  ///
  HDPH_API
  void SetMemoryRequestForTextureType(HdTextureType textureType, size_t memoryRequest);

  HDPH_API
  size_t GetNumberOfTextureHandles() const;

 private:

  void _ComputeMemoryRequest(HdPhTextureObjectSharedPtr const &);
  void _ComputeMemoryRequests(const std::set<HdPhTextureObjectSharedPtr> &);
  void _ComputeAllMemoryRequests();

  bool _GarbageCollectHandlesAndComputeTargetMemory();
  void _GarbageCollectAndComputeTargetMemory();
  std::set<HdPhShaderCodeSharedPtr> _Commit();

  class _TextureToHandlesMap;

  // Maps texture type to memory a single texture of that type can consume
  // (in bytes).
  // Will be taken into account when computing the maximum of all the
  // memory requests of the texture handles.
  std::map<HdTextureType, size_t> _textureTypeToMemoryRequest;
  // Has _textureTypeToMemoryRequest changed since the last commit.
  bool _textureTypeToMemoryRequestChanged;

  // Handles that are new or for which the underlying texture has
  // changed: samplers might need to be (re-)allocated and the
  // corresponding shader code might need to update the shader bar.
  tbb::concurrent_vector<HdPhTextureHandlePtr> _dirtyHandles;

  // Textures whose set of associated handles and target memory
  // might have changed.
  tbb::concurrent_vector<HdPhTextureObjectPtr> _dirtyTextures;

  // Shaders that dropped a texture handle also need to be notified
  // (for example because they re-allocated the shader bar after dropping
  // the texture).
  tbb::concurrent_vector<HdPhShaderCodePtr> _dirtyShaders;

  std::unique_ptr<class HdPh_SamplerObjectRegistry> _samplerObjectRegistry;
  std::unique_ptr<class HdPh_TextureObjectRegistry> _textureObjectRegistry;
  std::unique_ptr<_TextureToHandlesMap> _textureToHandlesMap;
};

WABI_NAMESPACE_END

#endif
