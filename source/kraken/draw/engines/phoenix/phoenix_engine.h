/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file Draw.
 * Spontaneous Expression.
 *
 * The Phoenix Render Engine.
 * The OpenSubdiv-based real-time render engine of the 21st century.
 */

extern RenderEngineType DRW_engine_viewport_phoenix_type;

#ifdef __cplusplus

#  include <wabi/imaging/hgi/hgi.h>
#  include <wabi/imaging/hd/renderDelegate.h>
#  include <wabi/imaging/hd/rendererPluginRegistry.h>

#  include "phoenix_api.h"

/**
 * ----------------------------------------------------------------------
 * @PHOENIX: RENDER ENGINE
 * ---------------------------------------------------------------------- */

class PhoenixRenderEngine final : public wabi::HdRenderDelegate
{
 public:

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CREATION AND DESTRUCTION
   * ---------------------------------------------------------------------- */

  PHOENIX_API
  PhoenixRenderEngine();

  PHOENIX_API
  PhoenixRenderEngine(wabi::HdRenderSettingsMap const &settingsMap);

  PHOENIX_API
  virtual ~PhoenixRenderEngine();

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : PAUSE SUPPORT
   * ----------------------------------------------------------------------
   * Advertise whether this render engine supports pausing and resuming of
   * background render threads.
   *
   * @returns @c true if successful. */

  PHOENIX_API
  virtual bool IsPauseSupported() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : PAUSED STATUS
   * ----------------------------------------------------------------------
   * Query the Phoenix Render Engine's pause state. Returns true if the
   * background rendering threads are currently paused.
   *
   * @returns @c true if successful. */

  PHOENIX_API
  virtual bool IsPaused() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : PAUSE
   * ----------------------------------------------------------------------
   * Pause all of the Phoenix Render Engine's background rendering threads.
   *
   * @returns @c true if successful. */

  PHOENIX_API
  virtual bool Pause() override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : RESUME
   * ----------------------------------------------------------------------
   * Resume all of the Phoenix Render Engine's background rendering threads
   * previously paused by a call to Pause.
   *
   * @returns @c true if successful. */

  PHOENIX_API
  virtual bool Resume() override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : STOP SUPPORT
   * ----------------------------------------------------------------------
   * Advertise whether the Phoenix Render Engine supports stopping and
   * restarting of background render threads.
   *
   * @returns @c true if successful. */

  HD_API
  virtual bool IsStopSupported() const;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : STOP STATUS
   * ----------------------------------------------------------------------
   * Query the Phoenix Render Engine's stop state. Returns true if the
   * background rendering threads are not currently active.
   *
   * @returns @c true if successful. */

  HD_API
  virtual bool IsStopped() const;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : STOP
   * ----------------------------------------------------------------------
   * Stop all of the Phoenix Render Engine's background rendering threads;
   * if blocking is true, the function waits until they exit.
   *
   * @returns @c true if successful. */

  HD_API
  virtual bool Stop(bool blocking = true);

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: CONTROL OF BACKGROUND RENDERING THREADS : RESTART
   * ----------------------------------------------------------------------
   * Restart all of this delegate's background rendering threads previously
   * stopped by a call to Stop.
   *
   * @returns @c true if successful. */

  HD_API
  virtual bool Restart();

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: DRIVERS
   * ----------------------------------------------------------------------
   * Set list of driver objects, such as a rendering context / devices.
   * This is automatically called from HdRenderIndex when a HdDriver is
   * provided during its construction. Default implementation does nothing. */

  PHOENIX_API
  virtual void SetDrivers(HdDriverVector const &drivers) override;


  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: RENDER PARAMETERS
   * ----------------------------------------------------------------------
   * Returns an opaque handle to a render param, that in turn is
   * passed to each prim created by the render delegate during sync
   * processing.  This avoids the need to store a global state pointer
   * in each prim.
   *
   * The typical lifetime of the renderParam would match that of the
   * RenderDelegate, however the minimal lifetime is that of the Sync
   * processing.  The param maybe queried multiple times during sync.
   *
   * A render delegate may return null for the param. */

