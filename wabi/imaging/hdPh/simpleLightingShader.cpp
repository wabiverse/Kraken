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
#include "wabi/imaging/hdPh/simpleLightingShader.h"
#include "wabi/imaging/hdPh/domeLightComputations.h"
#include "wabi/imaging/hdPh/dynamicUvTextureObject.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/textureBinder.h"
#include "wabi/imaging/hdPh/textureHandle.h"
#include "wabi/imaging/hdPh/textureIdentifier.h"
#include "wabi/imaging/hdPh/textureObject.h"
#include "wabi/imaging/hdPh/tokens.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/sceneDelegate.h"

#include "wabi/imaging/hf/perfLog.h"

#include "wabi/imaging/hio/glslfx.h"

#include "wabi/imaging/glf/bindingMap.h"
#include "wabi/imaging/glf/simpleLightingContext.h"

#include "wabi/base/tf/staticTokens.h"

#include <boost/functional/hash.hpp>

#include <sstream>

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens, (domeLightIrradiance)(domeLightPrefilter)(domeLightBRDF));

HdPhSimpleLightingShader::HdPhSimpleLightingShader()
    : _lightingContext(GlfSimpleLightingContext::New()),
      _bindingMap(TfCreateRefPtr(new GlfBindingMap())),
      _useLighting(true),
      _glslfx(std::make_unique<HioGlslfx>(HdPhPackageSimpleLightingShader()))
{}

HdPhSimpleLightingShader::~HdPhSimpleLightingShader() = default;

/* virtual */
HdPhSimpleLightingShader::ID HdPhSimpleLightingShader::ComputeHash() const
{
  HD_TRACE_FUNCTION();

  const TfToken glslfxFile = HdPhPackageSimpleLightingShader();
  const size_t numLights   = _useLighting ? _lightingContext->GetNumLightsUsed() : 0;
  const bool useShadows    = _useLighting ? _lightingContext->GetUseShadows() : false;
  const size_t numShadows  = useShadows ? _lightingContext->ComputeNumShadowsUsed() : 0;

  size_t hash = glslfxFile.Hash();
  boost::hash_combine(hash, numLights);
  boost::hash_combine(hash, useShadows);
  boost::hash_combine(hash, numShadows);
  boost::hash_combine(hash, _lightingContext->ComputeShaderSourceHash());

  return (ID)hash;
}

/* virtual */
std::string HdPhSimpleLightingShader::GetSource(TfToken const &shaderStageKey) const
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  const std::string source = _glslfx->GetSource(shaderStageKey);

  if (source.empty())
    return source;

  std::stringstream defineStream;
  const size_t numLights  = _useLighting ? _lightingContext->GetNumLightsUsed() : 0;
  const bool useShadows   = _useLighting ? _lightingContext->GetUseShadows() : false;
  const size_t numShadows = useShadows ? _lightingContext->ComputeNumShadowsUsed() : 0;
  defineStream << "#define NUM_LIGHTS " << numLights << "\n";
  defineStream << "#define USE_SHADOWS " << (int)(useShadows) << "\n";
  defineStream << "#define NUM_SHADOWS " << numShadows << "\n";
  if (useShadows) {
    const bool useBindlessShadowMaps = GlfSimpleShadowArray::GetBindlessShadowMapsEnabled();
    ;
    defineStream << "#define USE_BINDLESS_SHADOW_TEXTURES " << int(useBindlessShadowMaps) << "\n";
  }

  const std::string postSurfaceShader = _lightingContext->ComputeShaderSource(shaderStageKey);

  if (!postSurfaceShader.empty()) {
    defineStream << "#define HD_HAS_postSurfaceShader\n";
  }

  return defineStream.str() + postSurfaceShader + source;
}

/* virtual */
void HdPhSimpleLightingShader::SetCamera(GfMatrix4d const &worldToViewMatrix,
                                         GfMatrix4d const &projectionMatrix)
{
  _lightingContext->SetCamera(worldToViewMatrix, projectionMatrix);
}

