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
#include "wabi/imaging/hgi/graphicsCmdsDesc.h"
#include "wabi/imaging/hgiMetal/buffer.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/graphicsCmds.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/graphicsPipeline.h"
#include "wabi/imaging/hgiMetal/resourceBindings.h"
#include "wabi/imaging/hgiMetal/texture.h"

#include "wabi/base/work/dispatcher.h"
#include "wabi/base/work/loops.h"
#include "wabi/base/work/withScopedParallelism.h"

#include "wabi/base/arch/defines.h"

WABI_NAMESPACE_BEGIN

static void _VegaIndirectFix(HgiMetal *hgi,
                             MTL::RenderCommandEncoder *encoder,
                             MTL::PrimitiveType primType)
{
  if (hgi->GetCapabilities()->requiresIndirectDrawFix) {
    // Fix for Vega in macOS before 12.0.  There is state leakage between
    // indirect draw of different prim types which results in a GPU crash.
    // Flush with a null draw through the direct path.
    encoder->drawPrimitives(primType, NS::UInteger(0), NS::UInteger(0));
  }
}

HgiMetalGraphicsCmds::CachedEncoderState::CachedEncoderState()
{
  numVertexBufferBindings = MAX_METAL_VERTEX_BUFFER_BINDINGS;
  ResetCachedEncoderState();
}

void HgiMetalGraphicsCmds::CachedEncoderState::ResetCachedEncoderState()
{
  for (uint32_t i = 0; i < numVertexBufferBindings; i++) {
    VertexBufferBindingDesc[i].vertexBuffer = nil;
    VertexBufferBindingDesc[i].byteOffset = 0;
  }

  resourceBindings = nil;
  graphicsPipeline = nil;
  argumentBuffer = nil;
  numVertexBufferBindings = 0;
}

void HgiMetalGraphicsCmds::CachedEncoderState::AddVertexBinding(uint32_t bindingIndex,
                                                                MTL::Buffer *buffer,
                                                                uint32_t byteOffset)
{
  if (bindingIndex >= MAX_METAL_VERTEX_BUFFER_BINDINGS) {
    TF_CODING_ERROR("Binding index exceeds space for cached state");
    return;
  }
  VertexBufferBindingDesc[bindingIndex].vertexBuffer = buffer;
  VertexBufferBindingDesc[bindingIndex].byteOffset = byteOffset;
  numVertexBufferBindings = std::max(numVertexBufferBindings, bindingIndex + 1);
}

