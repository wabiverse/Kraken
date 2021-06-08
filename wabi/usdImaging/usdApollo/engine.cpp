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

/**
 * @file
 * Apollo Engine.
 * I'm On Fire. */

#include "wabi/usdImaging/usdApollo/engine.h"

#include "wabi/usdImaging/usdImaging/delegate.h"

#include "wabi/usd/usdGeom/camera.h"
#include "wabi/usd/usdGeom/tokens.h"

#include "wabi/imaging/hd/rendererPlugin.h"
#include "wabi/imaging/hd/rendererPluginRegistry.h"
#include "wabi/imaging/hdx/pickTask.h"
#include "wabi/imaging/hdx/taskController.h"
#include "wabi/imaging/hdx/tokens.h"

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"
#include "wabi/imaging/hgiVulkan/hgi.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/getenv.h"
#include "wabi/base/tf/stl.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"

#include <string>

WABI_NAMESPACE_BEGIN

TF_DEFINE_ENV_SETTING(USDAPOLLO_ENGINE_DEBUG_SCENE_DELEGATE_ID,
                      "/",
                      "Default usdImaging scene delegate id");

namespace {

bool _GetHydraEnabledEnvVar()
{
  /**
   * XXX: Note that we don't cache the result here. This is primarily because
   * of the way usdview currently interacts with this setting. This should be
   * cleaned up, and the class hierarchy around UsdApolloEngine makes it much
   * easier to do so. */
  return TfGetenv("HD_ENABLED", "1") == "1";
}

SdfPath const &_GetUsdImagingDelegateId()
{
  static SdfPath const delegateId = SdfPath(
      TfGetEnvSetting(USDAPOLLO_ENGINE_DEBUG_SCENE_DELEGATE_ID));
  return delegateId;
}

bool _IsHydraEnabled()
{
  if (!_GetHydraEnabledEnvVar()) {
    return false;
  }

  /**
   * Check to see if we have a default plugin for the renderer. */
  TfToken defaultPlugin = HdRendererPluginRegistry::GetInstance().GetDefaultPluginId();

  return !defaultPlugin.IsEmpty();
}

}  // namespace

/**
 * ----------------------------------------------------------------------------
 *  Global State
 * ---------------------------------------------------------------------------- */

/* static. */
bool UsdApolloEngine::IsHydraEnabled()
{
  static bool isHydraEnabled = _IsHydraEnabled();
  return isHydraEnabled;
}

/**
 * ----------------------------------------------------------------------------
 *  Construction
 * ---------------------------------------------------------------------------- */

UsdApolloEngine::UsdApolloEngine(const HdDriver &driver)
    : UsdApolloEngine(SdfPath::AbsoluteRootPath(), {}, {}, _GetUsdImagingDelegateId(), driver)
{}

UsdApolloEngine::UsdApolloEngine(const SdfPath &rootPath,
                                 const SdfPathVector &excludedPaths,
                                 const SdfPathVector &invisedPaths,
                                 const SdfPath &sceneDelegateID,
                                 const HdDriver &driver)
    : m_hgi(),
      m_hgiDriver(driver),
      m_sceneDelegateId(sceneDelegateID),
      m_selTracker(std::make_shared<HdxSelectionTracker>()),
      m_selectionColor(1.0f, 1.0f, 0.0f, 1.0f),
      m_rootPath(rootPath),
      m_excludedPrimPaths(excludedPaths),
      m_invisedPrimPaths(invisedPaths),
      m_isPopulated(false)
{
  if (!SetRendererPlugin(APOLLO_GetDefaultRendererPluginId())) {
    TF_CODING_ERROR("No renderer plugins found! Check before creation.");
  }
}

void UsdApolloEngine::APOLLO_DestroyHydraObjects()
{
  /**
   * Destroy objects in opposite order of construction. */
  m_engine         = nullptr;
  m_taskController = nullptr;
  m_sceneDelegate  = nullptr;
  m_renderIndex    = nullptr;
  m_renderDelegate = nullptr;
}

UsdApolloEngine::~UsdApolloEngine()
{
  TF_PY_ALLOW_THREADS_IN_SCOPE();

  APOLLO_DestroyHydraObjects();
}

/**
 * ----------------------------------------------------------------------------
 *  Rendering
 * ---------------------------------------------------------------------------- */

void UsdApolloEngine::PrepareBatch(const UsdPrim &root, const UsdApolloRenderParams &params)
{
  HD_TRACE_FUNCTION();

  TF_VERIFY(m_sceneDelegate);

  if (APOLLO_CanPrepare(root)) {
    if (!m_isPopulated) {
      m_sceneDelegate->SetUsdDrawModesEnabled(params.enableUsdDrawModes);
      m_sceneDelegate->Populate(root.GetStage()->GetPrimAtPath(m_rootPath), m_excludedPrimPaths);
      m_sceneDelegate->SetInvisedPrimPaths(m_invisedPrimPaths);
      m_isPopulated = true;
    }

    APOLLO_PreSetTime(params);
    /**
     * SetTime will only react if time actually changes. */
    m_sceneDelegate->SetTime(params.frame);
    APOLLO_PostSetTime(params);
  }
}

void UsdApolloEngine::APOLLO_PrepareRender(const UsdApolloRenderParams &params)
{
  TF_VERIFY(m_taskController);

  m_taskController->SetFreeCameraClipPlanes(params.clipPlanes);

  TfTokenVector renderTags;
  APOLLO_ComputeRenderTags(params, &renderTags);
  m_taskController->SetRenderTags(renderTags);

  m_taskController->SetRenderParams(APOLLO_MakeHydraUsdApolloRenderParams(params));

  /**
   * Forward scene materials enable option to delegate. */
  m_sceneDelegate->SetSceneMaterialsEnabled(params.enableSceneMaterials);
  m_sceneDelegate->SetSceneLightsEnabled(params.enableSceneLights);
}

