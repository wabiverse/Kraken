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
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "wabi/imaging/hgiMetal/buffer.h"
#include "wabi/imaging/hgiMetal/capabilities.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/texture.h"


WABI_NAMESPACE_BEGIN

HgiMetalTexture::HgiMetalTexture(HgiMetal *hgi, HgiTextureDesc const &desc)
  : HgiTexture(desc),
    _textureId(nil)
{
  MTL::ResourceOptions resourceOptions = MTL::ResourceStorageModePrivate;
  MTL::TextureUsage usage = MTL::TextureUsageShaderRead;

  if (desc.initialData && desc.pixelsByteSize > 0) {
    resourceOptions = MTL::ResourceStorageModeManaged;
  }

  MTL::PixelFormat mtlFormat = HgiMetalConversions::GetPixelFormat(desc.format, desc.usage);

  if (desc.usage & (HgiTextureUsageBitsColorTarget | HgiTextureUsageBitsDepthTarget)) {
    usage |= MTL::TextureUsageRenderTarget;
  }

  if (desc.usage & HgiTextureUsageBitsShaderWrite) {
    usage |= MTL::TextureUsageShaderWrite;
  }

  const size_t width = desc.dimensions[0];
  const size_t height = desc.dimensions[1];
  const size_t depth = desc.dimensions[2];

  MTL::TextureDescriptor *texDesc;

  texDesc = MTL::TextureDescriptor::texture2DDescriptor(mtlFormat, width, height, NO);

  texDesc->setMipmapLevelCount(desc.mipLevels);

  texDesc->setArrayLength(desc.layerCount);
  texDesc->setResourceOptions(resourceOptions);
  texDesc->setUsage(usage);

#if defined(ARCH_OS_MACOS)
  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{10, 15, 0})) {
#elif defined(ARCH_OS_IOS)
  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{13, 0, 0})) {
#else  /* ARCH_OS_IPHONE */
  if (false) {
#endif /* ARCH_OS_MACOS */
    size_t numChannels = HgiGetComponentCount(desc.format);

    if (usage == MTL::TextureUsageShaderRead && numChannels == 1) {
      MTL::TextureSwizzle s = HgiMetalConversions::GetComponentSwizzle(desc.componentMapping.r);
      texDesc->setSwizzle({s, s, s, s});
    } else {
      texDesc->setSwizzle({HgiMetalConversions::GetComponentSwizzle(desc.componentMapping.r),
                           HgiMetalConversions::GetComponentSwizzle(desc.componentMapping.g),
                           HgiMetalConversions::GetComponentSwizzle(desc.componentMapping.b),
                           HgiMetalConversions::GetComponentSwizzle(desc.componentMapping.a)});
    }
  }

  if (desc.type == HgiTextureType3D) {
    texDesc->setDepth(depth);
    texDesc->setTextureType(MTL::TextureType3D);
  } else if (desc.type == HgiTextureType2DArray) {
    texDesc->setTextureType(MTL::TextureType2DArray);
  } else if (desc.type == HgiTextureType1D) {
    texDesc->setTextureType(MTL::TextureType1D);
  } else if (desc.type == HgiTextureType1DArray) {
    texDesc->setTextureType(MTL::TextureType1DArray);
  }

  if (desc.sampleCount > 1) {
    texDesc->setSampleCount(desc.sampleCount);
    texDesc->setTextureType(MTL::TextureType2DMultisample);
  }

  _textureId = hgi->GetPrimaryDevice()->newTexture(texDesc);

  if (desc.initialData && desc.pixelsByteSize > 0) {
    size_t perPixelSize = HgiGetDataSizeOfFormat(desc.format);

    // Upload each (available) mip
    const std::vector<HgiMipInfo> mipInfos = HgiGetMipInfos(desc.format,
                                                            desc.dimensions,
                                                            desc.layerCount,
                                                            desc.pixelsByteSize);
    const size_t mipLevels = std::min(mipInfos.size(), size_t(desc.mipLevels));
    const char *const initialData = reinterpret_cast<const char *>(desc.initialData);

    for (size_t mip = 0; mip < mipLevels; mip++) {
      const HgiMipInfo &mipInfo = mipInfos[mip];

      const size_t width = mipInfo.dimensions[0];
      const size_t height = mipInfo.dimensions[1];
      const size_t bytesPerRow = perPixelSize * width;

      if (desc.type == HgiTextureType1D) {
        _textureId->replaceRegion(MTL::Region::Make1D(0, width),
                                  mip,
                                  initialData + mipInfo.byteOffset,
                                  bytesPerRow);

      } else if (desc.type == HgiTextureType2D) {
        _textureId->replaceRegion(MTL::Region::Region::Make2D(0, 0, width, height),
                                  mip,
                                  initialData + mipInfo.byteOffset,
                                  bytesPerRow);

      } else if (desc.type == HgiTextureType3D) {
        const size_t depth = mipInfo.dimensions[2];
        const size_t imageBytes = bytesPerRow * height;
        for (size_t d = 0; d < depth; d++) {
          const size_t offset = d * imageBytes;
          _textureId->replaceRegion(MTL::Region::Region::Make3D(0, 0, d, width, height, 1),
                                    mip,
                                    0,
                                    initialData + mipInfo.byteOffset + offset,
                                    bytesPerRow,
                                    0);
        }

      } else if (desc.type == HgiTextureType2DArray) {
        const size_t imageBytes = bytesPerRow * height;
        for (int slice = 0; slice < desc.layerCount; slice++) {
          char const *sliceBase = static_cast<char const *>(initialData) + mipInfo.byteOffset +
                                  imageBytes * slice;
          _textureId->replaceRegion(MTL::Region::Region::Make2D(0, 0, width, height),
                                    mip,
                                    slice,
                                    sliceBase,
                                    bytesPerRow,
                                    0);
        }

      } else if (desc.type == HgiTextureType1DArray) {
        const size_t imageBytes = bytesPerRow;
        for (int slice = 0; slice < desc.layerCount; slice++) {
          char const *sliceBase = static_cast<char const *>(initialData) + mipInfo.byteOffset +
                                  imageBytes * slice;
          _textureId->replaceRegion(MTL::Region::Region::Make1D(0, width),
                                    mip,
                                    slice,
                                    sliceBase,
                                    bytesPerRow,
                                    0);
        }

      } else {
        TF_CODING_ERROR("Missing Texture upload implementation");
      }
    }
  }

  if (!(usage & MTL::TextureUsageRenderTarget)) {
    MTL::CommandBuffer *commandBuffer = hgi->GetQueue()->commandBuffer();
    MTL::BlitCommandEncoder *blitCommandEncoder = commandBuffer->blitCommandEncoder();
    blitCommandEncoder->optimizeContentsForGPUAccess(_textureId);
    blitCommandEncoder->endEncoding();
    commandBuffer->commit();
  }

  HGIMETAL_DEBUG_LABEL(_textureId, _descriptor.debugName.c_str());
}

