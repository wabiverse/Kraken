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
#include "wabi/imaging/hdPh/shaderCode.h"

#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"

#include "wabi/imaging/hd/tokens.h"

#include "wabi/base/tf/iterator.h"

#include <boost/functional/hash.hpp>

WABI_NAMESPACE_BEGIN

HdPhShaderCode::HdPhShaderCode() = default;

/*virtual*/
HdPhShaderCode::~HdPhShaderCode() = default;

/* static */
size_t HdPhShaderCode::ComputeHash(HdPhShaderCodeSharedPtrVector const &shaders)
{
  size_t hash = 0;

  TF_FOR_ALL(it, shaders)
  {
    boost::hash_combine(hash, (*it)->ComputeHash());
  }

  return hash;
}

/* virtual */
TfToken HdPhShaderCode::GetMaterialTag() const
{
  return TfToken();
}

/*virtual*/
HdPh_MaterialParamVector const &HdPhShaderCode::GetParams() const
{
  static HdPh_MaterialParamVector const empty;
  return empty;
}

/* virtual */
bool HdPhShaderCode::IsEnabledPrimvarFiltering() const
{
  return false;
}

/* virtual */
TfTokenVector const &HdPhShaderCode::GetPrimvarNames() const
{
  static const TfTokenVector EMPTY;
  return EMPTY;
}

/*virtual*/
HdBufferArrayRangeSharedPtr const &HdPhShaderCode::GetShaderData() const
{
  static HdBufferArrayRangeSharedPtr EMPTY;
  return EMPTY;
}

/* virtual */
HdPhShaderCode::NamedTextureHandleVector const &HdPhShaderCode::GetNamedTextureHandles() const
{
  static HdPhShaderCode::NamedTextureHandleVector empty;
  return empty;
}

/*virtual*/
void HdPhShaderCode::AddResourcesFromTextures(ResourceContext &ctx) const
{}

HdPhShaderCode::ID HdPhShaderCode::ComputeTextureSourceHash() const
{
  return 0;
}

void HdPhShaderCode::ResourceContext::AddSource(HdBufferArrayRangeSharedPtr const &range,
                                                HdBufferSourceSharedPtr const &source)
{
  _registry->AddSource(range, source);
}

void HdPhShaderCode::ResourceContext::AddSources(HdBufferArrayRangeSharedPtr const &range,
                                                 HdBufferSourceSharedPtrVector &&sources)
{
  _registry->AddSources(range, std::move(sources));
}

void HdPhShaderCode::ResourceContext::AddComputation(HdBufferArrayRangeSharedPtr const &range,
                                                     HdComputationSharedPtr const &computation,
                                                     HdPhComputeQueue const queue)
{
  _registry->AddComputation(range, computation, queue);
}

HdPhShaderCode::ResourceContext::ResourceContext(HdPhResourceRegistry *const registry)
    : _registry(registry)
{}

WABI_NAMESPACE_END