HgiMetalGraphicsCmds::HgiMetalGraphicsCmds(HgiMetal *hgi, HgiGraphicsCmdsDesc const &desc)
  : HgiGraphicsCmds(),
    _hgi(hgi),
    _renderPassDescriptor(nil),
    _parallelEncoder(nil),
    _argumentBuffer(nil),
    _descriptor(desc),
    _primitiveType(HgiPrimitiveTypeTriangleList),
    _primitiveIndexSize(0),
    _debugLabel(nil),
    _hasWork(false),
    _viewportSet(false),
    _scissorRectSet(false),
    _enableParallelEncoder(false),
    _primitiveTypeChanged(false),
    _maxNumEncoders(1)
{
  TF_VERIFY(desc.colorTextures.size() == desc.colorAttachmentDescs.size());

  if (!desc.colorResolveTextures.empty() &&
      desc.colorResolveTextures.size() != desc.colorTextures.size()) {
    TF_CODING_ERROR("color and resolve texture count mismatch.");
    return;
  }

  if (desc.depthResolveTexture && !desc.depthTexture) {
    TF_CODING_ERROR("DepthResolve texture without depth texture.");
    return;
  }

  static const size_t _maxStepFunctionDescs = 4;
  _vertexBufferStepFunctionDescs.reserve(_maxStepFunctionDescs);
  _patchBaseVertexBufferStepFunctionDescs.reserve(_maxStepFunctionDescs);

  _renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();

  // The GPU culling pass is only a vertex shader, so it doesn't have any
  // render targets bound to it.  To prevent an API validation error, set
  // some default values for the target.
  if (!desc.HasAttachments()) {
    _renderPassDescriptor->setRenderTargetWidth(256);
    _renderPassDescriptor->setRenderTargetHeight(256);
    _renderPassDescriptor->setDefaultRasterSampleCount(1);
  }

  // Color attachments
  bool resolvingColor = !desc.colorResolveTextures.empty();
  bool hasClear = false;
  for (size_t i = 0; i < desc.colorAttachmentDescs.size(); i++) {
    HgiAttachmentDesc const &hgiColorAttachment = desc.colorAttachmentDescs[i];
    MTL::RenderPassColorAttachmentDescriptor *metalColorAttachment =
      _renderPassDescriptor->colorAttachments()->object(i);

    if (hgiColorAttachment.loadOp == HgiAttachmentLoadOpClear) {
      hasClear = true;
    }

#ifdef ARCH_OS_IOS
    metalColorAttachment->setLoadAction(MTL::LoadActionLoad);
#else
    metalColorAttachment->setLoadAction(
      HgiMetalConversions::GetAttachmentLoadOp(hgiColorAttachment.loadOp));
#endif /* ARCH_OS_IOS */

    metalColorAttachment->setStoreAction(
      HgiMetalConversions::GetAttachmentStoreOp(hgiColorAttachment.storeOp));
    if (hgiColorAttachment.loadOp == HgiAttachmentLoadOpClear) {
      GfVec4f const &clearColor = hgiColorAttachment.clearValue;
      metalColorAttachment->setClearColor(
        MTL::ClearColor::Make(clearColor[0], clearColor[1], clearColor[2], clearColor[3]));
    }

    HgiMetalTexture *colorTexture = static_cast<HgiMetalTexture *>(desc.colorTextures[i].Get());

    TF_VERIFY(colorTexture->GetDescriptor().format == hgiColorAttachment.format);
    metalColorAttachment->setTexture(colorTexture->GetTextureId());

    if (resolvingColor) {
      HgiMetalTexture *resolveTexture = static_cast<HgiMetalTexture *>(
        desc.colorResolveTextures[i].Get());

      metalColorAttachment->setResolveTexture(resolveTexture->GetTextureId());

      if (hgiColorAttachment.storeOp == HgiAttachmentStoreOpStore) {
        metalColorAttachment->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
      } else {
        metalColorAttachment->setStoreAction(MTL::StoreActionMultisampleResolve);
      }
    }
  }

  // Depth attachment
  if (desc.depthTexture) {
    HgiAttachmentDesc const &hgiDepthAttachment = desc.depthAttachmentDesc;
    MTL::RenderPassDepthAttachmentDescriptor *metalDepthAttachment =
      _renderPassDescriptor->depthAttachment();

    if (hgiDepthAttachment.loadOp == HgiAttachmentLoadOpClear) {
      hasClear = true;
    }

    metalDepthAttachment->setLoadAction(
      HgiMetalConversions::GetAttachmentLoadOp(hgiDepthAttachment.loadOp));
    metalDepthAttachment->setStoreAction(
      HgiMetalConversions::GetAttachmentStoreOp(hgiDepthAttachment.storeOp));

    metalDepthAttachment->setClearDepth(hgiDepthAttachment.clearValue[0]);

    HgiMetalTexture *depthTexture = static_cast<HgiMetalTexture *>(desc.depthTexture.Get());

    TF_VERIFY(depthTexture->GetDescriptor().format == hgiDepthAttachment.format);
    metalDepthAttachment->setTexture(depthTexture->GetTextureId());

    if (desc.depthResolveTexture) {
      HgiMetalTexture *resolveTexture = static_cast<HgiMetalTexture *>(
        desc.depthResolveTexture.Get());

      metalDepthAttachment->setResolveTexture(resolveTexture->GetTextureId());

      if (hgiDepthAttachment.storeOp == HgiAttachmentStoreOpStore) {
        metalDepthAttachment->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
      } else {
        metalDepthAttachment->setStoreAction(MTL::StoreActionMultisampleResolve);
      }
    }

    // Stencil attachment
    if (depthTexture->GetDescriptor().format == HgiFormatFloat32UInt8) {
      MTL::RenderPassStencilAttachmentDescriptor *stencilAttachment =
        _renderPassDescriptor->stencilAttachment();
      stencilAttachment->setLoadAction(metalDepthAttachment->loadAction());
      stencilAttachment->setStoreAction(metalDepthAttachment->storeAction());
      stencilAttachment->setClearStencil(hgiDepthAttachment.clearValue[0]);
      stencilAttachment->setTexture(metalDepthAttachment->texture());

      if (desc.depthResolveTexture) {
        stencilAttachment->setResolveTexture(metalDepthAttachment->resolveTexture());
        stencilAttachment->setStencilResolveFilter(
          MTL::MultisampleStencilResolveFilterDepthResolvedSample);
        stencilAttachment->setStoreAction(metalDepthAttachment->storeAction());
      }
    }
  }

  _enableParallelEncoder = _hgi->GetCapabilities()->useParallelEncoder;

  if (_enableParallelEncoder) {
    _maxNumEncoders = WorkGetPhysicalConcurrencyLimit() / 2;
  } else {
    _maxNumEncoders = 1;
  }

  if (hasClear) {
    _GetEncoder();
    _CreateArgumentBuffer();
  }
}

