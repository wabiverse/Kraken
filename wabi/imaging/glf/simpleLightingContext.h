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
#ifndef WABI_IMAGING_GLF_SIMPLE_LIGHTING_CONTEXT_H
#define WABI_IMAGING_GLF_SIMPLE_LIGHTING_CONTEXT_H

/// \file glf/simpleLightingContext.h

#include "wabi/imaging/glf/api.h"
#include "wabi/imaging/glf/simpleLight.h"
#include "wabi/imaging/glf/simpleMaterial.h"
#include "wabi/imaging/glf/simpleShadowArray.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec4f.h"

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/refBase.h"
#include "wabi/base/tf/token.h"
#include "wabi/base/tf/weakBase.h"

#include <memory>

WABI_NAMESPACE_BEGIN

TF_DECLARE_WEAK_AND_REF_PTRS(GlfBindingMap);
TF_DECLARE_WEAK_AND_REF_PTRS(GlfUniformBlock);
TF_DECLARE_WEAK_AND_REF_PTRS(GlfSimpleLightingContext);
TF_DECLARE_WEAK_AND_REF_PTRS(GlfSimpleShadowArray);

class GlfSimpleLightingContext : public TfRefBase, public TfWeakBase
{
 public:

  typedef GlfSimpleLightingContext This;

  GLF_API
  static GlfSimpleLightingContextRefPtr New();

  GLF_API
  void SetLights(GlfSimpleLightVector const &lights);
  GLF_API
  GlfSimpleLightVector const &GetLights() const;

  // returns the effective number of lights taken into account
  // in composable/compatible shader constraints
  GLF_API
  int GetNumLightsUsed() const;

  // returns the number of shadow maps needed, by summing shadow maps
  // allocated to each light.
  GLF_API
  int ComputeNumShadowsUsed() const;

  GLF_API
  void SetShadows(GlfSimpleShadowArrayRefPtr const &shadows);
  GLF_API
  GlfSimpleShadowArrayRefPtr const &GetShadows() const;

  GLF_API
  void SetMaterial(GlfSimpleMaterial const &material);
  GLF_API
  GlfSimpleMaterial const &GetMaterial() const;

  GLF_API
  void SetSceneAmbient(GfVec4f const &sceneAmbient);
  GLF_API
  GfVec4f const &GetSceneAmbient() const;

  GLF_API
  void SetCamera(GfMatrix4d const &worldToViewMatrix, GfMatrix4d const &projectionMatrix);

  GLF_API
  void SetUseLighting(bool val);
  GLF_API
  bool GetUseLighting() const;

  // returns true if any light has shadow enabled.
  GLF_API
  bool GetUseShadows() const;

  GLF_API
  void SetUseColorMaterialDiffuse(bool val);
  GLF_API
  bool GetUseColorMaterialDiffuse() const;

  GLF_API
  void InitUniformBlockBindings(GlfBindingMapPtr const &bindingMap) const;
  GLF_API
  void InitSamplerUnitBindings(GlfBindingMapPtr const &bindingMap) const;

  GLF_API
  void BindUniformBlocks(GlfBindingMapPtr const &bindingMap);
  GLF_API
  void BindSamplers(GlfBindingMapPtr const &bindingMap);

  GLF_API
  void UnbindSamplers(GlfBindingMapPtr const &bindingMap);

  GLF_API
  void SetStateFromOpenGL();

  /// \name Post Surface Lighting
  ///
  /// This context can provide additional shader source, currently
  /// used to implement post surface lighting, along with a hash
  /// to help de-duplicate use by client shader programs.
  ///
  /// @{

  GLF_API
  size_t ComputeShaderSourceHash();

  GLF_API
  std::string const &ComputeShaderSource(TfToken const &shaderStageKey);

  /// @}

 protected:

  GLF_API
  GlfSimpleLightingContext();
  GLF_API
  ~GlfSimpleLightingContext();

  void _ComputePostSurfaceShaderState();
  void _BindPostSurfaceShaderParams(GlfBindingMapPtr const &bindingMap);

 private:

  GlfSimpleLightVector _lights;
  GlfSimpleShadowArrayRefPtr _shadows;

  GfMatrix4d _worldToViewMatrix;
  GfMatrix4d _projectionMatrix;

  GlfSimpleMaterial _material;
  GfVec4f _sceneAmbient;

  bool _useLighting;
  bool _useShadows;
  bool _useColorMaterialDiffuse;

  GlfUniformBlockRefPtr _lightingUniformBlock;
  GlfUniformBlockRefPtr _shadowUniformBlock;
  GlfUniformBlockRefPtr _materialUniformBlock;
  GlfUniformBlockRefPtr _bindlessShadowlUniformBlock;

  class _PostSurfaceShaderState;
  std::unique_ptr<_PostSurfaceShaderState> _postSurfaceShaderState;

  bool _lightingUniformBlockValid;
  bool _shadowUniformBlockValid;
  bool _materialUniformBlockValid;
  bool _postSurfaceShaderStateValid;
};

WABI_NAMESPACE_END

#endif
