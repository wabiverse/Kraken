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
#ifndef WABI_IMAGING_HD_PH_MIXIN_SHADER_H
#define WABI_IMAGING_HD_PH_MIXIN_SHADER_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class HdPh_MixinShader
///
/// A final shader code class representing a mixin of a shader with a
/// base shader.
///
/// This interface provides a simple way for clients to extend a given
/// shader without mutating the original shader.
class HdPhMixinShader final : public HdPhShaderCode {
 public:
  HDPH_API
  HdPhMixinShader(std::string mixinSource, HdPhShaderCodeSharedPtr baseShader);

  HDPH_API
  ~HdPhMixinShader() override;

  /// Returns the hash value of this shader.
  HDPH_API
  HdPhShaderCode::ID ComputeHash() const override;

  HDPH_API
  ID ComputeTextureSourceHash() const override;

  /// Returns the shader source provided by this shader
  /// for \a shaderStageKey
  HDPH_API
  std::string GetSource(TfToken const &shaderStageKey) const override;

  HDPH_API
  HdPh_MaterialParamVector const &GetParams() const override;

  HDPH_API
  bool IsEnabledPrimvarFiltering() const override;

  HDPH_API
  TfTokenVector const &GetPrimvarNames() const override;

  /// Returns a buffer which stores parameter fallback values and texture
  /// handles.
  HDPH_API
  HdBufferArrayRangeSharedPtr const &GetShaderData() const override;

  /// Binds shader-specific resources to \a program
  HDPH_API
  void BindResources(int program,
                     HdPh_ResourceBinder const &binder,
                     HdRenderPassState const &state) override;

  /// Unbinds shader-specific resources.
  HDPH_API
  void UnbindResources(int program,
                       HdPh_ResourceBinder const &binder,
                       HdRenderPassState const &state) override;

  /// Add custom bindings (used by codegen)
  HDPH_API
  void AddBindings(HdBindingRequestVector *customBindings) override;

  /// Returns the render pass tag of this shader.
  HDPH_API
  TfToken GetMaterialTag() const override;

 private:
  std::string _mixinSource;
  HdPhShaderCodeSharedPtr _baseShader;

  HdPhMixinShader()                        = delete;
  HdPhMixinShader(const HdPhMixinShader &) = delete;
  HdPhMixinShader &operator=(const HdPhMixinShader &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_MIXIN_SHADER_H