HgiMetalGraphicsCmds::~HgiMetalGraphicsCmds()
{
  TF_VERIFY(_encoders.empty(), "Encoder created, but never commited.");

  _renderPassDescriptor->release();
  if (_debugLabel) {
    _debugLabel->release();
  }
}

void HgiMetalGraphicsCmds::EnableParallelEncoder(bool enable)
{
  _enableParallelEncoder = enable;
}

uint32_t HgiMetalGraphicsCmds::_GetNumEncoders()
{
  return (uint32_t)_encoders.size();
}

void HgiMetalGraphicsCmds::_SetCachedEncoderState(MTL::RenderCommandEncoder *encoder)
{
  if (_viewportSet) {
    encoder->setViewport(_CachedEncState.viewport);
  }
  if (_scissorRectSet) {
    encoder->setScissorRect(_CachedEncState.scissorRect);
  }
  if (_CachedEncState.graphicsPipeline) {
    _CachedEncState.graphicsPipeline->BindPipeline(encoder);
  }
  if (_CachedEncState.resourceBindings) {
    _CachedEncState.resourceBindings->BindResources(_hgi, encoder, _CachedEncState.argumentBuffer);
  }

  for (uint32_t j = 0; j < _CachedEncState.numVertexBufferBindings; j++) {
    if (_CachedEncState.VertexBufferBindingDesc[j].vertexBuffer != nil) {
      encoder->setVertexBuffer(_CachedEncState.VertexBufferBindingDesc[j].vertexBuffer,
                               _CachedEncState.VertexBufferBindingDesc[j].byteOffset,
                               j);
    }
  }
}

void HgiMetalGraphicsCmds::_SetNumberParallelEncoders(uint32_t numEncoders)
{
  // Put a lock around the creation to prevent two requests colliding
  // (this should never happen...)
  std::lock_guard<std::mutex> lock(_encoderLock);

  uint32_t const numActiveEncoders = _GetNumEncoders();

  // Check if we already have enough
  if (numEncoders <= numActiveEncoders) {
    return;
  }

  if (_enableParallelEncoder) {
    // Do we need to create a parallel encoder
    if (!_parallelEncoder) {
      _parallelEncoder = _hgi->GetPrimaryCommandBuffer(this, false)
                           ->parallelRenderCommandEncoder(_renderPassDescriptor);

      if (_debugLabel) {
        _parallelEncoder->pushDebugGroup(_debugLabel);
      }
    }
    // Create any missing encoders
    for (uint32_t i = numActiveEncoders; i < numEncoders; i++) {
      MTL::RenderCommandEncoder *encoder = _parallelEncoder->renderCommandEncoder();
      _encoders.push_back(encoder);
    }
  } else {
    if (numEncoders > 1) {
      TF_CODING_ERROR("Only 1 encoder supported");
    }
    if (numActiveEncoders >= 1) {
      return;
    }

    MTL::RenderCommandEncoder *encoder =
      _hgi->GetPrimaryCommandBuffer(this, false)->renderCommandEncoder(_renderPassDescriptor);
    if (_debugLabel) {
      encoder->pushDebugGroup(_debugLabel);
    }
    _encoders.push_back(encoder);
  }

  for (uint32_t i = numActiveEncoders; i < numEncoders; i++) {
    // Setup any relevant state for the new encoder(s)
    _SetCachedEncoderState(_encoders[i]);
  }

  if (_debugLabel) {
    _debugLabel->release();
    _debugLabel = nil;
  }
}

