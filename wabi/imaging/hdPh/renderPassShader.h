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
#ifndef WABI_IMAGING_HD_ST_RENDER_PASS_SHADER_H
#define WABI_IMAGING_HD_ST_RENDER_PASS_SHADER_H

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/imaging/hio/glslfx.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/token.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdPhRenderPassShaderSharedPtr = std::shared_ptr<class HdPhRenderPassShader>;
class HdRenderIndex;
using HdRenderPassAovBindingVector = std::vector<struct HdRenderPassAovBinding>;

/// \class HdPhRenderPassShader
///
/// A shader that supports common renderPass functionality.
///
class HdPhRenderPassShader : public HdPhShaderCode
{
 public:

  HDPH_API
  HdPhRenderPassShader();
  HDPH_API
  HdPhRenderPassShader(TfToken const &glslfxFile);
  HDPH_API
  ~HdPhRenderPassShader() override;

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
  HDPH_API
  HdPh_MaterialParamVector const &GetParams() const override;

  HDPH_API
  NamedTextureHandleVector const &GetNamedTextureHandles() const override;

  /// Add a custom binding request for use when this shader executes.
  HDPH_API
  void AddBufferBinding(HdBindingRequest const &req);

  /// Remove \p name from custom binding.
  HDPH_API
  void RemoveBufferBinding(TfToken const &name);

  /// Clear all custom bindings associated with this shader.
  HDPH_API
  void ClearBufferBindings();

  HdCullStyle GetCullStyle() const
  {
    return _cullStyle;
  }

  void SetCullStyle(HdCullStyle cullStyle)
  {
    _cullStyle = cullStyle;
  }

  // Sets the textures and params such that the shader can access
  // the requested aovs with HdGet_AOVNAMEReadback().
  //
  // Needs to be called in task prepare or sync since it is
  // allocating texture handles.
  //
  HDPH_API
  void UpdateAovInputTextures(HdRenderPassAovBindingVector const &aovInputBindings,
                              HdRenderIndex *const renderIndex);

 private:

  TfToken _glslfxFile;
  std::unique_ptr<HioGlslfx> _glslfx;
  mutable size_t _hash;
  mutable bool _hashValid;

  TfHashMap<TfToken, HdBindingRequest, TfToken::HashFunctor> _customBuffers;
  HdCullStyle _cullStyle;

  NamedTextureHandleVector _namedTextureHandles;

  HdPh_MaterialParamVector _params;

  // No copying
  HdPhRenderPassShader(const HdPhRenderPassShader &) = delete;
  HdPhRenderPassShader &operator=(const HdPhRenderPassShader &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_RENDER_PASS_SHADER_H
