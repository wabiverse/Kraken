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

#ifndef WABI_USD_IMAGING_USD_APOLLO_ENGINE_H
#define WABI_USD_IMAGING_USD_APOLLO_ENGINE_H

#ifdef WITH_VULKAN
# include <vulkan/vulkan.h>
#endif /* WITH_VULKAN */

#include "wabi/usdImaging/usdApollo/api.h"
#include "wabi/usdImaging/usdApollo/version.h"
#include "wabi/usdImaging/usdImaging/version.h"
#include "wabi/wabi.h"

#include "wabi/usdImaging/usdApollo/renderParams.h"
#include "wabi/usdImaging/usdApollo/rendererSettings.h"

#include "wabi/imaging/cameraUtil/conformWindow.h"

#include "wabi/imaging/hd/driver.h"
#include "wabi/imaging/hd/engine.h"
#include "wabi/imaging/hd/pluginRenderDelegateUniqueHandle.h"
#include "wabi/imaging/hd/rprimCollection.h"

#include "wabi/imaging/hdx/renderSetupTask.h"
#include "wabi/imaging/hdx/selectionTracker.h"

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgiVulkan/hgi.h"

#include "wabi/usd/sdf/path.h"
#include "wabi/usd/usd/timeCode.h"

#include "wabi/base/gf/frustum.h"
#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec4d.h"
#include "wabi/base/gf/vec4f.h"
#include "wabi/base/gf/vec4i.h"

#include "wabi/base/vt/dictionary.h"

#include "wabi/base/tf/declarePtrs.h"

WABI_NAMESPACE_BEGIN

class UsdPrim;
class HdRenderIndex;
class HdxTaskController;
class UsdImagingDelegate;

/**
 * @class UsdApolloEngine
 * The UsdApolloEngine is the main entry point API for rendering
 * full scale USD scenes with VULKAN. */

class UsdApolloEngine
{
 public:
  /**
   * ---------------------------------------------------------------------
   *  @name Global State
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Returns true if Hydra is enabled for VULKAN drawing. */
  USDAPOLLO_API
  static bool IsHydraEnabled();

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Construction
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * A HdDriver, containing the Hgi of your choice, can be optionally passed
   * in during construction. This can be helpful if your application creates
   * multiple UsdApolloEngines that wish to use the same HdDriver / Hgi. */
  USDAPOLLO_API
  UsdApolloEngine(const HdDriver &driver = HdDriver());

  USDAPOLLO_API
  UsdApolloEngine(const SdfPath &rootPath,
                  const SdfPathVector &excludedPaths,
                  const SdfPathVector &invisedPaths = SdfPathVector(),
                  const SdfPath &sceneDelegateID = SdfPath::AbsoluteRootPath(),
                  const HdDriver &driver = HdDriver());

  /**
   * Disallow copies. */
  UsdApolloEngine(const UsdApolloEngine &) = delete;
  UsdApolloEngine &operator=(const UsdApolloEngine &) = delete;

  USDAPOLLO_API
  ~UsdApolloEngine();

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Rendering
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Support for batched drawing. */
  USDAPOLLO_API
  void PrepareBatch(const UsdPrim &root, const UsdApolloRenderParams &params);
  USDAPOLLO_API
  void RenderBatch(const SdfPathVector &paths, const UsdApolloRenderParams &params);

  /**
   * Entry point for kicking off a render. */
  USDAPOLLO_API
  void Render(const UsdPrim &root, const UsdApolloRenderParams &params);

  /**
   * Returns true if the resulting image is fully converged. Otherwise,
   * caller may need to call Render() again to refine the result. */
  USDAPOLLO_API
  bool IsConverged() const;

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Root Transform and Visibility
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Sets the root transform. */
  USDAPOLLO_API
  void SetRootTransform(GfMatrix4d const &xf);

  /**
   * Sets the root visibility. */
  USDAPOLLO_API
  void SetRootVisibility(bool isVisible);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Camera State
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Scene camera API
   * Set the scene camera path to use for rendering. */
  USDAPOLLO_API
  void SetCameraPath(SdfPath const &id);

  /**
   * Determines how the filmback of the camera is mapped into
   * the pixels of the render buffer and what pixels of the render
   * buffer will be rendered into. */
  USDAPOLLO_API
  void SetFraming(CameraUtilFraming const &framing);

