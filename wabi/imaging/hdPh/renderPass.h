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
#ifndef WABI_IMAGING_HD_PH_RENDER_PASS_H
#define WABI_IMAGING_HD_PH_RENDER_PASS_H

#include "wabi/imaging/hd/renderPass.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/commandBuffer.h"
#include "wabi/wabi.h"

#include <unordered_map>

WABI_NAMESPACE_BEGIN

class Hgi;

/// \class HdPh_RenderPass
///
/// A single draw pass to a render target/buffer. Stream implementation.
///
class HdPh_RenderPass : public HdRenderPass {
 public:
  HDPH_API
  HdPh_RenderPass(HdRenderIndex *index, HdRprimCollection const &collection);
  HDPH_API
  virtual ~HdPh_RenderPass();

  /// Returns the number of draw items used by this render pass.
  /// Will only return the correct value after Prepare() has been called on
  /// HdRenderPass. Calling this during Sync() will return last frame's
  /// drawItem count.
  HDPH_API
  size_t GetDrawItemCount() const;

 protected:
  virtual void _Prepare(TfTokenVector const &renderTags) override;

  /// Execute the buckets corresponding to renderTags
  virtual void _Execute(HdRenderPassStateSharedPtr const &renderPassState,
                        TfTokenVector const &renderTags) override;

  virtual void _MarkCollectionDirty() override;

 private:
  void _PrepareDrawItems(TfTokenVector const &renderTags);
  void _PrepareCommandBuffer(TfTokenVector const &renderTags);

  // XXX: This should really be in HdPh_DrawBatch::PrepareDraw.
  void _FrustumCullCPU(HdPhRenderPassStateSharedPtr const &renderPasssState);

  // -----------------------------------------------------------------------
  // Drawing state
  HdPhCommandBuffer _cmdBuffer;

  int _lastSettingsVersion;
  bool _useTinyPrimCulling;

  // -----------------------------------------------------------------------
  // Change tracking state.

  // The version number of the currently held collection.
  int _collectionVersion;

  // The version number of the currently active render tags
  int _renderTagVersion;

  // The version number of the material tags (of the rprims).
  unsigned int _materialTagsVersion;

  // A flag indicating that the held collection changed since this renderPass
  // was last drawn.
  //
  // When _collectionChanged is true, it indicates that _collectionVersion is
  // no longer accurate, because _collectionVersion was stored for the
  // previously held collection.
  bool _collectionChanged;

  // DrawItems that are used to build the draw batches.
  HdRenderIndex::HdDrawItemPtrVector _drawItems;
  size_t _drawItemCount;
  bool _drawItemsChanged;

  Hgi *_hgi;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_RENDER_PASS_H
