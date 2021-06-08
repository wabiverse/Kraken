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
//

#include "wabi/imaging/hdPh/mixinShader.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/base/arch/hash.h"

#include <boost/functional/hash.hpp>

WABI_NAMESPACE_BEGIN

HdPhMixinShader::HdPhMixinShader(std::string mixinSource, HdPhShaderCodeSharedPtr baseShader)
    : HdPhShaderCode(),
      _mixinSource(mixinSource),
      _baseShader(baseShader)
{}

HdPhMixinShader::~HdPhMixinShader() = default;

HdPhShaderCode::ID HdPhMixinShader::ComputeHash() const
{
  HdPhShaderCode::ID hash = 0;
  boost::hash_combine(hash, ArchHash(_mixinSource.c_str(), _mixinSource.size()));
  boost::hash_combine(hash, _baseShader->ComputeHash());
  return hash;
}

HdPhShaderCode::ID HdPhMixinShader::ComputeTextureSourceHash() const
{
  return _baseShader->ComputeTextureSourceHash();
}

std::string HdPhMixinShader::GetSource(TfToken const &shaderStageKey) const
{
  std::string baseSource = _baseShader->GetSource(shaderStageKey);
  if (shaderStageKey == HdShaderTokens->fragmentShader) {
    return _mixinSource + baseSource;
  }
  return baseSource;
}

HdPh_MaterialParamVector const &HdPhMixinShader::GetParams() const
{
  return _baseShader->GetParams();
}

bool HdPhMixinShader::IsEnabledPrimvarFiltering() const
{
  return _baseShader->IsEnabledPrimvarFiltering();
}

TfTokenVector const &HdPhMixinShader::GetPrimvarNames() const
{
  return _baseShader->GetPrimvarNames();
}

HdBufferArrayRangeSharedPtr const &HdPhMixinShader::GetShaderData() const
{
  return _baseShader->GetShaderData();
}

void HdPhMixinShader::BindResources(const int program,
                                    HdPh_ResourceBinder const &binder,
                                    HdRenderPassState const &state)
{
  _baseShader->BindResources(program, binder, state);
}

void HdPhMixinShader::UnbindResources(const int program,
                                      HdPh_ResourceBinder const &binder,
                                      HdRenderPassState const &state)
{
  _baseShader->UnbindResources(program, binder, state);
}

void HdPhMixinShader::AddBindings(HdBindingRequestVector *customBindings)
{
  _baseShader->AddBindings(customBindings);
}

TfToken HdPhMixinShader::GetMaterialTag() const
{
  return _baseShader->GetMaterialTag();
}

WABI_NAMESPACE_END
