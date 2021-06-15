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
#ifndef WABI_IMAGING_HD_ST_INDIRECT_DRAW_BATCH_H
#define WABI_IMAGING_HD_ST_INDIRECT_DRAW_BATCH_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/dispatchBuffer.h"
#include "wabi/imaging/hdPh/drawBatch.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

using HdBindingRequestVector = std::vector<HdBindingRequest>;

/// \class HdPh_IndirectDrawBatch
///
/// Drawing batch that is executed from an indirect dispatch buffer.
///
/// An indirect drawing batch accepts draw items that have the same
/// primitive mode and that share aggregated drawing resources,
/// e.g. uniform and non uniform primvar buffers.
///
class HdPh_IndirectDrawBatch : public HdPh_DrawBatch {
 public:
  HDPH_API
  HdPh_IndirectDrawBatch(HdPhDrawItemInstance *drawItemInstance);
  HDPH_API
  ~HdPh_IndirectDrawBatch() override;

  // HdPh_DrawBatch overrides
  HDPH_API
  ValidationResult Validate(bool deepValidation) override;

  /// Prepare draw commands and apply view frustum culling for this batch.
  HDPH_API
  void PrepareDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                   HdPhResourceRegistrySharedPtr const &resourceRegistry) override;

  /// Executes the drawing commands for this batch.
  HDPH_API
  void ExecuteDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                   HdPhResourceRegistrySharedPtr const &resourceRegistry) override;

  HDPH_API
  void DrawItemInstanceChanged(HdPhDrawItemInstance const *instance) override;

  HDPH_API
  void SetEnableTinyPrimCulling(bool tinyPrimCulling) override;

  /// Returns whether to do frustum culling on the GPU
  HDPH_API
  static bool IsEnabledGPUFrustumCulling();

  /// Returns whether to read back the count of visible items from the GPU
  /// Disabled by default, since there is some performance penalty.
  HDPH_API
  static bool IsEnabledGPUCountVisibleInstances();

  /// Returns whether to do per-instance culling on the GPU
  HDPH_API
  static bool IsEnabledGPUInstanceFrustumCulling();

 protected:
  HDPH_API
  void _Init(HdPhDrawItemInstance *drawItemInstance) override;

 private:
  void _ValidateCompatibility(HdPhBufferArrayRangeSharedPtr const &constantBar,
                              HdPhBufferArrayRangeSharedPtr const &indexBar,
                              HdPhBufferArrayRangeSharedPtr const &topologyVisibilityBar,
                              HdPhBufferArrayRangeSharedPtr const &elementBar,
                              HdPhBufferArrayRangeSharedPtr const &fvarBar,
                              HdPhBufferArrayRangeSharedPtr const &varyingBar,
                              HdPhBufferArrayRangeSharedPtr const &vertexBar,
                              int instancerNumLevels,
                              HdPhBufferArrayRangeSharedPtr const &instanceIndexBar,
                              std::vector<HdPhBufferArrayRangeSharedPtr> const &instanceBars) const;

  // Culling requires custom resource binding.
  class _CullingProgram : public _DrawingProgram {
   public:
    _CullingProgram() : _useDrawArrays(false), _useInstanceCulling(false), _bufferArrayHash(0)
    {}
    void Initialize(bool useDrawArrays, bool useInstanceCulling, size_t bufferArrayHash);

   protected:
    // _DrawingProgram overrides
    void _GetCustomBindings(HdBindingRequestVector *customBindings, bool *enableInstanceDraw) const override;

   private:
    bool _useDrawArrays;
    bool _useInstanceCulling;
    size_t _bufferArrayHash;
  };

  _CullingProgram &_GetCullingProgram(HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _CompileBatch(HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _GPUFrustumInstanceCulling(HdPhDrawItem const *item,
                                  GfMatrix4f const &cullMatrix,
                                  GfVec2f const &drawRangeNdc,
                                  HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _GPUFrustumNonInstanceCulling(HdPhDrawItem const *item,
                                     GfMatrix4f const &cullMatrix,
                                     GfVec2f const &drawRangeNdc,
                                     HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _BeginGPUCountVisibleInstances(HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _EndGPUCountVisibleInstances(HdPhResourceRegistrySharedPtr const &resourceRegistry, size_t *result);

  HdPhDispatchBufferSharedPtr _dispatchBuffer;
  HdPhDispatchBufferSharedPtr _dispatchBufferCullInput;

  std::vector<uint32_t> _drawCommandBuffer;
  bool _drawCommandBufferDirty;
  size_t _bufferArraysHash;
  size_t _barElementOffsetsHash;

  HdPhBufferResourceSharedPtr _resultBuffer;

  size_t _numVisibleItems;
  size_t _numTotalVertices;
  size_t _numTotalElements;

  _CullingProgram _cullingProgram;
  bool _useTinyPrimCulling;
  bool _dirtyCullingProgram;

  bool _useDrawArrays;
  bool _useInstancing;
  bool _useGpuCulling;
  bool _useGpuInstanceCulling;

  int _instanceCountOffset;
  int _cullInstanceCountOffset;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_INDIRECT_DRAW_BATCH_H
