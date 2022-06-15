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
#ifndef WABI_IMAGING_HDPH_ASSET_UV_TEXTURE_CPU_DATA_H
#define WABI_IMAGING_HDPH_ASSET_UV_TEXTURE_CPU_DATA_H

#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/textureCpuData.h"
#include "wabi/imaging/hgi/texture.h"
#include "wabi/imaging/hio/image.h"
#include "wabi/wabi.h"

#include <memory>
#include <string>

WABI_NAMESPACE_BEGIN

/// \class HdPhAssetUvTextureCpuData
///
/// Implements HdPhTextureCpuData by reading a uv texture from
/// a file.
///
class HdPhAssetUvTextureCpuData : public HdPhTextureCpuData
{
 public:

  HDPH_API
  HdPhAssetUvTextureCpuData(std::string const &filePath,
                            size_t targetMemory,
                            bool premultiplyAlpha,
                            HioImage::ImageOriginLocation originLocation,
                            HioImage::SourceColorSpace sourceColorSpace);

  HDPH_API
  ~HdPhAssetUvTextureCpuData() override;

  HDPH_API
  const HgiTextureDesc &GetTextureDesc() const override;

  HDPH_API
  bool GetGenerateMipmaps() const override;

  HDPH_API
  bool IsValid() const override;

  /// The wrap info extracted from the image file.
  HDPH_API
  const std::pair<HdWrap, HdWrap> &GetWrapInfo() const
  {
    return _wrapInfo;
  }

 private:

  void _SetWrapInfo(HioImageSharedPtr const &image);

  // Pointer to the potentially converted data.
  std::unique_ptr<unsigned char[]> _rawBuffer;

  // The result, including a pointer to the potentially
  // converted texture data in _textureDesc.initialData.
  HgiTextureDesc _textureDesc;

  // If true, initialData only contains mip level 0 data
  // and the GPU is supposed to generate the other mip levels.
  bool _generateMipmaps;

  // Wrap modes
  std::pair<HdWrap, HdWrap> _wrapInfo;
};

WABI_NAMESPACE_END

#endif
