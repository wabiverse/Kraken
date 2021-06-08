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
#include "wabi/imaging/hdPh/samplerObjectRegistry.h"

#include "wabi/imaging/hdPh/ptexTextureObject.h"
#include "wabi/imaging/hdPh/samplerObject.h"
#include "wabi/imaging/hdPh/textureObject.h"
#include "wabi/imaging/hdPh/udimTextureObject.h"

#include "wabi/base/trace/trace.h"

WABI_NAMESPACE_BEGIN

HdPh_SamplerObjectRegistry::HdPh_SamplerObjectRegistry(HdPhResourceRegistry *registry)
    : _garbageCollectionNeeded(false),
      _resourceRegistry(registry)
{}

HdPh_SamplerObjectRegistry::~HdPh_SamplerObjectRegistry() = default;

template<HdTextureType textureType>
static HdPhSamplerObjectSharedPtr _MakeTypedSamplerObject(
    HdPhTextureObjectSharedPtr const &texture,
    HdSamplerParameters const &samplerParameters,
    const bool createBindlessHandle,
    HdPh_SamplerObjectRegistry *const samplerObjectRegistry)
{
  // e.g. HdPhUvTextureObject
  using TextureObject = HdPhTypedTextureObject<textureType>;
  // e.g. HdPhUvSamplerObject
  using SamplerObject = HdPhTypedSamplerObject<textureType>;

  const TextureObject *const typedTexture = dynamic_cast<TextureObject *>(texture.get());
  if (!typedTexture) {
    TF_CODING_ERROR("Bad texture object");
    return nullptr;
  }

  return std::make_shared<SamplerObject>(
      *typedTexture, samplerParameters, createBindlessHandle, samplerObjectRegistry);
}

static HdPhSamplerObjectSharedPtr _MakeSamplerObject(
    HdPhTextureObjectSharedPtr const &texture,
    HdSamplerParameters const &samplerParameters,
    const bool createBindlessHandle,
    HdPh_SamplerObjectRegistry *const samplerObjectRegistry)
{
  switch (texture->GetTextureType()) {
    case HdTextureType::Uv:
      return _MakeTypedSamplerObject<HdTextureType::Uv>(
          texture, samplerParameters, createBindlessHandle, samplerObjectRegistry);
    case HdTextureType::Field:
      return _MakeTypedSamplerObject<HdTextureType::Field>(
          texture, samplerParameters, createBindlessHandle, samplerObjectRegistry);
    case HdTextureType::Ptex:
      return _MakeTypedSamplerObject<HdTextureType::Ptex>(
          texture, samplerParameters, createBindlessHandle, samplerObjectRegistry);
    case HdTextureType::Udim:
      return _MakeTypedSamplerObject<HdTextureType::Udim>(
          texture, samplerParameters, createBindlessHandle, samplerObjectRegistry);
  }

  TF_CODING_ERROR("Unsupported texture type");
  return nullptr;
}

HdPhSamplerObjectSharedPtr HdPh_SamplerObjectRegistry::AllocateSampler(
    HdPhTextureObjectSharedPtr const &texture,
    HdSamplerParameters const &samplerParameters,
    bool const createBindlessHandle)
{
  TRACE_FUNCTION();

  HdPhSamplerObjectSharedPtr const result = _MakeSamplerObject(
      texture, samplerParameters, createBindlessHandle, this);

  if (result) {
    // Record sampler object
    _samplerObjects.push_back(result);
  }

  return result;
}

void HdPh_SamplerObjectRegistry::MarkGarbageCollectionNeeded()
{
  _garbageCollectionNeeded = true;
}

HdPhResourceRegistry *HdPh_SamplerObjectRegistry::GetResourceRegistry() const
{
  return _resourceRegistry;
}

// Remove all shared pointers to objects not referenced by any client.
void HdPh_SamplerObjectRegistry::GarbageCollect()
{
  TRACE_FUNCTION();

  if (!_garbageCollectionNeeded) {
    return;
  }

  // Go from left to right, filling slots that became empty
  // with "shared" shared pointers from the right.
  size_t last = _samplerObjects.size();

  for (size_t i = 0; i < last; i++) {
    if (_samplerObjects[i].unique()) {
      while (true) {
        last--;
        if (i == last) {
          break;
        }
        if (!_samplerObjects[last].unique()) {
          _samplerObjects[i] = _samplerObjects[last];
          break;
        }
      }
    }
  }

  _samplerObjects.resize(last);

  _garbageCollectionNeeded = false;
}

WABI_NAMESPACE_END
