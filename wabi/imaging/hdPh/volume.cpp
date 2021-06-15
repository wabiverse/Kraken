//
// Copyright 2019 Pixar
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
#include "wabi/imaging/hdPh/volume.h"

#include "wabi/imaging/hdPh/drawItem.h"
#include "wabi/imaging/hdPh/field.h"
#include "wabi/imaging/hdPh/material.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/primUtils.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/surfaceShader.h"
#include "wabi/imaging/hdPh/textureBinder.h"
#include "wabi/imaging/hdPh/tokens.h"
#include "wabi/imaging/hdPh/volumeShader.h"
#include "wabi/imaging/hdPh/volumeShaderKey.h"

#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/imaging/hf/diagnostic.h"

#include "wabi/imaging/hio/glslfx.h"

#include "wabi/imaging/glf/contextCaps.h"

#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_fallbackShaderTokens,

                         (density)(emission));

const float HdPhVolume::defaultStepSize = 1.0f;
const float HdPhVolume::defaultStepSizeLighting = 10.0f;
const float HdPhVolume::defaultMaxTextureMemoryPerField = 128.0f;

HdPhVolume::HdPhVolume(SdfPath const &id) : HdVolume(id)
{}

HdPhVolume::~HdPhVolume() = default;

// Dirty bits requiring recomputing the material shader and the
// bounding box.
static const int _shaderAndBBoxComputationDirtyBitsMask = HdChangeTracker::Clean |
                                                          HdChangeTracker::DirtyExtent |
                                                          HdChangeTracker::DirtyMaterialId |
                                                          HdChangeTracker::DirtyRepr |
                                                          HdChangeTracker::DirtyVolumeField;

static const int _initialDirtyBitsMask = _shaderAndBBoxComputationDirtyBitsMask |
                                         HdChangeTracker::DirtyPrimID | HdChangeTracker::DirtyPrimvar |
                                         HdChangeTracker::DirtyTransform | HdChangeTracker::DirtyVisibility |
                                         HdChangeTracker::DirtyInstancer;

HdDirtyBits HdPhVolume::GetInitialDirtyBitsMask() const
{
  int mask = _initialDirtyBitsMask;
  return (HdDirtyBits)mask;
}

HdDirtyBits HdPhVolume::_PropagateDirtyBits(HdDirtyBits bits) const
{
  return bits;
}

void HdPhVolume::_InitRepr(TfToken const &reprToken, HdDirtyBits *dirtyBits)
{
  // All representations point to _volumeRepr.
  if (!_volumeRepr) {
    _volumeRepr = std::make_shared<HdRepr>();
    _volumeRepr->AddDrawItem(std::make_unique<HdPhDrawItem>(&_sharedData));
    *dirtyBits |= HdChangeTracker::NewRepr;
  }

  _ReprVector::iterator it = std::find_if(_reprs.begin(), _reprs.end(), _ReprComparator(reprToken));
  bool isNew = it == _reprs.end();
  if (isNew) {
    // add new repr
    it = _reprs.insert(_reprs.end(), std::make_pair(reprToken, _volumeRepr));
  }
}

void HdPhVolume::Sync(HdSceneDelegate *delegate,
                      HdRenderParam *renderParam,
                      HdDirtyBits *dirtyBits,
                      TfToken const &reprToken)
{
  if (*dirtyBits & HdChangeTracker::DirtyMaterialId) {
    HdPhSetMaterialId(delegate, renderParam, this);
    SetMaterialTag(HdPhMaterialTagTokens->volume);
  }

  _UpdateRepr(delegate, renderParam, reprToken, dirtyBits);

  // This clears all the non-custom dirty bits. This ensures that the rprim
  // doesn't have pending dirty bits that add it to the dirty list every
  // frame.
  // XXX: GetInitialDirtyBitsMask sets certain dirty bits that aren't
  // reset (e.g. DirtyExtent, DirtyPrimID) that make this necessary.
  *dirtyBits &= ~HdChangeTracker::AllSceneDirtyBits;
}

void HdPhVolume::Finalize(HdRenderParam *renderParam)
{
  HdPhMarkGarbageCollectionNeeded(renderParam);
}

