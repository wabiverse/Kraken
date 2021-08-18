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
#ifndef WABI_IMAGING_HGIINTEROP_HGIINTEROP_H
#define WABI_IMAGING_HGIINTEROP_HGIINTEROP_H

#include "wabi/base/gf/vec4i.h"
#include "wabi/base/tf/token.h"
#include "wabi/imaging/hgi/texture.h"
#include "wabi/imaging/hgiInterop/api.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class Hgi;
class HgiInteropMetal;
class HgiInteropOpenGL;
class HgiInteropVulkan;
class HgiInteropDX3D;
class VtValue;

/// \class HgiInterop
///
/// Hydra Graphics Interface Interop.
///
/// HgiInterop provides functionality to transfer render targets between
/// supported APIs as efficiently as possible.
///
class HgiInterop final
{
 public:
  HGIINTEROP_API
  HgiInterop();

  HGIINTEROP_API
  ~HgiInterop();

  /// Composite the provided textures over the application / viewer's
  /// framebuffer contents.
  /// `srcHgi`:
  ///     Determines the source format/platform of the textures.
  ///     Eg. if hgi is of type HgiMetal, the textures are HgiMetalTexture.
  /// `srcColor`: is the source color aov texture to composite to screen.
  /// `srcDepth`: (optional) is the depth aov texture to composite to screen.
  /// `dstApi`:
  ///     Determines what target format/platform the application is using.
  ///     E.g. If hgi==HgiMetal and dstApi==OpenGL then TransferToApp
  ///     will present the metal textures to the gl application.
  /// `dstFramebuffer`:
  ///     The framebuffer that the source textures are presented into. This
  ///     is a VtValue that encoding a framebuffer in a dstApi specific way.
  ///     E.g., a uint32_t (aka GLuint) for framebuffer object for
  ///     dstApi==OpenGL. For backwards compatibility, the currently bound
  ///     framebuffer is used when the VtValue is empty.
  ///
  /// `dstRegion`:
  ///     Subrect region of the framebuffer over which to composite.
  ///     Coordinates are (left, BOTTOM, width, height) which is the same
  ///     convention as OpenGL viewport coordinates.
  ///
  /// Note:
  /// To composite correctly, blending is enabled.
  /// If `srcDepth` is provided, depth testing is enabled.
  /// As a result, the contents of the application framebuffer matter.
  /// In order to use the contents of `srcColor` and `srcDepth` as-is
  /// (i.e., blit), the color attachment should be cleared to (0,0,0,0) and
  /// the depth attachment needs to be cleared to 1.
  ///
  HGIINTEROP_API
  void TransferToApp(Hgi *srcHgi,
                     HgiTextureHandle const &srcColor,
                     HgiTextureHandle const &srcDepth,
                     TfToken const &dstApi,
                     VtValue const &dstFramebuffer,
                     GfVec4i const &dstRegion);

 private:
  HgiInterop &operator=(const HgiInterop &) = delete;
  HgiInterop(const HgiInterop &) = delete;

#if defined(WITH_METAL)
  std::unique_ptr<HgiInteropMetal> _metalToOpenGL;
#endif /* WITH_METAL */

#if defined(WITH_DIRECTX)
  std::unique_ptr<HgiInteropDX3D> _dX3DToOpenGL;
#endif /* WITH_DIRECTX */

#if defined(WITH_VULKAN)
  std::unique_ptr<HgiInteropVulkan> _vulkanToOpenGL;
#endif /* WITH_VULKAN */

  std::unique_ptr<HgiInteropOpenGL> _openGLToOpenGL;
};

WABI_NAMESPACE_END

#endif
