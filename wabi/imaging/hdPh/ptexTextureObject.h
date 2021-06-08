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
#ifndef WABI_IMAGING_HD_PH_PTEX_TEXTURE_OBJECT_H
#define WABI_IMAGING_HD_PH_PTEX_TEXTURE_OBJECT_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/textureObject.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hgi/handle.h"

#include "wabi/base/gf/vec2i.h"
#include "wabi/base/gf/vec3i.h"

#ifdef WITH_PTEX
#  include "wabi/imaging/hdPh/ptexMipmapTextureLoader.h"
#endif

WABI_NAMESPACE_BEGIN

/// Returns true if the file given by \p imageFilePath represents a ptex file,
/// and false otherwise.
///
/// This function simply checks the extension of the file name and does not
/// otherwise guarantee that the file is in any way valid for reading.
///
/// If ptex support is disabled, this function will always return false.
///
HDPH_API bool HdPhIsSupportedPtexTexture(std::string const &imageFilePath);

enum HgiFormat : int;
using HgiTextureHandle = HgiHandle<class HgiTexture>;

/// \class HdPhPtexTextureObject
///
/// A Ptex texture.
///
class HdPhPtexTextureObject final : public HdPhTextureObject {
 public:
  HDPH_API
  HdPhPtexTextureObject(const HdPhTextureIdentifier &textureId,
                        HdPh_TextureObjectRegistry *textureObjectRegistry);

  HDPH_API
  ~HdPhPtexTextureObject() override;

  /// Get the GPU texture handle for the texels
  ///
  /// Only valid after commit phase.
  ///
  HgiTextureHandle GetTexelTexture() const
  {
    return _texelTexture;
  }

  /// Get the GPU texture handle for the layout
  ///
  /// Only valid after commit phase.
  ///
  HgiTextureHandle GetLayoutTexture() const
  {
    return _layoutTexture;
  }

  HDPH_API
  bool IsValid() const override;

  HDPH_API
  HdTextureType GetTextureType() const override;

 protected:
  HDPH_API
  void _Load() override;

  HDPH_API
  void _Commit() override;

 private:
  HgiFormat _format;
  GfVec3i _texelDimensions;
  int _texelLayers;
  size_t _texelDataSize;
  GfVec2i _layoutDimensions;
  size_t _layoutDataSize;

  std::unique_ptr<uint8_t[]> _texelData;
  std::unique_ptr<uint8_t[]> _layoutData;

  HgiTextureHandle _texelTexture;
  HgiTextureHandle _layoutTexture;

  void _DestroyTextures();
};

template<> struct HdPh_TypedTextureObjectHelper<HdTextureType::Ptex> {
  using type = HdPhPtexTextureObject;
};

WABI_NAMESPACE_END

#endif