void HdPhVolume::_UpdateRepr(HdSceneDelegate *sceneDelegate,
                             HdRenderParam *renderParam,
                             TfToken const &reprToken,
                             HdDirtyBits *dirtyBits)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdReprSharedPtr const &curRepr = _volumeRepr;

  if (TfDebug::IsEnabled(HD_RPRIM_UPDATED)) {
    HdChangeTracker::DumpDirtyBits(*dirtyBits);
  }

  HdPhDrawItem *const drawItem = static_cast<HdPhDrawItem *>(curRepr->GetDrawItem(0));

  if (HdChangeTracker::IsDirty(*dirtyBits)) {
    _UpdateDrawItem(sceneDelegate, renderParam, drawItem, dirtyBits);
  }

  *dirtyBits &= ~HdChangeTracker::NewRepr;
}

namespace {

// Fallback volume shader created from shaders/fallbackVolume.glslfx
HdPhShaderCodeSharedPtr _MakeFallbackVolumeShader()
{
  const HioGlslfx glslfx(HdPhPackageFallbackVolumeShader());

  // Note that we use HdPhSurfaceShader for a volume shader.
  // Despite its name, HdPhSurfaceShader is really just a pair of
  // GLSL code and bindings and not specific to surface shading.
  HdPhSurfaceShaderSharedPtr const result = std::make_shared<HdPhSurfaceShader>();

  result->SetFragmentSource(glslfx.GetVolumeSource());
  result->SetParams({HdPh_MaterialParam(HdPh_MaterialParam::ParamTypeFieldRedirect,
                                        _fallbackShaderTokens->density,
                                        VtValue(0.0f),
                                        {_fallbackShaderTokens->density}),
                     HdPh_MaterialParam(HdPh_MaterialParam::ParamTypeFieldRedirect,
                                        _fallbackShaderTokens->emission,
                                        VtValue(GfVec3f(0.0, 0.0, 0.0)),
                                        {_fallbackShaderTokens->emission})});

  return result;
}

HdPhShaderCodeSharedPtr _ComputeVolumeShader(const HdPhMaterial *const material)
{
  if (material) {
    // Use the shader from the HdPhMaterial as volume shader.
    //
    // Note that rprims should query the material whether they want
    // a surface or volume shader instead of just asking for "some"
    // shader with HdPhMaterial::GetShaderCode().
    // We can use HdPhMaterial::GetShaderCode() here because the
    // UsdImagingGLHydraMaterialAdapter is following the outputs:volume
    // input of a material if the outputs:surface is unconnected.
    //
    // We should revisit the API an rprim is using to ask HdPhMaterial
    // for a shader once we switched over to HdMaterialNetworkMap's.
    return material->GetShaderCode();
  }
  else {
    // Instantiate fallback volume shader only once
    //
    // Note that the default HdPhMaterial provides a fallback surface
    // shader and we need a volume shader, so we create the shader here
    // ourselves.
    static const HdPhShaderCodeSharedPtr fallbackVolumeShader = _MakeFallbackVolumeShader();
    return fallbackVolumeShader;
  }
}

// A map from name to HdPhVolumeFieldDescriptor (identifying a
// field prim).
//
// Initialized from a volume prim identified by its path. In the usd world,
// this map is created by following the field:NAME relationships on the volume
// prim to the targeted field prims. The information identifiying the field
// prim is inserted under the key NAME.
//
class _NameToFieldDescriptor {
 public:
  // Get information from scene delegate and create map.
  //
  // Issue validation error if relationship did not target a field prim.
  //
  _NameToFieldDescriptor(HdSceneDelegate *const sceneDelegate, const SdfPath &id)
    : _descriptors(sceneDelegate->GetVolumeFieldDescriptors(id))
  {
    for (const HdVolumeFieldDescriptor &desc : _descriptors) {
      if (dynamic_cast<HdPhField *>(
            sceneDelegate->GetRenderIndex().GetBprim(desc.fieldPrimType, desc.fieldId))) {

        _nameToDescriptor.insert({desc.fieldName, &desc});
      }
      else {
        HF_VALIDATION_WARN(
          id, "Volume has field relationship to non-field prim %s.", desc.fieldId.GetText());
      }
    }
  }

  // Get information identifiying field prim associated to given name.
  //
  // Returns nullptr if no such field prim. Lifetime of returned object
  // is tied to _NameToFieldDescriptor.
  //
  const HdVolumeFieldDescriptor *GetDescriptor(const TfToken &name) const
  {
    const auto it = _nameToDescriptor.find(name);
    if (it == _nameToDescriptor.end()) {
      return nullptr;
    }
    return it->second;
  }

