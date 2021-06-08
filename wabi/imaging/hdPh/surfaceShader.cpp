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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/surfaceShader.h"
#include "wabi/imaging/hdPh/textureBinder.h"
#include "wabi/imaging/hdPh/textureHandle.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/bufferArrayRange.h"
#include "wabi/imaging/hd/resource.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/imaging/glf/contextCaps.h"

#include "wabi/base/arch/hash.h"
#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_ENV_SETTING(HDPH_ENABLE_MATERIAL_PRIMVAR_FILTERING,
                      true,
                      "Enables filtering of primvar signals by material binding.");

static bool _IsEnabledMaterialPrimvarFiltering()
{
  return TfGetEnvSetting(HDPH_ENABLE_MATERIAL_PRIMVAR_FILTERING);
}

static TfTokenVector _CollectPrimvarNames(const HdPh_MaterialParamVector &params);

HdPhSurfaceShader::HdPhSurfaceShader()
    : HdPhShaderCode(),
      _fragmentSource(),
      _geometrySource(),
      _params(),
      _paramSpec(),
      _paramArray(),
      _primvarNames(_CollectPrimvarNames(_params)),
      _isEnabledPrimvarFiltering(_IsEnabledMaterialPrimvarFiltering()),
      _computedHash(0),
      _isValidComputedHash(false),
      _computedTextureSourceHash(0),
      _isValidComputedTextureSourceHash(false),
      _materialTag()
{}

HdPhSurfaceShader::~HdPhSurfaceShader() = default;

void HdPhSurfaceShader::_SetSource(TfToken const &shaderStageKey, std::string const &source)
{
  if (shaderStageKey == HdShaderTokens->fragmentShader) {
    _fragmentSource      = source;
    _isValidComputedHash = false;
  }
  else if (shaderStageKey == HdShaderTokens->geometryShader) {
    _geometrySource      = source;
    _isValidComputedHash = false;
  }
}

// -------------------------------------------------------------------------- //
// HdShader Virtual Interface                                                 //
// -------------------------------------------------------------------------- //

/*virtual*/
std::string HdPhSurfaceShader::GetSource(TfToken const &shaderStageKey) const
{
  if (shaderStageKey == HdShaderTokens->fragmentShader) {
    return _fragmentSource;
  }
  else if (shaderStageKey == HdShaderTokens->geometryShader) {
    return _geometrySource;
  }

  return std::string();
}
/*virtual*/
HdPh_MaterialParamVector const &HdPhSurfaceShader::GetParams() const
{
  return _params;
}
void HdPhSurfaceShader::SetEnabledPrimvarFiltering(bool enabled)
{
  _isEnabledPrimvarFiltering = enabled && _IsEnabledMaterialPrimvarFiltering();
}
/* virtual */
bool HdPhSurfaceShader::IsEnabledPrimvarFiltering() const
{
  return _isEnabledPrimvarFiltering;
}
/*virtual*/
TfTokenVector const &HdPhSurfaceShader::GetPrimvarNames() const
{
  return _primvarNames;
}
/*virtual*/
HdBufferArrayRangeSharedPtr const &HdPhSurfaceShader::GetShaderData() const
{
  return _paramArray;
}

HdPhShaderCode::NamedTextureHandleVector const &HdPhSurfaceShader::GetNamedTextureHandles() const
{
  return _namedTextureHandles;
}

/*virtual*/
void HdPhSurfaceShader::BindResources(const int program,
                                      HdPh_ResourceBinder const &binder,
                                      HdRenderPassState const &state)
{
  // const bool bindlessTextureEnabled = VkfContextCaps::GetInstance().bindlessTextureEnabled;
  const bool bindlessTextureEnabled = true;

  HdPh_TextureBinder::BindResources(binder, bindlessTextureEnabled, _namedTextureHandles);

  glActiveTexture(GL_TEXTURE0);

  binder.BindShaderResources(this);
}
/*virtual*/
void HdPhSurfaceShader::UnbindResources(const int program,
                                        HdPh_ResourceBinder const &binder,
                                        HdRenderPassState const &state)
{
  binder.UnbindShaderResources(this);

  const bool bindlessTextureEnabled = GlfContextCaps::GetInstance().bindlessTextureEnabled;

  HdPh_TextureBinder::UnbindResources(binder, bindlessTextureEnabled, _namedTextureHandles);

  glActiveTexture(GL_TEXTURE0);
}
/*virtual*/
void HdPhSurfaceShader::AddBindings(HdBindingRequestVector *customBindings)
{}

