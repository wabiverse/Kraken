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

#include "wabi/base/tf/diagnostic.h"

#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/graphicsPipeline.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/resourceBindings.h"
#include "wabi/imaging/hgiMetal/shaderFunction.h"
#include "wabi/imaging/hgiMetal/shaderProgram.h"

WABI_NAMESPACE_BEGIN

HgiMetalGraphicsPipeline::HgiMetalGraphicsPipeline(HgiMetal *hgi, HgiGraphicsPipelineDesc const &desc)
  : HgiGraphicsPipeline(desc),
    _vertexDescriptor(nil),
    _depthStencilState(nil),
    _renderPipelineState(nil)
{
  _CreateVertexDescriptor();
  _CreateDepthStencilState(hgi->GetPrimaryDevice());
  _CreateRenderPipelineState(hgi->GetPrimaryDevice());
}

HgiMetalGraphicsPipeline::~HgiMetalGraphicsPipeline()
{
  if (_renderPipelineState)
  {
    [_renderPipelineState release];
  }
  if (_depthStencilState)
  {
    [_depthStencilState release];
  }
  if (_vertexDescriptor)
  {
    [_vertexDescriptor release];
  }
}

void HgiMetalGraphicsPipeline::_CreateVertexDescriptor()
{
  _vertexDescriptor = [[MTLVertexDescriptor alloc] init];

  int index = 0;
  for (HgiVertexBufferDesc const &vbo : _descriptor.vertexBuffers)
  {

    HgiVertexAttributeDescVector const &vas = vbo.vertexAttributes;

    _vertexDescriptor.layouts[index].stepFunction = MTLVertexStepFunctionPerVertex;
    _vertexDescriptor.layouts[index].stepRate = 1;
    _vertexDescriptor.layouts[index].stride = vbo.vertexStride;

    // Describe each vertex attribute in the vertex buffer
    for (size_t loc = 0; loc < vas.size(); loc++)
    {
      HgiVertexAttributeDesc const &va = vas[loc];

      uint32_t idx = va.shaderBindLocation;
      _vertexDescriptor.attributes[idx].format = HgiMetalConversions::GetVertexFormat(va.format);
      _vertexDescriptor.attributes[idx].bufferIndex = vbo.bindingIndex;
      _vertexDescriptor.attributes[idx].offset = va.offset;
    }
    index++;
  }
}

void HgiMetalGraphicsPipeline::_CreateRenderPipelineState(id<MTLDevice> device)
{
  MTLRenderPipelineDescriptor *stateDesc = [[MTLRenderPipelineDescriptor alloc] init];

  // Create a new render pipeline state object
  HGIMETAL_DEBUG_LABEL(stateDesc, _descriptor.debugName.c_str());
  stateDesc.rasterSampleCount = 1;

  stateDesc.inputPrimitiveTopology = HgiMetalConversions::GetPrimitiveClass(_descriptor.primitiveType);

  HgiMetalShaderProgram const *metalProgram = static_cast<HgiMetalShaderProgram *>(
    _descriptor.shaderProgram.Get());

  stateDesc.vertexFunction = metalProgram->GetVertexFunction();
  id<MTLFunction> fragFunction = metalProgram->GetFragmentFunction();
  if (fragFunction && _descriptor.rasterizationState.rasterizerEnabled)
  {
    stateDesc.fragmentFunction = fragFunction;
    stateDesc.rasterizationEnabled = YES;
  }
  else
  {
    stateDesc.rasterizationEnabled = NO;
  }

  // Color attachments
  for (size_t i = 0; i < _descriptor.colorAttachmentDescs.size(); i++)
  {
    HgiAttachmentDesc const &hgiColorAttachment = _descriptor.colorAttachmentDescs[i];
    MTLRenderPipelineColorAttachmentDescriptor *metalColorAttachment = stateDesc.colorAttachments[i];

    metalColorAttachment.pixelFormat = HgiMetalConversions::GetPixelFormat(hgiColorAttachment.format);

    if (hgiColorAttachment.blendEnabled)
    {
      metalColorAttachment.blendingEnabled = YES;

      metalColorAttachment.sourceRGBBlendFactor = HgiMetalConversions::GetBlendFactor(
        hgiColorAttachment.srcColorBlendFactor);
      metalColorAttachment.destinationRGBBlendFactor = HgiMetalConversions::GetBlendFactor(
        hgiColorAttachment.dstColorBlendFactor);

      metalColorAttachment.sourceAlphaBlendFactor = HgiMetalConversions::GetBlendFactor(
        hgiColorAttachment.srcAlphaBlendFactor);
      metalColorAttachment.destinationAlphaBlendFactor = HgiMetalConversions::GetBlendFactor(
        hgiColorAttachment.dstAlphaBlendFactor);

      metalColorAttachment.rgbBlendOperation = HgiMetalConversions::GetBlendEquation(
        hgiColorAttachment.colorBlendOp);
      metalColorAttachment.alphaBlendOperation = HgiMetalConversions::GetBlendEquation(
        hgiColorAttachment.alphaBlendOp);
    }
    else
    {
      metalColorAttachment.blendingEnabled = NO;
    }
  }

  HgiAttachmentDesc const &hgiDepthAttachment = _descriptor.depthAttachmentDesc;

  stateDesc.depthAttachmentPixelFormat = HgiMetalConversions::GetPixelFormat(hgiDepthAttachment.format);

  stateDesc.sampleCount = _descriptor.multiSampleState.sampleCount;
  if (_descriptor.multiSampleState.alphaToCoverageEnable)
  {
    stateDesc.alphaToCoverageEnabled = YES;
  }
  else
  {
    stateDesc.alphaToCoverageEnabled = NO;
  }

  stateDesc.vertexDescriptor = _vertexDescriptor;

  NSError *error = NULL;
  _renderPipelineState = [device newRenderPipelineStateWithDescriptor:stateDesc error:&error];
  [stateDesc release];

  if (!_renderPipelineState)
  {
    NSString *err = [error localizedDescription];
    TF_WARN("Failed to created pipeline state, error %s", [err UTF8String]);
  }
}

