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

#include "wabi/imaging/plugin/hdPrmanLoader/rendererPlugin.h"
#include "wabi/base/arch/library.h"
#include "wabi/base/plug/plugin.h"
#include "wabi/base/plug/registry.h"
#include "wabi/base/tf/getenv.h"
#include "wabi/base/tf/setenv.h"
#include "wabi/imaging/hd/rendererPluginRegistry.h"

WABI_NAMESPACE_BEGIN

typedef HdRenderDelegate *(*CreateDelegateFunc)(HdRenderSettingsMap const &settingsMap);
typedef void (*DeleteDelegateFunc)(HdRenderDelegate *renderDelegate);

static const std::string k_RMANTREE("RMANTREE");
#if defined(ARCH_OS_WINDOWS)
static const std::string k_PATH("PATH");
#endif

// This holds the OS specific plugin info data
static struct HdPrmanLoader {
  static void Load();
  ~HdPrmanLoader();
#if defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN)
  void *libprman = nullptr;
#endif
  void *hdxPrman                = nullptr;
  CreateDelegateFunc createFunc = nullptr;
  DeleteDelegateFunc deleteFunc = nullptr;
  bool valid                    = false;
} _hdPrman;

void HdPrmanLoader::Load()
{
  static bool inited = false;
  if (inited) {
    return;
  }
  inited = true;

  const std::string rmantree = TfGetenv(k_RMANTREE);
  if (rmantree.empty()) {
    TF_WARN("The hdPrmanLoader backend requires $RMANTREE to be set.");
    return;
  }

#if defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN)
  // Open $RMANTREE/lib/libprman.so into the global namespace
  const std::string libprmanPath = TfStringCatPaths(rmantree, "lib/libprman" ARCH_LIBRARY_SUFFIX);
  _hdPrman.libprman = ArchLibraryOpen(libprmanPath, ARCH_LIBRARY_NOW | ARCH_LIBRARY_GLOBAL);
  if (!_hdPrman.libprman) {
    TF_WARN("Could not load libprman.");
    return;
  }
#elif defined(ARCH_OS_WINDOWS)
  // Append PATH environment with %RMANTREE%\bin and %RMANTREE%\lib
  std::string path = TfGetenv(k_PATH);
  path += ';' + TfStringCatPaths(rmantree, "bin");
  path += ';' + TfStringCatPaths(rmantree, "lib");
  TfSetenv(k_PATH, path.c_str());
#endif

  // hdxPrman is assumed to be next to hdPrmanLoader (this plugin)
  PlugPluginPtr plugin = PlugRegistry::GetInstance().GetPluginWithName("hdxPrman");
  if (plugin) {
    _hdPrman.hdxPrman = ArchLibraryOpen(plugin->GetPath(), ARCH_LIBRARY_NOW | ARCH_LIBRARY_LOCAL);
  }
  if (!_hdPrman.hdxPrman) {
    TF_WARN("Could not load versioned hdPrman backend: %s", ArchLibraryError().c_str());
    return;
  }

  _hdPrman.createFunc = reinterpret_cast<CreateDelegateFunc>(
      ArchLibraryGetSymbolAddress(_hdPrman.hdxPrman, "HdPrmanLoaderCreateDelegate"));
  _hdPrman.deleteFunc = reinterpret_cast<DeleteDelegateFunc>(
      ArchLibraryGetSymbolAddress(_hdPrman.hdxPrman, "HdPrmanLoaderDeleteDelegate"));
  if (!_hdPrman.createFunc || !_hdPrman.deleteFunc) {
    TF_WARN("hdPrmanLoader factory methods could not be found.");
    return;
  }

  _hdPrman.valid = true;
}

HdPrmanLoader::~HdPrmanLoader()
{
  if (hdxPrman) {
    // Note: OSX does not support clean unloading of hdxPrman.dylib symbols
    ArchLibraryClose(hdxPrman);
    hdxPrman = nullptr;
  }
#if defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN)
  if (libprman) {
    ArchLibraryClose(libprman);
    libprman = nullptr;
  }
#endif
}

// Register the hdPrman loader plugin
TF_REGISTRY_FUNCTION(TfType)
{
  HdRendererPluginRegistry::Define<HdPrmanLoaderRendererPlugin>();
}

HdPrmanLoaderRendererPlugin::HdPrmanLoaderRendererPlugin()
{
  HdPrmanLoader::Load();
}

HdPrmanLoaderRendererPlugin::~HdPrmanLoaderRendererPlugin()
{}

HdRenderDelegate *HdPrmanLoaderRendererPlugin::CreateRenderDelegate()
{
  if (_hdPrman.valid) {
    HdRenderSettingsMap settingsMap;
    return _hdPrman.createFunc(settingsMap);
  }
  return nullptr;
}

HdRenderDelegate *HdPrmanLoaderRendererPlugin::CreateRenderDelegate(
    HdRenderSettingsMap const &settingsMap)
{
  if (_hdPrman.valid) {
    return _hdPrman.createFunc(settingsMap);
  }
  return nullptr;
}

void HdPrmanLoaderRendererPlugin::DeleteRenderDelegate(HdRenderDelegate *renderDelegate)
{
  if (_hdPrman.valid) {
    _hdPrman.deleteFunc(renderDelegate);
  }
}

bool HdPrmanLoaderRendererPlugin::IsSupported() const
{
  return _hdPrman.valid;
}

WABI_NAMESPACE_END