  /**
   * Specifies whether to force a window policy when conforming
   * the frustum of the camera to match the display window of
   * the camera framing.
   *
   * If set to {false, ...}, the window policy of the specified camera
   * will be used.
   *
   * Note: std::pair<bool, ...> is used instead of std::optional<...>
   * because the latter is only available in C++17 or later. */
  USDAPOLLO_API
  void SetOverrideWindowPolicy(const std::pair<bool, CameraUtilConformWindowPolicy> &policy);

  /**
   * Set the size of the render buffers baking the AOVs.
   * GUI applications should set this to the size of the window. */
  USDAPOLLO_API
  void SetRenderBufferSize(GfVec2i const &size);

  /**
   * Set the window policy to use.
   * XXX: This is currently used for scene cameras set via SetCameraPath.
   * See comment in SetCameraState for the free cam. */
  USDAPOLLO_API
  void SetWindowPolicy(CameraUtilConformWindowPolicy policy);

  /**
   * Free camera API
   * Set camera framing state directly (without pointing to a camera on the
   * USD stage). The projection matrix is expected to be pre-adjusted for the
   * window policy. */
  USDAPOLLO_API
  void SetCameraState(const GfMatrix4d &viewMatrix, const GfMatrix4d &projectionMatrix);

  /**
   * Helper function to extract camera and viewport state from
   * vulkan and then call SetCameraState and SetRenderViewport. */
  USDAPOLLO_API
  void SetCameraStateFromVulkan();

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Light State
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Helper function to extract lighting state
   * from vulkan and then call SetLights. */
  USDAPOLLO_API
  void SetLightingStateFromVulkan();

  /**
   * Copy lighting state from another lighting context. */
  USDAPOLLO_API
  void SetLightingState(/** TODO: GlfSimpleLightingContextPtr const &src*/ void);

  /**
   * Set lighting state
   * Derived classes should ensure that passing an empty lights
   * vector disables lighting.
   * @param lights is the set of lights to use, or empty to disable lighting. */
  USDAPOLLO_API
  void SetLightingState(/** TODO: VkfSimpleLightVector const &lights,
                            TODO: VkfSimpleMaterial const &material, */
                        GfVec4f const &sceneAmbient);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Selection Highlighting
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Sets (replaces) the list of prim paths that should be included in
   * selection highlighting. These paths may include root paths which will
   * be expanded internally. */
  USDAPOLLO_API
  void SetSelected(SdfPathVector const &paths);

  /**
   * Clear the list of prim paths that should be included in selection
   * highlighting. */
  USDAPOLLO_API
  void ClearSelected();

  /**
   * Add a path with instanceIndex to the list of prim paths that should be
   * included in selection highlighting. UsdImagingDelegate::ALL_INSTANCES
   * can be used for highlighting all instances if path is an instancer. */
  USDAPOLLO_API
  void AddSelected(SdfPath const &path, int instanceIndex);

  /**
   * Sets the selection highlighting color. */
  USDAPOLLO_API
  void SetSelectionColor(GfVec4f const &color);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Renderer Plugin Management
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Finds closest point of intersection with a frustum by rendering.
   *
   * This method uses a PickRender and a customized depth buffer to find an
   * approximate point of intersection by rendering. This is less accurate
   * than implicit methods or rendering with GL_SELECT, but leverages any
   * data already cached in the renderer.
   *
   * Returns whether a hit occurred and if so, @p outHitPoint will contain
   * the intersection point in world space (i.e. @p projectionMatrix and
   * @p viewMatrix factored back out of the result), and @p outHitNormal
   * will contain the world space normal at that point.
   *
   * @p outHitPrimPath will point to the gprim selected by the pick.
   * @p outHitInstancerPath will point to the point instancer (if applicable)
   * of that gprim. For nested instancing, outHitInstancerPath points to
   * the closest instancer. */
  USDAPOLLO_API
  bool TestIntersection(const GfMatrix4d &viewMatrix,
                        const GfMatrix4d &projectionMatrix,
                        const UsdPrim &root,
                        const UsdApolloRenderParams &params,
                        GfVec3d *outHitPoint,
                        GfVec3d *outHitNormal,
                        SdfPath *outHitPrimPath = NULL,
                        SdfPath *outHitInstancerPath = NULL,
                        int *outHitInstanceIndex = NULL,
                        HdInstancerContext *outInstancerContext = NULL);