HgiMetalTexture::HgiMetalTexture(HgiMetal *hgi, HgiTextureViewDesc const &desc)
  : HgiTexture(desc.sourceTexture->GetDescriptor()),
    _textureId(nil)
{
  HgiMetalTexture *srcTexture = static_cast<HgiMetalTexture *>(desc.sourceTexture.Get());
  NS::Range levels = NS::Range::Make(desc.sourceFirstMip, desc.mipLevels);
  NS::Range slices = NS::Range::Make(desc.sourceFirstLayer, desc.layerCount);
  MTL::PixelFormat mtlFormat = HgiMetalConversions::GetPixelFormat(desc.format,
                                                                   HgiTextureUsageBitsColorTarget);

  _textureId = srcTexture->GetTextureId()->newTextureView(
    mtlFormat,
    srcTexture->GetTextureId()->textureType(),
    levels,
    slices);

  // Update the texture descriptor to reflect the above
  _descriptor.debugName = desc.debugName;
  _descriptor.format = desc.format;
  _descriptor.layerCount = desc.layerCount;
  _descriptor.mipLevels = desc.mipLevels;
}

HgiMetalTexture::~HgiMetalTexture()
{
  if (_textureId != nil) {
    _textureId->release();
    _textureId = nil;
  }
}

size_t HgiMetalTexture::GetByteSizeOfResource() const
{
  return _GetByteSizeOfResource(_descriptor);
}

uint64_t HgiMetalTexture::GetRawResource() const
{
  return (uint64_t)_textureId;
}

MTL::Texture *HgiMetalTexture::GetTextureId() const
{
  return _textureId;
}

WABI_NAMESPACE_END