 private:
  using _NameToDescriptor =
    std::unordered_map<TfToken, const HdVolumeFieldDescriptor *, TfToken::HashFunctor>;
  HdVolumeFieldDescriptorVector _descriptors;
  _NameToDescriptor _nameToDescriptor;
};

// Add GLSL code such as "HdGet_density(vec3 p)" for sampling the fields
// to the volume shader code and add necessary 3d textures and other
// parameters and buffer sources to the resulting HdPh_VolumeShader.
// HdMaterialParam's are consulted to figure out the names of the fields
// to sample and the names of the associated sampling functions to generate.
//
// The resulting shader can also fill the points bar of the volume computed
// from the bounding box of the volume.
//
HdPh_VolumeShaderSharedPtr _ComputeMaterialShader(HdSceneDelegate *const sceneDelegate,
                                                  const SdfPath &id,
                                                  const HdPhShaderCodeSharedPtr &volumeShader,
                                                  const GfRange3d &authoredExtents)
{
  TRACE_FUNCTION();

  HdPhResourceRegistrySharedPtr const resourceRegistry = std::static_pointer_cast<HdPhResourceRegistry>(
    sceneDelegate->GetRenderIndex().GetResourceRegistry());

  // Generate new shader from volume shader
  HdPh_VolumeShaderSharedPtr const result = std::make_shared<HdPh_VolumeShader>(
    sceneDelegate->GetRenderIndex().GetRenderDelegate());

  // Buffer specs and source for the shader BAR
  HdBufferSpecVector bufferSpecs;
  HdBufferSourceSharedPtrVector bufferSources;

  // The names of the fields read by field readers.
  std::set<TfToken> fieldNames;

  // Make a copy of the original params
  HdPh_MaterialParamVector params = volumeShader->GetParams();

  for (const auto &param : params) {
    // Scan original parameters...
    if (param.IsFieldRedirect() || param.IsPrimvarRedirect() || param.IsFallback()) {
      // Add fallback values for parameters
      HdPhSurfaceShader::AddFallbackValueToSpecsAndSources(param, &bufferSpecs, &bufferSources);

      if (param.IsFieldRedirect()) {
        // Determine the name of the field the field reader requests.
        TfTokenVector const &names = param.samplerCoords;
        if (!names.empty()) {
          fieldNames.insert(names[0]);
        }
      }
    }
    // Ignoring 2D texture parameters for volumes.
  }

  // Note that it is a requirement of HdPh_VolumeShader that
  // namedTextureHandles and fieldDescs line up.
  HdPhShaderCode::NamedTextureHandleVector namedTextureHandles;
  HdVolumeFieldDescriptorVector fieldDescs;

  const _NameToFieldDescriptor _nameToFieldDescriptor(sceneDelegate, id);

  // For each requested field name, record the information needed to
  // allocate the necessary texture later:
  // - a texture HdPh_MaterialParam
  // - an HdVolumeFieldDescriptor identifying the HdPhField prim holding
  //   the path to the texture
  // - a HdPhShader::NamedTextureHandle initialized with a null-handle.
  //
  for (const auto &fieldName : fieldNames) {
    // See whether we have the the field in the volume field
    // descriptors given to us by the scene delegate.
    const HdVolumeFieldDescriptor *const desc = _nameToFieldDescriptor.GetDescriptor(fieldName);
    if (!desc) {
      // Invalid field prim, skip.
      continue;
    }

    // Record field descriptor
    fieldDescs.push_back(*desc);

    const TfToken textureName(fieldName.GetString() + HdPh_ResourceBindingSuffixTokens->texture.GetString());
    static const HdTextureType textureType = HdTextureType::Field;

    // Produce HdGet_FIELDNAME_texture(vec3 p) to sample
    // the texture.
    const HdPh_MaterialParam param(
      HdPh_MaterialParam::ParamTypeTexture, textureName, VtValue(GfVec4f(0)), TfTokenVector(), textureType);

    HdPhSurfaceShader::AddFallbackValueToSpecsAndSources(param, &bufferSpecs, &bufferSources);

    params.push_back(param);

    namedTextureHandles.push_back({textureName, textureType, nullptr, desc->fieldId.GetHash()});
  }

  const bool bindlessTextureEnabled = GlfContextCaps::GetInstance().bindlessTextureEnabled;

  // Get buffer specs for textures (i.e., for
  // field sampling transforms and bindless texture handles).
  HdPh_TextureBinder::GetBufferSpecs(namedTextureHandles, bindlessTextureEnabled, &bufferSpecs);

  // Create params (so that HdGet_... are created) and buffer specs,
  // to communicate volume bounding box and sample distance to shader.
  HdPh_VolumeShader::GetParamsAndBufferSpecsForBBoxAndSampleDistance(&params, &bufferSpecs);

  const bool hasField = !namedTextureHandles.empty();

  // If there is a field, we postpone giving buffer sources for
  // the volume bounding box until after the textures have been
  // committed.
  if (!hasField) {
    HdPh_VolumeShader::GetBufferSourcesForBBoxAndSampleDistance({GfBBox3d(authoredExtents), 1.0f},
                                                                &bufferSources);
  }

  // Make volume shader responsible if we have fields with bounding
  // boxes.
  result->SetFillsPointsBar(hasField);
  result->SetParams(params);
  result->SetBufferSources(bufferSpecs, std::move(bufferSources), resourceRegistry);
  result->SetNamedTextureHandles(namedTextureHandles);
  result->SetFieldDescriptors(fieldDescs);

  // Append the volume shader (calling into the GLSL functions
  // generated above)
  result->SetFragmentSource(volumeShader->GetSource(HdShaderTokens->fragmentShader));

  return result;
}

VtValue _ComputeBBoxVertices(GfRange3d const &range)
{
  VtVec3fArray result(8);

  const GfVec3d min = HdPh_VolumeShader::GetSafeMin(range);
  const GfVec3d &max = HdPh_VolumeShader::GetSafeMax(range);

  int i = 0;

  for (const double x : {min[0], max[0]}) {
    for (const double y : {min[1], max[1]}) {
      for (const double z : {min[2], max[2]}) {
        result[i] = GfVec3f(x, y, z);
        i++;
      }
    }
  }

  return VtValue(result);
}

const VtValue &_GetCubeTriangleIndices()
{
  static const VtValue result(VtVec3iArray{GfVec3i(1, 3, 2),
                                           GfVec3i(0, 1, 2),

                                           GfVec3i(7, 5, 4),
                                           GfVec3i(6, 7, 4),

                                           GfVec3i(5, 1, 0),
                                           GfVec3i(4, 5, 0),

                                           GfVec3i(3, 7, 6),
                                           GfVec3i(2, 3, 6),

                                           GfVec3i(2, 6, 4),
                                           GfVec3i(0, 2, 4),

                                           GfVec3i(7, 3, 1),
                                           GfVec3i(5, 7, 1)});

  return result;
}

}  // end namespace

