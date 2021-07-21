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
#ifndef WABI_IMAGING_HD_ST_UNIT_TEST_HELPER_H
#define WABI_IMAGING_HD_ST_UNIT_TEST_HELPER_H

#include "wabi/imaging/hdPh/lightingShader.h"
#include "wabi/imaging/hdPh/renderDelegate.h"
#include "wabi/imaging/hdPh/renderPassState.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/driver.h"
#include "wabi/imaging/hd/engine.h"
#include "wabi/imaging/hd/renderPass.h"
#include "wabi/imaging/hd/unitTestDelegate.h"
#include "wabi/imaging/hio/glslfx.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec4d.h"

#include <memory>
#include <vector>

WABI_NAMESPACE_BEGIN

using HgiUniquePtr = std::unique_ptr<class Hgi>;

/// \class HdPh_TestDriver
///
/// A unit test driver that exercises the core engine.
///
/// \note This test driver does NOT assume OpenGL is available; in the even
/// that is is not available, all OpenGL calls become no-ops, but all other work
/// is performed as usual.
///
class HdPh_TestDriver final
{
 public:
  HdPh_TestDriver();
  HdPh_TestDriver(TfToken const &reprName);
  HdPh_TestDriver(HdReprSelector const &reprToken);
  ~HdPh_TestDriver();

  /// Draw
  void Draw(bool withGuides = false);

  /// Draw with external renderPass
  void Draw(HdRenderPassSharedPtr const &renderPass, bool withGuides);

  /// Set camera to renderpass
  void SetCamera(GfMatrix4d const &modelViewMatrix,
                 GfMatrix4d const &projectionMatrix,
                 GfVec4d const &viewport);

  void SetCameraClipPlanes(std::vector<GfVec4d> const &clipPlanes);

  /// Set cull style
  void SetCullStyle(HdCullStyle cullStyle);

  /// Returns the renderpass
  HdRenderPassSharedPtr const &GetRenderPass();

  /// Returns the renderPassState
  HdPhRenderPassStateSharedPtr const &GetRenderPassState() const
  {
    return _renderPassState;
  }

  /// Returns the UnitTest delegate
  HdUnitTestDelegate &GetDelegate()
  {
    return *_sceneDelegate;
  }

  /// Switch repr
  void SetRepr(HdReprSelector const &reprToken);

 private:
  void _Init(HdReprSelector const &reprToken);

  // Hgi and HdDriver should be constructed before HdEngine to ensure they
  // are destructed last. Hgi may be used during engine/delegate destruction.
  HgiUniquePtr _hgi;
  HdDriver _hgiDriver;

  HdEngine _engine;
  HdPhRenderDelegate _renderDelegate;
  HdRenderIndex *_renderIndex;
  HdUnitTestDelegate *_sceneDelegate;

  SdfPath _cameraId;
  HdReprSelector _reprToken;

  HdRenderPassSharedPtr _renderPass;
  HdPhRenderPassStateSharedPtr _renderPassState;
  HdRprimCollection _collection;
};

/// \class HdPh_TestLightingShader
///
/// A custom lighting shader for unit tests.
///
using HdPh_TestLightingShaderSharedPtr = std::shared_ptr<class HdPh_TestLightingShader>;

class HdPh_TestLightingShader : public HdPhLightingShader
{
 public:
  HDPH_API
  HdPh_TestLightingShader();
  HDPH_API
  ~HdPh_TestLightingShader() override;

  /// HdPhShaderCode overrides
  HDPH_API
  ID ComputeHash() const override;
  std::string GetSource(TfToken const &shaderStageKey) const override;
  HDPH_API
  void BindResources(int program,
                     HdPh_ResourceBinder const &binder,
                     HdRenderPassState const &state) override;
  HDPH_API
  void UnbindResources(int program,
                       HdPh_ResourceBinder const &binder,
                       HdRenderPassState const &state) override;
  void AddBindings(HdBindingRequestVector *customBindings) override;

  /// HdPhLightingShader overrides
  void SetCamera(GfMatrix4d const &worldToViewMatrix, GfMatrix4d const &projectionMatrix) override;

  void SetSceneAmbient(GfVec3f const &color);
  void SetLight(int light, GfVec3f const &dir, GfVec3f const &color);

 private:
  struct Light
  {
    GfVec3f dir;
    GfVec3f eyeDir;
    GfVec3f color;
  };
  Light _lights[2];
  GfVec3f _sceneAmbient;
  std::unique_ptr<HioGlslfx> _glslfx;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_UNIT_TEST_HELPER_H