void UsdApolloEngine::RenderBatch(const SdfPathVector &paths, const UsdApolloRenderParams &params)
{
  TF_VERIFY(m_taskController);

  APOLLO_UpdateHydraCollection(&m_renderCollection, paths, params);
  m_taskController->SetCollection(m_renderCollection);

  APOLLO_PrepareRender(params);

  SetColorCorrectionSettings(params.colorCorrectionMode);

  /**
   * XXX: App sets the clear color via 'params' instead of setting up Aovs
   * that has clearColor in their descriptor. So for now we must pass this
   * clear color to the color AOV. */
  HdAovDescriptor colorAovDesc = m_taskController->GetRenderOutputSettings(HdAovTokens->color);
  if (colorAovDesc.format != HdFormatInvalid) {
    colorAovDesc.clearValue = VtValue(params.clearColor);
    m_taskController->SetRenderOutputSettings(HdAovTokens->color, colorAovDesc);
  }

  m_taskController->SetEnableSelection(params.highlight);
  VtValue selectionValue(m_selTracker);
  m_engine->SetTaskContextData(HdxTokens->selectionState, selectionValue);
  APOLLO_Execute(params, m_taskController->GetRenderingTasks());
}

void UsdApolloEngine::Render(const UsdPrim &root, const UsdApolloRenderParams &params)
{
  TF_VERIFY(m_taskController);

  PrepareBatch(root, params);

  /**
   * XXX: (UsdImagingPaths) Is it correct to map
   * USD root path directly to the cachePath here? */
  const SdfPath cachePath   = root.GetPath();
  const SdfPathVector paths = {m_sceneDelegate->ConvertCachePathToIndexPath(cachePath)};

  RenderBatch(paths, params);
}

bool UsdApolloEngine::IsConverged() const
{
  TF_VERIFY(m_taskController);
  return m_taskController->IsConverged();
}

/**
 * ----------------------------------------------------------------------------
 *  Root & Transform Visibility
 * ---------------------------------------------------------------------------- */

void UsdApolloEngine::SetRootTransform(GfMatrix4d const &xf)
{
  TF_VERIFY(m_sceneDelegate);
  m_sceneDelegate->SetRootTransform(xf);
}

void UsdApolloEngine::SetRootVisibility(bool isVisible)
{
  TF_VERIFY(m_sceneDelegate);
  m_sceneDelegate->SetRootVisibility(isVisible);
}

/**
 * ----------------------------------------------------------------------------
 *  Camera & Lighting State
 * ---------------------------------------------------------------------------- */

void UsdApolloEngine::SetFraming(CameraUtilFraming const &framing)
{
  if (TF_VERIFY(m_taskController)) {
    m_taskController->SetFraming(framing);
  }
}

void UsdApolloEngine::SetOverrideWindowPolicy(
    const std::optional<CameraUtilConformWindowPolicy> &policy)
{
  if (TF_VERIFY(m_taskController)) {
    m_taskController->SetOverrideWindowPolicy(policy);
  }
}

void UsdApolloEngine::SetRenderBufferSize(GfVec2i const &size)
{
  if (TF_VERIFY(m_taskController)) {
    m_taskController->SetRenderBufferSize(size);
  }
}

void UsdApolloEngine::SetWindowPolicy(CameraUtilConformWindowPolicy policy)
{
  TF_VERIFY(m_taskController);
  /**
   * Note: Free cam uses SetCameraState, which expects
   * frustum to be pre-adjusted for the viewport size. */

  /**
   * The usdImagingDelegate manages the window policy for scene cameras. */
  m_sceneDelegate->SetWindowPolicy(policy);
}

void UsdApolloEngine::SetCameraPath(SdfPath const &id)
{
  TF_VERIFY(m_taskController);
  m_taskController->SetCameraPath(id);

  /**
   * The camera that is set for viewing
   * will also be used for time sampling. */
  m_sceneDelegate->SetCameraForSampling(id);
}

void UsdApolloEngine::SetCameraState(const GfMatrix4d &viewMatrix,
                                     const GfMatrix4d &projectionMatrix)
{
  TF_VERIFY(m_taskController);
  m_taskController->SetFreeCameraMatrices(viewMatrix, projectionMatrix);
}

void UsdApolloEngine::SetCameraStateFromVulkan()
{
  // GfMatrix4d viewMatrix, projectionMatrix;
  // GfVec4d viewport;
  // glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrix.GetArray());
  // glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix.GetArray());
  // glGetDoublev(GL_VIEWPORT, &viewport[0]);

  // SetCameraState(viewMatrix, projectionMatrix);
  // SetRenderViewport(viewport);
}

void UsdApolloEngine::SetLightingStateFromVulkan()
{
  TF_VERIFY(m_taskController);

  // if (!m_lightingContextForVulkanState) {
  //   m_lightingContextForVulkanState = VkfSimpleLightingContext::New();
  // }
  // m_lightingContextForVulkanState->SetStateFromOpenGL();

  // m_taskController->SetLightingState(m_lightingContextForVulkanState);
}

void UsdApolloEngine::SetLightingState(/** TODO: VOID***GlfSimpleLightingContextPtr const &src */)
{
  TF_VERIFY(m_taskController);
  // m_taskController->SetLightingState(src);
}

