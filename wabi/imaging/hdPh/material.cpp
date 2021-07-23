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
#include "wabi/imaging/hdPh/material.h"
#include "wabi/imaging/hdPh/debugCodes.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/primUtils.h"
#include "wabi/imaging/hdPh/renderBuffer.h"
#include "wabi/imaging/hdPh/renderParam.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/imaging/hdPh/surfaceShader.h"
#include "wabi/imaging/hdPh/textureBinder.h"
#include "wabi/imaging/hdPh/textureHandle.h"
#include "wabi/imaging/hdPh/textureObject.h"
#include "wabi/imaging/hdPh/tokens.h"

#include "wabi/imaging/hd/changeTracker.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/glf/contextCaps.h"
#include "wabi/imaging/hio/glslfx.h"

#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens, (limitSurfaceEvaluation)(opacity));

HioGlslfx *HdPhMaterial::_fallbackGlslfx = nullptr;

HdPhMaterial::HdPhMaterial(SdfPath const &id)
  : HdMaterial(id),
    _surfaceShader(std::make_shared<HdPhSurfaceShader>()),
    _isInitialized(false),
    _hasPtex(false),
    _hasLimitSurfaceEvaluation(false),
    _hasDisplacement(false),
    _materialTag(HdPhMaterialTagTokens->defaultMaterialTag),
    _textureHash(0)
{
  TF_DEBUG(HDPH_MATERIAL_ADDED).Msg("HdPhMaterial Created: %s\n", id.GetText());
}

HdPhMaterial::~HdPhMaterial()
{
  TF_DEBUG(HDPH_MATERIAL_REMOVED).Msg("HdPhMaterial Removed: %s\n", GetId().GetText());
}

// Check whether the texture node points to a render buffer and
// use information from it to get the texture identifier.
//
static HdPhTextureIdentifier _GetTextureIdentifier(HdPhMaterialNetwork::TextureDescriptor const &desc,
                                                   HdSceneDelegate *const sceneDelegate)
{
  if (!desc.useTexturePrimToFindTexture)
  {
    return desc.textureId;
  }

  // Get render buffer texture node is pointing to.
  if (HdPhRenderBuffer *const renderBuffer = dynamic_cast<HdPhRenderBuffer *>(
        sceneDelegate->GetRenderIndex().GetBprim(HdPrimTypeTokens->renderBuffer, desc.texturePrim)))
  {

    if (desc.type == HdTextureType::Uv)
    {
      return renderBuffer->GetTextureIdentifier(
        /* multiSampled = */ false);
    }

    TF_CODING_ERROR("Non-UV texture type in descriptor using render buffer.");
  }

  return HdPhTextureIdentifier();
}

static size_t _GetTextureHandleHash(HdPhTextureHandleSharedPtr const &textureHandle)
{
  const HdSamplerParameters &samplerParams = textureHandle->GetSamplerParameters();

  return TfHash::Combine(textureHandle->GetTextureObject()->GetTextureIdentifier(),
                         samplerParams.wrapS,
                         samplerParams.wrapT,
                         samplerParams.wrapR,
                         samplerParams.minFilter,
                         samplerParams.magFilter);
}

void HdPhMaterial::_ProcessTextureDescriptors(
  HdSceneDelegate *const sceneDelegate,
  HdPhResourceRegistrySharedPtr const &resourceRegistry,
  std::weak_ptr<HdPhShaderCode> const &shaderCode,
  HdPhMaterialNetwork::TextureDescriptorVector const &descs,
  HdPhShaderCode::NamedTextureHandleVector *const texturesFromPhoenix,
  HdBufferSpecVector *const specs,
  HdBufferSourceSharedPtrVector *const sources)
{
  const bool bindlessTextureEnabled = GlfContextCaps::GetInstance().bindlessTextureEnabled;

  for (HdPhMaterialNetwork::TextureDescriptor const &desc : descs)
  {
    HdPhTextureHandleSharedPtr const textureHandle = resourceRegistry->AllocateTextureHandle(
      _GetTextureIdentifier(desc, sceneDelegate),
      desc.type,
      desc.samplerParameters,
      desc.memoryRequest,
      bindlessTextureEnabled,
      shaderCode);

    // Note about batching hashes:
    // If this is our first sync, try to hash using the asset path.
    // If we're on our 2nd+ sync, just use the texture prim path.
    //
    // This will aggressively batch textured prims together as long as
    // they are 100% static; if they are dynamic, we assume that the
    // textures are dynamic, and we split the batches of textured prims.
    //
    // This tries to balance two competing concerns:
    // 1.) Static textured simple geometry (like billboard placeholders)
    //     really need to be batched together; otherwise, the render
    //     cost is dominated by the switching cost between batches.
    // 2.) Objects with animated textures will change their texture hash
    //     every frame.  If the hash is based on asset path, we need to
    //     rebuild batches every frame, which is untenable. If it's
    //     based on scene graph path (meaning split into its own batch),
    //     we don't need to update batching.
    //
    // Better batch invalidation (i.e., not global) would really help
    // us here, as would hints from the scene about how likely the
    // textures are to change.  Maybe in the future...

    texturesFromPhoenix->push_back(
      {desc.name,
       desc.type,
       textureHandle,
       _isInitialized ? hash_value(desc.texturePrim) : _GetTextureHandleHash(textureHandle)});
  }

  HdPh_TextureBinder::GetBufferSpecs(*texturesFromPhoenix, bindlessTextureEnabled, specs);
}

