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

#include "wabi/imaging/hgi/graphicsCmdsDesc.h"
#include "wabi/imaging/hgiGL/buffer.h"
#include "wabi/imaging/hgiGL/conversions.h"
#include "wabi/imaging/hgiGL/device.h"
#include "wabi/imaging/hgiGL/diagnostic.h"
#include "wabi/imaging/hgiGL/graphicsCmds.h"
#include "wabi/imaging/hgiGL/graphicsPipeline.h"
#include "wabi/imaging/hgiGL/hgi.h"
#include "wabi/imaging/hgiGL/ops.h"
#include "wabi/imaging/hgiGL/resourceBindings.h"
#include "wabi/imaging/hgiGL/scopedStateHolder.h"

WABI_NAMESPACE_BEGIN

HgiGLGraphicsCmds::HgiGLGraphicsCmds(HgiGLDevice *device, HgiGraphicsCmdsDesc const &desc)
    : HgiGraphicsCmds(),
      _recording(true),
      _descriptor(desc),
      _primitiveType(HgiPrimitiveTypeTriangleList),
      _pushStack(0)
{
  if (desc.HasAttachments()) {
    _ops.push_back(HgiGLOps::BindFramebufferOp(device, desc));
  }
}

HgiGLGraphicsCmds::~HgiGLGraphicsCmds() = default;

void HgiGLGraphicsCmds::InsertFunctionOp(std::function<void(void)> const &fn)
{
  _ops.push_back(fn);
}

void HgiGLGraphicsCmds::SetViewport(GfVec4i const &vp)
{
  _ops.push_back(HgiGLOps::SetViewport(vp));
}

void HgiGLGraphicsCmds::SetScissor(GfVec4i const &sc)
{
  _ops.push_back(HgiGLOps::SetScissor(sc));
}

void HgiGLGraphicsCmds::BindPipeline(HgiGraphicsPipelineHandle pipeline)
{
  _primitiveType = pipeline->GetDescriptor().primitiveType;
  _ops.push_back(HgiGLOps::BindPipeline(pipeline));
}

void HgiGLGraphicsCmds::BindResources(HgiResourceBindingsHandle res)
{
  _ops.push_back(HgiGLOps::BindResources(res));
}

void HgiGLGraphicsCmds::SetConstantValues(HgiGraphicsPipelineHandle pipeline,
                                          HgiShaderStage stages,
                                          uint32_t bindIndex,
                                          uint32_t byteSize,
                                          const void *data)
{
  _ops.push_back(HgiGLOps::SetConstantValues(pipeline, stages, bindIndex, byteSize, data));
}

void HgiGLGraphicsCmds::BindVertexBuffers(uint32_t firstBinding,
                                          HgiBufferHandleVector const &vertexBuffers,
                                          std::vector<uint32_t> const &byteOffsets)
{
  _ops.push_back(HgiGLOps::BindVertexBuffers(firstBinding, vertexBuffers, byteOffsets));
}

void HgiGLGraphicsCmds::Draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount)
{
  _ops.push_back(HgiGLOps::Draw(_primitiveType, vertexCount, vertexOffset, instanceCount));
}

void HgiGLGraphicsCmds::DrawIndirect(HgiBufferHandle const &drawParameterBuffer,
                                     uint32_t drawBufferOffset,
                                     uint32_t drawCount,
                                     uint32_t stride)
{
  _ops.push_back(HgiGLOps::DrawIndirect(
      _primitiveType, drawParameterBuffer, drawBufferOffset, drawCount, stride));
}

void HgiGLGraphicsCmds::DrawIndexed(HgiBufferHandle const &indexBuffer,
                                    uint32_t indexCount,
                                    uint32_t indexBufferByteOffset,
                                    uint32_t vertexOffset,
                                    uint32_t instanceCount)
{
  _ops.push_back(HgiGLOps::DrawIndexed(_primitiveType,
                                       indexBuffer,
                                       indexCount,
                                       indexBufferByteOffset,
                                       vertexOffset,
                                       instanceCount));
}

void HgiGLGraphicsCmds::DrawIndexedIndirect(HgiBufferHandle const &indexBuffer,
                                            HgiBufferHandle const &drawParameterBuffer,
                                            uint32_t drawBufferOffset,
                                            uint32_t drawCount,
                                            uint32_t stride)
{
  _ops.push_back(HgiGLOps::DrawIndexedIndirect(
      _primitiveType, indexBuffer, drawParameterBuffer, drawBufferOffset, drawCount, stride));
}

void HgiGLGraphicsCmds::PushDebugGroup(const char *label)
{
  if (HgiGLDebugEnabled()) {
    _pushStack++;
    _ops.push_back(HgiGLOps::PushDebugGroup(label));
  }
}

void HgiGLGraphicsCmds::PopDebugGroup()
{
  if (HgiGLDebugEnabled()) {
    _pushStack--;
    _ops.push_back(HgiGLOps::PopDebugGroup());
  }
}

void HgiGLGraphicsCmds::MemoryBarrier(HgiMemoryBarrier barrier)
{
  _ops.push_back(HgiGLOps::MemoryBarrier(barrier));
}

bool HgiGLGraphicsCmds::_Submit(Hgi *hgi, HgiSubmitWaitType wait)
{
  if (_ops.empty()) {
    return false;
  }

  TF_VERIFY(_pushStack == 0, "Push and PopDebugGroup do not even out");

  // Capture OpenGL state before executing the 'ops' and restore it when this
  // function ends. We do this defensively because parts of our pipeline may
  // not set and restore all relevant gl state.
  HgiGL_ScopedStateHolder openglStateGuard;

  // Resolve multisample textures
  HgiGL *hgiGL        = static_cast<HgiGL *>(hgi);
  HgiGLDevice *device = hgiGL->GetPrimaryDevice();
  _AddResolveToOps(device);

  device->SubmitOps(_ops);
  return true;
}

void HgiGLGraphicsCmds::_AddResolveToOps(HgiGLDevice *device)
{
  if (!_recording) {
    return;
  }

  if (!_descriptor.colorResolveTextures.empty() &&
      _descriptor.colorResolveTextures.size() != _descriptor.colorTextures.size()) {
    TF_CODING_ERROR("color and resolve texture count mismatch.");
    return;
  }

  if (_descriptor.depthResolveTexture && !_descriptor.depthTexture) {
    TF_CODING_ERROR("DepthResolve texture without depth texture.");
    return;
  }

  if ((!_descriptor.colorResolveTextures.empty()) || _descriptor.depthResolveTexture) {
    // At the end of the GraphicsCmd we resolve the multisample
    // textures.  This emulates what happens in Metal or Vulkan
    // when the multisample resolve happens at the end of a render
    // pass.
    _ops.push_back(HgiGLOps::ResolveFramebuffer(device, _descriptor));
  }

  _recording = false;
}

WABI_NAMESPACE_END
