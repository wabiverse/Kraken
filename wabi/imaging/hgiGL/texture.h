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
#ifndef WABI_IMAGING_HGI_GL_TEXTURE_H
#define WABI_IMAGING_HGI_GL_TEXTURE_H

#include "wabi/imaging/hgi/texture.h"
#include "wabi/imaging/hgiGL/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class HgiGLTexture
///
/// Represents a OpenGL GPU texture resource.
///
class HgiGLTexture final : public HgiTexture {
 public:
  HGIGL_API
  ~HgiGLTexture() override;

  HGIGL_API
  size_t GetByteSizeOfResource() const override;

  HGIGL_API
  uint64_t GetRawResource() const override;

  /// Returns the OpenGL id / name of the texture.
  uint32_t GetTextureId() const
  {
    return _textureId;
  }

 protected:
  friend class HgiGL;

  HGIGL_API
  HgiGLTexture(HgiTextureDesc const &desc);

  HGIGL_API
  HgiGLTexture(HgiTextureViewDesc const &desc);

 private:
  HgiGLTexture()        = delete;
  HgiGLTexture &operator=(const HgiGLTexture &) = delete;
  HgiGLTexture(const HgiGLTexture &)            = delete;

  uint32_t _textureId;
};

WABI_NAMESPACE_END

#endif
