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
#ifndef WABI_IMAGING_HD_ST_RENDER_PASS_STATE_H
#define WABI_IMAGING_HD_ST_RENDER_PASS_STATE_H

#include "wabi/imaging/hd/renderPassState.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hgi/graphicsCmdsDesc.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdResourceRegistrySharedPtr = std::shared_ptr<class HdResourceRegistry>;
using HdPhRenderPassStateSharedPtr = std::shared_ptr<class HdPhRenderPassState>;

using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;

using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;
using HdPhLightingShaderSharedPtr = std::shared_ptr<class HdPhLightingShader>;
using HdPhRenderPassShaderSharedPtr = std::shared_ptr<class HdPhRenderPassShader>;
using HdPh_FallbackLightingShaderSharedPtr = std::shared_ptr<class HdPh_FallbackLightingShader>;
using HdPhShaderCodeSharedPtrVector = std::vector<HdPhShaderCodeSharedPtr>;
class HdRenderIndex;

/// \class HdPhRenderPassState
///
/// A set of rendering parameters used among render passes.
///
/// Parameters are expressed as GL states, uniforms or shaders.
///
class HdPhRenderPassState : public HdRenderPassState {
 public:
  HDPH_API
  HdPhRenderPassState();
  HDPH_API
  HdPhRenderPassState(HdPhRenderPassShaderSharedPtr const &shader);
  HDPH_API
  ~HdPhRenderPassState() override;

  HDPH_API
  void Prepare(HdResourceRegistrySharedPtr const &resourceRegistry) override;

  /// XXX: Bind and Unbind set./restore the following GL state.
  /// This will be reworked to use Hgi in the near future.
  /// Following states may be changed and restored to
  /// the GL default at Unbind().
  ///   glEnable(GL_BLEND);
  ///   glEnable(GL_CULL_FACE);
  ///   glEnable(GL_POLYGON_OFFSET_FILL)
  ///   glEnable(GL_PROGRAM_POINT_SIZE);
  ///   glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE)
  ///   glEnable(GL_DEPTH_TEST);
  ///   glEnable(GL_STENCIL_TEST);
  ///   glPolygonOffset()
  ///   glBlend*()
  ///   glColorMask()
  ///   glCullFace()
  ///   glDepthFunc()
  ///   glDepthMask()
  ///   glLineWidth()
  ///   glStencilFunc()
  ///   glStencilOp()
  HDPH_API
  void Bind();
  HDPH_API
  void Unbind();

  /// If set to true (default) and the render pass is rendering into a
  /// multi-sampled aovs, the aovs will be resolved at the end of the render
  /// pass. If false or the aov is not multi-sampled or the render pass is not
  /// rendering into the multi-sampled aov, no resolution takes place.
  HD_API
  void SetResolveAovMultiSample(bool state);
  HD_API
  bool GetResolveAovMultiSample() const;

  /// Set lighting shader
  HDPH_API
  void SetLightingShader(HdPhLightingShaderSharedPtr const &lightingShader);
  HdPhLightingShaderSharedPtr const &GetLightingShader() const
  {
    return _lightingShader;
  }

  /// renderpass shader
  HDPH_API
  void SetRenderPassShader(HdPhRenderPassShaderSharedPtr const &renderPassShader);
  HdPhRenderPassShaderSharedPtr const &GetRenderPassShader() const
  {
    return _renderPassShader;
  }

  /// scene materials
  HDPH_API
  void SetUseSceneMaterials(bool state);
  bool GetUseSceneMaterials() const
  {
    return _useSceneMaterials;
  }

  /// returns shaders (lighting/renderpass)
  HDPH_API
  HdPhShaderCodeSharedPtrVector GetShaders() const;

  HDPH_API
  size_t GetShaderHash() const;

  /// Camera setter API
  /// Option 1: Specify matrices, viewport and clipping planes (defined in
  /// camera space) directly.
  HD_API
  void SetCameraFramingState(GfMatrix4d const &worldToViewMatrix,
                             GfMatrix4d const &projectionMatrix,
                             GfVec4d const &viewport,
                             ClipPlanesVector const &clipPlanes);

  // Helper to get graphics cmds descriptor describing textures
  // we render into and the blend state, constructed from
  // AOV bindings.
  //
  HDPH_API
  HgiGraphicsCmdsDesc MakeGraphicsCmdsDesc(const HdRenderIndex *) const;

 private:
  bool _UseAlphaMask() const;

  // ---------------------------------------------------------------------- //
  // Shader Objects
  // ---------------------------------------------------------------------- //
  HdPhRenderPassShaderSharedPtr _renderPassShader;
  HdPh_FallbackLightingShaderSharedPtr _fallbackLightingShader;
  HdPhLightingShaderSharedPtr _lightingShader;

  HdBufferArrayRangeSharedPtr _renderPassStateBar;
  size_t _clipPlanesBufferSize;
  float _alphaThresholdCurrent;
  bool _resolveMultiSampleAov;
  bool _useSceneMaterials;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_RENDER_PASS_STATE_H
