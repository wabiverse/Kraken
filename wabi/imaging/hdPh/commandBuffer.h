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
#ifndef WABI_IMAGING_HD_ST_COMMAND_BUFFER_H
#define WABI_IMAGING_HD_ST_COMMAND_BUFFER_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/drawItemInstance.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/matrix4f.h"
#include "wabi/base/gf/vec2f.h"

#include <memory>
#include <vector>

WABI_NAMESPACE_BEGIN

class HdPhDrawItem;
class HdPhDrawItemInstance;

using HdPhRenderPassStateSharedPtr = std::shared_ptr<class HdPhRenderPassState>;
using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

using HdPh_DrawBatchSharedPtr = std::shared_ptr<class HdPh_DrawBatch>;
using HdPh_DrawBatchSharedPtrVector = std::vector<HdPh_DrawBatchSharedPtr>;

/// \class HdPhCommandBuffer
///
/// A buffer of commands (HdPhDrawItem or HdComputeItem objects) to be executed.
///
/// The HdPhCommandBuffer is responsible for accumulating draw items and sorting
/// them for correctness (e.g. alpha transparency) and efficiency (e.g. the
/// fewest number of GPU state changes).
///
class HdPhCommandBuffer {
 public:
  HDPH_API
  HdPhCommandBuffer();
  HDPH_API
  ~HdPhCommandBuffer();

  /// Prepare the command buffer for draw
  HDPH_API
  void PrepareDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                   HdPhResourceRegistrySharedPtr const &resourceRegistry);

  /// Execute the command buffer
  HDPH_API
  void ExecuteDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                   HdPhResourceRegistrySharedPtr const &resourceRegistry);

  /// Cull drawItemInstances based on passed in combined view and projection matrix
  HDPH_API
  void FrustumCull(GfMatrix4d const &cullMatrix);

  /// Sync visibility state from RprimSharedState to DrawItemInstances.
  HDPH_API
  void SyncDrawItemVisibility(unsigned visChangeCount);

  /// Destructively swaps the contents of \p items with the internal list of
  /// all draw items. Culling state is reset, with no items visible.
  HDPH_API
  void SwapDrawItems(std::vector<HdPhDrawItem const *> *items, unsigned currentBatchVersion);

  /// Rebuild all draw batches if any underlying buffer array is invalidated.
  HDPH_API
  void RebuildDrawBatchesIfNeeded(unsigned currentBatchVersion);

  /// Returns the total number of draw items, including culled items.
  size_t GetTotalSize() const
  {
    return _drawItems.size();
  }

  /// Returns the number of draw items, excluding culled items.
  size_t GetVisibleSize() const
  {
    return _visibleSize;
  }

  /// Returns the number of culled draw items.
  size_t GetCulledSize() const
  {
    return _drawItems.size() - _visibleSize;
  }

  HDPH_API
  void SetEnableTinyPrimCulling(bool tinyPrimCulling);

 private:
  void _RebuildDrawBatches();

  std::vector<HdPhDrawItem const *> _drawItems;
  std::vector<HdPhDrawItemInstance> _drawItemInstances;
  HdPh_DrawBatchSharedPtrVector _drawBatches;
  size_t _visibleSize;
  unsigned int _visChangeCount;
  unsigned int _drawBatchesVersion;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_COMMAND_BUFFER_H
