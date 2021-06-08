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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/imaging/hgi/blitCmdsOps.h"
#include "wabi/imaging/hgiGL/blitCmds.h"
#include "wabi/imaging/hgiGL/buffer.h"
#include "wabi/imaging/hgiGL/conversions.h"
#include "wabi/imaging/hgiGL/device.h"
#include "wabi/imaging/hgiGL/diagnostic.h"
#include "wabi/imaging/hgiGL/hgi.h"
#include "wabi/imaging/hgiGL/ops.h"
#include "wabi/imaging/hgiGL/scopedStateHolder.h"
#include "wabi/imaging/hgiGL/texture.h"

WABI_NAMESPACE_BEGIN

HgiGLBlitCmds::HgiGLBlitCmds() : HgiBlitCmds(), _pushStack(0)
{}

HgiGLBlitCmds::~HgiGLBlitCmds() = default;

void HgiGLBlitCmds::PushDebugGroup(const char *label)
{
  if (HgiGLDebugEnabled()) {
    _pushStack++;
    _ops.push_back(HgiGLOps::PushDebugGroup(label));
  }
}

void HgiGLBlitCmds::PopDebugGroup()
{
  if (HgiGLDebugEnabled()) {
    _pushStack--;
    _ops.push_back(HgiGLOps::PopDebugGroup());
  }
}

void HgiGLBlitCmds::CopyTextureGpuToCpu(HgiTextureGpuToCpuOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyTextureGpuToCpu(copyOp));
}

void HgiGLBlitCmds::CopyTextureCpuToGpu(HgiTextureCpuToGpuOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyTextureCpuToGpu(copyOp));
}

void HgiGLBlitCmds::CopyBufferGpuToGpu(HgiBufferGpuToGpuOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyBufferGpuToGpu(copyOp));
}

void HgiGLBlitCmds::CopyBufferCpuToGpu(HgiBufferCpuToGpuOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyBufferCpuToGpu(copyOp));
}

void HgiGLBlitCmds::CopyBufferGpuToCpu(HgiBufferGpuToCpuOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyBufferGpuToCpu(copyOp));
}

void HgiGLBlitCmds::CopyTextureToBuffer(HgiTextureToBufferOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyTextureToBuffer(copyOp));
}

void HgiGLBlitCmds::CopyBufferToTexture(HgiBufferToTextureOp const &copyOp)
{
  _ops.push_back(HgiGLOps::CopyBufferToTexture(copyOp));
}

void HgiGLBlitCmds::GenerateMipMaps(HgiTextureHandle const &texture)
{
  _ops.push_back(HgiGLOps::GenerateMipMaps(texture));
}

void HgiGLBlitCmds::MemoryBarrier(HgiMemoryBarrier barrier)
{
  _ops.push_back(HgiGLOps::MemoryBarrier(barrier));
}

bool HgiGLBlitCmds::_Submit(Hgi *hgi, HgiSubmitWaitType wait)
{
  if (_ops.empty()) {
    return false;
  }

  TF_VERIFY(_pushStack == 0, "Push and PopDebugGroup do not even out");

  // Capture OpenGL state before executing the 'ops' and restore it when this
  // function ends. We do this defensively because parts of our pipeline may
  // not set and restore all relevant gl state.
  HgiGL_ScopedStateHolder openglStateGuard;

  HgiGL *hgiGL        = static_cast<HgiGL *>(hgi);
  HgiGLDevice *device = hgiGL->GetPrimaryDevice();
  device->SubmitOps(_ops);
  return true;
}

WABI_NAMESPACE_END