MTL::RenderCommandEncoder *HgiMetalGraphicsCmds::_GetEncoder(uint32_t encoderIndex)
{
  uint32_t numActiveEncoders = _GetNumEncoders();

  // Do we need to create an intial encoder
  if (!numActiveEncoders) {
    _SetNumberParallelEncoders(1);
    numActiveEncoders = _GetNumEncoders();
  }

  // Check if we have this encoder (it's OK not to have it)
  if (encoderIndex >= numActiveEncoders) {
    TF_CODING_ERROR("Invalid render encoder index specified");
    return nil;
  }

  return _encoders[encoderIndex];
}

void HgiMetalGraphicsCmds::_CreateArgumentBuffer()
{
  if (!_argumentBuffer) {
    _argumentBuffer = _hgi->GetArgBuffer();
  }
}

void HgiMetalGraphicsCmds::_SyncArgumentBuffer()
{
  if (_argumentBuffer) {
    if (_argumentBuffer->storageMode() != MTL::StorageModeShared) {
      NS::Range range = NS::Range::Make(0, _argumentBuffer->length());
      _argumentBuffer->didModifyRange(range);
    }
    _argumentBuffer = nil;
  }
}

void HgiMetalGraphicsCmds::SetViewport(GfVec4i const &vp)
{
  double x = vp[0];
  double y = vp[1];
  double w = vp[2];
  double h = vp[3];

  // Viewport is inverted in the y. Along with the front face winding order
  // being inverted.
  // This combination allows us to emulate the OpenGL coordinate space on
  // Metal
  _CachedEncState.viewport = (MTL::Viewport){x, h - y, w, -h, 0.0, 1.0};

  for (auto &encoder : _encoders) {
    encoder->setViewport(_CachedEncState.viewport);
  }

  _viewportSet = true;
}

void HgiMetalGraphicsCmds::SetScissor(GfVec4i const &sc)
{
  uint32_t x = sc[0];
  uint32_t y = sc[1];
  uint32_t w = sc[2];
  uint32_t h = sc[3];

  _CachedEncState.scissorRect = (MTL::ScissorRect){x, y, w, h};

  for (auto &encoder : _encoders) {
    encoder->setScissorRect(_CachedEncState.scissorRect);
  }

  _scissorRectSet = true;
}

void HgiMetalGraphicsCmds::BindPipeline(HgiGraphicsPipelineHandle pipeline)
{
  _primitiveTypeChanged = (_primitiveType != pipeline->GetDescriptor().primitiveType);
  _primitiveType = pipeline->GetDescriptor().primitiveType;

  _primitiveIndexSize = pipeline->GetDescriptor().tessellationState.primitiveIndexSize;
  _InitVertexBufferStepFunction(pipeline.Get());

  _CachedEncState.graphicsPipeline = static_cast<HgiMetalGraphicsPipeline *>(pipeline.Get());

  if (_CachedEncState.graphicsPipeline) {
    for (auto &encoder : _encoders) {
      _CachedEncState.graphicsPipeline->BindPipeline(encoder);
    }
  }
}

