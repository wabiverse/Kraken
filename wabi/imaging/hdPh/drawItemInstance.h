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
#ifndef WABI_IMAGING_HD_ST_DRAW_ITEM_INSTANCE_H
#define WABI_IMAGING_HD_ST_DRAW_ITEM_INSTANCE_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/drawItem.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

using HdPh_DrawBatchSharedPtr = std::shared_ptr<class HdPh_DrawBatch>;

/// \class HdPhDrawItemInstance
///
/// A container to store instance state for a drawitem.
///
/// During culling, the visibility state will be set. If the instance
/// has a batch, the batch will get a DrawItemInstanceChanged
/// callback.
///
/// The Batch is responsible for calling SetBatch and SetBatchIndex
/// when adding / appending the instance. If the batch does not require
/// the DrawItemInstanceChanged callback, then this step can be skipped
///
class HdPhDrawItemInstance
{
 public:

  HDPH_API
  HdPhDrawItemInstance(HdPhDrawItem const *drawItem);
  HDPH_API
  ~HdPhDrawItemInstance();

  /// Set visibility state
  HDPH_API
  void SetVisible(bool visible);

  /// Query visibility state
  bool IsVisible() const
  {
    return _visible;
  }

  /// Set index into batch list. Can be used by
  /// batch during DrawItemInstanceChanged callback
  HDPH_API
  void SetBatchIndex(size_t batchIndex);

  /// Query batch index
  size_t GetBatchIndex() const
  {
    return _batchIndex;
  }

  /// Set the batch that will receive the DrawItemInstanceChanged
  /// callback when visibility is updated. Setting batch to NULL
  /// will disable this callback.
  // HDPH_API
  void SetBatch(HdPh_DrawBatch *batch);

  /// Return a const pointer to draw item
  HdPhDrawItem const *GetDrawItem() const
  {
    return _drawItem;
  }

 private:

  HdPhDrawItemInstance();

  HdPh_DrawBatch *_batch;
  HdPhDrawItem const *_drawItem;
  size_t _batchIndex;
  bool _visible;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_DRAW_ITEM_INSTANCE_H