/* virtual */
void HdPhMaterial::Sync(HdSceneDelegate *sceneDelegate, HdRenderParam *renderParam, HdDirtyBits *dirtyBits)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPhResourceRegistrySharedPtr const &resourceRegistry = std::static_pointer_cast<HdPhResourceRegistry>(
    sceneDelegate->GetRenderIndex().GetResourceRegistry());

  HdDirtyBits bits = *dirtyBits;

  if (!(bits & DirtyResource) && !(bits & DirtyParams))
  {
    *dirtyBits = Clean;
    return;
  }

  bool needsRprimMaterialStateUpdate = false;
  bool markBatchesDirty = false;

  std::string fragmentSource;
  std::string geometrySource;
  VtDictionary materialMetadata;
  TfToken materialTag = _materialTag;
  HdPh_MaterialParamVector params;
  HdPhMaterialNetwork::TextureDescriptorVector textureDescriptors;

  VtValue vtMat = sceneDelegate->GetMaterialResource(GetId());
  if (vtMat.IsHolding<HdMaterialNetworkMap>())
  {
    HdMaterialNetworkMap const &hdNetworkMap = vtMat.UncheckedGet<HdMaterialNetworkMap>();
    if (!hdNetworkMap.terminals.empty() && !hdNetworkMap.map.empty())
    {
      _networkProcessor.ProcessMaterialNetwork(GetId(), hdNetworkMap, resourceRegistry.get());
      fragmentSource = _networkProcessor.GetFragmentCode();
      geometrySource = _networkProcessor.GetGeometryCode();
      materialMetadata = _networkProcessor.GetMetadata();
      materialTag = _networkProcessor.GetMaterialTag();
      params = _networkProcessor.GetMaterialParams();
      textureDescriptors = _networkProcessor.GetTextureDescriptors();
    }
  }

  if (fragmentSource.empty() && geometrySource.empty())
  {
    _InitFallbackShader();
    fragmentSource = _fallbackGlslfx->GetSurfaceSource();
    // Note that we don't want displacement on purpose for the
    // fallback material.
    geometrySource = std::string();
    materialMetadata = _fallbackGlslfx->GetMetadata();
  }

  // If we're updating the fragment or geometry source, we need to rebatch
  // anything that uses this material.
  std::string const &oldFragmentSource = _surfaceShader->GetSource(HdShaderTokens->fragmentShader);
  std::string const &oldGeometrySource = _surfaceShader->GetSource(HdShaderTokens->geometryShader);

  markBatchesDirty |= (oldFragmentSource != fragmentSource) || (oldGeometrySource != geometrySource);

  _surfaceShader->SetFragmentSource(fragmentSource);
  _surfaceShader->SetGeometrySource(geometrySource);

  bool hasDisplacement = !(geometrySource.empty());

  if (_hasDisplacement != hasDisplacement)
  {
    _hasDisplacement = hasDisplacement;
    needsRprimMaterialStateUpdate = true;
  }

  bool hasLimitSurfaceEvaluation = _GetHasLimitSurfaceEvaluation(materialMetadata);

  if (_hasLimitSurfaceEvaluation != hasLimitSurfaceEvaluation)
  {
    _hasLimitSurfaceEvaluation = hasLimitSurfaceEvaluation;
    needsRprimMaterialStateUpdate = true;
  }

  if (_materialTag != materialTag)
  {
    _materialTag = materialTag;
    _surfaceShader->SetMaterialTag(_materialTag);
    needsRprimMaterialStateUpdate = true;

    // If the material tag changes, we'll need to rebatch.
    markBatchesDirty = true;
  }

  _surfaceShader->SetEnabledPrimvarFiltering(true);

  //
  // Update material parameters
  //
  _surfaceShader->SetParams(params);

  HdBufferSpecVector specs;
  HdBufferSourceSharedPtrVector sources;

  bool hasPtex = false;
  for (HdPh_MaterialParam const &param : params)
  {
    if (param.IsPrimvarRedirect() || param.IsFallback() || param.IsTransform2d())
    {
      HdPhSurfaceShader::AddFallbackValueToSpecsAndSources(param, &specs, &sources);
    } else if (param.IsTexture())
    {
      // Fallback value only supported for Uv and Field textures.
      if (param.textureType == HdTextureType::Uv || param.textureType == HdTextureType::Field)
      {
        HdPhSurfaceShader::AddFallbackValueToSpecsAndSources(param, &specs, &sources);
      }
      if (param.textureType == HdTextureType::Ptex)
      {
        hasPtex = true;
      }
    }
  }

  // Textures created using Phoenix texture system.
  HdPhShaderCode::NamedTextureHandleVector textures;

  _ProcessTextureDescriptors(sceneDelegate,
                             resourceRegistry,
                             _surfaceShader,
                             textureDescriptors,
                             &textures,
                             &specs,
                             &sources);

  // Check if the texture hash has changed; if so, we need to ask for a
  // re-batch.  We only look at NamedTextureHandles because the legacy system
  // hashes based on scene graph path, meaning each textured gprim gets its
  // own batch.
  size_t textureHash = 0;
  for (HdPhShaderCode::NamedTextureHandle const &tex : textures)
  {
    textureHash = TfHash::Combine(textureHash, tex.hash);
  }

  if (_textureHash != textureHash)
  {
    _textureHash = textureHash;
    markBatchesDirty = true;
  }

  _surfaceShader->SetNamedTextureHandles(textures);
  _surfaceShader->SetBufferSources(specs, std::move(sources), resourceRegistry);

  if (_hasPtex != hasPtex)
  {
    _hasPtex = hasPtex;
    needsRprimMaterialStateUpdate = true;
  }

  if (markBatchesDirty && _isInitialized)
  {
    // Only invalidate batches if this isn't our first round through sync.
    // If this is the initial sync, we haven't formed batches yet.
    HdPhMarkDrawBatchesDirty(renderParam);
  }

  if (needsRprimMaterialStateUpdate && _isInitialized)
  {
    // XXX Forcing rprims to have a dirty material id to re-evaluate
    // their material state as we don't know which rprims are bound to
    // this one. We can skip this invalidation the first time this
    // material is Sync'd since any affected Rprim should already be
    // marked with a dirty material id.
    HdChangeTracker &changeTracker = sceneDelegate->GetRenderIndex().GetChangeTracker();
    changeTracker.MarkAllRprimsDirty(HdChangeTracker::DirtyMaterialId);
  }

  _isInitialized = true;
  *dirtyBits = Clean;
}

