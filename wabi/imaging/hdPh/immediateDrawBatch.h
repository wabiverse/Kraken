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
#ifndef WABI_IMAGING_HD_ST_IMMEDIATE_DRAW_BATCH_H
#define WABI_IMAGING_HD_ST_IMMEDIATE_DRAW_BATCH_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/drawBatch.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

/// \class HdPh_ImmediateDrawBatch
///
/// Drawing batch that is executed immediately.
///
class HdPh_ImmediateDrawBatch : public HdPh_DrawBatch {
 public:
  HDPH_API
  HdPh_ImmediateDrawBatch(HdPhDrawItemInstance *drawItemInstance);
  HDPH_API
  ~HdPh_ImmediateDrawBatch() override;

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

 protected:
  HDPH_API
  void _Init(HdPhDrawItemInstance *drawItemInstance) override;

 private:
  size_t _bufferArraysHash;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_IMMEDIATE_DRAW_BATCH_H