static bool _HasDomeLight(GlfSimpleLightingContextRefPtr const &ctx)
{
  for (auto const &light : ctx->GetLights()) {
    if (light.IsDomeLight()) {
      return true;
    }
  }
  return false;
}

/* virtual */
void HdPhSimpleLightingShader::BindResources(const int program,
                                             HdPh_ResourceBinder const &binder,
                                             HdRenderPassState const &state)
{
  // XXX: we'd like to use HdPh_ResourceBinder instead of GlfBindingMap.
  _bindingMap->ResetUniformBindings(binder.GetNumReservedUniformBlockLocations());
  _lightingContext->InitUniformBlockBindings(_bindingMap);
  _bindingMap->AssignUniformBindingsToProgram(program);
  _lightingContext->BindUniformBlocks(_bindingMap);

  _bindingMap->ResetSamplerBindings(binder.GetNumReservedTextureUnits());
  _lightingContext->InitSamplerUnitBindings(_bindingMap);
  _bindingMap->AssignSamplerUnitsToProgram(program);
  _lightingContext->BindSamplers(_bindingMap);

  HdPh_TextureBinder::BindResources(binder, false, _namedTextureHandles);

  binder.BindShaderResources(this);
}

/* virtual */
void HdPhSimpleLightingShader::UnbindResources(const int program,
                                               HdPh_ResourceBinder const &binder,
                                               HdRenderPassState const &state)
{
  // XXX: we'd like to use HdPh_ResourceBinder instead of GlfBindingMap.
  //
  _lightingContext->UnbindSamplers(_bindingMap);

  HdPh_TextureBinder::UnbindResources(binder, false, _namedTextureHandles);
}

/*virtual*/
void HdPhSimpleLightingShader::AddBindings(HdBindingRequestVector *customBindings)
{
  // For now we assume that the only simple light with a texture is
  // a domeLight (ignoring RectLights, and multiple domeLights)

  _lightTextureParams.clear();
  if (_HasDomeLight(_lightingContext)) {
    // irradiance map
    _lightTextureParams.push_back(HdPh_MaterialParam(HdPh_MaterialParam::ParamTypeTexture,
                                                     _tokens->domeLightIrradiance,
                                                     VtValue(GfVec4f(0.0)),
                                                     TfTokenVector(),
                                                     HdTextureType::Uv));
    // prefilter map
    _lightTextureParams.push_back(HdPh_MaterialParam(HdPh_MaterialParam::ParamTypeTexture,
                                                     _tokens->domeLightPrefilter,
                                                     VtValue(GfVec4f(0.0)),
                                                     TfTokenVector(),
                                                     HdTextureType::Uv));
    // BRDF texture
    _lightTextureParams.push_back(HdPh_MaterialParam(HdPh_MaterialParam::ParamTypeTexture,
                                                     _tokens->domeLightBRDF,
                                                     VtValue(GfVec4f(0.0)),
                                                     TfTokenVector(),
                                                     HdTextureType::Uv));
  }
}

HdPh_MaterialParamVector const &HdPhSimpleLightingShader::GetParams() const
{
  return _lightTextureParams;
}

void HdPhSimpleLightingShader::SetLightingStateFromOpenGL()
{
  _lightingContext->SetStateFromOpenGL();
}

void HdPhSimpleLightingShader::SetLightingState(GlfSimpleLightingContextPtr const &src)
{
  if (src) {
    _useLighting = true;
    _lightingContext->SetUseLighting(!src->GetLights().empty());
    _lightingContext->SetLights(src->GetLights());
    _lightingContext->SetMaterial(src->GetMaterial());
    _lightingContext->SetSceneAmbient(src->GetSceneAmbient());
    _lightingContext->SetShadows(src->GetShadows());
  }
  else {
    // XXX:
    // if src is null, turn off lights (this is temporary used for shadowmap drawing).
    // see GprimUsdBaseIcBatch::Draw()
    _useLighting = false;
  }
}

