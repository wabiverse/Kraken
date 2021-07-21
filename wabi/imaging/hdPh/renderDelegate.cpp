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
#include "wabi/imaging/hdPh/renderDelegate.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hdPh/basisCurves.h"
#include "wabi/imaging/hdPh/drawTarget.h"
#include "wabi/imaging/hdPh/extComputation.h"
#include "wabi/imaging/hdPh/field.h"
#include "wabi/imaging/hdPh/glslfxShader.h"
#include "wabi/imaging/hdPh/instancer.h"
#include "wabi/imaging/hdPh/light.h"
#include "wabi/imaging/hdPh/material.h"
#include "wabi/imaging/hdPh/mesh.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/points.h"
#include "wabi/imaging/hdPh/renderBuffer.h"
#include "wabi/imaging/hdPh/renderParam.h"
#include "wabi/imaging/hdPh/renderPass.h"
#include "wabi/imaging/hdPh/renderPassState.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/tokens.h"
#include "wabi/imaging/hdPh/volume.h"

#include "wabi/imaging/hd/aov.h"
#include "wabi/imaging/hd/camera.h"
#include "wabi/imaging/hd/driver.h"
#include "wabi/imaging/hd/extComputation.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"

#include "wabi/imaging/glf/contextCaps.h"
#include "wabi/imaging/glf/diagnostic.h"
#include "wabi/imaging/hio/glslfx.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/getenv.h"
#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_ENV_SETTING(HD_ENABLE_GPU_TINY_PRIM_CULLING, false, "Enable tiny prim culling");

const TfTokenVector HdPhRenderDelegate::SUPPORTED_RPRIM_TYPES = {HdPrimTypeTokens->mesh,
                                                                 HdPrimTypeTokens->basisCurves,
                                                                 HdPrimTypeTokens->points,
                                                                 HdPrimTypeTokens->volume};

const TfTokenVector HdPhRenderDelegate::SUPPORTED_SPRIM_TYPES = {HdPrimTypeTokens->camera,
                                                                 HdPrimTypeTokens->drawTarget,
                                                                 HdPrimTypeTokens->extComputation,
                                                                 HdPrimTypeTokens->material,
                                                                 HdPrimTypeTokens->domeLight,
                                                                 HdPrimTypeTokens->rectLight,
                                                                 HdPrimTypeTokens->simpleLight,
                                                                 HdPrimTypeTokens->sphereLight};

TF_DEFINE_PRIVATE_TOKENS(_tokens, (mtlx));

using HdPhResourceRegistryWeakPtr = std::weak_ptr<HdPhResourceRegistry>;

namespace
{

//
// Map from Hgi instances to resource registries.
//
// An entry is kept alive until the last shared_ptr to a resource
// registry is dropped.
//
class _HgiToResourceRegistryMap final
{
 public:
  // Map is a singleton.
  static _HgiToResourceRegistryMap &GetInstance()
  {
    static _HgiToResourceRegistryMap instance;
    return instance;
  }

  // Look-up resource registry by Hgi instance, create resource
  // registry for the instance if it didn't exist.
  HdPhResourceRegistrySharedPtr GetOrCreateRegistry(Hgi *const hgi)
  {
    std::lock_guard<std::mutex> guard(_mutex);

    // Previous entry exists, use it.
    auto it = _map.find(hgi);
    if (it != _map.end())
    {
      HdPhResourceRegistryWeakPtr const &registry = it->second;
      return HdPhResourceRegistrySharedPtr(registry);
    }

    // Create resource registry, custom deleter to remove corresponding
    // entry from map.
    HdPhResourceRegistrySharedPtr const result(
      new HdPhResourceRegistry(hgi), [this](HdPhResourceRegistry *registry) { this->_Destroy(registry); });

    // Insert into map.
    _map.insert({hgi, result});

    // Also register with HdPerfLog.
    //
    HdPerfLog::GetInstance().AddResourceRegistry(result.get());

    return result;
  }

 private:
  void _Destroy(HdPhResourceRegistry *const registry)
  {
    TRACE_FUNCTION();

    std::lock_guard<std::mutex> guard(_mutex);

    HdPerfLog::GetInstance().RemoveResourceRegistry(registry);

    _map.erase(registry->GetHgi());
    delete registry;
  }

  using _Map = std::unordered_map<Hgi *, HdPhResourceRegistryWeakPtr>;

