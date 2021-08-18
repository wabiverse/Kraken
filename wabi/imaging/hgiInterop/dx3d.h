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
 * Apache License.-
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#ifndef WABI_IMAGING_HGIINTEROP_HGIINTEROPDX3D_H
#define WABI_IMAGING_HGIINTEROP_HGIINTEROPDX3D_H

/* clang-format off */
#include "wabi/wabi.h"
#include "wabi/base/gf/vec4i.h"
#include "wabi/imaging/hgi/texture.h"
#include "wabi/imaging/hgiInterop/api.h"
/* clang-format on */

WABI_NAMESPACE_BEGIN

class HgiDX3D;
class VtValue;

/**
 * @class HgiInteropDX3D
 *
 * Provides DirectX/GL interop. */
class HgiInteropDX3D final
{
 public:
  HGIINTEROP_API
  HgiInteropDX3D(Hgi *hgiDX3D);

  HGIINTEROP_API
  ~HgiInteropDX3D();

  /**
   * Composite provided color (and optional depth)
   * textures over app's framebuffer contents. */
  HGIINTEROP_API
  void CompositeToInterop(HgiTextureHandle const &color,
                          HgiTextureHandle const &depth,
                          VtValue const &framebuffer,
                          GfVec4i const &viewport);

 private:
  HgiInteropDX3D() = delete;

  HgiDX3D *_hgiDX3D;
  uint32_t _vs;
  uint32_t _fsNoDepth;
  uint32_t _fsDepth;
  uint32_t _prgNoDepth;
  uint32_t _prgDepth;
  uint32_t _vertexBuffer;

  /**
   * XXX We tmp copy DirectX's GPU texture to
   * CPU and then to GL texture once we share
   * GPU memory between DirectX and GL we can
   * remove this. */
  uint32_t _glColorTex;
  uint32_t _glDepthTex;
};

WABI_NAMESPACE_END

#endif /* WABI_IMAGING_HGIINTEROP_HGIINTEROPDX3D_H */