void HgiMetalGraphicsPipeline::_CreateDepthStencilState(id<MTLDevice> device)
{
  MTLDepthStencilDescriptor *depthStencilStateDescriptor = [[MTLDepthStencilDescriptor alloc] init];

  HGIMETAL_DEBUG_LABEL(depthStencilStateDescriptor, _descriptor.debugName.c_str());

  if (_descriptor.depthState.depthWriteEnabled)
  {
    depthStencilStateDescriptor.depthWriteEnabled = YES;
  }
  else
  {
    depthStencilStateDescriptor.depthWriteEnabled = NO;
  }
  if (_descriptor.depthState.depthTestEnabled)
  {
    MTLCompareFunction depthFn = HgiMetalConversions::GetDepthCompareFunction(
      _descriptor.depthState.depthCompareFn);
    depthStencilStateDescriptor.depthCompareFunction = depthFn;
  }
  else
  {
    // Even if there is no depth attachment, some drivers may still perform
    // the depth test. So we pick Always over Never.
    depthStencilStateDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
  }

  if (_descriptor.depthState.stencilTestEnabled)
  {
    TF_CODING_ERROR("Missing implementation stencil mask enabled");
  }
  else
  {
    depthStencilStateDescriptor.backFaceStencil = nil;
    depthStencilStateDescriptor.frontFaceStencil = nil;
  }

  _depthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilStateDescriptor];
  [depthStencilStateDescriptor release];

  TF_VERIFY(_depthStencilState, "Failed to created depth stencil state");
}

void HgiMetalGraphicsPipeline::BindPipeline(id<MTLRenderCommandEncoder> renderEncoder)
{
  [renderEncoder setRenderPipelineState:_renderPipelineState];

  //
  // Rasterization state
  //
  [renderEncoder setCullMode:HgiMetalConversions::GetCullMode(_descriptor.rasterizationState.cullMode)];
  [renderEncoder
    setTriangleFillMode:HgiMetalConversions::GetPolygonMode(_descriptor.rasterizationState.polygonMode)];
  [renderEncoder
    setFrontFacingWinding:HgiMetalConversions::GetWinding(_descriptor.rasterizationState.winding)];
  [renderEncoder setDepthStencilState:_depthStencilState];

  TF_VERIFY(_descriptor.rasterizationState.lineWidth == 1.0f, "Missing implementation buffers");
}

WABI_NAMESPACE_END