static const std::string &_GetResolvedDomeLightEnvironmentFilePath(
    const GlfSimpleLightingContextRefPtr &ctx)
{
  static const std::string empty;

  if (!ctx) {
    return empty;
  }

  const GlfSimpleLightVector &lights = ctx->GetLights();
  for (auto it = lights.rbegin(); it != lights.rend(); ++it) {
    if (it->IsDomeLight()) {
      const SdfAssetPath &path     = it->GetDomeLightTextureFile();
      const std::string &assetPath = path.GetAssetPath();
      if (assetPath.empty()) {
        TF_WARN("Dome light has no texture asset path.");
        return empty;
      }

      const std::string &resolvedPath = path.GetResolvedPath();
      if (resolvedPath.empty()) {
        TF_WARN("Texture asset path '%s' for dome light could not be resolved.",
                assetPath.c_str());
      }
      return resolvedPath;
    }
  }

  return empty;
}

const HdPhTextureHandleSharedPtr &HdPhSimpleLightingShader::GetTextureHandle(
    const TfToken &name) const
{
  for (auto const &namedTextureHandle : _namedTextureHandles) {
    if (namedTextureHandle.name == name) {
      return namedTextureHandle.handle;
    }
  }

  static const HdPhTextureHandleSharedPtr empty;
  return empty;
}

static HdPhShaderCode::NamedTextureHandle _MakeNamedTextureHandle(
    const TfToken &name,
    const std::string &texturePath,
    const HdWrap wrapModeS,
    const HdWrap wrapModeT,
    const HdWrap wrapModeR,
    const HdMinFilter minFilter,
    HdPhResourceRegistry *const resourceRegistry,
    HdPhShaderCodeSharedPtr const &shader)
{
  const HdPhTextureIdentifier textureId(TfToken(texturePath + "[" + name.GetString() + "]"),
                                        std::make_unique<HdPhDynamicUvSubtextureIdentifier>());

  const HdSamplerParameters samplerParameters{
      wrapModeS, wrapModeT, wrapModeR, minFilter, HdMagFilterLinear};

  HdPhTextureHandleSharedPtr const textureHandle = resourceRegistry->AllocateTextureHandle(
      textureId,
      HdTextureType::Uv,
      samplerParameters,
      /* memoryRequest = */ 0,
      /* createBindlessHandle = */ false,
      shader);

  return {name, HdTextureType::Uv, textureHandle, name.Hash()};
}

void HdPhSimpleLightingShader::AllocateTextureHandles(HdSceneDelegate *const delegate)
{
  const std::string &resolvedPath = _GetResolvedDomeLightEnvironmentFilePath(_lightingContext);
  if (resolvedPath.empty()) {
    _domeLightEnvironmentTextureHandle = nullptr;
    _namedTextureHandles.clear();
    return;
  }

  if (_domeLightEnvironmentTextureHandle) {
    HdPhTextureObjectSharedPtr const &textureObject =
        _domeLightEnvironmentTextureHandle->GetTextureObject();
    HdPhTextureIdentifier const &textureId = textureObject->GetTextureIdentifier();
    if (textureId.GetFilePath() == resolvedPath) {
      // Same environment map, no need to recompute
      // dome light textures.
      return;
    }
  }

  HdPhResourceRegistry *const resourceRegistry = dynamic_cast<HdPhResourceRegistry *>(
      delegate->GetRenderIndex().GetResourceRegistry().get());
  if (!TF_VERIFY(resourceRegistry)) {
    return;
  }

  const HdPhTextureIdentifier textureId(TfToken(resolvedPath),
                                        std::make_unique<HdPhAssetUvSubtextureIdentifier>(
                                            /* flipVertically = */ true,
                                            /* premultiplyAlpha = */ false,
                                            /* sourceColorSpace = */ HdPhTokens->colorSpaceAuto));

  static const HdSamplerParameters envSamplerParameters{
      HdWrapRepeat, HdWrapClamp, HdWrapClamp, HdMinFilterLinearMipmapLinear, HdMagFilterLinear};

  _domeLightEnvironmentTextureHandle = resourceRegistry->AllocateTextureHandle(
      textureId,
      HdTextureType::Uv,
      envSamplerParameters,
      /* targetMemory = */ 0,
      /* createBindlessHandle = */ false,
      shared_from_this());

  _namedTextureHandles = {_MakeNamedTextureHandle(_tokens->domeLightIrradiance,
                                                  resolvedPath,
                                                  HdWrapRepeat,
                                                  HdWrapClamp,
                                                  HdWrapRepeat,
                                                  HdMinFilterLinear,
                                                  resourceRegistry,
                                                  shared_from_this()),

                          _MakeNamedTextureHandle(_tokens->domeLightPrefilter,
                                                  resolvedPath,
                                                  HdWrapRepeat,
                                                  HdWrapClamp,
                                                  HdWrapRepeat,
                                                  HdMinFilterLinearMipmapLinear,
                                                  resourceRegistry,
                                                  shared_from_this()),

                          _MakeNamedTextureHandle(_tokens->domeLightBRDF,
                                                  resolvedPath,
                                                  HdWrapClamp,
                                                  HdWrapClamp,
                                                  HdWrapClamp,
                                                  HdMinFilterLinear,
                                                  resourceRegistry,
                                                  shared_from_this())};
}

