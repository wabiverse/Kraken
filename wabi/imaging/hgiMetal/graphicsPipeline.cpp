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

#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/graphicsPipeline.h"
#include "wabi/imaging/hgiMetal/resourceBindings.h"
#include "wabi/imaging/hgiMetal/shaderProgram.h"
#include "wabi/imaging/hgiMetal/shaderFunction.h"

#include "wabi/base/gf/half.h"

#include "wabi/base/tf/diagnostic.h"

WABI_NAMESPACE_BEGIN

HgiMetalGraphicsPipeline::HgiMetalGraphicsPipeline(HgiMetal *hgi,
                                                   HgiGraphicsPipelineDesc const &desc)
  : HgiGraphicsPipeline(desc),
    _vertexDescriptor(nil),
    _depthStencilState(nil),
    _renderPipelineState(nil),
    _constantTessFactors(nil)
{
  _CreateVertexDescriptor();
  _CreateDepthStencilState(hgi->GetPrimaryDevice());
  _CreateRenderPipelineState(hgi->GetPrimaryDevice());
}

HgiMetalGraphicsPipeline::~HgiMetalGraphicsPipeline()
{
  if (_renderPipelineState) {
    _renderPipelineState->release();
  }
  if (_depthStencilState) {
    _depthStencilState->release();
  }
  if (_vertexDescriptor) {
    _vertexDescriptor->release();
  }
  if (_constantTessFactors) {
    _constantTessFactors->release();
  }
}

void HgiMetalGraphicsPipeline::_CreateVertexDescriptor()
{
  _vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  int index = 0;
  for (HgiVertexBufferDesc const &vbo : _descriptor.vertexBuffers) {

    HgiVertexAttributeDescVector const &vas = vbo.vertexAttributes;
    _vertexDescriptor->layouts()->object(index)->setStride(vbo.vertexStride);

    // Set the vertex step rate such that the attribute index
    // will advance only according to the base instance at the
    // start of each draw command of a multi-draw. To do this
    // we set the vertex attribute to be constant and advance
    // the vertex buffer offset appropriately when encoding
    // draw commands.
    if (vbo.vertexStepFunction == HgiVertexBufferStepFunctionConstant ||
        vbo.vertexStepFunction == HgiVertexBufferStepFunctionPerDrawCommand) {
      _vertexDescriptor->layouts()->object(index)->setStepFunction(
        MTL::VertexStepFunctionConstant);
      _vertexDescriptor->layouts()->object(index)->setStepRate(0);
    } else if (vbo.vertexStepFunction == HgiVertexBufferStepFunctionPerPatchControlPoint) {
      _vertexDescriptor->layouts()->object(index)->setStepFunction(
        MTL::VertexStepFunctionPerPatchControlPoint);
      _vertexDescriptor->layouts()->object(index)->setStepRate(1);
    } else {
      _vertexDescriptor->layouts()->object(index)->setStepFunction(
        MTL::VertexStepFunctionPerVertex);
      _vertexDescriptor->layouts()->object(index)->setStepRate(1);
    }

    // Describe each vertex attribute in the vertex buffer
    for (size_t loc = 0; loc < vas.size(); loc++) {
      HgiVertexAttributeDesc const &va = vas[loc];

      uint32_t idx = va.shaderBindLocation;
      _vertexDescriptor->attributes()->object(idx)->setFormat(
        HgiMetalConversions::GetVertexFormat(va.format));
      _vertexDescriptor->attributes()->object(idx)->setBufferIndex(vbo.bindingIndex);
      _vertexDescriptor->attributes()->object(idx)->setOffset(va.offset);
    }

    index++;
  }
}

