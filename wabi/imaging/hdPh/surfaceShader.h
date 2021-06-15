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
#ifndef WABI_IMAGING_HD_ST_SURFACE_SHADER_H
#define WABI_IMAGING_HD_ST_SURFACE_SHADER_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/path.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/vt/value.h"

#include <memory>
#include <vector>

WABI_NAMESPACE_BEGIN

class HdSceneDelegate;

using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;

using HdPhSurfaceShaderSharedPtr = std::shared_ptr<class HdPhSurfaceShader>;

using HdBufferSpecVector = std::vector<struct HdBufferSpec>;
using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

/// \class HdPhSurfaceShader
///
/// A scene-based SurfaceShader object.
///
/// When surface shaders are expresed in the scene graph, the HdSceneDelegate
/// can use this object to express these surface shaders in Phoenix. In addition
/// to the shader itself, a binding from the Rprim to the SurfaceShader must be
/// expressed as well.
class HdPhSurfaceShader : public HdPhShaderCode {
 public:
  HDPH_API
  HdPhSurfaceShader();
  HDPH_API
  ~HdPhSurfaceShader() override;

  // ---------------------------------------------------------------------- //
  /// \name HdShader Virtual Interface                                      //
  // ---------------------------------------------------------------------- //
  HDPH_API
  std::string GetSource(TfToken const &shaderStageKey) const override;
  HDPH_API
  HdPh_MaterialParamVector const &GetParams() const override;
  HDPH_API
  void SetEnabledPrimvarFiltering(bool enabled);
  HDPH_API
  bool IsEnabledPrimvarFiltering() const override;
  HDPH_API
  TfTokenVector const &GetPrimvarNames() const override;
  HDPH_API
  HdBufferArrayRangeSharedPtr const &GetShaderData() const override;
  HDPH_API
  NamedTextureHandleVector const &GetNamedTextureHandles() const override;
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
  HDPH_API
  ID ComputeHash() const override;

  HDPH_API
  ID ComputeTextureSourceHash() const override;

  HDPH_API
  TfToken GetMaterialTag() const override;

  /// Setter method for prim
  HDPH_API
  void SetFragmentSource(const std::string &source);
  HDPH_API
  void SetGeometrySource(const std::string &source);
  HDPH_API
  void SetParams(const HdPh_MaterialParamVector &params);
  HDPH_API
  void SetNamedTextureHandles(const NamedTextureHandleVector &);
  HDPH_API
  void SetBufferSources(HdBufferSpecVector const &bufferSpecs,
                        HdBufferSourceSharedPtrVector &&bufferSources,
                        HdPhResourceRegistrySharedPtr const &resourceRegistry);

  /// Called after textures have been committed.
  ///
  /// Shader can return buffer sources for different BARs (most
  /// likely, the shader bar) that require texture metadata such as
  /// the bindless texture handle which is only available after the
  /// commit.
  ///
  HDPH_API
  void AddResourcesFromTextures(ResourceContext &ctx) const override;

  HDPH_API
  void SetMaterialTag(TfToken const &materialTag);

  /// If the prim is based on asset, reload that asset.
  HDPH_API
  virtual void Reload();

  /// Returns if the two shaders can be aggregated into the same draw batch.
  HDPH_API
  static bool CanAggregate(HdPhShaderCodeSharedPtr const &shaderA, HdPhShaderCodeSharedPtr const &shaderB);

  /// Adds the fallback value of the given material param to
  /// buffer specs and sources using the param's name.
  ///
  HDPH_API
  static void AddFallbackValueToSpecsAndSources(const HdPh_MaterialParam &param,
                                                HdBufferSpecVector *const specs,
                                                HdBufferSourceSharedPtrVector *const sources);

 protected:
  HDPH_API
  void _SetSource(TfToken const &shaderStageKey, std::string const &source);

  HDPH_API
  ID _ComputeHash() const;

  HDPH_API
  ID _ComputeTextureSourceHash() const;

 private:
  std::string _fragmentSource;
  std::string _geometrySource;

  // Shader Parameters
  HdPh_MaterialParamVector _params;
  HdBufferSpecVector _paramSpec;
  HdBufferArrayRangeSharedPtr _paramArray;
  TfTokenVector _primvarNames;
  bool _isEnabledPrimvarFiltering;

  mutable size_t _computedHash;
  mutable bool _isValidComputedHash;

  mutable size_t _computedTextureSourceHash;
  mutable bool _isValidComputedTextureSourceHash;

  NamedTextureHandleVector _namedTextureHandles;

  TfToken _materialTag;

  // No copying
  HdPhSurfaceShader(const HdPhSurfaceShader &) = delete;
  HdPhSurfaceShader &operator=(const HdPhSurfaceShader &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_SURFACE_SHADER_H
