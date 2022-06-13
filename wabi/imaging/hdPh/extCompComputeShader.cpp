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
#include "wabi/imaging/hdPh/extCompComputeShader.h"
#include "wabi/imaging/hdPh/extComputation.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/resource.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/resourceBinder.h"

#include "wabi/base/arch/hash.h"

WABI_NAMESPACE_BEGIN

HdPh_ExtCompComputeShader::HdPh_ExtCompComputeShader(HdExtComputation const *extComp)
  : _extComp(extComp)
{}

HdPh_ExtCompComputeShader::~HdPh_ExtCompComputeShader() = default;

// -------------------------------------------------------------------------- //
// HdPhShaderCode Virtual Interface                                           //
// -------------------------------------------------------------------------- //

/*virtual*/
std::string HdPh_ExtCompComputeShader::GetSource(TfToken const &shaderStageKey) const
{
  if (shaderStageKey == HdShaderTokens->computeShader) {
    if (TF_VERIFY(_extComp)) {
      return _extComp->GetGpuKernelSource();
    }
  }

  return std::string();
}

/*virtual*/
void HdPh_ExtCompComputeShader::BindResources(const int program,
                                              HdPh_ResourceBinder const &binder,
                                              HdRenderPassState const &state)
{
  // Compute shaders currently serve GPU ExtComputations, wherein
  // resource binding is managed explicitly.
  // See HdPhExtCompGpuComputationResource::Resolve() and
  // HdPhExtCompGpuComputation::Execute(..)
}

/*virtual*/
void HdPh_ExtCompComputeShader::UnbindResources(const int program,
                                                HdPh_ResourceBinder const &binder,
                                                HdRenderPassState const &state)
{
  // Resource binding is managed explicitly. See above comment.
}

/*virtual*/
void HdPh_ExtCompComputeShader::AddBindings(HdBindingRequestVector *customBindings)
{
  // Resource binding is managed explicitly. See above comment.
}

/*virtual*/
HdPhShaderCode::ID HdPh_ExtCompComputeShader::ComputeHash() const
{
  if (!TF_VERIFY(_extComp)) {
    return 0;
  }

  size_t hash = 0;
  std::string const &kernel = _extComp->GetGpuKernelSource();
  boost::hash_combine(hash, ArchHash(kernel.c_str(), kernel.size()));
  return hash;
}

SdfPath const &HdPh_ExtCompComputeShader::GetExtComputationId() const
{
  if (!TF_VERIFY(_extComp)) {
    return SdfPath::EmptyPath();
  }
  return _extComp->GetId();
}

WABI_NAMESPACE_END