void HgiMetalGraphicsCmds::BindResources(HgiResourceBindingsHandle r)
{
  _CreateArgumentBuffer();

  _CachedEncState.resourceBindings = static_cast<HgiMetalResourceBindings *>(r.Get());
  _CachedEncState.argumentBuffer = _argumentBuffer;

  if (_CachedEncState.resourceBindings) {
    for (auto &encoder : _encoders) {
      _CachedEncState.resourceBindings->BindResources(_hgi, encoder, _argumentBuffer);
    }
  }
}

void HgiMetalGraphicsCmds::SetConstantValues(HgiGraphicsPipelineHandle pipeline,
                                             HgiShaderStage stages,
                                             uint32_t bindIndex,
                                             uint32_t byteSize,
                                             const void *data)
{
  _CreateArgumentBuffer();

  HgiMetalResourceBindings::SetConstantValues(_argumentBuffer, stages, bindIndex, byteSize, data);
}

void HgiMetalGraphicsCmds::BindVertexBuffers(uint32_t firstBinding,
                                             HgiBufferHandleVector const &vertexBuffers,
                                             std::vector<uint32_t> const &byteOffsets)
{
  TF_VERIFY(byteOffsets.size() == vertexBuffers.size());

  for (size_t i = 0; i < vertexBuffers.size(); i++) {
    HgiBufferHandle bufHandle = vertexBuffers[i];
    HgiMetalBuffer *buf = static_cast<HgiMetalBuffer *>(bufHandle.Get());
    HgiBufferDesc const &desc = buf->GetDescriptor();

    uint32_t const byteOffset = byteOffsets[i];
    uint32_t const bindingIndex = firstBinding + (uint32_t)i;

    TF_VERIFY(desc.usage & HgiBufferUsageVertex);

    for (auto &encoder : _encoders) {
      encoder->setVertexBuffer(buf->GetBufferId(), byteOffset, bindingIndex);
    }
    _BindVertexBufferStepFunction(byteOffset, bindingIndex);

    _CachedEncState.AddVertexBinding(bindingIndex, buf->GetBufferId(), byteOffset);
  }
}

void HgiMetalGraphicsCmds::_InitVertexBufferStepFunction(HgiGraphicsPipeline const *pipeline)
{
  HgiGraphicsPipelineDesc const &descriptor = pipeline->GetDescriptor();

  _vertexBufferStepFunctionDescs.clear();
  _patchBaseVertexBufferStepFunctionDescs.clear();

  for (size_t index = 0; index < descriptor.vertexBuffers.size(); index++) {
    auto const &vbo = descriptor.vertexBuffers[index];
    if (vbo.vertexStepFunction == HgiVertexBufferStepFunctionPerDrawCommand) {
      _vertexBufferStepFunctionDescs.emplace_back(index, 0, vbo.vertexStride);
    } else if (vbo.vertexStepFunction == HgiVertexBufferStepFunctionPerPatchControlPoint) {
      _patchBaseVertexBufferStepFunctionDescs.emplace_back(index, 0, vbo.vertexStride);
    }
  }
}

void HgiMetalGraphicsCmds::_BindVertexBufferStepFunction(uint32_t byteOffset,
                                                         uint32_t bindingIndex)
{
  for (auto &stepFunction : _vertexBufferStepFunctionDescs) {
    if (stepFunction.bindingIndex == bindingIndex) {
      stepFunction.byteOffset = byteOffset;
    }
  }
  for (auto &stepFunction : _patchBaseVertexBufferStepFunctionDescs) {
    if (stepFunction.bindingIndex == bindingIndex) {
      stepFunction.byteOffset = byteOffset;
    }
  }
}

void HgiMetalGraphicsCmds::_SetVertexBufferStepFunctionOffsets(MTL::RenderCommandEncoder *encoder,
                                                               uint32_t baseInstance)
{
  for (auto const &stepFunction : _vertexBufferStepFunctionDescs) {
    uint32_t const offset = stepFunction.vertexStride * baseInstance + stepFunction.byteOffset;

    encoder->setVertexBufferOffset(offset, stepFunction.bindingIndex);
  }
}