void UsdApolloEngine::SetLightingState(/** TODO: VkfSimpleLightVector const &lights, */
                                       /** TODO: VkfSimpleMaterial const &material, */
                                       GfVec4f const &sceneAmbient)
{
  TF_VERIFY(m_taskController);

  /**
   * We still use m_lightingContextForVulkanState for convenience, but
   * set the values directly. */
  // if (!m_lightingContextForVulkanState) {
  //   m_lightingContextForVulkanState = VkfSimpleLightingContext::New();
  // }
  // m_lightingContextForVulkanState->SetLights(lights);
  // m_lightingContextForVulkanState->SetMaterial(material);
  // m_lightingContextForVulkanState->SetSceneAmbient(sceneAmbient);
  // m_lightingContextForVulkanState->SetUseLighting(lights.size() > 0);

  // m_taskController->SetLightingState(m_lightingContextForVulkanState);
}

/**
 * ----------------------------------------------------------------------------
 *  Selection Highlighting
 * ---------------------------------------------------------------------------- */

void UsdApolloEngine::SetSelected(SdfPathVector const &paths)
{
  TF_VERIFY(m_sceneDelegate);

  /**
   * Populate new selection. */
  HdSelectionSharedPtr const selection = std::make_shared<HdSelection>();
  /**
   * XXX: Usdview currently supports selection on click. If we extend
   * to rollover (locate) selection, we need to pass that mode here. */
  static const HdSelection::HighlightMode mode = HdSelection::HighlightModeSelect;
  for (SdfPath const &path : paths) {
    m_sceneDelegate->PopulateSelection(mode, path, UsdImagingDelegate::ALL_INSTANCES, selection);
  }

  /**
   * Set the result back to selection tracker. */
  m_selTracker->SetSelection(selection);
}

void UsdApolloEngine::ClearSelected()
{
  TF_VERIFY(m_selTracker);

  m_selTracker->SetSelection(std::make_shared<HdSelection>());
}

HdSelectionSharedPtr UsdApolloEngine::APOLLO_GetSelection() const
{
  if (HdSelectionSharedPtr const selection = m_selTracker->GetSelectionMap()) {
    return selection;
  }

  return std::make_shared<HdSelection>();
}

void UsdApolloEngine::AddSelected(SdfPath const &path, int instanceIndex)
{
  TF_VERIFY(m_sceneDelegate);

  HdSelectionSharedPtr const selection = APOLLO_GetSelection();

  /**
   * XXX: Usdview currently supports selection on click. If we extend
   * to rollover (locate) selection, we need to pass that mode here. */
  static const HdSelection::HighlightMode mode = HdSelection::HighlightModeSelect;
  m_sceneDelegate->PopulateSelection(mode, path, instanceIndex, selection);

  /**
   * Set the result back to selection tracker. */
  m_selTracker->SetSelection(selection);
}

void UsdApolloEngine::SetSelectionColor(GfVec4f const &color)
{
  TF_VERIFY(m_taskController);

  m_selectionColor = color;
  m_taskController->SetSelectionColor(m_selectionColor);
}

/**
 * ----------------------------------------------------------------------------
 *  Picking
 * ---------------------------------------------------------------------------- */

bool UsdApolloEngine::TestIntersection(const GfMatrix4d &viewMatrix,
                                       const GfMatrix4d &projectionMatrix,
                                       const UsdPrim &root,
                                       const UsdApolloRenderParams &params,
                                       GfVec3d *outHitPoint,
                                       GfVec3d *outHitNormal,
                                       SdfPath *outHitPrimPath,
                                       SdfPath *outHitInstancerPath,
                                       int *outHitInstanceIndex,
                                       HdInstancerContext *outInstancerContext)
{
  TF_VERIFY(m_sceneDelegate);
  TF_VERIFY(m_taskController);

  PrepareBatch(root, params);

  /**
   * XXX: (UsdImagingPaths) This is incorrect..."Root" points to a USD subtree,
   * but the subtree in the hydra namespace might be very different (e.g. for
   * native instancing). We need a translation step. */
  const SdfPath cachePath   = root.GetPath();
  const SdfPathVector roots = {m_sceneDelegate->ConvertCachePathToIndexPath(cachePath)};
  APOLLO_UpdateHydraCollection(&m_intersectCollection, roots, params);

  APOLLO_PrepareRender(params);

  HdxPickHitVector allHits;
  HdxPickTaskContextParams pickParams;
  pickParams.resolveMode      = HdxPickTokens->resolveNearestToCenter;
  pickParams.viewMatrix       = viewMatrix;
  pickParams.projectionMatrix = projectionMatrix;
  pickParams.clipPlanes       = params.clipPlanes;
  pickParams.collection       = m_intersectCollection;
  pickParams.outHits          = &allHits;
  const VtValue vtPickParams(pickParams);

  m_engine->SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);
  APOLLO_Execute(params, m_taskController->GetPickingTasks());

  /**
   * Since we are in nearest-hit mode, we expect allHits to have
   * a single point in it. */
  if (allHits.size() != 1) {
    return false;
  }

  HdxPickHit &hit = allHits[0];

  if (outHitPoint) {
    *outHitPoint = hit.worldSpaceHitPoint;
  }

  if (outHitNormal) {
    *outHitNormal = hit.worldSpaceHitNormal;
  }

  hit.objectId = m_sceneDelegate->GetScenePrimPath(
      hit.objectId, hit.instanceIndex, outInstancerContext);
  hit.instancerId =
      m_sceneDelegate->ConvertIndexPathToCachePath(hit.instancerId).GetAbsoluteRootOrPrimPath();

  if (outHitPrimPath) {
    *outHitPrimPath = hit.objectId;
  }
  if (outHitInstancerPath) {
    *outHitInstancerPath = hit.instancerId;
  }
  if (outHitInstanceIndex) {
    *outHitInstanceIndex = hit.instanceIndex;
  }

  return true;
}

