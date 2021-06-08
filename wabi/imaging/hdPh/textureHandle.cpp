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
#include "wabi/imaging/hdPh/textureHandle.h"

#include "wabi/imaging/hdPh/textureHandleRegistry.h"

#include "wabi/imaging/hdPh/samplerObject.h"
#include "wabi/imaging/hdPh/samplerObjectRegistry.h"

#include "wabi/base/tf/diagnostic.h"

WABI_NAMESPACE_BEGIN

HdPhTextureHandle::HdPhTextureHandle(HdPhTextureObjectSharedPtr const &textureObject,
                                     const HdSamplerParameters &samplerParams,
                                     const size_t memoryRequest,
                                     const bool createBindlessHandle,
                                     HdPhShaderCodePtr const &shaderCode,
                                     HdPh_TextureHandleRegistry *textureHandleRegistry)
    : _textureObject(textureObject),
      _samplerParams(samplerParams),
      _memoryRequest(memoryRequest),
      _createBindlessHandle(createBindlessHandle),
      _shaderCode(shaderCode),
      _textureHandleRegistry(textureHandleRegistry)
{}

HdPhTextureHandle::~HdPhTextureHandle()
{
  if (TF_VERIFY(_textureHandleRegistry)) {
    // The target memory of the texture might change, so mark dirty.
    _textureHandleRegistry->MarkDirty(_textureObject);
    // The shader needs to be updated after it dropped a texture
    // handle (i.e., because it re-allocated the shader bar after
    // dropping a texture).
    _textureHandleRegistry->MarkDirty(_shaderCode);
    _textureHandleRegistry->MarkSamplerGarbageCollectionNeeded();
  }
}

void HdPhTextureHandle::ReallocateSamplerIfNecessary()
{
  if (_samplerObject) {
    if (!_createBindlessHandle) {
      // There is no setter for sampler parameters,
      // so we only need to create a sampler once...
      return;
    }

    // ... except that the sampler object has a texture sampler
    // handle that needs to be re-created if the underlying texture
    // changes, so continue.

    if (TF_VERIFY(_textureHandleRegistry)) {
      _textureHandleRegistry->MarkSamplerGarbageCollectionNeeded();
    }

    _samplerObject = nullptr;
  }

  // Create sampler object through registry.
  HdPh_SamplerObjectRegistry *const samplerObjectRegistry =
      _textureHandleRegistry->GetSamplerObjectRegistry();

  _samplerObject = samplerObjectRegistry->AllocateSampler(
      _textureObject, _samplerParams, _createBindlessHandle);
}

WABI_NAMESPACE_END