void HgiMetalGraphicsCmds::_SetPatchBaseVertexBufferStepFunctionOffsets(
  MTL::RenderCommandEncoder *encoder,
  uint32_t baseVertex)
{
  for (auto const &stepFunction : _patchBaseVertexBufferStepFunctionDescs) {
    uint32_t const offset = stepFunction.vertexStride * baseVertex + stepFunction.byteOffset;

    encoder->setVertexBufferOffset(offset, stepFunction.bindingIndex);
  }
}

void HgiMetalGraphicsCmds::Draw(uint32_t vertexCount,
                                uint32_t baseVertex,
                                uint32_t instanceCount,
                                uint32_t baseInstance)
{
  _SyncArgumentBuffer();

  MTL::PrimitiveType type = HgiMetalConversions::GetPrimitiveType(_primitiveType);
  MTL::RenderCommandEncoder *encoder = _GetEncoder();

  _SetVertexBufferStepFunctionOffsets(encoder, baseInstance);

  if (_primitiveType == HgiPrimitiveTypePatchList) {
    const NS::UInteger controlPointCount = _primitiveIndexSize;
    encoder->drawPatches(controlPointCount,
                         0,
                         vertexCount / controlPointCount,
                         NULL,
                         0,
                         instanceCount,
                         baseInstance);
  } else {
    if (instanceCount == 1) {
      encoder->drawPrimitives(type, baseVertex, vertexCount);
    } else {
      encoder->drawPrimitives(type, baseVertex, vertexCount, instanceCount, baseInstance);
    }
  }

  _hasWork = true;
}

void HgiMetalGraphicsCmds::DrawIndirect(HgiBufferHandle const &drawParameterBuffer,
                                        uint32_t drawBufferByteOffset,
                                        uint32_t drawCount,
                                        uint32_t stride)
{
  _SyncArgumentBuffer();

  HgiMetalBuffer *drawBuf = static_cast<HgiMetalBuffer *>(drawParameterBuffer.Get());

  MTL::PrimitiveType type = HgiMetalConversions::GetPrimitiveType(_primitiveType);

  static const uint32_t _drawCallsPerThread = 256;
  const uint32_t numEncoders = std::min(std::max(drawCount / _drawCallsPerThread, 1U),
                                        _maxNumEncoders);
  const uint32_t normalCount = drawCount / numEncoders;
  const uint32_t finalCount = normalCount + (drawCount - normalCount * numEncoders);

  _SetNumberParallelEncoders(numEncoders);

  WorkWithScopedParallelism([&]() {
    WorkDispatcher wd;

    for (uint32_t i = 0; i < numEncoders; ++i) {
      const uint32_t encoderOffset = normalCount * i;
      // If this is the last encoder then ensure that we have all prims.
      const uint32_t encoderCount = (i == numEncoders - 1) ? finalCount : normalCount;
      wd.Run([&, i, encoderOffset, encoderCount]() {
        MTL::RenderCommandEncoder *encoder = _GetEncoder(i);

        if (_primitiveType == HgiPrimitiveTypePatchList) {
          const NS::UInteger controlPointCount = _primitiveIndexSize;
          for (uint32_t offset = encoderOffset; offset < encoderOffset + encoderCount; ++offset) {
            _SetVertexBufferStepFunctionOffsets(encoder, offset);
            const uint32_t bufferOffset = drawBufferByteOffset + (offset * stride);
            encoder->drawPatches(controlPointCount, NULL, 0, drawBuf->GetBufferId(), bufferOffset);
          }
        } else {
          if (_primitiveTypeChanged) {
            _VegaIndirectFix(_hgi, encoder, type);
          }
          for (uint32_t offset = encoderOffset; offset < encoderOffset + encoderCount; ++offset) {
            _SetVertexBufferStepFunctionOffsets(encoder, offset);
            const uint32_t bufferOffset = drawBufferByteOffset + (offset * stride);

            encoder->drawPrimitives(type, drawBuf->GetBufferId(), bufferOffset);
          }
        }
      });
    }
  });
}