/*virtual*/
HdPhShaderCode::ID HdPhSurfaceShader::ComputeHash() const
{
  // All mutator methods that might affect the hash must reset this (fragile).
  if (!_isValidComputedHash) {
    _computedHash        = _ComputeHash();
    _isValidComputedHash = true;
  }
  return _computedHash;
}

/*virtual*/
HdPhShaderCode::ID HdPhSurfaceShader::ComputeTextureSourceHash() const
{
  if (!_isValidComputedTextureSourceHash) {
    _computedTextureSourceHash        = _ComputeTextureSourceHash();
    _isValidComputedTextureSourceHash = true;
  }
  return _computedTextureSourceHash;
}

HdPhShaderCode::ID HdPhSurfaceShader::_ComputeHash() const
{
  size_t hash = HdPh_MaterialParam::ComputeHash(_params);

  boost::hash_combine(hash, ArchHash(_fragmentSource.c_str(), _fragmentSource.size()));
  boost::hash_combine(hash, ArchHash(_geometrySource.c_str(), _geometrySource.size()));

  // Codegen is inspecting the shader bar spec to generate some
  // of the struct's, so we should probably use _paramSpec
  // in the hash computation as well.
  //
  // In practise, _paramSpec is generated from the
  // HdPh_MaterialParam's so the above is sufficient.

  return hash;
}

HdPhShaderCode::ID HdPhSurfaceShader::_ComputeTextureSourceHash() const
{
  TRACE_FUNCTION();

  size_t hash = 0;

  for (const HdPhShaderCode::NamedTextureHandle &namedHandle : _namedTextureHandles) {

    // Use name, texture object and sampling parameters.
    boost::hash_combine(hash, namedHandle.name);
    boost::hash_combine(hash, namedHandle.hash);
  }

  return hash;
}

void HdPhSurfaceShader::SetFragmentSource(const std::string &source)
{
  _fragmentSource      = source;
  _isValidComputedHash = false;
}

void HdPhSurfaceShader::SetGeometrySource(const std::string &source)
{
  _geometrySource      = source;
  _isValidComputedHash = false;
}

void HdPhSurfaceShader::SetParams(const HdPh_MaterialParamVector &params)
{
  _params              = params;
  _primvarNames        = _CollectPrimvarNames(_params);
  _isValidComputedHash = false;
}

void HdPhSurfaceShader::SetNamedTextureHandles(const NamedTextureHandleVector &namedTextureHandles)
{
  _namedTextureHandles              = namedTextureHandles;
  _isValidComputedTextureSourceHash = false;
}

void HdPhSurfaceShader::SetBufferSources(HdBufferSpecVector const &bufferSpecs,
                                         HdBufferSourceSharedPtrVector &&bufferSources,
                                         HdPhResourceRegistrySharedPtr const &resourceRegistry)
{
  if (bufferSpecs.empty()) {
    if (!_paramSpec.empty()) {
      _isValidComputedHash = false;
    }

    _paramSpec.clear();
    _paramArray.reset();
  }
  else {
    if (!_paramArray || _paramSpec != bufferSpecs) {
      _paramSpec = bufferSpecs;

      // establish a buffer range
      HdBufferArrayRangeSharedPtr range = resourceRegistry->AllocateShaderStorageBufferArrayRange(
          HdTokens->materialParams, bufferSpecs, HdBufferArrayUsageHint());

      if (!TF_VERIFY(range->IsValid())) {
        _paramArray.reset();
      }
      else {
        _paramArray = range;
      }
      _isValidComputedHash = false;
    }

    if (_paramArray->IsValid()) {
      if (!bufferSources.empty()) {
        resourceRegistry->AddSources(_paramArray, std::move(bufferSources));
      }
    }
  }
}

TfToken HdPhSurfaceShader::GetMaterialTag() const
{
  return _materialTag;
}

void HdPhSurfaceShader::SetMaterialTag(TfToken const &tag)
{
  _materialTag         = tag;
  _isValidComputedHash = false;
}

/// If the prim is based on asset, reload that asset.
void HdPhSurfaceShader::Reload()
{
  // Nothing to do, this shader's sources are externally managed.
}