  PHOENIX_API
  virtual HdRenderParam *GetRenderParam() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: SUPPORTED PRIM TYPES
   * ----------------------------------------------------------------------
   * Returns a list of Pixar Type ID's of all supported types that Phoenix
   * supports, and is broken down into three classifications.
   *
   * @RPRIM: Renderable primitive types, like meshes, curves, volumes, etc.
   *
   * @SPRIM: State primitive types, like cameras, materials, lights, etc.
   *
   * @BPRIM: Buffer primitive types, like render buffers, openvdb assets, etc. */

  PHOENIX_API
  virtual const TfTokenVector &GetSupportedRprimTypes() const override;

  PHOENIX_API
  virtual const TfTokenVector &GetSupportedSprimTypes() const override;

  PHOENIX_API
  virtual const TfTokenVector &GetSupportedBprimTypes() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: RESOURCE REGISTRY
   * ----------------------------------------------------------------------
   * The central registry for GPU resources, returns a pointer to the shared
   * Hydra resource registry. */

  PHOENIX_API
  virtual HdResourceRegistrySharedPtr GetResourceRegistry() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: REQUEST A NEW RENDER PASS
   * ----------------------------------------------------------------------
   * @param index the render index to bind to the new renderpass.
   * @param collection the rprim collection to bind to the new renderpass.
   *
   * @return A shared pointer to the new renderpass or empty on error. */