  /**
   * Decodes a pick result given hydra prim ID/instance ID
   * (like you'd get from an ID render). */
  USDAPOLLO_API
  bool DecodeIntersection(unsigned char const primIdColor[4],
                          unsigned char const instanceIdColor[4],
                          SdfPath *outHitPrimPath = NULL,
                          SdfPath *outHitInstancerPath = NULL,
                          int *outHitInstanceIndex = NULL,
                          HdInstancerContext *outInstancerContext = NULL);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Renderer Plugin Management
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Return the vector of available render-graph delegate plugins. */
  USDAPOLLO_API
  static TfTokenVector GetRendererPlugins();

  /**
   * Return the user-friendly description of a renderer plugin. */
  USDAPOLLO_API
  static std::string GetRendererDisplayName(TfToken const &id);

  /**
   * Return the id of the currently used renderer plugin. */
  USDAPOLLO_API
  TfToken GetCurrentRendererId() const;

  /**
   * Set the current render-graph delegate to @p
   * id the plugin will be loaded if it's not yet. */
  USDAPOLLO_API
  bool SetRendererPlugin(TfToken const &id);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name AOVs and Renderer Settings
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Return the vector of available renderer AOV settings. */
  USDAPOLLO_API
  TfTokenVector GetRendererAovs() const;

  /**
   * Set the current renderer AOV to @p id. */
  USDAPOLLO_API
  bool SetRendererAov(TfToken const &id);

  /**
   * Returns an AOV texture handle for the given token. */
  USDAPOLLO_API
  HgiTextureHandle GetAovTexture(TfToken const &name) const;

  /**
   * Returns the list of renderer settings. */
  USDAPOLLO_API
  UsdApolloRendererSettingsList GetRendererSettingsList() const;

  /**
   * Gets a renderer setting's current value. */
  USDAPOLLO_API
  VtValue GetRendererSetting(TfToken const &id) const;

  /**
   * Sets a renderer setting's value. */
  USDAPOLLO_API
  void SetRendererSetting(TfToken const &id, VtValue const &value);

  /**
   * Enable / disable presenting the render to bound framebuffer.
   * An application may choose to manage the AOVs that are rendered
   * into itself and skip the engine's presentation. */
  USDAPOLLO_API
  void SetEnablePresentation(bool enabled);

  /**
   * The destination API (e.g., Vulkan, see hgiInterop for details)
   * and framebuffer that the AOVs are presented into. The framebuffer
   * is a VtValue that encoding a framebuffer in a destination API
   * specific way.
   * E.g., a ??? (aka ???) for framebuffer object?? for Vulkan. */
  USDAPOLLO_API
  void SetPresentationOutput(TfToken const &api, VtValue const &framebuffer);

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Renderer Command API
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Return command deescriptors for commands supported by the active
   * render delegate. */
  USDAPOLLO_API
  HdCommandDescriptors GetRendererCommandDescriptors() const;

  /**
   * Invokes command on the active render delegate. If successful, returns
   * @c true, returns @c false otherwise. Note that the command will not
   * succeeed if it is not among those returned by
   * GetRendererCommandDescriptors() for the same active render delegate. */
  USDAPOLLO_API
  bool InvokeRendererCommand(const TfToken &command, const HdCommandArgs &args = HdCommandArgs()) const;

  /**
   * ---------------------------------------------------------------------
   *  @name Control of Background Rendering Threads
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Query the renderer as to whether it supports pausing and resuming. */
  USDAPOLLO_API
  bool IsPauseRendererSupported() const;

  /**
   * Pause the renderer.
   *
   * Returns @c true if successful. */
  USDAPOLLO_API
  bool PauseRenderer();

  /**
   * Resume the renderer.
   *
   * Returns @c true if successful. */
  USDAPOLLO_API
  bool ResumeRenderer();

  /**
   * Query the renderer as to whether it supports stopping and restarting. */
  USDAPOLLO_API
  bool IsStopRendererSupported() const;

  /**
   * Stop the renderer.
   *
   * Returns @c true if successful. */
  USDAPOLLO_API
  bool StopRenderer();

  /**
   * Restart the renderer.
   *
   * Returns @c true if successful. */
  USDAPOLLO_API
  bool RestartRenderer();

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Color Correction
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Set @p id to one of the HdxColorCorrectionTokens. */
  USDAPOLLO_API
  void SetColorCorrectionSettings(TfToken const &id);

  /**
   * @} */

  /**
   * Returns true if the platform is color correction capable. */
  USDAPOLLO_API
  static bool IsColorCorrectionCapable();

