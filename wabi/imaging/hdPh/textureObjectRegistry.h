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
#ifndef WABI_IMAGING_HD_ST_TEXTURE_OBJECT_REGISTRY_H
#define WABI_IMAGING_HD_ST_TEXTURE_OBJECT_REGISTRY_H

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/instanceRegistry.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include <atomic>
#include <tbb/concurrent_vector.h>
#include <vector>

WABI_NAMESPACE_BEGIN

using HdPhTextureObjectSharedPtr = std::shared_ptr<class HdPhTextureObject>;
using HdPhTextureObjectPtr = std::weak_ptr<class HdPhTextureObject>;
using HdPhTextureObjectPtrVector = std::vector<HdPhTextureObjectPtr>;
class HdPhResourceRegistry;
class HdPhTextureIdentifier;

/// \class HdPh_TextureObjectRegistry
///
/// A central registry for texture GPU resources.
///
class HdPh_TextureObjectRegistry final {
 public:
  explicit HdPh_TextureObjectRegistry(HdPhResourceRegistry *registry);
  ~HdPh_TextureObjectRegistry();

  /// Allocate texture.
  ///
  /// This just creates the HdPhTextureObject, the actual GPU
  /// resources won't be allocated until the Commit phase.
  ///
  HDPH_API
  HdPhTextureObjectSharedPtr AllocateTextureObject(const HdPhTextureIdentifier &textureId,
                                                   HdTextureType textureType);

  /// Create GPU texture objects, load textures from files and
  /// upload to GPU.
  ///
  HDPH_API
  std::set<HdPhTextureObjectSharedPtr> Commit();

  /// Free GPU resources of textures not used by any client.
  ///
  HDPH_API
  void GarbageCollect();

  /// Mark texture file path as dirty. All textures whose identifier
  /// contains the file path will be reloaded during the next Commit.
  ///
  HDPH_API
  void MarkTextureFilePathDirty(const TfToken &filePath);

  /// Mark that the GPU resource for a texture needs to be
  /// (re-)loaded, e.g., because the memory request changed.
  ///
  HDPH_API
  void MarkTextureObjectDirty(HdPhTextureObjectPtr const &textureObject);

  /// Get resource registry
  ///
  HDPH_API
  HdPhResourceRegistry *GetResourceRegistry() const
  {
    return _resourceRegistry;
  }

  /// The total GPU memory consumed by all textures managed by this registry.
  ///
  int64_t GetTotalTextureMemory() const
  {
    return _totalTextureMemory;
  }

  /// Add signed number to total texture memory amount. Called from
  /// texture objects when (de-)allocated GPU resources.
  ///
  HDPH_API
  void AdjustTotalTextureMemory(int64_t memDiff);

  /// The number of texture objects.
  size_t GetNumberOfTextureObjects() const
  {
    return _textureObjectRegistry.size();
  }

 private:
  HdPhTextureObjectSharedPtr _MakeTextureObject(const HdPhTextureIdentifier &textureId,
                                                HdTextureType textureType);

  std::atomic<int64_t> _totalTextureMemory;

  // Registry for texture and sampler objects.
  HdInstanceRegistry<HdPhTextureObjectSharedPtr> _textureObjectRegistry;

  // Map file paths to texture objects for quick invalidation
  // by file path.
  std::unordered_map<TfToken, HdPhTextureObjectPtrVector, TfToken::HashFunctor> _filePathToTextureObjects;

  // File paths for which GPU resources need to be (re-)loaded
  tbb::concurrent_vector<TfToken> _dirtyFilePaths;

  // Texture for which GPU resources need to be (re-)loaded
  tbb::concurrent_vector<HdPhTextureObjectPtr> _dirtyTextures;

  HdPhResourceRegistry *_resourceRegistry;
};

WABI_NAMESPACE_END

#endif