bool UsdApolloEngine::DecodeIntersection(unsigned char const primIdColor[4],
                                         unsigned char const instanceIdColor[4],
                                         SdfPath *outHitPrimPath,
                                         SdfPath *outHitInstancerPath,
                                         int *outHitInstanceIndex,
                                         HdInstancerContext *outInstancerContext)
{
  TF_VERIFY(m_sceneDelegate);

  const int primId      = HdxPickTask::DecodeIDRenderColor(primIdColor);
  const int instanceIdx = HdxPickTask::DecodeIDRenderColor(instanceIdColor);
  SdfPath primPath      = m_sceneDelegate->GetRenderIndex().GetRprimPathFromPrimId(primId);

  if (primPath.IsEmpty()) {
    return false;
  }

  SdfPath delegateId, instancerId;
  m_sceneDelegate->GetRenderIndex().GetSceneDelegateAndInstancerIds(
      primPath, &delegateId, &instancerId);

  primPath = m_sceneDelegate->GetScenePrimPath(primPath, instanceIdx, outInstancerContext);
  instancerId =
      m_sceneDelegate->ConvertIndexPathToCachePath(instancerId).GetAbsoluteRootOrPrimPath();

  if (outHitPrimPath) {
    *outHitPrimPath = primPath;
  }
  if (outHitInstancerPath) {
    *outHitInstancerPath = instancerId;
  }
  if (outHitInstanceIndex) {
    *outHitInstanceIndex = instanceIdx;
  }

  return true;
}

/**
 * ----------------------------------------------------------------------------
 *  Renderer Plugin Management
 * ---------------------------------------------------------------------------- */

/* static */
TfTokenVector UsdApolloEngine::GetRendererPlugins()
{
  HfPluginDescVector pluginDescriptors;
  HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescriptors);

  TfTokenVector plugins;
  for (size_t i = 0; i < pluginDescriptors.size(); ++i) {
    plugins.push_back(pluginDescriptors[i].id);
  }
  return plugins;
}

/* static */
std::string UsdApolloEngine::GetRendererDisplayName(TfToken const &id)
{
  if (ARCH_UNLIKELY(!_GetHydraEnabledEnvVar() || id.IsEmpty())) {
    /**
     * No renderer name is returned if the user requested to disable Hydra,
     * or if the machine does not support any of the available renderers. */
    return std::string();
  }

  HfPluginDesc pluginDescriptor;
  if (!TF_VERIFY(HdRendererPluginRegistry::GetInstance().GetPluginDesc(id, &pluginDescriptor))) {
    return std::string();
  }

  return pluginDescriptor.displayName;
}

TfToken UsdApolloEngine::GetCurrentRendererId() const
{
  return m_renderDelegate.GetPluginId();
}

void UsdApolloEngine::APOLLO_InitializeHgiIfNecessary()
{
  /**
   * If the client of UsdApolloEngine does not provide a HdDriver,
   * we construct a default one that is owned by UsdApolloEngine.
   * The cleanest pattern is for the client app to provide this
   * since you may have multiple UsdApolloEngines in one app
   * that ideally all use the same HdDriver and Hgi to share
   * GPU resources. */
  if (m_hgiDriver.driver.IsEmpty()) {
    // m_hgi = Hgi::CreatePlatformDefaultHgi();
    m_hgi              = new HgiVulkan();
    m_hgiDriver.name   = HgiTokens->renderDriver;
    m_hgiDriver.driver = VtValue(m_hgi);
  }
}

bool UsdApolloEngine::SetRendererPlugin(TfToken const &id)
{
  APOLLO_InitializeHgiIfNecessary();

  HdRendererPluginRegistry &registry = HdRendererPluginRegistry::GetInstance();

  /**
   * Special case: id = TfToken() selects the first plugin in the list. */
  const TfToken resolvedId = id.IsEmpty() ? registry.GetDefaultPluginId() : id;

  if (m_renderDelegate && m_renderDelegate.GetPluginId() == resolvedId) {
    return true;
  }

  TF_PY_ALLOW_THREADS_IN_SCOPE();

  HdPluginRenderDelegateUniqueHandle renderDelegate = registry.CreateRenderDelegate(resolvedId);
  if (!renderDelegate) {
    return false;
  }

  APOLLO_SetRenderDelegateAndRestoreState(std::move(renderDelegate));

  return true;
}

void UsdApolloEngine::APOLLO_SetRenderDelegateAndRestoreState(
    HdPluginRenderDelegateUniqueHandle &&renderDelegate)
{
  /**
   * Pull old delegate/task controller state. */
  const GfMatrix4d rootTransform = m_sceneDelegate ? m_sceneDelegate->GetRootTransform() :
                                                     GfMatrix4d(1.0);
  const bool isVisible           = m_sceneDelegate ? m_sceneDelegate->GetRootVisibility() : true;
  HdSelectionSharedPtr const selection = APOLLO_GetSelection();

  APOLLO_SetRenderDelegate(std::move(renderDelegate));

  m_sceneDelegate->SetRootVisibility(isVisible);
  m_sceneDelegate->SetRootTransform(rootTransform);
  m_selTracker->SetSelection(selection);
  m_taskController->SetSelectionColor(m_selectionColor);
}