void HgiMetalGraphicsCmds::DrawIndexed(HgiBufferHandle const &indexBuffer,
                                       uint32_t indexCount,
                                       uint32_t indexBufferByteOffset,
                                       uint32_t baseVertex,
                                       uint32_t instanceCount,
                                       uint32_t baseInstance)
{
  _SyncArgumentBuffer();

  HgiMetalBuffer *indexBuf = static_cast<HgiMetalBuffer *>(indexBuffer.Get());

  MTL::PrimitiveType type = HgiMetalConversions::GetPrimitiveType(_primitiveType);

  MTL::RenderCommandEncoder *encoder = _GetEncoder();

  _SetVertexBufferStepFunctionOffsets(encoder, baseInstance);

  if (_primitiveType == HgiPrimitiveTypePatchList) {
    const NS::UInteger controlPointCount = _primitiveIndexSize;

    _SetPatchBaseVertexBufferStepFunctionOffsets(encoder, baseVertex);

    encoder->drawIndexedPatches(controlPointCount,
                                indexBufferByteOffset / sizeof(uint32_t),
                                indexCount,
                                nil,
                                0,
                                indexBuf->GetBufferId(),
                                0,
                                instanceCount,
                                baseInstance);
  } else {
    encoder->drawIndexedPrimitives(type,
                                   indexCount,
                                   MTL::IndexTypeUInt32,
                                   indexBuf->GetBufferId(),
                                   indexBufferByteOffset,
                                   instanceCount,
                                   baseVertex,
                                   baseInstance);
  }

  _hasWork = true;
}

void HgiMetalGraphicsCmds::DrawIndexedIndirect(
  HgiBufferHandle const &indexBuffer,
  HgiBufferHandle const &drawParameterBuffer,
  uint32_t drawBufferByteOffset,
  uint32_t drawCount,
  uint32_t stride,
  std::vector<uint32_t> const &drawParameterBufferUInt32,
  uint32_t patchBaseVertexByteOffset)
{
  _SyncArgumentBuffer();

  MTL::Buffer *indexBufferId = static_cast<HgiMetalBuffer *>(indexBuffer.Get())->GetBufferId();
  MTL::Buffer *drawBufferId =
    static_cast<HgiMetalBuffer *>(drawParameterBuffer.Get())->GetBufferId();

  MTL::PrimitiveType type = HgiMetalConversions::GetPrimitiveType(_primitiveType);

  static const uint32_t _drawCallsPerThread = 256;
  const uint32_t numEncoders = std::min(std::max(drawCount / _drawCallsPerThread, 1U),
                                        _maxNumEncoders);
  const uint32_t normalCount = drawCount / numEncoders;
  const uint32_t finalCount = normalCount + (drawCount - normalCount * numEncoders);

  _SetNumberParallelEncoders(numEncoders);

  WorkWithScopedParallelism([&]() {
    WorkDispatcher wd;

    for (uint32_t i = 0; i < numEncoders; ++i) {
      const uint32_t encoderOffset = normalCount * i;
      // If this is the last encoder then ensure that we have all prims.
      const uint32_t encoderCount = (i == numEncoders - 1) ? finalCount : normalCount;
      wd.Run([&, i, encoderOffset, encoderCount]() {
        MTL::RenderCommandEncoder *encoder = _GetEncoder(i);

        if (_primitiveType == HgiPrimitiveTypePatchList) {
          const NS::UInteger controlPointCount = _primitiveIndexSize;

          for (uint32_t offset = encoderOffset; offset < encoderOffset + encoderCount; ++offset) {
            _SetVertexBufferStepFunctionOffsets(encoder, offset);

            const uint32_t baseVertexIndex = (patchBaseVertexByteOffset + offset * stride) /
                                             sizeof(uint32_t);
            const uint32_t baseVertex = drawParameterBufferUInt32[baseVertexIndex];

            _SetPatchBaseVertexBufferStepFunctionOffsets(encoder, baseVertex);

            const uint32_t bufferOffset = drawBufferByteOffset + (offset * stride);
            encoder->drawIndexedPatches(controlPointCount,
                                        nil,
                                        0,
                                        indexBufferId,
                                        0,
                                        drawBufferId,
                                        bufferOffset);
          }
        } else {
          if (_primitiveTypeChanged) {
            _VegaIndirectFix(_hgi, encoder, type);
          }
          for (uint32_t offset = encoderOffset; offset < encoderOffset + encoderCount; ++offset) {
            _SetVertexBufferStepFunctionOffsets(encoder, offset);

            const uint32_t bufferOffset = drawBufferByteOffset + (offset * stride);

            encoder->drawIndexedPrimitives(type,
                                           MTL::IndexTypeUInt32,
                                           indexBufferId,
                                           0,
                                           drawBufferId,
                                           bufferOffset);
          }
        }
      });
    }
  });
}