/*virtual*/
void HdPhMaterial::Finalize(HdRenderParam *renderParam)
{
  // Flag GC to reclaim resources owned by the surface shader.
  HdPhMarkGarbageCollectionNeeded(renderParam);
}

bool HdPhMaterial::_GetHasLimitSurfaceEvaluation(VtDictionary const &metadata) const
{
  VtValue value = TfMapLookupByValue(metadata, _tokens->limitSurfaceEvaluation, VtValue());
  return value.IsHolding<bool>() && value.Get<bool>();
}

// virtual
HdDirtyBits HdPhMaterial::GetInitialDirtyBitsMask() const
{
  return AllDirty;
}

HdPhShaderCodeSharedPtr HdPhMaterial::GetShaderCode() const
{
  return _surfaceShader;
}

void HdPhMaterial::SetSurfaceShader(HdPhSurfaceShaderSharedPtr &shaderCode)
{
  _surfaceShader = shaderCode;
}

void HdPhMaterial::_InitFallbackShader()
{
  if (_fallbackGlslfx != nullptr)
  {
    return;
  }

  const TfToken &filePath = HdPhPackageFallbackSurfaceShader();

  _fallbackGlslfx = new HioGlslfx(filePath);

  // Check fallback shader loaded, if not continue with the invalid shader
  // this would mean the shader compilation fails and the prim would not
  // be drawn.
  TF_VERIFY(_fallbackGlslfx->IsValid(), "Failed to load fallback surface shader!");
}

WABI_NAMESPACE_END
