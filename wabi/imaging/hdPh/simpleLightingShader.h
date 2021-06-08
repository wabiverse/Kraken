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
#ifndef WABI_IMAGING_HDPH_SIMPLE_LIGHTING_SHADER_H
#define WABI_IMAGING_HDPH_SIMPLE_LIGHTING_SHADER_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/lightingShader.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/version.h"

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/imaging/glf/simpleLightingContext.h"

#include <memory>
#include <string>

WABI_NAMESPACE_BEGIN

class HdSceneDelegate;
using HdPhSimpleLightingShaderSharedPtr = std::shared_ptr<class HdPhSimpleLightingShader>;
TF_DECLARE_REF_PTRS(GlfBindingMap);

/// \class HdPhSimpleLightingShader
///
/// A shader that supports simple lighting functionality.
///
class HdPhSimpleLightingShader : public HdPhLightingShader {
 public:
  HDPH_API
  HdPhSimpleLightingShader();
  HDPH_API
  ~HdPhSimpleLightingShader() override;

  /// HdShader overrides
  HDPH_API
  ID ComputeHash() const override;
  HDPH_API
  std::string GetSource(TfToken const &shaderStageKey) const override;
  HDPH_API
  void BindResources(int program,
                     HdPh_ResourceBinder const &binder,
                     HdRenderPassState const &state) override;
  HDPH_API
  void UnbindResources(int program,
                       HdPh_ResourceBinder const &binder,
                       HdRenderPassState const &state) override;
  HDPH_API
  void AddBindings(HdBindingRequestVector *customBindings) override;

  /// Adds computations to create the dome light textures that
  /// are pre-calculated from the environment map texture.
  HDPH_API
  void AddResourcesFromTextures(ResourceContext &ctx) const override;

  /// HdPhShaderCode overrides
  HDPH_API
  HdPh_MaterialParamVector const &GetParams() const override;

  /// HdPhLightingShader overrides
  HDPH_API
  void SetCamera(GfMatrix4d const &worldToViewMatrix, GfMatrix4d const &projectionMatrix) override;
  HDPH_API
  void SetLightingStateFromOpenGL();
  HDPH_API
  void SetLightingState(GlfSimpleLightingContextPtr const &lightingContext);

  GlfSimpleLightingContextRefPtr GetLightingContext() const
  {
    return _lightingContext;
  };

  /// Allocates texture handles (texture loading happens later during commit)
  /// needed for lights.
  ///
  /// Call after lighting context has been set or updated in Sync-phase.
  ///
  HDPH_API
  void AllocateTextureHandles(HdSceneDelegate *delegate);

  /// The dome light environment map used as source for the other
  /// dome light textures.
  const HdPhTextureHandleSharedPtr &GetDomeLightEnvironmentTextureHandle() const
  {
    return _domeLightEnvironmentTextureHandle;
  }

  /// The textures computed from the dome light environment map that
  /// the shader needs to bind for the dome light shading.
  HDPH_API
  NamedTextureHandleVector const &GetNamedTextureHandles() const override;

  /// Get one of the textures that need to be computed from the dome
  /// light environment map.
  HDPH_API
  const HdPhTextureHandleSharedPtr &GetTextureHandle(const TfToken &name) const;

 private:
  GlfSimpleLightingContextRefPtr _lightingContext;
  GlfBindingMapRefPtr _bindingMap;
  bool _useLighting;
  std::unique_ptr<class HioGlslfx> _glslfx;

  // The environment map used as source for the dome light textures.
  //
  // Handle is allocated in AllocateTextureHandles. Actual loading
  // happens during commit.
  HdPhTextureHandleSharedPtr _domeLightEnvironmentTextureHandle;

  // Other dome light textures.
  NamedTextureHandleVector _namedTextureHandles;

  HdPh_MaterialParamVector _lightTextureParams;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HDPH_SIMPLE_LIGHTING_SHADER_H
