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
#ifndef WABI_IMAGING_HD_RENDERER_PLUGIN_H
#define WABI_IMAGING_HD_RENDERER_PLUGIN_H

#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/renderDelegate.h"
#include "wabi/imaging/hf/pluginBase.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class SdfPath;
class HdRenderIndex;
class HdPluginRenderDelegateUniqueHandle;

///
/// This class defines a renderer plugin interface for Hydra.
/// A renderer plugin is a dynamically discovered and loaded at run-time using
/// the Plug system.
///
/// This object has singleton behavior, in that is instantiated once per
/// library (managed by the plugin registry).
///
/// The class is used to factory objects that provide delegate support
/// to other parts of the Hydra Ecosystem.
///
class HdRendererPlugin : public HfPluginBase {
 public:
  ///
  /// Create a render delegate through the plugin and wrap it in a
  /// handle that keeps this plugin alive until render delegate is
  /// destroyed. Initial settings can be passed in.
  ///
  HD_API
  HdPluginRenderDelegateUniqueHandle CreateDelegate(HdRenderSettingsMap const &settingsMap = {});

  ///
  /// Look-up plugin id in plugin registry.
  ///
  HD_API
  TfToken GetPluginId() const;

  ///
  /// Clients should use CreateDelegate since this method
  /// will eventually become protected, use CreateRenderDelegateHandle
  /// instead.
  ///
  /// Factory a Render Delegate object, that Hydra can use to
  /// factory prims and communicate with a renderer.
  ///
  virtual HdRenderDelegate *CreateRenderDelegate() = 0;

  ///
  /// Clients should use CreateDelegate since this method
  /// will eventually become protected.
  ///
  /// Factory a Render Delegate object, that Hydra can use to
  /// factory prims and communicate with a renderer.  Pass in initial
  /// settings...
  ///
  HD_API
  virtual HdRenderDelegate *CreateRenderDelegate(HdRenderSettingsMap const &settingsMap);

  ///
  /// Clients should use CreateDelegate since this method
  /// will eventually become protected.
  ///
  /// Release the object factoried by CreateRenderDelegate().
  ///
  virtual void DeleteRenderDelegate(HdRenderDelegate *renderDelegate) = 0;

  ///
  /// Returns \c true if this renderer plugin is supported in the running
  /// process and \c false if not.
  ///
  /// This gives the plugin a chance to perform some runtime checks to make
  /// sure that the system meets minimum requirements.
  ///
  virtual bool IsSupported() const = 0;

 protected:
  HdRendererPlugin() = default;
  HD_API
  ~HdRendererPlugin() override;

 private:
  // This class doesn't require copy support.
  HdRendererPlugin(const HdRendererPlugin &) = delete;
  HdRendererPlugin &operator=(const HdRendererPlugin &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_RENDERER_PLUGIN_H