SdfPath UsdApolloEngine::APOLLO_ComputeControllerPath(
    const HdPluginRenderDelegateUniqueHandle &renderDelegate)
{
  const std::string pluginId = TfMakeValidIdentifier(renderDelegate.GetPluginId().GetText());
  const TfToken rendererName(TfStringPrintf("_UsdImaging_%s_%p", pluginId.c_str(), this));

  return m_sceneDelegateId.AppendChild(rendererName);
}

void UsdApolloEngine::APOLLO_SetRenderDelegate(HdPluginRenderDelegateUniqueHandle &&renderDelegate)
{
  /**
   * This relies on SetRendererPlugin to release the GIL... */

  /**
   * Destruction. */
  APOLLO_DestroyHydraObjects();

  m_isPopulated = false;

  /**
   * Creation. */

  /**
   * Use the new render delegate. */
  m_renderDelegate = std::move(renderDelegate);

  /**
   * Recreate the render index. */
  m_renderIndex.reset(HdRenderIndex::New(m_renderDelegate.Get(), {&m_hgiDriver}));

  /**
   * Create the new delegate. */
  m_sceneDelegate = std::make_unique<UsdImagingDelegate>(m_renderIndex.get(), m_sceneDelegateId);

  /**
   * Create the new task controller. */
  m_taskController = std::make_unique<HdxTaskController>(
      m_renderIndex.get(), APOLLO_ComputeControllerPath(m_renderDelegate));

  /**
   * The task context holds on to resources in the render
   * delegate, so we want to destroy it first and thus
   * create it last. */
  m_engine = std::make_unique<HdEngine>();
}

/**
 * ----------------------------------------------------------------------------
 *  AOVs & Renderer Settings
 * ---------------------------------------------------------------------------- */

TfTokenVector UsdApolloEngine::GetRendererAovs() const
{
  TF_VERIFY(m_renderIndex);

  if (m_renderIndex->IsBprimTypeSupported(HdPrimTypeTokens->renderBuffer)) {

    static const TfToken candidates[] = {HdAovTokens->primId,
                                         HdAovTokens->depth,
                                         HdAovTokens->normal,
                                         HdAovTokensMakePrimvar(TfToken("st"))};

    TfTokenVector aovs = {HdAovTokens->color};

    for (auto const &aov : candidates) {
      if (m_renderDelegate->GetDefaultAovDescriptor(aov).format != HdFormatInvalid) {
        aovs.push_back(aov);
      }
    }
    return aovs;
  }
  return TfTokenVector();
}

bool UsdApolloEngine::SetRendererAov(TfToken const &id)
{
  TF_VERIFY(m_renderIndex);
  if (m_renderIndex->IsBprimTypeSupported(HdPrimTypeTokens->renderBuffer)) {
    m_taskController->SetRenderOutputs({id});
    return true;
  }
  return false;
}

HgiTextureHandle UsdApolloEngine::GetAovTexture(TfToken const &name) const
{
  VtValue aov;
  HgiTextureHandle aovTexture;

  if (m_engine->GetTaskContextData(name, &aov)) {
    if (aov.IsHolding<HgiTextureHandle>()) {
      aovTexture = aov.Get<HgiTextureHandle>();
    }
  }

  return aovTexture;
}

UsdApolloRendererSettingsList UsdApolloEngine::GetRendererSettingsList() const
{
  TF_VERIFY(m_renderDelegate);

  const HdRenderSettingDescriptorList descriptors =
      m_renderDelegate->GetRenderSettingDescriptors();
  UsdApolloRendererSettingsList ret;

  for (auto const &desc : descriptors) {
    UsdApolloRendererSetting r;
    r.key      = desc.key;
    r.name     = desc.name;
    r.defValue = desc.defaultValue;

    /**
     * Use the type of the default value to tell us what kind of
     * widget to create... */
    if (r.defValue.IsHolding<bool>()) {
      r.type = UsdApolloRendererSetting::TYPE_FLAG;
    }

    else if (r.defValue.IsHolding<int>() || r.defValue.IsHolding<unsigned int>()) {
      r.type = UsdApolloRendererSetting::TYPE_INT;
    }

    else if (r.defValue.IsHolding<float>()) {
      r.type = UsdApolloRendererSetting::TYPE_FLOAT;
    }

    else if (r.defValue.IsHolding<std::string>()) {
      r.type = UsdApolloRendererSetting::TYPE_STRING;
    }

    else {
      TF_WARN("Setting '%s' with type '%s' doesn't have a UI implementation...",
              r.name.c_str(),
              r.defValue.GetTypeName().c_str());
      continue;
    }

    ret.push_back(r);
  }

  return ret;
}

VtValue UsdApolloEngine::GetRendererSetting(TfToken const &id) const
{
  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->GetRenderSetting(id);
}

void UsdApolloEngine::SetRendererSetting(TfToken const &id, VtValue const &value)
{
  TF_VERIFY(m_renderDelegate);
  m_renderDelegate->SetRenderSetting(id, value);
}

void UsdApolloEngine::SetEnablePresentation(bool enabled)
{
  if (TF_VERIFY(m_taskController)) {
    m_taskController->SetEnablePresentation(enabled);
  }
}

void UsdApolloEngine::SetPresentationOutput(TfToken const &api, VtValue const &framebuffer)
{
  if (TF_VERIFY(m_taskController)) {
    m_userFramebuffer = framebuffer;
    m_taskController->SetPresentationOutput(api, framebuffer);
  }
}

