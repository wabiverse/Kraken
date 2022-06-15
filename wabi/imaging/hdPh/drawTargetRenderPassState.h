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
#ifndef WABI_IMAGING_HD_ST_DRAW_TARGET_RENDER_PASS_STATE_H
#define WABI_IMAGING_HD_ST_DRAW_TARGET_RENDER_PASS_STATE_H

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/rprimCollection.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class VtValue;
using HdRenderPassAovBindingVector = std::vector<struct HdRenderPassAovBinding>;

/// \class HdPhDrawTargetRenderPassState
///
/// Represents common non-gl context specific render pass state for a draw
/// target.
///
/// \note This is a temporary API to aid transition to Phoenix, and is subject
/// to major changes.  It is likely this functionality will be absorbed into
/// the base class.
///
class HdPhDrawTargetRenderPassState final
{
 public:

  HDPH_API
  HdPhDrawTargetRenderPassState();
  HDPH_API
  ~HdPhDrawTargetRenderPassState();  // final no need to be virtual

  const HdRenderPassAovBindingVector &GetAovBindings() const
  {
    return _aovBindings;
  }

  HDPH_API
  void SetAovBindings(const HdRenderPassAovBindingVector &aovBindings);

  /// Sets the priority of values in the depth buffer.
  /// i.e. should pixels closer or further from the camera win.
  HDPH_API
  void SetDepthPriority(HdDepthPriority priority);

  /// Set the path to the camera to use to draw this render path from.
  HDPH_API
  void SetCamera(const SdfPath &cameraId);

  HDPH_API
  void SetRprimCollection(HdRprimCollection const &col);

  HdDepthPriority GetDepthPriority() const
  {
    return _depthPriority;
  }

  /// Returns the path to the camera to render from.
  const SdfPath &GetCamera() const
  {
    return _cameraId;
  }

  /// Returns an increasing version number for when the collection object
  /// is changed.
  /// Note: This tracks the actual object and not the contents of the
  /// collection.
  unsigned int GetRprimCollectionVersion() const
  {
    return _rprimCollectionVersion;
  }

  /// Returns the collection associated with this draw target.
  const HdRprimCollection &GetRprimCollection() const
  {
    return _rprimCollection;
  }

 private:

  HdRenderPassAovBindingVector _aovBindings;
  HdDepthPriority _depthPriority;

  SdfPath _cameraId;

  HdRprimCollection _rprimCollection;
  unsigned int _rprimCollectionVersion;

  HdPhDrawTargetRenderPassState(const HdPhDrawTargetRenderPassState &) = delete;
  HdPhDrawTargetRenderPassState &operator=(const HdPhDrawTargetRenderPassState &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_DRAW_TARGET_RENDER_PASS_STATE_H