  _HgiToResourceRegistryMap() = default;

  std::mutex _mutex;
  _Map _map;
};

}  // namespace

HdPhRenderDelegate::HdPhRenderDelegate()
  : HdPhRenderDelegate(HdRenderSettingsMap())
{}

HdPhRenderDelegate::HdPhRenderDelegate(HdRenderSettingsMap const &settingsMap)
  : HdRenderDelegate(settingsMap),
    _hgi(nullptr),
    _renderParam(std::make_unique<HdPhRenderParam>())
{
  // Initialize the settings and settings descriptors.
  _settingDescriptors = {
    HdRenderSettingDescriptor{"Enable Tiny Prim Culling",
                              HdPhRenderSettingsTokens->enableTinyPrimCulling,
                              VtValue(bool(TfGetEnvSetting(HD_ENABLE_GPU_TINY_PRIM_CULLING)))},
    HdRenderSettingDescriptor{"Step size when raymarching volume",
                              HdPhRenderSettingsTokens->volumeRaymarchingStepSize,
                              VtValue(HdPhVolume::defaultStepSize)},
    HdRenderSettingDescriptor{"Step size when raymarching volume for lighting computation",
                              HdPhRenderSettingsTokens->volumeRaymarchingStepSizeLighting,
                              VtValue(HdPhVolume::defaultStepSizeLighting)},
    HdRenderSettingDescriptor{"Maximum memory for a volume field texture in Mb "
                              "(unless overridden by field prim)",
                              HdPhRenderSettingsTokens->volumeMaxTextureMemoryPerField,
                              VtValue(HdPhVolume::defaultMaxTextureMemoryPerField)}};

  _PopulateDefaultSettings(_settingDescriptors);
}

HdRenderSettingDescriptorList HdPhRenderDelegate::GetRenderSettingDescriptors() const
{
  return _settingDescriptors;
}

VtDictionary HdPhRenderDelegate::GetRenderStats() const
{
  VtDictionary ra = _resourceRegistry->GetResourceAllocation();

  const VtDictionary::iterator gpuMemIt = ra.find(HdPerfTokens->gpuMemoryUsed.GetString());
  if (gpuMemIt != ra.end())
  {
    // If we find gpuMemoryUsed, add the texture memory to it.
    // XXX: We should look into fixing this in the resource registry itself
    size_t texMem = VtDictionaryGet<size_t>(ra, HdPerfTokens->textureMemory.GetString(), VtDefault = 0);
    size_t gpuMemTotal = gpuMemIt->second.Get<size_t>();
    gpuMemIt->second = VtValue(gpuMemTotal + texMem);
  }

  return ra;
}

HdPhRenderDelegate::~HdPhRenderDelegate() = default;

void HdPhRenderDelegate::SetDrivers(HdDriverVector const &drivers)
{
  if (_resourceRegistry)
  {
    TF_CODING_ERROR("Cannot set HdDriver twice for a render delegate.");
    return;
  }

  // For Phoenix we want to use the Hgi driver, so extract it.
  for (HdDriver *hdDriver : drivers)
  {
    if (hdDriver->name == HgiTokens->renderDriver && hdDriver->driver.IsHolding<Hgi *>())
    {
      _hgi = hdDriver->driver.UncheckedGet<Hgi *>();
      break;
    }
  }

  _resourceRegistry = _HgiToResourceRegistryMap::GetInstance().GetOrCreateRegistry(_hgi);

  TF_VERIFY(_hgi, "HdPh requires Hgi HdDriver");
}

const TfTokenVector &HdPhRenderDelegate::GetSupportedRprimTypes() const
{
  return SUPPORTED_RPRIM_TYPES;
}

const TfTokenVector &HdPhRenderDelegate::GetSupportedSprimTypes() const
{
  return SUPPORTED_SPRIM_TYPES;
}

static TfTokenVector _ComputeSupportedBprimTypes()
{
  TfTokenVector result;
  result.push_back(HdPrimTypeTokens->renderBuffer);

  for (const TfToken &primType : HdPhField::GetSupportedBprimTypes())
  {
    result.push_back(primType);
  }

  return result;
}

const TfTokenVector &HdPhRenderDelegate::GetSupportedBprimTypes() const
{
  static const TfTokenVector result = _ComputeSupportedBprimTypes();
  return result;
}

HdRenderParam *HdPhRenderDelegate::GetRenderParam() const
{
  return _renderParam.get();
}

