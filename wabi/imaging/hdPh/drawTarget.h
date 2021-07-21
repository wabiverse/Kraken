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
#ifndef WABI_IMAGING_HD_ST_DRAW_TARGET_H
#define WABI_IMAGING_HD_ST_DRAW_TARGET_H

#include "wabi/imaging/hd/rprimCollection.h"
#include "wabi/imaging/hd/sprim.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/drawTargetRenderPassState.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/staticTokens.h"
#include "wabi/usd/sdf/path.h"

#include <vector>

WABI_NAMESPACE_BEGIN

#define HDPH_DRAW_TARGET_TOKENS \
  (camera)(collection)(drawTargetSet)(enable)(resolution)(aovBindings)(depthPriority)

TF_DECLARE_PUBLIC_TOKENS(HdPhDrawTargetTokens, HDPH_API, HDPH_DRAW_TARGET_TOKENS);

class HdCamera;
class HdRenderIndex;
using HdPhDrawTargetPtrVector = std::vector<class HdPhDrawTarget *>;

/// \class HdPhDrawTarget
///
/// Represents an render to texture render pass.
///
/// \note This is a temporary API to aid transition to Phoenix, and is subject
/// to major changes.
///
class HdPhDrawTarget : public HdSprim
{
 public:
  HDPH_API
  HdPhDrawTarget(SdfPath const &id);
  HDPH_API
  ~HdPhDrawTarget() override;

  /// Dirty bits for the HdPhDrawTarget object
  ///
  /// When GetUsePhoenixTextureSystem() is true, "Legacy" dirty
  /// bits are ignored.
  ///
  enum DirtyBits : HdDirtyBits
  {
    Clean = 0,
    DirtyDTEnable = 1 << 0,
    DirtyDTCamera = 1 << 1,
    DirtyDTResolution = 1 << 2,
    DirtyDTAovBindings = 1 << 4,
    DirtyDTDepthPriority = 1 << 6,
    DirtyDTCollection = 1 << 7,
    AllDirty = (DirtyDTEnable | DirtyDTCamera | DirtyDTResolution | DirtyDTAovBindings |
                DirtyDTDepthPriority | DirtyDTCollection)
  };

  /// Synchronizes state from the delegate to this object.
  HDPH_API
  void Sync(HdSceneDelegate *sceneDelegate, HdRenderParam *renderParam, HdDirtyBits *dirtyBits) override;

  /// Returns the minimal set of dirty bits to place in the
  /// change tracker for use in the first sync of this prim.
  /// Typically this would be all dirty bits.
  HDPH_API
  HdDirtyBits GetInitialDirtyBitsMask() const override;

  // ---------------------------------------------------------------------- //
  /// \name Draw Target API
  // ---------------------------------------------------------------------- //
  bool IsEnabled() const
  {
    return _enabled;
  }
  const HdPhDrawTargetRenderPassState *GetDrawTargetRenderPassState() const
  {
    return &_drawTargetRenderPassState;
  }

  /// Returns collection of rprims the draw target draws.
  HDPH_API
  HdRprimCollection const &GetCollection() const
  {
    return _collection;
  }

  /// returns all HdPhDrawTargets in the render index
  HDPH_API
  static void GetDrawTargets(HdRenderIndex *renderIndex, HdPhDrawTargetPtrVector *drawTargets);

  /// Resolution.
  ///
  /// Set during sync.
  ///
  const GfVec2i &GetResolution() const
  {
    return _resolution;
  }

 private:
  bool _enabled;
  GfVec2i _resolution;
  HdRprimCollection _collection;

  HdPhDrawTargetRenderPassState _drawTargetRenderPassState;

  // No copy
  HdPhDrawTarget() = delete;
  HdPhDrawTarget(const HdPhDrawTarget &) = delete;
  HdPhDrawTarget &operator=(const HdPhDrawTarget &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_DRAW_TARGET_H
