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
#ifndef WABI_IMAGING_HDX_PRESENT_TASK_H
#define WABI_IMAGING_HDX_PRESENT_TASK_H

#include "wabi/imaging/hdx/api.h"
#include "wabi/imaging/hdx/task.h"
#include "wabi/imaging/hgi/tokens.h"
#include "wabi/imaging/hgi/types.h"
#include "wabi/imaging/hgiInterop/hgiInterop.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class HdxPresentTaskParams
///
/// PresentTask parameters.
///
struct HdxPresentTaskParams {
  HdxPresentTaskParams() : dstApi(HgiTokens->OpenGL), dstRegion(0), enabled(true)
  {}

  // The graphics lib that is used by the application / viewer.
  // (The 'interopSrc' is determined by checking Hgi->GetAPIName)
  TfToken dstApi;

  /// The framebuffer that the AOVs are presented into. This is a
  /// VtValue that encoding a framebuffer in a dstApi specific
  /// way.
  ///
  /// E.g., a uint32_t (aka GLuint) for framebuffer object for dstApi==OpenGL.
  /// For backwards compatibility, the currently bound framebuffer is used
  /// when the VtValue is empty.
  VtValue dstFramebuffer;

  // Subrectangular region of the framebuffer over which to composite aov
  // contents. Coordinates are (left, BOTTOM, width, height).
  GfVec4i dstRegion;

  // When not enabled, present task does not execute, but still calls
  // Hgi::EndFrame.
  bool enabled;
};

/// \class HdxPresentTask
///
/// A task for taking the final result of the aovs and compositing it over the
/// currently bound framebuffer.
/// This task uses the 'color' and optionally 'depth' aov's in the task
/// context. The 'color' aov is expected to use non-integer
/// (i.e., float or norm) types to keep the interop step simple.
///
class HdxPresentTask : public HdxTask {
 public:
  // Returns true if the format is supported for presentation. This is useful
  // for upstream tasks to prepare the AOV data accordingly, and keeps the
  // interop step simple.
  HDX_API
  static bool IsFormatSupported(HgiFormat aovFormat);

  HDX_API
  HdxPresentTask(HdSceneDelegate *delegate, SdfPath const &id);

  HDX_API
  ~HdxPresentTask() override;

  HDX_API
  void Prepare(HdTaskContext *ctx, HdRenderIndex *renderIndex) override;

  HDX_API
  void Execute(HdTaskContext *ctx) override;

 protected:
  HDX_API
  void _Sync(HdSceneDelegate *delegate, HdTaskContext *ctx, HdDirtyBits *dirtyBits) override;

 private:
  HdxPresentTaskParams _params;
  HgiInterop _interop;

  HdxPresentTask()                       = delete;
  HdxPresentTask(const HdxPresentTask &) = delete;
  HdxPresentTask &operator=(const HdxPresentTask &) = delete;
};

// VtValue requirements
HDX_API
std::ostream &operator<<(std::ostream &out, const HdxPresentTaskParams &pv);
HDX_API
bool operator==(const HdxPresentTaskParams &lhs, const HdxPresentTaskParams &rhs);
HDX_API
bool operator!=(const HdxPresentTaskParams &lhs, const HdxPresentTaskParams &rhs);

WABI_NAMESPACE_END

#endif
