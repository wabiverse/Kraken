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
#include "wabi/imaging/hgiInterop/hgiInterop.h"
#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"

/* clang-format off */
#if defined(WITH_METAL)
#  include "wabi/imaging/hgiInterop/metal.h"
#  include "wabi/imaging/hgiMetal/hgi.h"
#endif /* WITH_METAL */

#if defined(WITH_VULKAN)
#  include "wabi/imaging/hgiInterop/vulkan.h"
#  include "wabi/imaging/hgiVulkan/hgi.h"
#endif /* WITH_VULKAN */

#if defined(WITH_DIRECTX)
#  include "wabi/imaging/hgiInterop/dx3d.h"
#  include "wabi/imaging/hgiDX3D/hgi.h"
#endif /* WITH_DIRECTX */

#include "wabi/imaging/hgiInterop/opengl.h"
/* clang-format on */

WABI_NAMESPACE_BEGIN

HgiInterop::HgiInterop() = default;

HgiInterop::~HgiInterop() = default;

void HgiInterop::TransferToApp(Hgi *srcHgi,
                               HgiTextureHandle const &srcColor,
                               HgiTextureHandle const &srcDepth,
                               TfToken const &dstApi,
                               VtValue const &dstFramebuffer,
                               GfVec4i const &dstRegion)
{
  TfToken const &srcApi = srcHgi->GetAPIName();

/**
 * ----------------------------------------- Check Metal -> OpenGL first. ----- */
#if defined(WITH_METAL)
  if (srcApi == HgiTokens->Metal && dstApi == HgiTokens->OpenGL) {
    /**
     * Transfer Metal textures to OpenGL application. */
    if (!_metalToOpenGL) {
      _metalToOpenGL = std::make_unique<HgiInteropMetal>(srcHgi);
    }
    _metalToOpenGL->CompositeToInterop(srcColor, srcDepth, dstFramebuffer, dstRegion);
    return;
  }
#endif /* WITH_METAL */

/**
 * ------------------------------------ Otherwise check DirectX -> OpenGL. ----- */
#if defined(WITH_DIRECTX)
  if (srcApi == HgiTokens->DX3D && dstApi == HgiTokens->OpenGL) {
    /**
     * Transfer DirectX textures to OpenGL application. */
    if (!_dX3DToOpenGL) {
      _dX3DToOpenGL = std::make_unique<HgiInteropDX3D>(srcHgi);
    }
    _dX3DToOpenGL->CompositeToInterop(srcColor, srcDepth, dstFramebuffer, dstRegion);
    return;
  }
#endif /* WITH_DIRECTX */

/**
 * ------------------------------------ Otherwise check Vulkan -> OpenGL. ----- */
#if defined(WITH_VULKAN)
  if (srcApi == HgiTokens->Vulkan && dstApi == HgiTokens->OpenGL) {
    /**
     * Transfer Vulkan textures to OpenGL application. */
    if (!_vulkanToOpenGL) {
      _vulkanToOpenGL = std::make_unique<HgiInteropVulkan>(srcHgi);
    }
    _vulkanToOpenGL->CompositeToInterop(srcColor, srcDepth, dstFramebuffer, dstRegion);
    return;
  }
#endif /* WITH_VULKAN */

  /**
   * ---------------------------------------- Otherwise fallback to OpenGL. ----- */
  if (srcApi == HgiTokens->OpenGL && dstApi == HgiTokens->OpenGL) {
    /**
     * Transfer OpenGL textures to OpenGL application. */
    if (!_openGLToOpenGL) {
      _openGLToOpenGL = std::make_unique<HgiInteropOpenGL>();
    }
    _openGLToOpenGL->CompositeToInterop(srcColor, srcDepth, dstFramebuffer, dstRegion);
    return;
  }

  /**
   * --------------------- No backend outside of OpenGL, Vulkan, and Metal. ----- */
  TF_CODING_ERROR("Unsupported Hgi backend: %s", srcApi.GetText());
}

WABI_NAMESPACE_END
