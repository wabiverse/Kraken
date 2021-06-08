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
#ifndef WABI_IMAGING_HD_RENDERER_PLUGIN_HANDLE_H
#define WABI_IMAGING_HD_RENDERER_PLUGIN_HANDLE_H

#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/renderDelegate.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

#include <cstddef>

WABI_NAMESPACE_BEGIN

class HdRendererPlugin;
class HdPluginRenderDelegateUniqueHandle;

///
/// A handle for HdRendererPlugin also storing the plugin id.
///
/// Alleviates the need to ever call, e.g., ReleasePlugin since it
/// automatically decreases and increases the plugin's reference
/// counts with the plugin registry.
///
class HdRendererPluginHandle final {
 public:
  HdRendererPluginHandle() : _plugin(nullptr)
  {}
  HdRendererPluginHandle(const std::nullptr_t &) : _plugin(nullptr)
  {}

  HD_API
  HdRendererPluginHandle(const HdRendererPluginHandle &);

  HD_API
  ~HdRendererPluginHandle();

  HD_API
  HdRendererPluginHandle &operator=(const HdRendererPluginHandle &);

  HD_API
  HdRendererPluginHandle &operator=(const std::nullptr_t &);

  /// Get the wrapped HdRendererPlugin
  HdRendererPlugin *Get() const
  {
    return _plugin;
  }

  HdRendererPlugin *operator->() const
  {
    return _plugin;
  }
  HdRendererPlugin &operator*() const
  {
    return *_plugin;
  }

  /// Is the wrapped HdRendererPlugin valid?
  explicit operator bool() const
  {
    return _plugin;
  }

 private:
  friend class HdRendererPluginRegistry;
  friend class HdRendererPlugin;

  HdRendererPluginHandle(HdRendererPlugin *const plugin) : _plugin(plugin)
  {}

  HdRendererPlugin *_plugin;
};

WABI_NAMESPACE_END

#endif