void HgiMetalGraphicsPipeline::_CreateRenderPipelineState(MTL::Device *device)
{
  MTL::RenderPipelineDescriptor *stateDesc = MTL::RenderPipelineDescriptor::alloc()->init();

  // Create a new render pipeline state object
  HGIMETAL_DEBUG_LABEL(stateDesc, _descriptor.debugName.c_str());
  stateDesc->setRasterSampleCount(_descriptor.multiSampleState.sampleCount);

  stateDesc->setInputPrimitiveTopology(
    HgiMetalConversions::GetPrimitiveClass(_descriptor.primitiveType));

  HgiMetalShaderProgram const *metalProgram = static_cast<HgiMetalShaderProgram *>(
    _descriptor.shaderProgram.Get());

  if (_descriptor.primitiveType == HgiPrimitiveTypePatchList) {
    stateDesc->setVertexFunction(metalProgram->GetPostTessVertexFunction());

    MTL::Winding winding = HgiMetalConversions::GetWinding(_descriptor.rasterizationState.winding);
    // flip the tess winding
    winding = winding == MTL::WindingClockwise ? MTL::WindingCounterClockwise :
                                                 MTL::WindingClockwise;
    stateDesc->setTessellationOutputWindingOrder(winding);

    stateDesc->setTessellationControlPointIndexType(MTL::TessellationControlPointIndexTypeUInt32);
  } else {
    stateDesc->setVertexFunction(metalProgram->GetVertexFunction());
  }

  MTL::Function *fragFunction = metalProgram->GetFragmentFunction();
  if (fragFunction && _descriptor.rasterizationState.rasterizerEnabled) {
    stateDesc->setFragmentFunction(fragFunction);
    stateDesc->setRasterizationEnabled(YES);
  } else {
    stateDesc->setRasterizationEnabled(NO);
  }

  // Color attachments
  for (size_t i = 0; i < _descriptor.colorAttachmentDescs.size(); i++) {
    HgiAttachmentDesc const &hgiColorAttachment = _descriptor.colorAttachmentDescs[i];
    MTL::RenderPipelineColorAttachmentDescriptor *metalColorAttachment =
      stateDesc->colorAttachments()->object(i);

    metalColorAttachment->setPixelFormat(
      HgiMetalConversions::GetPixelFormat(hgiColorAttachment.format, hgiColorAttachment.usage));

    metalColorAttachment->setWriteMask(
      HgiMetalConversions::GetColorWriteMask(hgiColorAttachment.colorMask));

    if (hgiColorAttachment.blendEnabled) {
      metalColorAttachment->setBlendingEnabled(YES);

      metalColorAttachment->setSourceRGBBlendFactor(
        HgiMetalConversions::GetBlendFactor(hgiColorAttachment.srcColorBlendFactor));
      metalColorAttachment->setDestinationRGBBlendFactor(
        HgiMetalConversions::GetBlendFactor(hgiColorAttachment.dstColorBlendFactor));

      metalColorAttachment->setSourceAlphaBlendFactor(
        HgiMetalConversions::GetBlendFactor(hgiColorAttachment.srcAlphaBlendFactor));
      metalColorAttachment->setDestinationAlphaBlendFactor(
        HgiMetalConversions::GetBlendFactor(hgiColorAttachment.dstAlphaBlendFactor));

      metalColorAttachment->setRgbBlendOperation(
        HgiMetalConversions::GetBlendEquation(hgiColorAttachment.colorBlendOp));
      metalColorAttachment->setAlphaBlendOperation(
        HgiMetalConversions::GetBlendEquation(hgiColorAttachment.alphaBlendOp));
    } else {
      metalColorAttachment->setBlendingEnabled(NO);
    }
  }

  HgiAttachmentDesc const &hgiDepthAttachment = _descriptor.depthAttachmentDesc;

  MTL::PixelFormat depthPixelFormat = HgiMetalConversions::GetPixelFormat(
    hgiDepthAttachment.format,
    hgiDepthAttachment.usage);

  stateDesc->setDepthAttachmentPixelFormat(depthPixelFormat);

  if (_descriptor.depthAttachmentDesc.usage & HgiTextureUsageBitsStencilTarget) {
    stateDesc->setStencilAttachmentPixelFormat(depthPixelFormat);
  }

  stateDesc->setSampleCount(_descriptor.multiSampleState.sampleCount);
  if (_descriptor.multiSampleState.alphaToCoverageEnable) {
    stateDesc->setAlphaToCoverageEnabled(YES);
  } else {
    stateDesc->setAlphaToCoverageEnabled(NO);
  }
  if (_descriptor.multiSampleState.alphaToOneEnable) {
    stateDesc->setAlphaToOneEnabled(YES);
  } else {
    stateDesc->setAlphaToOneEnabled(NO);
  }

  stateDesc->setVertexDescriptor(_vertexDescriptor);

  NS::Error *error = NULL;
  _renderPipelineState = device->newRenderPipelineState(stateDesc, &error);
  stateDesc->release();

  if (!_renderPipelineState) {
    NS::String *err = error->localizedDescription();
    TF_WARN("Failed to created pipeline state, error %s", err->utf8String());
  }
}

static MTL::StencilDescriptor *_CreateStencilDescriptor(HgiStencilState const &stencilState)
{
  MTL::StencilDescriptor *stencilDescriptor = MTL::StencilDescriptor::alloc()->init();

  stencilDescriptor->setStencilCompareFunction(
    HgiMetalConversions::GetCompareFunction(stencilState.compareFn));
  stencilDescriptor->setStencilFailureOperation(
    HgiMetalConversions::GetStencilOp(stencilState.stencilFailOp));
  stencilDescriptor->setDepthFailureOperation(
    HgiMetalConversions::GetStencilOp(stencilState.depthFailOp));
  stencilDescriptor->setDepthStencilPassOperation(
    HgiMetalConversions::GetStencilOp(stencilState.depthStencilPassOp));
  stencilDescriptor->setReadMask(stencilState.readMask);
  stencilDescriptor->setWriteMask(stencilState.writeMask);

  return stencilDescriptor;
}

