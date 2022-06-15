//
// Copyright 2019 Pixar
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
#ifndef WABI_IMAGING_HD_ST_RENDER_BUFFER_H
#define WABI_IMAGING_HD_ST_RENDER_BUFFER_H

#include "wabi/base/gf/vec3i.h"
#include "wabi/imaging/hd/renderBuffer.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/texture.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
class HdPhTextureIdentifier;
using HdPhDynamicUvTextureObjectSharedPtr = std::shared_ptr<class HdPhDynamicUvTextureObject>;

class HdPhRenderBuffer : public HdRenderBuffer
{
 public:

  HDPH_API
  HdPhRenderBuffer(HdPhResourceRegistry *resourceRegistry, SdfPath const &id);

  HDPH_API
  ~HdPhRenderBuffer() override;

  HDPH_API
  void Sync(HdSceneDelegate *sceneDelegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits) override;

  HDPH_API
  bool Allocate(GfVec3i const &dimensions, HdFormat format, bool multiSampled) override;

  HDPH_API
  unsigned int GetWidth() const override;

  HDPH_API
  unsigned int GetHeight() const override;

  HDPH_API
  unsigned int GetDepth() const override;

  HDPH_API
  HdFormat GetFormat() const override
  {
    return _format;
  }

  HDPH_API
  bool IsMultiSampled() const override;

  /// Map the buffer for reading. The control flow should be Map(),
  /// before any I/O, followed by memory access, followed by Unmap() when
  /// done.
  ///   \return The address of the buffer.
  HDPH_API
  void *Map() override;

  /// Unmap the buffer.
  HDPH_API
  void Unmap() override;

  /// Return whether any clients have this buffer mapped currently.
  ///   \return True if the buffer is currently mapped by someone.
  HDPH_API
  bool IsMapped() const override
  {
    return _mappers.load() != 0;
  }

  /// Is the buffer converged?
  ///   \return True if the buffer is converged (not currently being
  ///           rendered to).
  HDPH_API
  bool IsConverged() const override
  {
    return true;
  }

  /// Resolve the sample buffer into final values.
  HDPH_API
  void Resolve() override;

  /// Returns the texture handle.
  HDPH_API
  VtValue GetResource(bool multiSampled) const override;

  /// The identifier that can be passed to, e.g.,
  /// HdPhResourceRegistry::AllocateTextureHandle so that a
  /// shader can bind this buffer as texture.
  HDPH_API
  HdPhTextureIdentifier GetTextureIdentifier(bool multiSampled);

 protected:

  void _Deallocate() override;

 private:

  // HdRenderBuffer::Allocate should take a scene delegate or
  // resource registry so that we do not need to save it here.
  HdPhResourceRegistry *_resourceRegistry;

  // Format saved here (somewhat redundantely) since the
  // Hgi texture descriptor holds an HgiFormat instead of HdFormat.
  HdFormat _format;

  uint32_t _msaaSampleCount;

  // The GPU texture resource
  HdPhDynamicUvTextureObjectSharedPtr _textureObject;

  // The GPU multi-sample texture resource (optional)
  HdPhDynamicUvTextureObjectSharedPtr _textureMSAAObject;

  // The number of callers mapping this buffer.
  std::atomic<int> _mappers;
  // Texels are temp captured into this buffer between map and unmap.
  std::vector<uint8_t> _mappedBuffer;
};

WABI_NAMESPACE_END

#endif