void HdPhVolume::_UpdateDrawItem(HdSceneDelegate *sceneDelegate,
                                 HdRenderParam *renderParam,
                                 HdPhDrawItem *drawItem,
                                 HdDirtyBits *dirtyBits)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  /* VISIBILITY */
  _UpdateVisibility(sceneDelegate, dirtyBits);

  if (HdPhShouldPopulateConstantPrimvars(dirtyBits, GetId())) {
    /* CONSTANT PRIMVARS, TRANSFORM AND EXTENT */
    const HdPrimvarDescriptorVector constantPrimvars = HdPhGetPrimvarDescriptors(
      this, drawItem, sceneDelegate, HdInterpolationConstant);
    HdPhPopulateConstantPrimvars(
      this, &_sharedData, sceneDelegate, renderParam, drawItem, dirtyBits, constantPrimvars);
  }

  if ((*dirtyBits) & HdChangeTracker::DirtyMaterialId) {
    /* MATERIAL SHADER (may affect subsequent primvar population) */

    // Note that the creation of the HdPh_VolumeShader and the
    // allocation of the necessary textures is driven by two
    // different dirtyBits (HdChangeTracker::DirtyMaterialId and
    // HdChangeTracker::DirtyVolumeField).
    //
    // This way, we do not need to re-create the shader on every frame
    // when the fields of a volume are animated.
    //
    const HdPhMaterial *const material = static_cast<const HdPhMaterial *>(
      sceneDelegate->GetRenderIndex().GetSprim(HdPrimTypeTokens->material, GetMaterialId()));

    // Compute the material shader by adding GLSL code such as
    // "HdGet_density(vec3 p)" for sampling the fields needed by the volume
    // shader.
    // The material shader will eventually be concatenated with
    // the geometry shader which does the raymarching and is calling into
    // GLSL functions such as "float scattering(vec3)" in the volume shader
    // to evaluate physical properties of a volume at the point p.

    drawItem->SetMaterialShader(_ComputeMaterialShader(
      sceneDelegate, GetId(), _ComputeVolumeShader(material), _sharedData.bounds.GetRange()));
  }

  HdPhResourceRegistrySharedPtr resourceRegistry = std::static_pointer_cast<HdPhResourceRegistry>(
    sceneDelegate->GetRenderIndex().GetResourceRegistry());

  HdPh_VolumeShaderSharedPtr const materialShader = std::dynamic_pointer_cast<HdPh_VolumeShader>(
    drawItem->GetMaterialShader());

  if (!materialShader) {
    TF_CODING_ERROR("Expected valid volume shader for draw item.");
    return;
  }

  if ((*dirtyBits) & (HdChangeTracker::DirtyVolumeField | HdChangeTracker::DirtyMaterialId)) {
    /* FIELD TEXTURES */

    // (Re-)Allocate the textures associated with the field prims.
    materialShader->UpdateTextureHandles(sceneDelegate);
  }

  /* VERTICES */
  if ((*dirtyBits) & _shaderAndBBoxComputationDirtyBitsMask) {
    // Any change to the bounding box requires us to recompute
    // the vertices
    //
    if (!HdPhIsValidBAR(drawItem->GetVertexPrimvarRange())) {
      static const HdBufferSpecVector bufferSpecs{
        HdBufferSpec(HdTokens->points, HdTupleType{HdTypeFloatVec3, 1})};

      HdBufferArrayRangeSharedPtr const range = resourceRegistry->AllocateNonUniformBufferArrayRange(
        HdTokens->primvar, bufferSpecs, HdBufferArrayUsageHint());
      _sharedData.barContainer.Set(drawItem->GetDrawingCoord()->GetVertexPrimvarIndex(), range);
    }

    // Let HdPh_VolumeShader know about the points bar so that it
    // can fill it with the vertices of the volume bounding box.
    materialShader->SetPointsBar(drawItem->GetVertexPrimvarRange());

    // If HdPh_VolumeShader is not in charge of filling the points bar
    // from the volume bounding box computed from the fields, ...
    if (!materialShader->GetFillsPointsBar()) {
      // ... fill the points from the authored extents.
      resourceRegistry->AddSource(drawItem->GetVertexPrimvarRange(),
                                  std::make_shared<HdVtBufferSource>(
                                    HdTokens->points, _ComputeBBoxVertices(_sharedData.bounds.GetRange())));
    }
  }

  if ((*dirtyBits) & HdChangeTracker::NewRepr) {
    // Bounding box topology and geometric shader key only need to
    // be initialized the first time we make the draw item.

    const HdPh_VolumeShaderKey shaderKey;
    drawItem->SetGeometricShader(HdPh_GeometricShader::Create(shaderKey, resourceRegistry));

    /* TRIANGLE INDICES */
    {
      // XXX:
      // Always the same triangle indices, should they be allocated only
      // once and shared across all volumes?
      HdBufferSourceSharedPtr const source = std::make_shared<HdVtBufferSource>(HdTokens->indices,
                                                                                _GetCubeTriangleIndices());

      HdBufferSourceSharedPtrVector sources = {source};

      if (!HdPhIsValidBAR(drawItem->GetTopologyRange())) {
        HdBufferSpecVector bufferSpecs;
        HdBufferSpec::GetBufferSpecs(sources, &bufferSpecs);

        HdBufferArrayRangeSharedPtr const range = resourceRegistry->AllocateNonUniformBufferArrayRange(
          HdTokens->primvar, bufferSpecs, HdBufferArrayUsageHint());
        _sharedData.barContainer.Set(drawItem->GetDrawingCoord()->GetTopologyIndex(), range);
      }

      resourceRegistry->AddSources(drawItem->GetTopologyRange(), std::move(sources));
    }
  }
}

WABI_NAMESPACE_END