  PHOENIX_API
  virtual HdRenderPassSharedPtr CreateRenderPass(HdRenderIndex *index,
                                                 HdRprimCollection const &collection) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: REQUEST TO CREATE A NEW RENDER PASS STATE
   * ----------------------------------------------------------------------
   * The default implementation creates an HdRenderPassState instance,
   * but derived render delegates may instantiate their own state type.
   * @param shader the render pass shader to use. If null, a new shared
   *               render pass (#HdRenderPassShared) will be created.
   *
   * @return A shared pointer to the new renderpass state. */

  PHOENIX_API
  virtual HdRenderPassStateSharedPtr CreateRenderPassState() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: INSTANCER FACTORY : CREATION / DESTRUCTION
   * ----------------------------------------------------------------------
   * Request to create a new instancer.
   * @param delegate The scene delegate, managing the scenegraph.
   * @param id The unique identifier of this instancer.
   *
   * @return A pointer to the new instancer or nullptr on error. */

  PHOENIX_API
  virtual HdInstancer *CreateInstancer(HdSceneDelegate *delegate, SdfPath const &id) override;

  PHOENIX_API
  virtual void DestroyInstancer(HdInstancer *instancer) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: PRIM FACTORIES : RENDERABLE PRIMITIVES
   * ----------------------------------------------------------------------
   * Request to Allocate and Construct a new RPrim.
   * @param typeId the type identifier of the prim to allocate
   * @param rprimId a unique identifier for the prim
   *
   * @return A pointer to the new prim or nullptr on error. */

  PHOENIX_API
  virtual HdRprim *CreateRprim(TfToken const &typeId, SdfPath const &rprimId) override;

  PHOENIX_API
  virtual void DestroyRprim(HdRprim *rPrim) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: PRIM FACTORIES : STATE PRIMITIVES
   * ----------------------------------------------------------------------
   * Request to Allocate and Construct a new SPrim.
   * @param typeId the type identifier of the prim to allocate
   * @param sprimId a unique identifier for the prim
   *
   * @return A pointer to the new prim or nullptr on error. */

  PHOENIX_API
  virtual HdSprim *CreateSprim(TfToken const &typeId, SdfPath const &sprimId) override;

  PHOENIX_API
  virtual HdSprim *CreateFallbackSprim(TfToken const &typeId) override;

  PHOENIX_API
  virtual void DestroySprim(HdSprim *sPrim) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: PRIM FACTORIES : BUFFER PRIMITIVES
   * ----------------------------------------------------------------------
   * Request to Allocate and Construct a new BPrim.
   * @param typeId the type identifier of the prim to allocate
   * @param bprimId a unique identifier for the prim
   *
   * @return A pointer to the new prim or nullptr on error. */

  PHOENIX_API
  virtual HdBprim *CreateBprim(TfToken const &typeId, SdfPath const &bprimId) override;

  PHOENIX_API
  virtual HdBprim *CreateFallbackBprim(TfToken const &typeId) override;

  PHOENIX_API
  virtual void DestroyBprim(HdBprim *bPrim) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: SYNCHRONIZATION, EXECUTION, & DISPATCH HOOKS
   * ----------------------------------------------------------------------
   * Notification point from the Engine to the delegate.
   * This notification occurs after all Sync's have completed and
   * before task execution.
   *
   * This notification gives the Render Delegate a chance to
   * update and move memory that the render may need.
   *
   * For example, the render delegate might fill primvar buffers or
   * texture memory. */

  PHOENIX_API
  virtual void CommitResources(HdChangeTracker *tracker) override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: MATERIALS : BINDINGS
   * ----------------------------------------------------------------------
   * Returns a token that indicates material bindings should be used,
   * based on the indicated purpose.  The default purpose is
   * HdTokens->preview.
   *
   * This notification gives the Render Delegate a chance to
   * update and move memory that the render may need.
   *
   * For example, the render delegate might fill primvar buffers or
   * texture memory. */

  PHOENIX_API
  virtual TfToken GetMaterialBindingPurpose() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: MATERIALS : MATERIAL NETWORKS
   * ----------------------------------------------------------------------
   * Returns a list, in descending order of preference, that can be used to
   * select among multiple material network implementations. The default
   * list contains an empty token. */

  PHOENIX_API
  virtual TfTokenVector GetMaterialRenderContexts() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: PRIMVARS : RENDERABLE PRIMVAR FILTERING
   * ----------------------------------------------------------------------
   * Return true to indicate that the render delegate wants rprim primvars
   * to be filtered by the scene delegate to reduce the amount of primvars
   * that are send to the render delegate. For example the scene delegate
   * may check the bound material primvar requirements and send only those
   * to the render delegate. Return false to not apply primvar filtering in
   * the scene delegate. Defaults to false. */

  PHOENIX_API
  virtual bool IsPrimvarFilteringNeeded() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: SHADERS : SUPPORTED SHADER SOURCE TYPES
   * ----------------------------------------------------------------------
   * Returns the ordered list of shader source types that the render delegate
   * supports. */

  PHOENIX_API
  virtual TfTokenVector GetShaderSourceTypes() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: AOVs
   * ----------------------------------------------------------------------
   * Returns a default AOV descriptor for the given named AOV, specifying
   * things like preferred format. */

  PHOENIX_API
  virtual HdAovDescriptor GetDefaultAovDescriptor(TfToken const &name) const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: BACKEND EXPORTED RENDER SETTING DESCRIPTORS
   * ----------------------------------------------------------------------
   * Returns a list of render settings supported by the Phoenix Rendering
   * engine. */

  PHOENIX_API
  virtual HdRenderSettingDescriptorList GetRenderSettingDescriptors() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: BENCHMARKING
   * ----------------------------------------------------------------------
   * Returns a open-format dictionary of render statistics. */

  PHOENIX_API
  virtual VtDictionary GetRenderStats() const override;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: HYDRA GRAPHICS INTERFACE
   * ----------------------------------------------------------------------
   * Returns a pointer to the underlying Hydra Graphics Interface (#Hgi). */

  PHOENIX_API
  Hgi *GetHgi();

 protected:

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: NO COPIES
   * ----------------------------------------------------------------------
   * This Phoenix Rendering Engine class is not intended to be copied. */

  PhoenixRenderEngine(const PhoenixRenderEngine &) = delete;
  PhoenixRenderEngine &operator=(const PhoenixRenderEngine &) = delete;

  /** Hydra Graphics Interface. */
  wabi::Hgi *m_hgi;

  /** Phoenix Render Engine Settings State. */
  HdRenderSettingsMap m_settingsMap;
  /** Phoenix Render Engine Version. */
  unsigned int m_phoenix_version;

 private:

  friend class wabi::HdRendererPluginRegistry;

  /**
   * ----------------------------------------------------------------------
   * @PHOENIX: RENDER ENGINE DISPLAY NAME
   * ----------------------------------------------------------------------
   * Populated when instantiated via the HdRendererPluginRegistry and
   * currently used to associate a Phoenix Render Engine instance with
   * related code and resources. */
  void set_renderer_display_name(const std::string &displayName)
  {
    m_displayName = displayName;
  }

  /** Phoenix Render Engine Display Name, typically "Phoenix". */
  std::string m_displayName;
};

#endif /* __cplusplus */