/**
 * ----------------------------------------------------------------------------
 *  Command API
 * ---------------------------------------------------------------------------- */

HdCommandDescriptors UsdApolloEngine::GetRendererCommandDescriptors() const
{
  if (ARCH_UNLIKELY(!m_renderDelegate)) {
    return HdCommandDescriptors();
  }

  return m_renderDelegate->GetCommandDescriptors();
}

bool UsdApolloEngine::InvokeRendererCommand(const TfToken &command,
                                            const HdCommandArgs &args) const
{
  if (ARCH_UNLIKELY(!m_renderDelegate)) {
    return false;
  }

  return m_renderDelegate->InvokeCommand(command, args);
}

/**
 * ----------------------------------------------------------------------------
 *  Control of Background Rendering Threads
 * ---------------------------------------------------------------------------- */

bool UsdApolloEngine::IsPauseRendererSupported() const
{
  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->IsPauseSupported();
}

bool UsdApolloEngine::PauseRenderer()
{
  TF_PY_ALLOW_THREADS_IN_SCOPE();

  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->Pause();
}

bool UsdApolloEngine::ResumeRenderer()
{
  TF_PY_ALLOW_THREADS_IN_SCOPE();

  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->Resume();
}

bool UsdApolloEngine::IsStopRendererSupported() const
{
  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->IsStopSupported();
}

bool UsdApolloEngine::StopRenderer()
{
  TF_PY_ALLOW_THREADS_IN_SCOPE();

  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->Stop();
}

bool UsdApolloEngine::RestartRenderer()
{
  TF_PY_ALLOW_THREADS_IN_SCOPE();

  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->Restart();
}

/**
 * ----------------------------------------------------------------------------
 *  Color Correction
 * ---------------------------------------------------------------------------- */

void UsdApolloEngine::SetColorCorrectionSettings(TfToken const &id)
{
  if (!IsColorCorrectionCapable()) {
    return;
  }

  TF_VERIFY(m_taskController);

  HdxColorCorrectionTaskParams hdParams;
  hdParams.colorCorrectionMode = id;
  m_taskController->SetColorCorrectionParams(hdParams);
}

bool UsdApolloEngine::IsColorCorrectionCapable()
{
  return true;
}

/**
 * ----------------------------------------------------------------------------
 *  Resource Information
 * ---------------------------------------------------------------------------- */

VtDictionary UsdApolloEngine::GetRenderStats() const
{
  TF_VERIFY(m_renderDelegate);
  return m_renderDelegate->GetRenderStats();
}

HgiVulkan *UsdApolloEngine::GetHgi()
{
  return m_hgi;
}

/**
 * ----------------------------------------------------------------------------
 *  Private :: Protected
 * ---------------------------------------------------------------------------- */

HdRenderIndex *UsdApolloEngine::APOLLO_GetRenderIndex() const
{
  return m_renderIndex.get();
}

void UsdApolloEngine::APOLLO_Execute(const UsdApolloRenderParams &params,
                                     HdTaskSharedPtrVector tasks)
{
  TF_VERIFY(m_sceneDelegate);

  // m_hgi->StartFrame();

  // const VkfDeviceCaps &caps = VkfDeviceCaps::GetInstance();

  /**
   * VkClearColorValue clearColor = {{
   *   params.clearColor[0],
   *   params.clearColor[1],
   *   params.clearColor[2],
   *   params.clearColor[3]
   * }}; */

  /**
   * VkClearDepthStencilValue clearDS = { 1.0f, 0 };
   * VkClearValue clearValues[2];
   * memset(clearValues, 0, sizeof(clearValues));
   * clearValues[0].color = clearColor;
   * clearValues[1].depthStencil = clearDS; */

  /**
   *
   * VkRenderPassBeginInfo rpBeginInfo;
   * memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
   * rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   * rpBeginInfo.renderPass = m_hgi->
   *
   * if(!m_userFramebuffer.IsEmpty()) {
   *   m_taskController->SetPresentationOutput(HgiTokens->Vulkan, VtValue(m_userFramebuffer));
   * }
   * const uiSize sz                      = m_viewport->swapChainImageSize();
   * rpBeginInfo.renderArea.extent.width  = sz.width();
   * rpBeginInfo.renderArea.extent.height = sz.height();
   * rpBeginInfo.clearValueCount          = 2;
   * rpBeginInfo.pClearValues             = clearValues;
   * VkCommandBuffer cmdBuf               = m_viewport->currentCommandBuffer();
   * m_device_funcs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
   * m_device_funcs->vkCmdEndRenderPass(cmdBuf); */

  // GLF_GROUP_FUNCTION();

  // GLint restoreReadFbo = 0;
  // GLint restoreDrawFbo = 0;
  // glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &restoreReadFbo);
  // glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &restoreDrawFbo);
  // if (m_userFramebuffer.IsEmpty()) {
  // If user supplied no framebuffer, use the currently bound
  // framebuffer.
  // m_taskController->SetPresentationOutput(HgiTokens->Vulkan, VtValue()/**
  // TODO:(static_cast<uint32_t>(restoreDrawFbo))*/);
  // }

  // GLuint vao;
  // if (isCoreProfileContext) {
  // We must bind a VAO (Vertex Array Object) because core profile
  // contexts do not have a default vertex array object. VAO objects are
  // container objects which are not shared between contexts, so we create
  // and bind a VAO here so that core rendering code does not have to
  // explicitly manage per-GL context state.
  // glGenVertexArrays(1, &vao);
  // glBindVertexArray(vao);
  // } else {
  // glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
  // }

  // hydra orients all geometry during topological processing so that
  // front faces have ccw winding.
  // if (params.flipFrontFacing) {
  // glFrontFace(GL_CW); // < State is pushed via GL_POLYGON_BIT
  // } else {
  // glFrontFace(GL_CCW); // < State is pushed via GL_POLYGON_BIT
  // }

  // if (params.applyRenderState) {
  // glDisable(GL_BLEND);
  // }

  // for points width
  // glEnable(GL_PROGRAM_POINT_SIZE);

  // TODO:
  //  * forceRefresh
  //  * showGuides, showRender, showProxy
  //  * gammaCorrectColors

  {
    /**
     * Release the GIL before calling into hydra, in case any hydra plugins
     * call into python. */
    TF_PY_ALLOW_THREADS_IN_SCOPE();
    // m_engine->Execute(m_renderIndex.get(), &tasks);
  }

  // if (isCoreProfileContext) {

  //   glBindVertexArray(0);
  //   // XXX: We should not delete the VAO on every draw call, but we
  //   // currently must because it is GL Context state and we do not control
  //   // the context.
  //   glDeleteVertexArrays(1, &vao);

  // } else {
  //   glPopAttrib(); // GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT
  // }

  // glBindFramebuffer(GL_READ_FRAMEBUFFER, restoreReadFbo);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, restoreDrawFbo);
  // m_hgi->EndFrame();
}