/*static*/
bool HdPhSurfaceShader::CanAggregate(HdPhShaderCodeSharedPtr const &shaderA,
                                     HdPhShaderCodeSharedPtr const &shaderB)
{
  // Can aggregate if the shaders are identical.
  if (shaderA == shaderB) {
    return true;
  }

  HdBufferArrayRangeSharedPtr dataA = shaderA->GetShaderData();
  HdBufferArrayRangeSharedPtr dataB = shaderB->GetShaderData();

  bool dataIsAggregated = (dataA == dataB) || (dataA && dataA->IsAggregatedWith(dataB));

  // We can't aggregate if the shaders have data buffers that aren't
  // aggregated or if the shaders don't match.
  if (!dataIsAggregated || shaderA->ComputeHash() != shaderB->ComputeHash()) {
    return false;
  }

  // if (!GlfContextCaps::GetInstance().bindlessTextureEnabled) {
  // if (shaderA->ComputeTextureSourceHash() != shaderB->ComputeTextureSourceHash()) {
  // return false;
  // }
  // }

  return true;
}

TF_DEFINE_PRIVATE_TOKENS(_tokens,

                         (ptexFaceOffset)  // geometric shader

                         (displayMetallic)   // simple lighting shader
                         (displayRoughness)  // simple lighting shader

                         (hullColor)                // terminal shader
                         (hullOpacity)              // terminal shader
                         (scalarOverride)           // terminal shader
                         (scalarOverrideColorRamp)  // terminal shader
                         (selectedWeight)           // terminal shader

                         (indicatorColor)          // renderPass shader
                         (indicatorWeight)         // renderPass shader
                         (overrideColor)           // renderPass shader
                         (overrideWireframeColor)  // renderPass shader
                         (maskColor)               // renderPass shader
                         (maskWeight)              // renderPass shader
                         (wireframeColor)          // renderPass shader
);

static TfTokenVector const &_GetExtraIncludedShaderPrimvarNames()
{
  static const TfTokenVector primvarNames = {HdTokens->displayColor,
                                             HdTokens->displayOpacity,

                                             // Include a few ad hoc primvar names that
                                             // are used by the built-in material shading system.

                                             _tokens->ptexFaceOffset,

                                             _tokens->displayMetallic,
                                             _tokens->displayRoughness,

                                             _tokens->hullColor,
                                             _tokens->hullOpacity,
                                             _tokens->scalarOverride,
                                             _tokens->scalarOverrideColorRamp,
                                             _tokens->selectedWeight,

                                             _tokens->indicatorColor,
                                             _tokens->indicatorWeight,
                                             _tokens->overrideColor,
                                             _tokens->overrideWireframeColor,
                                             _tokens->maskColor,
                                             _tokens->maskWeight,
                                             _tokens->wireframeColor};
  return primvarNames;
}

static TfTokenVector _CollectPrimvarNames(const HdPh_MaterialParamVector &params)
{
  TfTokenVector primvarNames = _GetExtraIncludedShaderPrimvarNames();

  for (HdPh_MaterialParam const &param : params) {
    if (param.IsFallback()) {
      primvarNames.push_back(param.name);
    }
    else if (param.IsPrimvarRedirect()) {
      primvarNames.push_back(param.name);
      // primvar redirect connections are encoded as sampler coords
      primvarNames.insert(
          primvarNames.end(), param.samplerCoords.begin(), param.samplerCoords.end());
    }
    else if (param.IsTexture()) {
      // include sampler coords for textures
      primvarNames.insert(
          primvarNames.end(), param.samplerCoords.begin(), param.samplerCoords.end());
    }
    else if (param.IsAdditionalPrimvar()) {
      primvarNames.push_back(param.name);
    }
  }
  return primvarNames;
}

void HdPhSurfaceShader::AddResourcesFromTextures(ResourceContext &ctx) const
{
  // const bool bindlessTextureEnabled = GlfContextCaps::GetInstance().bindlessTextureEnabled;
  const bool bindlessTextureEnabled = true;

  // Add buffer sources for bindless texture handles (and
  // other texture metadata such as the sampling transform for
  // a field texture).
  HdBufferSourceSharedPtrVector result;
  HdPh_TextureBinder::ComputeBufferSources(
      GetNamedTextureHandles(), bindlessTextureEnabled, &result);

  if (!result.empty()) {
    ctx.AddSources(GetShaderData(), std::move(result));
  }
}

void HdPhSurfaceShader::AddFallbackValueToSpecsAndSources(
    const HdPh_MaterialParam &param,
    HdBufferSpecVector *const specs,
    HdBufferSourceSharedPtrVector *const sources)
{
  const TfToken sourceName(param.name.GetString() +
                           HdPh_ResourceBindingSuffixTokens->fallback.GetString());

  HdBufferSourceSharedPtr const source = std::make_shared<HdVtBufferSource>(sourceName,
                                                                            param.fallbackValue);
  source->GetBufferSpecs(specs);
  sources->push_back(std::move(source));
}

WABI_NAMESPACE_END