void HgiMetalGraphicsCmds::PushDebugGroup(const char *label)
{
  if (!HgiMetalDebugEnabled()) {
    return;
  }
  if (_parallelEncoder) {
    HGIMETAL_DEBUG_PUSH_GROUP(_parallelEncoder, label)
  } else if (!_encoders.empty()) {
    HGIMETAL_DEBUG_PUSH_GROUP(_GetEncoder(), label)
  } else {
    _debugLabel = NS::String::string(label, NS::UTF8StringEncoding)->copy();
  }
}

void HgiMetalGraphicsCmds::PopDebugGroup()
{
  if (_parallelEncoder) {
    HGIMETAL_DEBUG_POP_GROUP(_parallelEncoder)
  } else if (!_encoders.empty()) {
    HGIMETAL_DEBUG_POP_GROUP(_GetEncoder());
  }
  if (_debugLabel) {
    _debugLabel->release();
    _debugLabel = nil;
  }
}

void HgiMetalGraphicsCmds::MemoryBarrier(HgiMemoryBarrier barrier)
{
  TF_VERIFY(barrier == HgiMemoryBarrierAll, "Unknown barrier");

  // Apple Silicon only support memory barriers between vertex stages after
  // macOS 12.3.
#if defined(__MAC_12_3) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_3
  MTL::BarrierScope scope = MTL::BarrierScopeBuffers;
  MTL::RenderStages srcStages = MTL::RenderStageVertex;
  MTL::RenderStages dstStages = MTL::RenderStageVertex;

  for (auto &encoder : _encoders) {
    encoder->memoryBarrier(scope, srcStages, dstStages);
  }
#endif /* defined(__MAC_12_3) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_3 */
}

static HgiMetal::CommitCommandBufferWaitType _ToHgiMetal(const HgiSubmitWaitType wait)
{
  switch (wait) {
    case HgiSubmitWaitTypeNoWait:
      return HgiMetal::CommitCommandBuffer_NoWait;
    case HgiSubmitWaitTypeWaitUntilCompleted:
      return HgiMetal::CommitCommandBuffer_WaitUntilCompleted;
  }

  TF_CODING_ERROR("Bad enum value for HgiSubmitWaitType");
  return HgiMetal::CommitCommandBuffer_WaitUntilCompleted;
}

bool HgiMetalGraphicsCmds::_Submit(Hgi *hgi, HgiSubmitWaitType wait)
{
  if (_parallelEncoder) {
    WorkParallelForEach(_encoders.begin(), _encoders.end(), [](auto &encoder) {
      encoder->endEncoding();
    });
    _parallelEncoder->endEncoding();
    _parallelEncoder = nil;

    _hgi->CommitPrimaryCommandBuffer(_ToHgiMetal(wait));
  } else if (!_encoders.empty()) {
    for (auto &encoder : _encoders) {
      encoder->endEncoding();
    }

    _hgi->CommitPrimaryCommandBuffer(_ToHgiMetal(wait));
  }

  std::lock_guard<std::mutex> lock(_encoderLock);

  _argumentBuffer = nil;
  _encoders.clear();
  _CachedEncState.ResetCachedEncoderState();

  return _hasWork;
}

WABI_NAMESPACE_END