bool UsdApolloEngine::APOLLO_CanPrepare(const UsdPrim &root)
{
  HD_TRACE_FUNCTION();

  if (!TF_VERIFY(root, "Attempting to draw an invalid/null prim\n"))
    return false;

  if (!root.GetPath().HasPrefix(m_rootPath)) {
    TF_CODING_ERROR("Attempting to draw path <%s>, but engine is rooted at <%s>\n",
                    root.GetPath().GetText(),
                    m_rootPath.GetText());
    return false;
  }

  return true;
}

static int _GetRefineLevel(float c)
{
  /**
   * TODO: Change complexity to refineLevel when we refactor UsdImaging.
   * Convert complexity float to refine level int. */
  int refineLevel = 0;

  /**
   * To avoid floating point inaccuracy (e.g. 1.3 > 1.3f). */
  c = std::min(c + 0.01f, 2.0f);

  if (1.0f <= c && c < 1.1f) {
    refineLevel = 0;
  }
  else if (1.1f <= c && c < 1.2f) {
    refineLevel = 1;
  }
  else if (1.2f <= c && c < 1.3f) {
    refineLevel = 2;
  }
  else if (1.3f <= c && c < 1.4f) {
    refineLevel = 3;
  }
  else if (1.4f <= c && c < 1.5f) {
    refineLevel = 4;
  }
  else if (1.5f <= c && c < 1.6f) {
    refineLevel = 5;
  }
  else if (1.6f <= c && c < 1.7f) {
    refineLevel = 6;
  }
  else if (1.7f <= c && c < 1.8f) {
    refineLevel = 7;
  }
  else if (1.8f <= c && c <= 2.0f) {
    refineLevel = 8;
  }
  else {
    TF_CODING_ERROR("Invalid complexity %f, expected range is [1.0,2.0]\n", c);
  }

  return refineLevel;
}

void UsdApolloEngine::APOLLO_PreSetTime(const UsdApolloRenderParams &params)
{
  HD_TRACE_FUNCTION();

  /**
   * Set the fallback refine level, if this changes
   * from the existing value, all prim refine levels
   * will be dirtied. */
  const int refineLevel = _GetRefineLevel(params.complexity);
  m_sceneDelegate->SetRefineLevelFallback(refineLevel);

  /**
   * Apply any queued up scene edits. */
  m_sceneDelegate->ApplyPendingUpdates();
}

void UsdApolloEngine::APOLLO_PostSetTime(const UsdApolloRenderParams &params)
{
  HD_TRACE_FUNCTION();
}

/* static */
bool UsdApolloEngine::APOLLO_UpdateHydraCollection(HdRprimCollection *collection,
                                                   SdfPathVector const &roots,
                                                   UsdApolloRenderParams const &params)
{
  if (collection == nullptr) {
    TF_CODING_ERROR("Null passed to APOLLO_UpdateHydraCollection");
    return false;
  }

  /**
   * Choose repr. */
  HdReprSelector reprSelector = HdReprSelector(HdReprTokens->smoothHull);
  const bool refined          = params.complexity > 1.0;

  if (params.drawMode == UsdApolloDrawMode::DRAW_POINTS) {
    reprSelector = HdReprSelector(HdReprTokens->points);
  }

  else if (params.drawMode == UsdApolloDrawMode::DRAW_GEOM_FLAT ||
           params.drawMode == UsdApolloDrawMode::DRAW_SHADED_FLAT) {
    /**
     * Flat shading. */
    reprSelector = HdReprSelector(HdReprTokens->hull);
  }

  else if (params.drawMode == UsdApolloDrawMode::DRAW_WIREFRAME_ON_SURFACE) {
    /**
     * Wireframe on surface. */
    reprSelector = HdReprSelector(refined ? HdReprTokens->refinedWireOnSurf :
                                            HdReprTokens->wireOnSurf);
  }

  else if (params.drawMode == UsdApolloDrawMode::DRAW_WIREFRAME) {
    /**
     * Wireframe. */
    reprSelector = HdReprSelector(refined ? HdReprTokens->refinedWire : HdReprTokens->wire);
  }

  else {
    /**
     * Smooth shading. */
    reprSelector = HdReprSelector(refined ? HdReprTokens->refined : HdReprTokens->smoothHull);
  }

  /**
   * By default our main collection will be called geometry. */
  const TfToken colName = HdTokens->geometry;

  /**
   * Check if the collection needs to be updated (so we can avoid the sort). */
  SdfPathVector const &oldRoots = collection->GetRootPaths();

  /**
   * Inexpensive comparison first. */
  bool match = collection->GetName() == colName && oldRoots.size() == roots.size() &&
               collection->GetReprSelector() == reprSelector;

  /**
   * Only take the time to compare root paths if everything else matches. */
  if (match) {
    /**
     * Note that oldRoots is guaranteed to be sorted. */
    for (size_t i = 0; i < roots.size(); i++) {
      /**
       * Avoid binary search when both vectors are sorted. */
      if (oldRoots[i] == roots[i])
        continue;
      /**
       * Binary search to find the current root. */
      if (!std::binary_search(oldRoots.begin(), oldRoots.end(), roots[i])) {
        match = false;
        break;
      }
    }

    /**
     * If everything matches, do nothing. */
    if (match)
      return false;
  }

  /**
   * Recreate the collection. */
  *collection = HdRprimCollection(colName, reprSelector);
  collection->SetRootPaths(roots);

  return true;
}