  /**
   * ---------------------------------------------------------------------
   *  @name Render Statistics
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Returns render statistics.
   *
   * The contents of the dictionary will depend
   * on the current render delegate. */
  USDAPOLLO_API
  VtDictionary GetRenderStats() const;

  /**
   * @} */

  /**
   * ---------------------------------------------------------------------
   *  @name Hydra Graphics Interface (HGI)
   *  @{
   * --------------------------------------------------------------------- */

  /**
   * Returns the HGI interface. */
  USDAPOLLO_API
  Hgi *GetHgi();

  /**
   * @} */

 protected:
  /**
   * Returns the render index of the engine, if any.  This is only used for
   * whitebox testing. */
  USDAPOLLO_API
  HdRenderIndex *APOLLO_GetRenderIndex() const;

  USDAPOLLO_API
  void APOLLO_Execute(const UsdApolloRenderParams &params, HdTaskSharedPtrVector tasks);

  USDAPOLLO_API
  bool APOLLO_CanPrepare(const UsdPrim &root);
  USDAPOLLO_API
  void APOLLO_PreSetTime(const UsdApolloRenderParams &params);
  USDAPOLLO_API
  void APOLLO_PostSetTime(const UsdApolloRenderParams &params);

  USDAPOLLO_API
  void APOLLO_PrepareRender(const UsdApolloRenderParams &params);

  /**
   * Create a hydra collection given root paths and render params.
   * Returns true if the collection was updated. */
  USDAPOLLO_API
  static bool APOLLO_UpdateHydraCollection(HdRprimCollection *collection,
                                           SdfPathVector const &roots,
                                           UsdApolloRenderParams const &params);
  USDAPOLLO_API
  static HdxRenderTaskParams APOLLO_MakeHydraUsdApolloRenderParams(UsdApolloRenderParams const &params);

  USDAPOLLO_API
  static void APOLLO_ComputeRenderTags(UsdApolloRenderParams const &params, TfTokenVector *renderTags);

  USDAPOLLO_API
  void APOLLO_InitializeHgiIfNecessary();

  USDAPOLLO_API
  void APOLLO_SetRenderDelegateAndRestoreState(HdPluginRenderDelegateUniqueHandle &&);

  USDAPOLLO_API
  void APOLLO_SetRenderDelegate(HdPluginRenderDelegateUniqueHandle &&);

  USDAPOLLO_API
  SdfPath APOLLO_ComputeControllerPath(const HdPluginRenderDelegateUniqueHandle &);

  USDAPOLLO_API
  static TfToken APOLLO_GetDefaultRendererPluginId();

  USDAPOLLO_API
  UsdImagingDelegate *APOLLO_GetSceneDelegate() const;

  USDAPOLLO_API
  HdEngine *APOLLO_GetHdEngine();

  USDAPOLLO_API
  HdxTaskController *APOLLO_GetTaskController() const;

  USDAPOLLO_API
  bool APOLLO_IsUsingLegacyImpl() const;

  USDAPOLLO_API
  HdSelectionSharedPtr APOLLO_GetSelection() const;

 protected:
  HgiUniquePtr m_hgi;
  HdDriver m_hgiDriver;
  VtValue m_userFramebuffer;

  HdPluginRenderDelegateUniqueHandle m_renderDelegate;
  std::unique_ptr<HdRenderIndex> m_renderIndex;

  SdfPath const m_sceneDelegateId;

  std::unique_ptr<HdxTaskController> m_taskController;

  HdxSelectionTrackerSharedPtr m_selTracker;
  HdRprimCollection m_renderCollection;
  HdRprimCollection m_intersectCollection;

  /** Data we want to live across render plugin switches. */
  GfVec4f m_selectionColor;

  SdfPath m_rootPath;
  SdfPathVector m_excludedPrimPaths;
  SdfPathVector m_invisedPrimPaths;
  bool m_isPopulated;

 private:
  void APOLLO_DestroyHydraObjects();

  std::unique_ptr<UsdImagingDelegate> m_sceneDelegate;
  std::unique_ptr<HdEngine> m_engine;
};

WABI_NAMESPACE_END

/** Apollo Hydra Engine. */
typedef std::shared_ptr<class wabi::UsdApolloEngine> APOLLO_EnginePtr;

#endif /* WABI_USD_IMAGING_USD_APOLLO_ENGINE_H */
