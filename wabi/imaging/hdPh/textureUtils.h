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
#ifndef WABI_IMAGING_HD_ST_TEXTURE_UTILS_H
#define WABI_IMAGING_HD_ST_TEXTURE_UTILS_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hgi/types.h"
#include "wabi/imaging/hio/image.h"
#include "wabi/imaging/hio/types.h"

#include <memory>

WABI_NAMESPACE_BEGIN

/// \class HdPhTextureUtils
///
/// Helpers for loading textures.
///
class HdPhTextureUtils
{
 public:
  /// Converts given number of texels.
  ///
  /// Conversion can be in-place if the ends of the source and destination
  /// buffers align.
  using ConversionFunction = void (*)(const void *src, size_t numTexels, void *dst);

  /// Get the Hgi format suitable for a given Hio format.
  ///
  /// Premultiply alpha indicates whether a conversion function
  /// multiplying RGB with alpha should be created.
  HDPH_API
  static HgiFormat GetHgiFormat(HioFormat hioFormat, bool premultiplyAlpha);

  /// Returns the conversion function to return a HioFormat
  /// to the corresponding HgiFormat given by GetHgiFormat.
  ///
  /// Returns nullptr if no conversion necessary.
  HDPH_API
  static ConversionFunction GetHioToHgiConversion(HioFormat hioFormat, bool premultiplyAlpha);

  /// Get all mip levels from a file.
  HDPH_API
  static std::vector<HioImageSharedPtr> GetAllMipImages(const std::string &filePath,
                                                        HioImage::SourceColorSpace sourceColorSpace);

  // Compute dimensions so that all tiles fit into the given target memory.
  // First by traversing the given images and then by computing a mip chain
  // starting with the lowest resolution image.
  // Optionally, can also give the index of the image in mips that was used
  // to compute the dimensions.
  HDPH_API
  static GfVec3i ComputeDimensionsFromTargetMemory(const std::vector<HioImageSharedPtr> &mips,
                                                   HgiFormat targetFormat,
                                                   size_t tileCount,
                                                   size_t targetMemory,
                                                   size_t *mipIndex = nullptr);

  // Read given HioImage and convert it to corresponding Hgi format.
  // Returns false if reading the HioImage failed.
  //
  // bufferStart is assumed to point at the beginning of a mip chain
  // with mipInfo describing what mip level of the mip chain to be
  // filled. layer gives the layer number if the mip chain is for an
  // array texture.
  HDPH_API
  static bool ReadAndConvertImage(HioImageSharedPtr const &image,
                                  bool flipped,
                                  bool premultiplyAlpha,
                                  const HgiMipInfo &mipInfo,
                                  size_t layer,
                                  void *bufferStart);
};

WABI_NAMESPACE_END

#endif
