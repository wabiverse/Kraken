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
#ifndef WABI_IMAGING_HGI_METAL_PIPELINE_H
#define WABI_IMAGING_HGI_METAL_PIPELINE_H

#include "wabi/wabi.h"
#include "wabi/imaging/hgi/graphicsCmdsDesc.h"
#include "wabi/imaging/hgi/graphicsPipeline.h"

#include "wabi/imaging/hgiMetal/api.h"

#include <vector>


WABI_NAMESPACE_BEGIN

class HgiMetal;

/// \class HgiMetalPipeline
///
/// Metal implementation of HgiGraphicsPipeline.
///
class HgiMetalGraphicsPipeline final : public HgiGraphicsPipeline
{
 public:

  HGIMETAL_API
  HgiMetalGraphicsPipeline(HgiMetal *hgi, HgiGraphicsPipelineDesc const &desc);

  HGIMETAL_API
  ~HgiMetalGraphicsPipeline() override;

  /// Apply pipeline state
  HGIMETAL_API
  void BindPipeline(MTL::RenderCommandEncoder *renderEncoder);

 private:

  HgiMetalGraphicsPipeline() = delete;
  HgiMetalGraphicsPipeline &operator=(const HgiMetalGraphicsPipeline &) = delete;
  HgiMetalGraphicsPipeline(const HgiMetalGraphicsPipeline &) = delete;

  void _CreateVertexDescriptor();
  void _CreateDepthStencilState(MTL::Device *device);
  void _CreateRenderPipelineState(MTL::Device *device);

  MTL::VertexDescriptor *_vertexDescriptor;
  MTL::DepthStencilState *_depthStencilState;
  MTL::RenderPipelineState *_renderPipelineState;
  MTL::Buffer *_constantTessFactors;
};


WABI_NAMESPACE_END

#endif
