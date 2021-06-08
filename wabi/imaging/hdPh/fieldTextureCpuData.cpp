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

#include "wabi/imaging/hdPh/fieldTextureCpuData.h"

#include "wabi/imaging/hdPh/textureUtils.h"

#include "wabi/imaging/hio/fieldTextureData.h"

#include "wabi/base/trace/trace.h"

WABI_NAMESPACE_BEGIN

namespace {

bool _IsValid(HioFieldTextureDataSharedPtr const &textureData)
{
  return textureData->ResizedWidth() > 0 && textureData->ResizedHeight() > 0 &&
         textureData->ResizedDepth() > 0 && textureData->HasRawBuffer();
}

}  // anonymous namespace

HdPh_FieldTextureCpuData::HdPh_FieldTextureCpuData(HioFieldTextureDataSharedPtr const &textureData,
                                                   const std::string &debugName,
                                                   const bool premultiplyAlpha)
    : _generateMipmaps(false)
{
  TRACE_FUNCTION();

  _textureDesc.debugName = debugName;

  // Bail if we don't have texture data.
  if (!textureData) {
    return;
  }

  // Sanity checks
  if (!_IsValid(textureData)) {
    return;
  }

  // If there is no file at the given path, we should have bailed
  // by now and left _textureDesc.initalData null indicating to
  // our clients that the texture is invalid.

  _textureDesc.type = HgiTextureType3D;

  // Determine the format (e.g., float/byte, RED/RGBA) and give
  // function to convert data if necessary.
  // Possible conversions are:
  // - Unsigned byte RGB to RGBA (since the former is not support
  //   by modern graphics APIs)
  // - Pre-multiply alpha.

  const HioFormat hioFormat = textureData->GetFormat();

  _textureDesc.format = HdPhTextureUtils::GetHgiFormat(hioFormat, premultiplyAlpha);
  const HdPhTextureUtils::ConversionFunction conversionFunction =
      HdPhTextureUtils::GetHioToHgiConversion(hioFormat, premultiplyAlpha);

  // Handle grayscale textures by expanding value to green and blue.
  if (HgiGetComponentCount(_textureDesc.format) == 1) {
    _textureDesc.componentMapping = {
        HgiComponentSwizzleR, HgiComponentSwizzleR, HgiComponentSwizzleR, HgiComponentSwizzleOne};
  }

  _textureDesc.dimensions = GfVec3i(
      textureData->ResizedWidth(), textureData->ResizedHeight(), textureData->ResizedDepth());

  const std::vector<HgiMipInfo> mipInfos = HgiGetMipInfos(
      _textureDesc.format, _textureDesc.dimensions, _textureDesc.layerCount);

  // How many mipmaps to use from the file.
  unsigned int numGivenMipmaps = 1;
  const HgiMipInfo &mipInfo    = mipInfos[numGivenMipmaps - 1];

  // Size of initial data.
  _textureDesc.pixelsByteSize = mipInfo.byteOffset + mipInfo.byteSizePerLayer;

  if (conversionFunction) {
    const size_t numPixels = _textureDesc.pixelsByteSize /
                             HgiGetDataSizeOfFormat(_textureDesc.format);

    // Convert the texture data
    std::unique_ptr<unsigned char[]> convertedData = std::make_unique<unsigned char[]>(
        _textureDesc.pixelsByteSize);
    conversionFunction(textureData->GetRawBuffer(), numPixels, convertedData.get());
    _convertedData = std::move(convertedData);

    // Point to converted data
    _textureDesc.initialData = _convertedData.get();
  }
  else {
    // Ensure that texture data are not deleted
    _textureData = textureData;
    // Point to raw buffer inside texture data
    _textureDesc.initialData = textureData->GetRawBuffer();
  }
}

HdPh_FieldTextureCpuData::~HdPh_FieldTextureCpuData() = default;

const HgiTextureDesc &HdPh_FieldTextureCpuData::GetTextureDesc() const
{
  return _textureDesc;
}

bool HdPh_FieldTextureCpuData::GetGenerateMipmaps() const
{
  return _generateMipmaps;
}

bool HdPh_FieldTextureCpuData::IsValid() const
{
  return _textureDesc.initialData;
}

WABI_NAMESPACE_END
