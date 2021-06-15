//
// Copyright 2019 Pixar
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
#ifndef WABI_IMAGING_HD_ST_IMAGE_SHADER_RENDER_PASS_H
#define WABI_IMAGING_HD_ST_IMAGE_SHADER_RENDER_PASS_H

#include "wabi/imaging/hd/renderPass.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/drawItemInstance.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

using HdPh_ImageShaderRenderPassSharedPtr = std::shared_ptr<class HdPh_ImageShaderRenderPass>;
using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

class Hgi;

/// \class HdPh_ImageShaderRenderPass
///
/// A single, full-screen triangle render pass.
/// The task that creates this RenderPass should set a RenderPassShader on the
/// RenderPassState. The RenderPassShader is your full-screen post-effect.
/// The benefit of using RenderPassShader is that it participates in codegen.
/// This means your full-screen shader can use buffers created by other tasks.
///
class HdPh_ImageShaderRenderPass final : public HdRenderPass {
 public:
  HDPH_API
  HdPh_ImageShaderRenderPass(HdRenderIndex *index, HdRprimCollection const &collection);

  HDPH_API
  virtual ~HdPh_ImageShaderRenderPass();

 protected:
  virtual void _Prepare(TfTokenVector const &renderTags) override;

  virtual void _Execute(HdRenderPassStateSharedPtr const &renderPassState,
                        TfTokenVector const &renderTags) override;

  virtual void _MarkCollectionDirty() override;

 private:
  // Setup a BAR for a single triangle
  void _SetupVertexPrimvarBAR(HdPhResourceRegistrySharedPtr const &);

  // We re-use the immediateBatch to draw the full-screen triangle.
  HdRprimSharedData _sharedData;
  HdPhDrawItem _drawItem;
  HdPhDrawItemInstance _drawItemInstance;
  HdPh_DrawBatchSharedPtr _immediateBatch;
  Hgi *_hgi;
};

WABI_NAMESPACE_END

#endif
