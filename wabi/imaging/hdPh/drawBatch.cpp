//
// Copyright 2016 Pixar
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
#include "wabi/imaging/hdPh/drawBatch.h"
#include "wabi/imaging/hdPh/codeGen.h"
#include "wabi/imaging/hdPh/commandBuffer.h"
#include "wabi/imaging/hdPh/debugCodes.h"
#include "wabi/imaging/hdPh/geometricShader.h"
#include "wabi/imaging/hdPh/glslProgram.h"
#include "wabi/imaging/hdPh/glslfxShader.h"
#include "wabi/imaging/hdPh/lightingShader.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/renderPassShader.h"
#include "wabi/imaging/hdPh/renderPassState.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/surfaceShader.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/imaging/hio/glslfx.h"

#include "wabi/base/tf/getenv.h"

#include <mutex>

WABI_NAMESPACE_BEGIN

HdPh_DrawBatch::HdPh_DrawBatch(HdPhDrawItemInstance *drawItemInstance) : _shaderHash(0) {}

/*virtual*/
void HdPh_DrawBatch::_Init(HdPhDrawItemInstance *drawItemInstance)
{
  _drawItemInstances.push_back(drawItemInstance);

  // Force shader to refresh.
  // XXX: Why is this necessary? If the draw item state changes in a
  // significant way such that the shader needs to be recompiled, that value
  // should be part of the shader hash and this shouldn't be required.
  _shaderHash = 0;
}

HdPh_DrawBatch::~HdPh_DrawBatch() {}

void HdPh_DrawBatch::DrawItemInstanceChanged(HdPhDrawItemInstance const * /*instance*/) {}

void HdPh_DrawBatch::SetEnableTinyPrimCulling(bool tinyPrimCulling) {}

namespace
{
  inline bool isAggregated(HdBufferArrayRangeSharedPtr const &rangeA,
                           HdBufferArrayRangeSharedPtr const &rangeB)
  {
    if (rangeA) {
      return rangeA->IsAggregatedWith(rangeB);
    } else {
      if (!rangeB) {
        // can batch together if both ranges are empty.
        return true;
      }
    }
    return false;
  }
}  // namespace

bool HdPh_DrawBatch::Append(HdPhDrawItemInstance *drawItemInstance)
{
  if (!TF_VERIFY(!_drawItemInstances.empty())) {
    return false;
  }

  // XXX: we'll soon refactor this function out and centralize batch
  // bucketing and reordering logic in HdPhCommandBuffer.

  HdPhDrawItem const *drawItem = static_cast<const HdPhDrawItem *>(
    drawItemInstance->GetDrawItem());
  HdPhDrawItem const *batchItem = static_cast<const HdPhDrawItem *>(
    _drawItemInstances.front()->GetDrawItem());
  TF_VERIFY(batchItem);

  if (_IsAggregated(drawItem, batchItem)) {
    drawItemInstance->SetBatchIndex(_drawItemInstances.size());
    drawItemInstance->SetBatch(this);
    _drawItemInstances.push_back(drawItemInstance);
    return true;
  } else {
    return false;
  }
}

