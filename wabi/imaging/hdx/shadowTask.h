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
#ifndef WABI_IMAGING_HDX_SHADOW_TASK_H
#define WABI_IMAGING_HDX_SHADOW_TASK_H

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/task.h"
#include "wabi/imaging/hdx/api.h"
#include "wabi/imaging/hdx/version.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/vec4d.h"
#include "wabi/base/gf/vec4f.h"
#include "wabi/base/tf/declarePtrs.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class GlfSimpleLight;
class HdRenderIndex;
class HdSceneDelegate;
class HdPhRenderPassState;

using HdPhRenderPassShaderSharedPtr = std::shared_ptr<class HdPhRenderPassShader>;
using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;

using HdRenderPassSharedPtr = std::shared_ptr<class HdRenderPass>;
using HdRenderPassSharedPtrVector = std::vector<HdRenderPassSharedPtr>;
using HdPhRenderPassStateSharedPtr = std::shared_ptr<class HdPhRenderPassState>;
using HdPhRenderPassStateSharedPtrVector = std::vector<HdPhRenderPassStateSharedPtr>;

TF_DECLARE_WEAK_AND_REF_PTRS(GlfSimpleShadowArray);

struct HdxShadowTaskParams
{
  HdxShadowTaskParams()
    : overrideColor(0.0),
      wireframeColor(0.0),
      enableLighting(false),
      enableIdRender(false),
      enableSceneMaterials(true),
      alphaThreshold(0.0),
      depthBiasEnable(false),
      depthBiasConstantFactor(0.0f),
      depthBiasSlopeFactor(1.0f),
      depthFunc(HdCmpFuncLEqual),
      cullStyle(HdCullStyleBackUnlessDoubleSided)
  {}

  // RenderPassState
  GfVec4f overrideColor;
  GfVec4f wireframeColor;
  bool enableLighting;
  bool enableIdRender;
  bool enableSceneMaterials;
  float alphaThreshold;
  bool depthBiasEnable;
  float depthBiasConstantFactor;
  float depthBiasSlopeFactor;
  HdCompareFunction depthFunc;
  HdCullStyle cullStyle;
};

/// \class HdxShadowTask
///
/// A task for generating shadow maps.
///
class HdxShadowTask : public HdTask
{
 public:

  HDX_API
  HdxShadowTask(HdSceneDelegate *delegate, SdfPath const &id);

  HDX_API
  ~HdxShadowTask() override;

  /// Sync the render pass resources
  HDX_API
  void Sync(HdSceneDelegate *delegate, HdTaskContext *ctx, HdDirtyBits *dirtyBits) override;

  /// Prepare the tasks resources
  HDX_API
  void Prepare(HdTaskContext *ctx, HdRenderIndex *renderIndex) override;

  /// Execute render pass task
  HDX_API
  void Execute(HdTaskContext *ctx) override;

  /// Collect Render Tags used by the task.
  HDX_API
  const TfTokenVector &GetRenderTags() const override;

 private:

  void _UpdateDirtyParams(HdPhRenderPassStateSharedPtr &renderPassState,
                          HdxShadowTaskParams const &params);

  HdRenderPassSharedPtrVector _passes;
  HdPhRenderPassStateSharedPtrVector _renderPassStates;
  HdxShadowTaskParams _params;
  TfTokenVector _renderTags;

  HdxShadowTask() = delete;
  HdxShadowTask(const HdxShadowTask &) = delete;
  HdxShadowTask &operator=(const HdxShadowTask &) = delete;
};

// VtValue requirements
HDX_API
std::ostream &operator<<(std::ostream &out, const HdxShadowTaskParams &pv);
HDX_API
bool operator==(const HdxShadowTaskParams &lhs, const HdxShadowTaskParams &rhs);
HDX_API
bool operator!=(const HdxShadowTaskParams &lhs, const HdxShadowTaskParams &rhs);

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HDX_SHADOW_TASK_H