void HdPhSimpleLightingShader::AddResourcesFromTextures(ResourceContext &ctx) const
{
  if (!_domeLightEnvironmentTextureHandle) {
    // No dome lights, bail.
    return;
  }

  // Non-const weak pointer of this
  HdPhSimpleLightingShaderPtr const thisShader =
      std::dynamic_pointer_cast<HdPhSimpleLightingShader>(
          std::const_pointer_cast<HdPhShaderCode, const HdPhShaderCode>(shared_from_this()));

  // Irriadiance map computations.
  ctx.AddComputation(
      nullptr,
      std::make_shared<HdPh_DomeLightComputationGPU>(_tokens->domeLightIrradiance, thisShader),
      HdPhComputeQueueZero);

  // Calculate the number of mips for the prefilter texture
  // Note that the size of the prefilter texture is half the size of the
  // original Environment Map (srcTextureObject)
  const HdPhUvTextureObject *const srcTextureObject = dynamic_cast<HdPhUvTextureObject *>(
      _domeLightEnvironmentTextureHandle->GetTextureObject().get());
  if (!TF_VERIFY(srcTextureObject)) {
    return;
  }
  const HgiTexture *const srcTexture = srcTextureObject->GetTexture().Get();
  if (!srcTexture) {
    TF_WARN("Invalid texture for dome light environment map at %s",
            srcTextureObject->GetTextureIdentifier().GetFilePath().GetText());
    return;
  }
  const GfVec3i srcDim = srcTexture->GetDescriptor().dimensions;

  const unsigned int numPrefilterLevels = (unsigned int)std::log2(std::max(srcDim[0], srcDim[1]));

  // Prefilter map computations. mipLevel = 0 allocates texture.
  for (unsigned int mipLevel = 0; mipLevel < numPrefilterLevels; ++mipLevel) {
    const float roughness = (float)mipLevel / (float)(numPrefilterLevels - 1);

    ctx.AddComputation(
        nullptr,
        std::make_shared<HdPh_DomeLightComputationGPU>(
            _tokens->domeLightPrefilter, thisShader, numPrefilterLevels, mipLevel, roughness),
        HdPhComputeQueueZero);
  }

  // Brdf map computation
  ctx.AddComputation(
      nullptr,
      std::make_shared<HdPh_DomeLightComputationGPU>(_tokens->domeLightBRDF, thisShader),
      HdPhComputeQueueZero);
}

HdPhShaderCode::NamedTextureHandleVector const &HdPhSimpleLightingShader::GetNamedTextureHandles()
    const
{
  return _namedTextureHandles;
}

WABI_NAMESPACE_END
