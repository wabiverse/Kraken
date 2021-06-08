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
#ifndef WABI_IMAGING_HGI_GL_CONVERSIONS_H
#define WABI_IMAGING_HGI_GL_CONVERSIONS_H

#include "wabi/imaging/garch/glApi.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgi/types.h"
#include "wabi/imaging/hgiGL/api.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

///
/// \class HgiGLConversions
///
/// Converts from Hgi types to GL types.
///
class HgiGLConversions final {
 public:
  HGIGL_API
  static void GetFormat(HgiFormat inFormat,
                        GLenum *outFormat,
                        GLenum *outType,
                        GLenum *outInternalFormat = nullptr);

  HGIGL_API
  static GLenum GetFormatType(HgiFormat inFormat);

  HGIGL_API
  static std::vector<GLenum> GetShaderStages(HgiShaderStage ss);

  HGIGL_API
  static GLenum GetCullMode(HgiCullMode cm);

  HGIGL_API
  static GLenum GetPolygonMode(HgiPolygonMode pm);

  HGIGL_API
  static GLenum GetBlendFactor(HgiBlendFactor bf);

  HGIGL_API
  static GLenum GetBlendEquation(HgiBlendOp bo);

  HGIGL_API
  static GLenum GetDepthCompareFunction(HgiCompareFunction cf);

  HGIGL_API
  static GLenum GetTextureType(HgiTextureType tt);

  HGIGL_API
  static GLenum GetSamplerAddressMode(HgiSamplerAddressMode am);

  HGIGL_API
  static GLenum GetMagFilter(HgiSamplerFilter mf);

  HGIGL_API
  static GLenum GetMinFilter(HgiSamplerFilter minFilter, HgiMipFilter mipFilter);

  HGIGL_API
  static GLenum GetComponentSwizzle(HgiComponentSwizzle);

  HGIGL_API
  static GLenum GetPrimitiveType(HgiPrimitiveType pt);
};

WABI_NAMESPACE_END

#endif