void HgiMetalGraphicsPipeline::_CreateDepthStencilState(MTL::Device *device)
{
  MTL::DepthStencilDescriptor *depthStencilStateDescriptor =
    MTL::DepthStencilDescriptor::alloc()->init();

  HGIMETAL_DEBUG_LABEL(depthStencilStateDescriptor, _descriptor.debugName.c_str());

  if (_descriptor.depthState.depthTestEnabled) {
    MTL::CompareFunction depthFn = HgiMetalConversions::GetCompareFunction(
      _descriptor.depthState.depthCompareFn);
    depthStencilStateDescriptor->setDepthCompareFunction(depthFn);
    if (_descriptor.depthState.depthWriteEnabled) {
      depthStencilStateDescriptor->setDepthWriteEnabled(YES);
    } else {
      depthStencilStateDescriptor->setDepthWriteEnabled(NO);
    }
  } else {
    // Even if there is no depth attachment, some drivers may still perform
    // the depth test. So we pick Always over Never.
    depthStencilStateDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthStencilStateDescriptor->setDepthWriteEnabled(NO);
  }

  if (_descriptor.depthState.stencilTestEnabled) {
    depthStencilStateDescriptor->setBackFaceStencil(
      _CreateStencilDescriptor(_descriptor.depthState.stencilFront));
    depthStencilStateDescriptor->setFrontFaceStencil(
      _CreateStencilDescriptor(_descriptor.depthState.stencilBack));
  }

  _depthStencilState = device->newDepthStencilState(depthStencilStateDescriptor);
  depthStencilStateDescriptor->release();

  TF_VERIFY(_depthStencilState, "Failed to created depth stencil state");
}

void HgiMetalGraphicsPipeline::BindPipeline(MTL::RenderCommandEncoder *renderEncoder)
{
  renderEncoder->setRenderPipelineState(_renderPipelineState);
  if (_descriptor.primitiveType == HgiPrimitiveTypePatchList) {
    if (_constantTessFactors == nullptr) {

      // tess factors are half floats encoded as uint16_t
      uint16_t const factorZero = reinterpret_cast<uint16_t>(GfHalf(0.0f).bits());
      uint16_t const factorOne = reinterpret_cast<uint16_t>(GfHalf(1.0f).bits());

      if (_descriptor.tessellationState.patchType == HgiTessellationState::PatchType::Triangle) {
        MTL::TriangleTessellationFactorsHalf triangleFactors;
        triangleFactors.insideTessellationFactor = factorZero;
        triangleFactors.edgeTessellationFactor[0] = factorOne;
        triangleFactors.edgeTessellationFactor[1] = factorOne;
        triangleFactors.edgeTessellationFactor[2] = factorOne;
        _constantTessFactors = renderEncoder->device()->newBuffer(&triangleFactors,
                                                                  sizeof(triangleFactors),
                                                                  MTL::ResourceStorageModeShared);
      } else {  // is Quad tess factor
        MTL::QuadTessellationFactorsHalf quadFactors;
        quadFactors.insideTessellationFactor[0] = factorZero;
        quadFactors.insideTessellationFactor[1] = factorZero;
        quadFactors.edgeTessellationFactor[0] = factorOne;
        quadFactors.edgeTessellationFactor[1] = factorOne;
        quadFactors.edgeTessellationFactor[2] = factorOne;
        quadFactors.edgeTessellationFactor[3] = factorOne;
        _constantTessFactors = renderEncoder->device()->newBuffer(&quadFactors,
                                                                  sizeof(quadFactors),
                                                                  MTL::ResourceStorageModeShared);
      }
    }
    renderEncoder->setTessellationFactorBuffer(_constantTessFactors, 0, 0);
  }

  //
  // DepthStencil state
  //
  HgiDepthStencilState const &dsState = _descriptor.depthState;
  if (_descriptor.depthState.depthBiasEnabled) {
    renderEncoder->setDepthBias(dsState.depthBiasConstantFactor,
                                dsState.depthBiasSlopeFactor,
                                0.0f);
  }

  if (_descriptor.depthState.stencilTestEnabled) {
    renderEncoder->setStencilFrontReferenceValue(dsState.stencilFront.referenceValue,
                                                 dsState.stencilBack.referenceValue);
  }

  //
  // Rasterization state
  //
  renderEncoder->setCullMode(
    HgiMetalConversions::GetCullMode(_descriptor.rasterizationState.cullMode));
  renderEncoder->setTriangleFillMode(
    HgiMetalConversions::GetPolygonMode(_descriptor.rasterizationState.polygonMode));
  renderEncoder->setFrontFacingWinding(
    HgiMetalConversions::GetWinding(_descriptor.rasterizationState.winding));
  renderEncoder->setDepthStencilState(_depthStencilState);

  if (_descriptor.rasterizationState.depthClampEnabled) {
    renderEncoder->setDepthClipMode(MTL::DepthClipModeClamp);
  }

  TF_VERIFY(_descriptor.rasterizationState.lineWidth == 1.0f, "Missing implementation buffers");
}

WABI_NAMESPACE_END