HdResourceRegistrySharedPtr HdPhRenderDelegate::GetResourceRegistry() const
{
  return _resourceRegistry;
}

HdAovDescriptor HdPhRenderDelegate::GetDefaultAovDescriptor(TfToken const &name) const
{
  const bool colorDepthMSAA = true;  // GL requires color/depth to be matching.

  if (name == HdAovTokens->color)
  {
    HdFormat colorFormat = HdFormatFloat16Vec4;
    return HdAovDescriptor(colorFormat, colorDepthMSAA, VtValue(GfVec4f(0)));
  }
  else if (HdAovHasDepthSemantic(name))
  {
    return HdAovDescriptor(HdFormatFloat32, colorDepthMSAA, VtValue(1.0f));
  }

  return HdAovDescriptor();
}

HdRenderPassSharedPtr HdPhRenderDelegate::CreateRenderPass(HdRenderIndex *index,
                                                           HdRprimCollection const &collection)
{
  return std::make_shared<HdPh_RenderPass>(index, collection);
}

HdRenderPassStateSharedPtr HdPhRenderDelegate::CreateRenderPassState() const
{
  return std::make_shared<HdPhRenderPassState>();
}

HdInstancer *HdPhRenderDelegate::CreateInstancer(HdSceneDelegate *delegate, SdfPath const &id)
{
  return new HdPhInstancer(delegate, id);
}

void HdPhRenderDelegate::DestroyInstancer(HdInstancer *instancer)
{
  delete instancer;
}

HdRprim *HdPhRenderDelegate::CreateRprim(TfToken const &typeId, SdfPath const &rprimId)
{
  if (typeId == HdPrimTypeTokens->mesh)
  {
    return new HdPhMesh(rprimId);
  }
  else if (typeId == HdPrimTypeTokens->basisCurves)
  {
    return new HdPhBasisCurves(rprimId);
  }
  else if (typeId == HdPrimTypeTokens->points)
  {
    return new HdPhPoints(rprimId);
  }
  else if (typeId == HdPrimTypeTokens->volume)
  {
    return new HdPhVolume(rprimId);
  }
  else
  {
    TF_CODING_ERROR("Unknown Rprim Type %s", typeId.GetText());
  }

  return nullptr;
}

void HdPhRenderDelegate::DestroyRprim(HdRprim *rPrim)
{
  delete rPrim;
}

HdSprim *HdPhRenderDelegate::CreateSprim(TfToken const &typeId, SdfPath const &sprimId)
{
  if (typeId == HdPrimTypeTokens->camera)
  {
    return new HdCamera(sprimId);
  }
  else if (typeId == HdPrimTypeTokens->drawTarget)
  {
    return new HdPhDrawTarget(sprimId);
  }
  else if (typeId == HdPrimTypeTokens->extComputation)
  {
    return new HdPhExtComputation(sprimId);
  }
  else if (typeId == HdPrimTypeTokens->material)
  {
    return new HdPhMaterial(sprimId);
  }
  else if (typeId == HdPrimTypeTokens->domeLight || typeId == HdPrimTypeTokens->simpleLight ||
           typeId == HdPrimTypeTokens->sphereLight || typeId == HdPrimTypeTokens->rectLight)
  {
    return new HdPhLight(sprimId, typeId);
  }
  else
  {
    TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
  }

  return nullptr;
}

HdSprim *HdPhRenderDelegate::CreateFallbackSprim(TfToken const &typeId)
{
  if (typeId == HdPrimTypeTokens->camera)
  {
    return new HdCamera(SdfPath::EmptyPath());
  }
  else if (typeId == HdPrimTypeTokens->drawTarget)
  {
    return new HdPhDrawTarget(SdfPath::EmptyPath());
  }
  else if (typeId == HdPrimTypeTokens->extComputation)
  {
    return new HdPhExtComputation(SdfPath::EmptyPath());
  }
  else if (typeId == HdPrimTypeTokens->material)
  {
    return _CreateFallbackMaterialPrim();
  }
  else if (typeId == HdPrimTypeTokens->domeLight || typeId == HdPrimTypeTokens->simpleLight ||
           typeId == HdPrimTypeTokens->sphereLight || typeId == HdPrimTypeTokens->rectLight)
  {
    return new HdPhLight(SdfPath::EmptyPath(), typeId);
  }
  else
  {
    TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
  }

  return nullptr;
}