/*static*/
bool HdPh_DrawBatch::_IsAggregated(HdPhDrawItem const *drawItem0, HdPhDrawItem const *drawItem1)
{
  if (!HdPhSurfaceShader::CanAggregate(drawItem0->GetMaterialShader(),
                                       drawItem1->GetMaterialShader())) {
    return false;
  }

  if (drawItem0->GetGeometricShader() == drawItem1->GetGeometricShader() &&
      drawItem0->GetInstancePrimvarNumLevels() == drawItem1->GetInstancePrimvarNumLevels() &&
      isAggregated(drawItem0->GetTopologyRange(), drawItem1->GetTopologyRange()) &&
      isAggregated(drawItem0->GetTopologyVisibilityRange(),
                   drawItem1->GetTopologyVisibilityRange()) &&
      isAggregated(drawItem0->GetVertexPrimvarRange(), drawItem1->GetVertexPrimvarRange()) &&
      isAggregated(drawItem0->GetVaryingPrimvarRange(), drawItem1->GetVaryingPrimvarRange()) &&
      isAggregated(drawItem0->GetElementPrimvarRange(), drawItem1->GetElementPrimvarRange()) &&
      isAggregated(drawItem0->GetFaceVaryingPrimvarRange(),
                   drawItem1->GetFaceVaryingPrimvarRange()) &&
      isAggregated(drawItem0->GetConstantPrimvarRange(), drawItem1->GetConstantPrimvarRange()) &&
      isAggregated(drawItem0->GetInstanceIndexRange(), drawItem1->GetInstanceIndexRange())) {
    int numLevels = drawItem0->GetInstancePrimvarNumLevels();
    for (int i = 0; i < numLevels; ++i) {
      if (!isAggregated(drawItem0->GetInstancePrimvarRange(i),
                        drawItem1->GetInstancePrimvarRange(i))) {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool HdPh_DrawBatch::Rebuild()
{
  std::vector<HdPhDrawItemInstance const *> instances;
  instances.swap(_drawItemInstances);
  _drawItemInstances.reserve(instances.size());

  // Ensure all batch state initialized from items/instances is refreshed.
  HdPhDrawItemInstance *batchItem = const_cast<HdPhDrawItemInstance *>(instances.front());
  if (!TF_VERIFY(batchItem->GetDrawItem()->GetGeometricShader())) {
    return false;
  }
  _Init(batchItem);
  if (!TF_VERIFY(!_drawItemInstances.empty())) {
    return false;
  }

  // Start this loop at i=1 because the 0th element was pushed via _Init
  for (size_t i = 1; i < instances.size(); ++i) {
    HdPhDrawItemInstance *item = const_cast<HdPhDrawItemInstance *>(instances[i]);
    if (!TF_VERIFY(item->GetDrawItem()->GetGeometricShader())) {
      return false;
    }
    if (!Append(item)) {
      TF_DEBUG(HDPH_DRAW_BATCH).Msg("   Rebuild failed for batch %p\n", (void *)(this));
      return false;
    }
  }

  TF_DEBUG(HDPH_DRAW_BATCH).Msg("   Rebuild success for batch %p\n", (void *)(this));

  return true;
}

static HdPhSurfaceShaderSharedPtr _GetFallbackSurfaceShader()
{
  static std::once_flag once;
  static HdPhSurfaceShaderSharedPtr fallbackSurfaceShader;

  std::call_once(once, []() {
    HioGlslfxSharedPtr glslfx = std::make_shared<HioGlslfx>(HdPhPackageFallbackSurfaceShader());

    fallbackSurfaceShader.reset(new HdPhGLSLFXShader(glslfx));
  });

  return fallbackSurfaceShader;
}

HdPh_DrawBatch::_DrawingProgram &HdPh_DrawBatch::_GetDrawingProgram(
  HdPhRenderPassStateSharedPtr const &state,
  bool indirect,
  HdPhResourceRegistrySharedPtr const &resourceRegistry)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPhDrawItem const *firstDrawItem = _drawItemInstances[0]->GetDrawItem();

  // Calculate unique hash to detect if the shader (composed) has changed
  // recently and we need to recompile it.
  size_t shaderHash = state->GetShaderHash();
  boost::hash_combine(shaderHash, firstDrawItem->GetGeometricShader()->ComputeHash());
  HdPhShaderCodeSharedPtr surfaceShader = state->GetUseSceneMaterials() ?
                                            firstDrawItem->GetMaterialShader() :
                                            _GetFallbackSurfaceShader();
  size_t surfaceHash = surfaceShader ? surfaceShader->ComputeHash() : 0;
  boost::hash_combine(shaderHash, surfaceHash);
  bool shaderChanged = (_shaderHash != shaderHash);

  // Set shaders (lighting and renderpass) to the program.
  // We need to do this before checking if the shaderChanged because
  // it is possible that the shader does not need to
  // be recompiled but some of the parameters have changed.
  HdPhShaderCodeSharedPtrVector shaders = state->GetShaders();
  _program.SetShaders(shaders);
  _program.SetGeometricShader(firstDrawItem->GetGeometricShader());

  // XXX: if this function appears to be expensive, we might consider caching
  //      programs by shaderHash.
  if (!_program.GetGLSLProgram() || shaderChanged) {

    _program.SetSurfaceShader(surfaceShader);

    // Try to compile the shader and if it fails to compile we go back
    // to use the specified fallback surface shader.
    if (!_program.CompileShader(firstDrawItem, indirect, resourceRegistry)) {

      // While the code should gracefully handle shader compilation
      // failures, it is also undesirable for shaders to silently fail.
      TF_CODING_ERROR("Failed to compile shader for prim %s.",
                      firstDrawItem->GetRprimID().GetText());

      // If we failed to compile the surface shader, replace it with the
      // fallback surface shader and try again.
      // XXX: Note that we only say "surface shader" here because it is
      // currently the only one that we allow customization for.  We
      // expect all the other shaders to compile or else the shipping
      // code is broken and needs to be fixed.  When we open up more
      // shaders for customization, we will need to check them as well.

      _program.SetSurfaceShader(_GetFallbackSurfaceShader());

      bool res = _program.CompileShader(firstDrawItem, indirect, resourceRegistry);
      // We expect the fallback shader to always compile.
      TF_VERIFY(res);
    }

    _shaderHash = shaderHash;
  }

  return _program;
}

bool HdPh_DrawBatch::_DrawingProgram::CompileShader(
  HdPhDrawItem const *drawItem,
  bool indirect,
  HdPhResourceRegistrySharedPtr const &resourceRegistry)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!_geometricShader) {
    TF_CODING_ERROR("Can not compile a shader without a geometric shader");
    return false;
  }

  // determine binding points and populate metaData
  HdBindingRequestVector customBindings;
  bool instanceDraw = true;
  _GetCustomBindings(&customBindings, &instanceDraw);

  // also (surface, renderPass) shaders use their bindings
  HdPhShaderCodeSharedPtrVector shaders = GetComposedShaders();

  TF_FOR_ALL (it, shaders) {
    (*it)->AddBindings(&customBindings);
  }

  HdPh_CodeGen codeGen(_geometricShader, shaders, drawItem->GetMaterialTag());

  // let resourcebinder resolve bindings and populate metadata
  // which is owned by codegen.
  _resourceBinder.ResolveBindings(drawItem,
                                  shaders,
                                  codeGen.GetMetaData(),
                                  indirect,
                                  instanceDraw,
                                  customBindings);

  HdPhGLSLProgram::ID hash = codeGen.ComputeHash();

  {
    // ask registry to see if there's already compiled program
    HdInstance<HdPhGLSLProgramSharedPtr> programInstance = resourceRegistry->RegisterGLSLProgram(
      hash);

    if (programInstance.IsFirstInstance()) {
      HdPhGLSLProgramSharedPtr glslProgram = codeGen.Compile(resourceRegistry.get());
      if (glslProgram && _Link(glslProgram)) {
        // store the program into the program registry.
        programInstance.SetValue(glslProgram);
      }
    }

    _glslProgram = programInstance.GetValue();

    if (_glslProgram) {
      _resourceBinder.IntrospectBindings(_glslProgram->GetProgram());
    } else {
      // Failed to compile and link a valid glsl program.
      return false;
    }
  }
  return true;
}

/* virtual */
void HdPh_DrawBatch::_DrawingProgram::_GetCustomBindings(HdBindingRequestVector *customBindings,
                                                         bool *enableInstanceDraw) const
{
  if (!TF_VERIFY(enableInstanceDraw))
    return;

  // set enableInstanceDraw true by default, which means the shader is
  // expected to be invoked by instanced-draw call.
  // XFB culling is an exception, which uses glDrawArrays.
  *enableInstanceDraw = true;
}

/* virtual */
bool HdPh_DrawBatch::_DrawingProgram::_Link(HdPhGLSLProgramSharedPtr const &glslProgram)
{
  if (!TF_VERIFY(glslProgram))
    return false;

  return glslProgram->Link();
}

WABI_NAMESPACE_END
