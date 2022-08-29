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
#include "wabi/imaging/hgiMetal/buffer.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/resourceBindings.h"
#include "wabi/imaging/hgiMetal/sampler.h"
#include "wabi/imaging/hgiMetal/texture.h"

WABI_NAMESPACE_BEGIN

enum HgiMetalArgmentOffset
{
  HgiMetalArgumentOffsetBufferVS = 0,
  HgiMetalArgumentOffsetBufferFS = 512,
  HgiMetalArgumentOffsetSamplerVS = 1024,
  HgiMetalArgumentOffsetSamplerFS = 1536,
  HgiMetalArgumentOffsetTextureVS = 2048,
  HgiMetalArgumentOffsetTextureFS = 2560,

  HgiMetalArgumentOffsetBufferCS = 0,
  HgiMetalArgumentOffsetSamplerCS = 1024,
  HgiMetalArgumentOffsetTextureCS = 2048,

  HgiMetalArgumentOffsetConstants = 3072,
};

HgiMetalResourceBindings::HgiMetalResourceBindings(HgiResourceBindingsDesc const &desc)
  : HgiResourceBindings(desc)
{}

HgiMetalResourceBindings::~HgiMetalResourceBindings() = default;

void HgiMetalResourceBindings::BindResources(HgiMetal *hgi,
                                             MTL::RenderCommandEncoder *renderEncoder,
                                             MTL::Buffer *argBuffer)
{
  MTL::Device *device = hgi->GetPrimaryDevice();
  MTL::ArgumentEncoder *argEncoderBuffer = hgi->GetBufferArgumentEncoder();
  MTL::ArgumentEncoder *argEncoderSampler = hgi->GetSamplerArgumentEncoder();
  MTL::ArgumentEncoder *argEncoderTexture = hgi->GetTextureArgumentEncoder();

  //
  // Bind Textures and Samplers
  //

  for (HgiTextureBindDesc const &texDesc : _descriptor.textures) {
    if (!TF_VERIFY(texDesc.textures.size() == 1))
      continue;

    MTL::Texture *metalTexture = nil;
    HgiTextureHandle const &texHandle = texDesc.textures.front();
    HgiMetalTexture *hgiMetalTexture = static_cast<HgiMetalTexture *>(texHandle.Get());
    if (hgiMetalTexture) {
      metalTexture = hgiMetalTexture->GetTextureId();
    }

    MTL::SamplerState *metalSampler = nil;
    if (texDesc.samplers.size()) {
      HgiSamplerHandle const &smpHandle = texDesc.samplers.front();
      HgiMetalSampler *hgiMetalSampler = static_cast<HgiMetalSampler *>(smpHandle.Get());
      if (hgiMetalSampler) {
        metalSampler = hgiMetalSampler->GetSamplerId();
      }
    }

    if ((texDesc.stageUsage & HgiShaderStageVertex) ||
        texDesc.stageUsage & HgiShaderStagePostTessellationVertex) {
      size_t offsetSampler = HgiMetalArgumentOffsetSamplerVS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderSampler->setArgumentBuffer(argBuffer, offsetSampler);
      argEncoderSampler->setSamplerState(metalSampler, 0);

      size_t offsetTexture = HgiMetalArgumentOffsetTextureVS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderTexture->setArgumentBuffer(argBuffer, offsetTexture);
      argEncoderTexture->setTexture(metalTexture, 0);
    }

    if (texDesc.stageUsage & HgiShaderStageFragment) {
      size_t offsetSampler = HgiMetalArgumentOffsetSamplerFS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderSampler->setArgumentBuffer(argBuffer, offsetSampler);
      argEncoderSampler->setSamplerState(metalSampler, 0);

      size_t offsetTexture = HgiMetalArgumentOffsetTextureFS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderTexture->setArgumentBuffer(argBuffer, offsetTexture);
      argEncoderTexture->setTexture(metalTexture, 0);
    }
    if (metalTexture) {
      if (metalSampler) {
        renderEncoder->useResource(metalTexture,
                                   MTL::ResourceUsageSample | MTL::ResourceUsageRead |
                                     MTL::ResourceUsageWrite);
      } else {
        renderEncoder->useResource(metalTexture, MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
      }
    }
  }

  renderEncoder->setVertexBuffer(argBuffer,
                                 HgiMetalArgumentOffsetSamplerVS,
                                 HgiMetalArgumentIndexSamplers);
  renderEncoder->setVertexBuffer(argBuffer,
                                 HgiMetalArgumentOffsetTextureVS,
                                 HgiMetalArgumentIndexTextures);

  renderEncoder->setFragmentBuffer(argBuffer,
                                   HgiMetalArgumentOffsetSamplerFS,
                                   HgiMetalArgumentIndexSamplers);
  renderEncoder->setFragmentBuffer(argBuffer,
                                   HgiMetalArgumentOffsetTextureFS,
                                   HgiMetalArgumentIndexTextures);

  //
  // Bind Buffers
  //

  // Note that index and vertex buffers are not bound here.
  // They are bound via the GraphicsEncoder.

  for (HgiBufferBindDesc const &bufDesc : _descriptor.buffers) {
    if (!TF_VERIFY(bufDesc.buffers.size() == 1))
      continue;

    HgiBufferHandle const &bufHandle = bufDesc.buffers.front();
    HgiMetalBuffer *metalbuffer = static_cast<HgiMetalBuffer *>(bufHandle.Get());

    MTL::Buffer *bufferId = metalbuffer->GetBufferId();
    NS::UInteger offset = bufDesc.offsets.front();

    if ((bufDesc.stageUsage & HgiShaderStageVertex) ||
        (bufDesc.stageUsage & HgiShaderStagePostTessellationVertex)) {
      NS::UInteger argBufferOffset = HgiMetalArgumentOffsetBufferVS +
                                     bufDesc.bindingIndex * sizeof(void *);
      argEncoderBuffer->setArgumentBuffer(argBuffer, argBufferOffset);
      argEncoderBuffer->setBuffer(bufferId, offset, 0);
    }

    if (bufDesc.stageUsage & HgiShaderStageFragment) {
      NS::UInteger argBufferOffset = HgiMetalArgumentOffsetBufferFS +
                                     bufDesc.bindingIndex * sizeof(void *);
      argEncoderBuffer->setArgumentBuffer(argBuffer, argBufferOffset);
      argEncoderBuffer->setBuffer(bufferId, offset, 0);
    }

    renderEncoder->useResource(bufferId, MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
  }


  renderEncoder->setVertexBuffer(argBuffer,
                                 HgiMetalArgumentOffsetBufferVS,
                                 HgiMetalArgumentIndexBuffers);
  renderEncoder->setFragmentBuffer(argBuffer,
                                   HgiMetalArgumentOffsetBufferFS,
                                   HgiMetalArgumentIndexBuffers);

  // Bind constants

  {
    argEncoderBuffer->setArgumentBuffer(argBuffer, HgiMetalArgumentOffsetConstants);
  }

  renderEncoder->setVertexBuffer(argBuffer,
                                 HgiMetalArgumentOffsetConstants,
                                 HgiMetalArgumentIndexConstants);
  renderEncoder->setFragmentBuffer(argBuffer,
                                   HgiMetalArgumentOffsetConstants,
                                   HgiMetalArgumentIndexConstants);
}

void HgiMetalResourceBindings::BindResources(HgiMetal *hgi,
                                             MTL::ComputeCommandEncoder *computeEncoder,
                                             MTL::Buffer *argBuffer)
{
  MTL::Device *device = hgi->GetPrimaryDevice();
  MTL::ArgumentEncoder *argEncoderBuffer = hgi->GetBufferArgumentEncoder();
  MTL::ArgumentEncoder *argEncoderSampler = hgi->GetSamplerArgumentEncoder();
  MTL::ArgumentEncoder *argEncoderTexture = hgi->GetTextureArgumentEncoder();

  //
  // Bind Textures and Samplers
  //

  for (HgiTextureBindDesc const &texDesc : _descriptor.textures) {
    if (!TF_VERIFY(texDesc.textures.size() == 1))
      continue;

    HgiTextureHandle const &texHandle = texDesc.textures.front();
    HgiMetalTexture *metalTexture = static_cast<HgiMetalTexture *>(texHandle.Get());

    HgiSamplerHandle const &smpHandle = texDesc.samplers.front();
    HgiMetalSampler *metalSmp = static_cast<HgiMetalSampler *>(smpHandle.Get());

    if (texDesc.stageUsage & HgiShaderStageCompute) {
      size_t offsetSampler = HgiMetalArgumentOffsetSamplerCS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderSampler->setArgumentBuffer(argBuffer, offsetSampler);
      argEncoderSampler->setSamplerState(metalSmp->GetSamplerId(), 0);

      size_t offsetTexture = HgiMetalArgumentOffsetTextureCS +
                             (texDesc.bindingIndex * sizeof(void *));
      argEncoderTexture->setArgumentBuffer(argBuffer, offsetTexture);
      argEncoderTexture->setTexture(metalTexture->GetTextureId(), 0);
      if (metalSmp) {
        computeEncoder->useResource(metalTexture->GetTextureId(),
                                    MTL::ResourceUsageSample | MTL::ResourceUsageRead |
                                      MTL::ResourceUsageWrite);
      } else {
        computeEncoder->useResource(metalTexture->GetTextureId(),
                                    MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
      }
    }
  }

  computeEncoder->setBuffer(argBuffer,
                            HgiMetalArgumentOffsetSamplerCS,
                            HgiMetalArgumentIndexSamplers);
  computeEncoder->setBuffer(argBuffer,
                            HgiMetalArgumentOffsetTextureCS,
                            HgiMetalArgumentIndexTextures);

  //
  // Bind Buffers
  //

  // Note that index and vertex buffers are not bound here.
  // They are bound via the GraphicsEncoder.

  for (HgiBufferBindDesc const &bufDesc : _descriptor.buffers) {
    if (!TF_VERIFY(bufDesc.buffers.size() == 1))
      continue;
    if (!(bufDesc.stageUsage & HgiShaderStageCompute))
      continue;

    HgiBufferHandle const &bufHandle = bufDesc.buffers.front();
    HgiMetalBuffer *metalbuffer = static_cast<HgiMetalBuffer *>(bufHandle.Get());

    MTL::Buffer *bufferId = metalbuffer->GetBufferId();
    NS::UInteger offset = bufDesc.offsets.front();
    size_t argBufferOffset = HgiMetalArgumentOffsetBufferCS +
                             bufDesc.bindingIndex * sizeof(void *);
    argEncoderBuffer->setArgumentBuffer(argBuffer, argBufferOffset);
    argEncoderBuffer->setBuffer(bufferId, offset, 0);
    computeEncoder->useResource(bufferId, MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
  }

  computeEncoder->setBuffer(argBuffer,
                            HgiMetalArgumentOffsetBufferCS,
                            HgiMetalArgumentIndexBuffers);

  //
  // Bind Constants
  //

  {
    argEncoderBuffer->setArgumentBuffer(argBuffer, HgiMetalArgumentOffsetConstants);
  }

  computeEncoder->setBuffer(argBuffer,
                            HgiMetalArgumentOffsetConstants,
                            HgiMetalArgumentIndexConstants);
}

void HgiMetalResourceBindings::SetConstantValues(MTL::Buffer *argumentBuffer,
                                                 HgiShaderStage stages,
                                                 uint32_t bindIndex,
                                                 uint32_t byteSize,
                                                 const void *data)
{
  if (argumentBuffer->length() - HgiMetalArgumentOffsetConstants < byteSize) {
    TF_CODING_ERROR("Not enough space reserved for constants");
    byteSize = argumentBuffer->length() - HgiMetalArgumentOffsetConstants;
  }
  uint8_t *bufferContents = (uint8_t *)argumentBuffer->contents();
  memcpy(bufferContents + HgiMetalArgumentOffsetConstants, data, byteSize);
}


WABI_NAMESPACE_END
