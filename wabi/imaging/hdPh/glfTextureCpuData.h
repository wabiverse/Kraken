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
#ifndef WABI_IMAGING_HD_PH_GLF_TEXTURE_CPU_DATA_H
#define WABI_IMAGING_HD_PH_GLF_TEXTURE_CPU_DATA_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/imaging/hdPh/textureCpuData.h"
#include "wabi/imaging/hgi/texture.h"
#include "wabi/imaging/hio/image.h"

#include <memory>

WABI_NAMESPACE_BEGIN

TF_DECLARE_REF_PTRS(GlfBaseTextureData);

/// \class HdPhTextureCpuData
///
/// An implmentation of HdPhTextureCpuData that can be initialized
/// from GlfBaseTextureData.
///
class HdPhGlfTextureCpuData : public HdPhTextureCpuData {
 public:
  /// It is assumed that Read(...) has already been called
  /// on textureData.

  HDPH_API
  HdPhGlfTextureCpuData(GlfBaseTextureDataConstRefPtr const &textureData,
                        const std::string &debugName,
                        bool useOrGenerateMips = false,
                        bool premultiplyAlpha  = true);

  HDPH_API
  ~HdPhGlfTextureCpuData() override;

  HDPH_API
  const HgiTextureDesc &GetTextureDesc() const override;

  HDPH_API
  bool GetGenerateMipmaps() const override;

  HDPH_API
  bool IsValid() const override;

 private:
  // The result, including a pointer to the potentially
  // converted texture data in _textureDesc.initialData.
  HgiTextureDesc _textureDesc;

  // If true, initialData only contains mip level 0 data
  // and the GPU is supposed to generate the other mip levels.
  bool _generateMipmaps;

  // To avoid a copy, hold on to original data if we
  // can use them.
  GlfBaseTextureDataConstRefPtr _textureData;

  // Buffer if we had to convert the data.
  std::unique_ptr<const unsigned char[]> _convertedData;
};

WABI_NAMESPACE_END

#endif
