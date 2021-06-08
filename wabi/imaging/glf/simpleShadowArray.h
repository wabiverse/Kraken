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
#ifndef WABI_IMAGING_GLF_SIMPLE_SHADOW_ARRAY_H
#define WABI_IMAGING_GLF_SIMPLE_SHADOW_ARRAY_H

/// \file glf/simpleShadowArray.h

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec2i.h"
#include "wabi/base/gf/vec4d.h"
#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/refPtr.h"
#include "wabi/base/tf/weakPtr.h"
#include "wabi/imaging/garch/glApi.h"
#include "wabi/imaging/glf/api.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

class GlfSimpleShadowArray : public TfRefBase, public TfWeakBase {
 public:
  GLF_API
  GlfSimpleShadowArray();
  GLF_API
  ~GlfSimpleShadowArray() override;

  // Disallow copies
  GlfSimpleShadowArray(const GlfSimpleShadowArray &) = delete;
  GlfSimpleShadowArray &operator=(const GlfSimpleShadowArray &) = delete;

  // Driven by the env var GLF_ENABLE_BINDLESS_SHADOW_TEXTURE, this returns
  // whether bindless shadow maps are enabled, which in turn dictates the API
  // to use. See below.
  GLF_API static bool GetBindlessShadowMapsEnabled();

  ///  Bindful API:

  // Set the 2D size of the shadow map texture array.
  GLF_API
  void SetSize(GfVec2i const &size);

  // Set the depth of the shadow map texture array, which corresponds to the
  // number of shadow maps necessary. Each shadow casting light uses one
  // shadow map.
  GLF_API
  void SetNumLayers(size_t numLayers);

  // Returns the GL texture id of the texture array.
  GLF_API
  GLuint GetShadowMapTexture() const;

  // Returns the GL sampler id of the sampler object used to read the raw
  // depth values.
  GLF_API
  GLuint GetShadowMapDepthSampler() const;

  // Returns the GL sampler id of the sampler object used for depth comparison
  GLF_API
  GLuint GetShadowMapCompareSampler() const;

  /// Bindless API:

  // Set the resolutions of all the shadow maps necessary. The number of
  // resolutions corresponds to the number of shadow map textures necessary,
  // which is currently one per shadow casting light.
  GLF_API
  void SetShadowMapResolutions(std::vector<GfVec2i> const &resolutions);

  // Returns a vector of the 64bit bindless handles corresponding to the
  // bindless shadow map textures.
  GLF_API
  std::vector<uint64_t> const &GetBindlessShadowMapHandles() const;

  /// Common API (for shadow map generation)

  // Returns the number of shadow map generation passes required, which is
  // currently one per shadow map (corresponding to a shadow casting light).
  GLF_API
  size_t GetNumShadowMapPasses() const;

  // Returns the shadow map resolution for a given pass. For bindful shadows,
  // this returns a single size for all passes, while for bindless, it returns
  // the resolution of the corresponding shadow map,
  GLF_API
  GfVec2i GetShadowMapSize(size_t pass) const;

  // Get/Set the view (world to shadow camera) transform to use for a given
  // shadow map generation pass.
  GLF_API
  GfMatrix4d GetViewMatrix(size_t index) const;
  GLF_API
  void SetViewMatrix(size_t index, GfMatrix4d const &matrix);

  // Get/Set the projection transform to use for a given shadow map generation
  // pass.
  GLF_API
  GfMatrix4d GetProjectionMatrix(size_t index) const;
  GLF_API
  void SetProjectionMatrix(size_t index, GfMatrix4d const &matrix);

  GLF_API
  GfMatrix4d GetWorldToShadowMatrix(size_t index) const;

  // Bind necessary resources for a given shadow map generation pass.
  GLF_API
  void BeginCapture(size_t index, bool clear);

  // Unbind necssary resources after a shadow map gneration pass.
  GLF_API
  void EndCapture(size_t index);

 private:
  void _AllocResources();
  void _AllocBindfulTextures();
  void _AllocBindlessTextures();
  void _FreeResources();
  void _FreeBindfulTextures();
  void _FreeBindlessTextures();
  bool _ShadowMapExists() const;
  void _BindFramebuffer(size_t index);
  void _UnbindFramebuffer();

 private:
  // bindful state
  GfVec2i _size;
  size_t _numLayers;
  GLuint _bindfulTexture;
  GLuint _shadowDepthSampler;

  // bindless state
  std::vector<GfVec2i> _resolutions;
  std::vector<GLuint> _bindlessTextures;
  std::vector<uint64_t> _bindlessTextureHandles;

  // common state
  std::vector<GfMatrix4d> _viewMatrix;
  std::vector<GfMatrix4d> _projectionMatrix;

  GLuint _framebuffer;

  GLuint _shadowCompareSampler;

  GLuint _unbindRestoreDrawFramebuffer;
  GLuint _unbindRestoreReadFramebuffer;

  GLint _unbindRestoreViewport[4];
};

WABI_NAMESPACE_END

#endif
