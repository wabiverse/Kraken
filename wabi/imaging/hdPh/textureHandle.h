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
#ifndef HD_ST_TEXTURE_HANDLE_H
#define HD_ST_TEXTURE_HANDLE_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/types.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdPhShaderCodePtr = std::weak_ptr<class HdPhShaderCode>;
using HdPhTextureObjectSharedPtr = std::shared_ptr<class HdPhTextureObject>;
using HdPhSamplerObjectSharedPtr = std::shared_ptr<class HdPhSamplerObject>;

using HdPhTextureHandleSharedPtr = std::shared_ptr<class HdPhTextureHandle>;

class HdPh_TextureHandleRegistry;

/// \class HdPhTextureHandle
///
/// Represents a texture and sampler that will be allocated and loaded
/// from a texture file during commit, possibly a texture sampler
/// handle and a memory request. It is intended for HdPhShaderCode and
/// HdPhShaderCode::AddResourcesFromTextures() is called whenever
/// the underlying texture and sampler gets allocated and (re-)loaded
/// so that the shader code can react to, e.g., changing texture
/// sampler handle for bindless or changing texture metadata such as a
/// field bounding box for volumes.
///
class HdPhTextureHandle
{
 public:

  /// See HdPhResourceRegistry::AllocateTextureHandle for details.
  HDPH_API
  HdPhTextureHandle(HdPhTextureObjectSharedPtr const &textureObject,
                    const HdSamplerParameters &samplerParams,
                    size_t memoryRequest,
                    bool createBindlessHandle,
                    HdPhShaderCodePtr const &shaderCode,
                    HdPh_TextureHandleRegistry *textureHandleRegistry);

  HDPH_API
  ~HdPhTextureHandle();

  /// Get texture object.
  ///
  /// Can be accessed after commit.
  HdPhTextureObjectSharedPtr const &GetTextureObject() const
  {
    return _textureObject;
  }

  /// Get sampler object.
  ///
  /// Can be accessed after commit.
  HdPhSamplerObjectSharedPtr const &GetSamplerObject() const
  {
    return _samplerObject;
  }

  /// Get sampler parameters.
  ///
  HdSamplerParameters const &GetSamplerParameters() const
  {
    return _samplerParams;
  }

  /// Get how much memory this handle requested for the texture.
  ///
  size_t GetMemoryRequest() const
  {
    return _memoryRequest;
  }

  /// Get the shader code associated with this handle.
  ///
  HdPhShaderCodePtr const &GetShaderCode() const
  {
    return _shaderCode;
  }

  /// Allocate sampler for this handle (not thread-safe).
  ///
  /// This also creates the texture sampler handle (for bindless
  /// textures) and updates it on subsequent calls.
  ///
  HDPH_API
  void ReallocateSamplerIfNecessary();

 private:

  HdPhTextureObjectSharedPtr _textureObject;
  HdPhSamplerObjectSharedPtr _samplerObject;
  HdSamplerParameters _samplerParams;
  size_t _memoryRequest;
  bool _createBindlessHandle;
  HdPhShaderCodePtr _shaderCode;
  HdPh_TextureHandleRegistry *_textureHandleRegistry;
};

WABI_NAMESPACE_END

#endif
