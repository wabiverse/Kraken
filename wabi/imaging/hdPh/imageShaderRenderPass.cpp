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

#include "wabi/imaging/hdPh/imageShaderRenderPass.h"
#include "wabi/imaging/hd/drawingCoord.h"
#include "wabi/imaging/hd/vtBufferSource.h"
#include "wabi/imaging/hdPh/geometricShader.h"
#include "wabi/imaging/hdPh/glUtils.h"
#include "wabi/imaging/hdPh/glslfxShader.h"
#include "wabi/imaging/hdPh/imageShaderShaderKey.h"
#include "wabi/imaging/hdPh/immediateDrawBatch.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/renderDelegate.h"
#include "wabi/imaging/hdPh/renderPassShader.h"
#include "wabi/imaging/hdPh/renderPassState.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/surfaceShader.h"
#include "wabi/imaging/hgi/graphicsCmds.h"
#include "wabi/imaging/hgi/graphicsCmdsDesc.h"
#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"

// XXX We do not want to include specific HgiXX backends, but we need to do
// this temporarily until Phoenix has transitioned fully to Hgi.
#include "wabi/imaging/hgiGL/graphicsCmds.h"

WABI_NAMESPACE_BEGIN

void _ExecuteDraw(HdPh_DrawBatchSharedPtr const &drawBatch,
                  HdPhRenderPassStateSharedPtr const &stRenderPassState,
                  HdPhResourceRegistrySharedPtr const &resourceRegistry)
{
  drawBatch->ExecuteDraw(stRenderPassState, resourceRegistry);
}

HdPh_ImageShaderRenderPass::HdPh_ImageShaderRenderPass(HdRenderIndex *index,
                                                       HdRprimCollection const &collection)
    : HdRenderPass(index, collection),
      _sharedData(1),
      _drawItem(&_sharedData),
      _drawItemInstance(&_drawItem),
      _hgi(nullptr)
{
  _sharedData.instancerLevels = 0;
  _sharedData.rprimID         = SdfPath("/imageShaderRenderPass");
  _immediateBatch             = std::make_shared<HdPh_ImmediateDrawBatch>(&_drawItemInstance);

  HdPhRenderDelegate *renderDelegate = static_cast<HdPhRenderDelegate *>(
      index->GetRenderDelegate());
  _hgi = renderDelegate->GetHgi();
}

HdPh_ImageShaderRenderPass::~HdPh_ImageShaderRenderPass()
{}

void HdPh_ImageShaderRenderPass::_SetupVertexPrimvarBAR(
    HdPhResourceRegistrySharedPtr const &registry)
{
  // The current logic in HdPh_ImmediateDrawBatch::ExecuteDraw will use
  // glDrawArraysInstanced if it finds a VertexPrimvar buffer but no
  // index buffer, We setup the BAR to meet this requirement to draw our
  // full-screen triangle for post-process shaders.

  HdBufferSourceSharedPtrVector sources = {
      std::make_shared<HdVtBufferSource>(HdTokens->points, VtValue(VtVec3fArray(3)))};

  HdBufferSpecVector bufferSpecs;
  HdBufferSpec::GetBufferSpecs(sources, &bufferSpecs);

  HdBufferArrayRangeSharedPtr vertexPrimvarRange = registry->AllocateNonUniformBufferArrayRange(
      HdTokens->primvar, bufferSpecs, HdBufferArrayUsageHint());

  registry->AddSources(vertexPrimvarRange, std::move(sources));

  HdDrawingCoord *drawingCoord = _drawItem.GetDrawingCoord();
  _sharedData.barContainer.Set(drawingCoord->GetVertexPrimvarIndex(), vertexPrimvarRange);
}

void HdPh_ImageShaderRenderPass::_Prepare(TfTokenVector const &renderTags)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPhResourceRegistrySharedPtr const &resourceRegistry =
      std::dynamic_pointer_cast<HdPhResourceRegistry>(GetRenderIndex()->GetResourceRegistry());
  TF_VERIFY(resourceRegistry);

  // First time we must create a VertexPrimvar BAR for the triangle and setup
  // the geometric shader that provides the vertex and fragment shaders.
  if (!_sharedData.barContainer.Get(_drawItem.GetDrawingCoord()->GetVertexPrimvarIndex())) {
    _SetupVertexPrimvarBAR(resourceRegistry);

    HdPh_ImageShaderShaderKey shaderKey;
    HdPh_GeometricShaderSharedPtr geometricShader = HdPh_GeometricShader::Create(shaderKey,
                                                                                 resourceRegistry);

    _drawItem.SetGeometricShader(geometricShader);
  }
}

void HdPh_ImageShaderRenderPass::_Execute(HdRenderPassStateSharedPtr const &renderPassState,
                                          TfTokenVector const &renderTags)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  // Downcast render pass state
  HdPhRenderPassStateSharedPtr stRenderPassState = std::dynamic_pointer_cast<HdPhRenderPassState>(
      renderPassState);
  if (!TF_VERIFY(stRenderPassState))
    return;

  HdPhResourceRegistrySharedPtr const &resourceRegistry =
      std::dynamic_pointer_cast<HdPhResourceRegistry>(GetRenderIndex()->GetResourceRegistry());
  TF_VERIFY(resourceRegistry);

  _immediateBatch->PrepareDraw(stRenderPassState, resourceRegistry);

  // Create graphics work to render into aovs.
  const HgiGraphicsCmdsDesc desc   = stRenderPassState->MakeGraphicsCmdsDesc(GetRenderIndex());
  HgiGraphicsCmdsUniquePtr gfxCmds = _hgi->CreateGraphicsCmds(desc);

  // XXX When there are no aovBindings we get a null work object.
  // This would ideally never happen, but currently happens for some
  // custom prims that spawn an imagingGLengine  with a task controller that
  // has no aovBindings.

  if (gfxCmds) {
    gfxCmds->PushDebugGroup(__ARCH_PRETTY_FUNCTION__);
  }

  // XXX: The Bind/Unbind calls below set/restore GL state.
  // This will be reworked to use Hgi.
  stRenderPassState->Bind();

  // Draw
  HdPh_DrawBatchSharedPtr const &batch = _immediateBatch;
  HgiGLGraphicsCmds *glGfxCmds         = dynamic_cast<HgiGLGraphicsCmds *>(gfxCmds.get());

  if (gfxCmds && glGfxCmds) {
    // XXX Tmp code path to allow non-hgi code to insert functions into
    // HgiGL ops-stack. Will be removed once Phoenixs uses Hgi everywhere
    auto executeDrawOp = [batch, stRenderPassState, resourceRegistry] {
      _ExecuteDraw(batch, stRenderPassState, resourceRegistry);
    };
    glGfxCmds->InsertFunctionOp(executeDrawOp);
  }
  else {
    _ExecuteDraw(batch, stRenderPassState, resourceRegistry);
  }

  if (gfxCmds) {
    gfxCmds->PopDebugGroup();
    _hgi->SubmitCmds(gfxCmds.get());
  }

  stRenderPassState->Unbind();
}

void HdPh_ImageShaderRenderPass::_MarkCollectionDirty()
{}

WABI_NAMESPACE_END