void HdPhRenderDelegate::DestroySprim(HdSprim *sPrim)
{
  delete sPrim;
}

HdBprim *HdPhRenderDelegate::CreateBprim(TfToken const &typeId, SdfPath const &bprimId)
{
  if (HdPhField::IsSupportedBprimType(typeId))
  {
    return new HdPhField(bprimId, typeId);
  }
  else if (typeId == HdPrimTypeTokens->renderBuffer)
  {
    return new HdPhRenderBuffer(_resourceRegistry.get(), bprimId);
  }
  else
  {
    TF_CODING_ERROR("Unknown Bprim Type %s", typeId.GetText());
  }

  return nullptr;
}

HdBprim *HdPhRenderDelegate::CreateFallbackBprim(TfToken const &typeId)
{
  if (HdPhField::IsSupportedBprimType(typeId))
  {
    return new HdPhField(SdfPath::EmptyPath(), typeId);
  }
  else if (typeId == HdPrimTypeTokens->renderBuffer)
  {
    return new HdPhRenderBuffer(_resourceRegistry.get(), SdfPath::EmptyPath());
  }
  else
  {
    TF_CODING_ERROR("Unknown Bprim Type %s", typeId.GetText());
  }

  return nullptr;
}

void HdPhRenderDelegate::DestroyBprim(HdBprim *bPrim)
{
  delete bPrim;
}

HdSprim *HdPhRenderDelegate::_CreateFallbackMaterialPrim()
{
  HioGlslfxSharedPtr glslfx = std::make_shared<HioGlslfx>(HdPhPackageFallbackSurfaceShader());

  HdPhSurfaceShaderSharedPtr fallbackShaderCode = std::make_shared<HdPhGLSLFXShader>(glslfx);

  HdPhMaterial *material = new HdPhMaterial(SdfPath::EmptyPath());
  material->SetSurfaceShader(fallbackShaderCode);

  return material;
}

void HdPhRenderDelegate::CommitResources(HdChangeTracker *tracker)
{
  TF_UNUSED(tracker);
  GLF_GROUP_FUNCTION();

  _ApplyTextureSettings();

  // --------------------------------------------------------------------- //
  // RESOLVE, COMPUTE & COMMIT PHASE
  // --------------------------------------------------------------------- //
  // All the required input data is now resident in memory, next we must:
  //
  //     1) Execute compute as needed for normals, tessellation, etc.
  //     2) Commit resources to the GPU.
  //     3) Update any scene-level acceleration structures.

  // Commit all pending source data.
  _resourceRegistry->Commit();

  HdPhRenderParam *stRenderParam = _renderParam.get();
  if (stRenderParam->IsGarbageCollectionNeeded())
  {
    _resourceRegistry->GarbageCollect();
    stRenderParam->ClearGarbageCollectionNeeded();
  }

  // see bug126621. currently dispatch buffers need to be released
  //                more frequently than we expect.
  _resourceRegistry->GarbageCollectDispatchBuffers();
}

bool HdPhRenderDelegate::IsSupported()
{
  return (GlfContextCaps::GetInstance().glVersion >= 400);
}

TfTokenVector HdPhRenderDelegate::GetShaderSourceTypes() const
{
#ifdef WITH_MATERIALX
  return {HioGlslfxTokens->glslfx, _tokens->mtlx};
#else
  return {HioGlslfxTokens->glslfx};
#endif
}

TfTokenVector HdPhRenderDelegate::GetMaterialRenderContexts() const
{
#ifdef WITH_MATERIALX
  return {HioGlslfxTokens->glslfx, _tokens->mtlx};
#else
  return {HioGlslfxTokens->glslfx};
#endif
}

bool HdPhRenderDelegate::IsPrimvarFilteringNeeded() const
{
  return true;
}

Hgi *HdPhRenderDelegate::GetHgi()
{
  return _hgi;
}

void HdPhRenderDelegate::_ApplyTextureSettings()
{
  const float memInMb = std::max(
    0.0f,
    GetRenderSetting<float>(HdPhRenderSettingsTokens->volumeMaxTextureMemoryPerField,
                            HdPhVolume::defaultMaxTextureMemoryPerField));

  _resourceRegistry->SetMemoryRequestForTextureType(HdTextureType::Field, 1048576 * memInMb);
}

WABI_NAMESPACE_END
