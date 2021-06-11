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
#ifndef WABI_IMAGING_HD_ST_SAMPLER_OBJECT_H
#define WABI_IMAGING_HD_ST_SAMPLER_OBJECT_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hgi/handle.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class Hgi;
class HdPhUvTextureObject;
class HdPhFieldTextureObject;
class HdPhPtexTextureObject;
class HdPhUdimTextureObject;
class HdPh_SamplerObjectRegistry;
using HgiSamplerHandle = HgiHandle<class HgiSampler>;

using HdPhSamplerObjectSharedPtr = std::shared_ptr<class HdPhSamplerObject>;

/// \class HdPhSamplerObject
///
/// A base class encapsulating a GPU sampler object and, optionally, a
/// texture sampler handle (for bindless textures).
///
/// The subclasses of HdPhSamplerObject mirror the subclasses of
/// HdPhTextureObject with the intention that they will be used in
/// conjunction (e.g., the not yet existing HdPhPtexSamplerObject will
/// have two samplers and texture sampler handles for the texels and
/// layout texture in a HdPhPtexTextureObject).
///
/// The GPU resources is con-/destructed immediately in the
/// c'tor/d'tor. By going through the HdPh_SamplerObjectRegistry, we
/// can obtain a shared pointer that can safely be dropped in a
/// different thread. The HdPh_SamplerObjectRegistry is also dispatching
/// by texture type to construct the corresponding sampler type.
///
class HdPhSamplerObject {
 public:
  virtual ~HdPhSamplerObject() = 0;

 protected:
  explicit HdPhSamplerObject(HdPh_SamplerObjectRegistry *samplerObjectRegistry);

  Hgi *_GetHgi() const;
  HdPh_SamplerObjectRegistry *const _samplerObjectRegistry;
};

/// \class HdPhUvSamplerObject
///
/// A sampler suitable for HdPhUvTextureObject.
///
class HdPhUvSamplerObject final : public HdPhSamplerObject {
 public:
  HDPH_API
  HdPhUvSamplerObject(HdPhUvTextureObject const &uvTexture,
                      HdSamplerParameters const &samplerParameters,
                      bool createBindlessHandle,
                      HdPh_SamplerObjectRegistry *samplerObjectRegistry);

  HDPH_API
  ~HdPhUvSamplerObject() override;

  /// The sampler.
  ///
  const HgiSamplerHandle &GetSampler() const
  {
    return _sampler;
  }

  /// The GL sampler texture handle for bindless textures (as returned by
  /// glGetTextureSamplerHandleARB).
  ///
  /// Only available when requested.
  ///
  uint64_t GetGLTextureSamplerHandle() const
  {
    return _glTextureSamplerHandle;
  }

 private:
  HgiSamplerHandle _sampler;
  const uint64_t _glTextureSamplerHandle;
};

/// \class HdPhFieldSamplerObject
///
/// A sampler suitable for HdPhFieldTextureObject.
///
class HdPhFieldSamplerObject final : public HdPhSamplerObject {
 public:
  HdPhFieldSamplerObject(HdPhFieldTextureObject const &uvTexture,
                         HdSamplerParameters const &samplerParameters,
                         bool createBindlessHandle,
                         HdPh_SamplerObjectRegistry *samplerObjectRegistry);

  ~HdPhFieldSamplerObject() override;

  /// The sampler.
  ///
  const HgiSamplerHandle &GetSampler() const
  {
    return _sampler;
  }

  /// The GL sampler texture handle for bindless textures (as returned by
  /// glGetTextureSamplerHandleARB).
  ///
  /// Only available when requested.
  ///
  uint64_t GetGLTextureSamplerHandle() const
  {
    return _glTextureSamplerHandle;
  }

 private:
  HgiSamplerHandle _sampler;
  const uint64_t _glTextureSamplerHandle;
};

/// \class HdPhPtexSamplerObject
///
/// Ptex doesn't bind samplers, so this class is just holding the
/// texture handles for bindless textures.
///
class HdPhPtexSamplerObject final : public HdPhSamplerObject {
 public:
  HdPhPtexSamplerObject(HdPhPtexTextureObject const &ptexTexture,
                        // samplerParameters are ignored by ptex
                        HdSamplerParameters const &samplerParameters,
                        bool createBindlessHandle,
                        HdPh_SamplerObjectRegistry *samplerObjectRegistry);

  ~HdPhPtexSamplerObject() override;

  /// The GPU sampler object for the texels texture.
  ///
  const HgiSamplerHandle &GetTexelsSampler() const
  {
    return _texelsSampler;
  }

  /// The GL texture handle for bindless textures (as returned by
  /// glGetTextureHandleARB). This is for texels.
  ///
  /// Only available when requested.
  ///
  uint64_t GetTexelsGLTextureHandle() const
  {
    return _texelsGLTextureHandle;
  }

  /// Similar to GetGLTexelsTextureHandle but for layout.
  ///
  uint64_t GetLayoutGLTextureHandle() const
  {
    return _layoutGLTextureHandle;
  }

 private:
  HgiSamplerHandle _texelsSampler;

  const uint64_t _texelsGLTextureHandle;
  const uint64_t _layoutGLTextureHandle;
};

/// \class HdPhUdimSamplerObject
///
/// A sampler suitable for Udim textures (wraps one GPU sampler
/// for the texels texture).
///
class HdPhUdimSamplerObject final : public HdPhSamplerObject {
 public:
  HdPhUdimSamplerObject(HdPhUdimTextureObject const &ptexTexture,
                        // samplerParameters are ignored by udim (at least for now)
                        HdSamplerParameters const &samplerParameters,
                        bool createBindlessHandle,
                        HdPh_SamplerObjectRegistry *samplerObjectRegistry);

  ~HdPhUdimSamplerObject() override;

  /// The GPU sampler object for the texels texture.
  ///
  const HgiSamplerHandle &GetTexelsSampler() const
  {
    return _texelsSampler;
  }

  /// The GL texture handle for bindless textures (as returned by
  /// glGetTextureHandleARB). This is for texels.
  ///
  /// Only available when requested.
  ///
  uint64_t GetTexelsGLTextureHandle() const
  {
    return _texelsGLTextureHandle;
  }

  /// Similar to GetGLTexelsTextureHandle but for layout.
  ///
  uint64_t GetLayoutGLTextureHandle() const
  {
    return _layoutGLTextureHandle;
  }

 private:
  HgiSamplerHandle _texelsSampler;

  const uint64_t _texelsGLTextureHandle;
  const uint64_t _layoutGLTextureHandle;
};

template<HdTextureType textureType> struct HdPh_TypedSamplerObjectHelper;

/// \class HdPhTypedSamplerObject
///
/// A template alias such that, e.g., HdPhUvSamplerObject can be
/// accessed as HdPhTypedSamplerObject<HdTextureType::Uv>.
///
template<HdTextureType textureType>
using HdPhTypedSamplerObject = typename HdPh_TypedSamplerObjectHelper<textureType>::type;

template<> struct HdPh_TypedSamplerObjectHelper<HdTextureType::Uv> {
  using type = HdPhUvSamplerObject;
};

template<> struct HdPh_TypedSamplerObjectHelper<HdTextureType::Field> {
  using type = HdPhFieldSamplerObject;
};

template<> struct HdPh_TypedSamplerObjectHelper<HdTextureType::Ptex> {
  using type = HdPhPtexSamplerObject;
};

template<> struct HdPh_TypedSamplerObjectHelper<HdTextureType::Udim> {
  using type = HdPhUdimSamplerObject;
};

WABI_NAMESPACE_END

#endif