/* static */
HdxRenderTaskParams UsdApolloEngine::APOLLO_MakeHydraUsdApolloRenderParams(
    UsdApolloRenderParams const &renderParams)
{
  /**
   * Note this table is dangerous and making changes to the order of the
   * enums in UsdApolloCullStyle, will affect this with no compiler help. */
  static const HdCullStyle USD_2_HD_CULL_STYLE[] = {
      HdCullStyleDontCare,              /** <- Cull No Opinion (unused)            */
      HdCullStyleNothing,               /** <- CULL_STYLE_NOTHING,                 */
      HdCullStyleBack,                  /** <- CULL_STYLE_BACK,                    */
      HdCullStyleFront,                 /** <- CULL_STYLE_FRONT,                   */
      HdCullStyleBackUnlessDoubleSided, /** <- CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED */
  };
  static_assert(((sizeof(USD_2_HD_CULL_STYLE) / sizeof(USD_2_HD_CULL_STYLE[0])) ==
                 (size_t)UsdApolloCullStyle::CULL_STYLE_COUNT),
                "enum size mismatch");

  HdxRenderTaskParams params;

  params.overrideColor  = renderParams.overrideColor;
  params.wireframeColor = renderParams.wireframeColor;

  if (renderParams.drawMode == UsdApolloDrawMode::DRAW_GEOM_ONLY ||
      renderParams.drawMode == UsdApolloDrawMode::DRAW_POINTS) {
    params.enableLighting = false;
  }
  else {
    params.enableLighting = renderParams.enableLighting && !renderParams.enableIdRender;
  }

  params.enableIdRender      = renderParams.enableIdRender;
  params.depthBiasUseDefault = true;
  params.depthFunc           = HdCmpFuncLess;
  params.cullStyle           = USD_2_HD_CULL_STYLE[(size_t)renderParams.cullStyle];

  /**
   * Decrease the alpha threshold if we are
   * using sample alpha to coverage. */
  if (renderParams.alphaThreshold < 0.0) {
    params.alphaThreshold = renderParams.enableSampleAlphaToCoverage ? 0.1f : 0.5f;
  }
  else {
    params.alphaThreshold = renderParams.alphaThreshold;
  }

  params.enableSceneMaterials = renderParams.enableSceneMaterials;
  params.enableSceneLights    = renderParams.enableSceneLights;

  /**
   * We don't provide the following because task controller ignores them:
   * - params.camera
   * - params.viewport */

  return params;
}

/* static */
void UsdApolloEngine::APOLLO_ComputeRenderTags(UsdApolloRenderParams const &params,
                                               TfTokenVector *renderTags)
{
  /**
   * Calculate the rendertags needed based
   * on the parameters passed by the app. */
  renderTags->clear();
  renderTags->reserve(4);
  renderTags->push_back(HdRenderTagTokens->geometry);
  if (params.showGuides) {
    renderTags->push_back(HdRenderTagTokens->guide);
  }
  if (params.showProxy) {
    renderTags->push_back(HdRenderTagTokens->proxy);
  }
  if (params.showRender) {
    renderTags->push_back(HdRenderTagTokens->render);
  }
}

/* static */
TfToken UsdApolloEngine::APOLLO_GetDefaultRendererPluginId()
{
  static const std::string defaultRendererDisplayName = TfGetenv("HD_DEFAULT_RENDERER", "");

  if (defaultRendererDisplayName.empty()) {
    return TfToken();
  }

  HfPluginDescVector pluginDescs;
  HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescs);

  /**
   * Look for the one with the matching display name. */
  for (size_t i = 0; i < pluginDescs.size(); ++i) {
    if (pluginDescs[i].displayName == defaultRendererDisplayName) {
      return pluginDescs[i].id;
    }
  }

  TF_WARN("Failed to find default renderer with display name '%s'.",
          defaultRendererDisplayName.c_str());

  return TfToken();
}

UsdImagingDelegate *UsdApolloEngine::APOLLO_GetSceneDelegate() const
{
  return m_sceneDelegate.get();
}

HdEngine *UsdApolloEngine::APOLLO_GetHdEngine()
{
  return m_engine.get();
}

HdxTaskController *UsdApolloEngine::APOLLO_GetTaskController() const
{
  return m_taskController.get();
}

WABI_NAMESPACE_END
