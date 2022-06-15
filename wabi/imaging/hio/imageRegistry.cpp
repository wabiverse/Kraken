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
#include "wabi/imaging/hio/imageRegistry.h"
#include "wabi/imaging/hio/image.h"
#include "wabi/imaging/hio/rankedTypeMap.h"

#include "wabi/imaging/hio/debugCodes.h"

#include "wabi/usd/ar/resolver.h"

#include "wabi/base/plug/plugin.h"
#include "wabi/base/plug/registry.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/instantiateSingleton.h"
#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

#include "wabi/base/trace/trace.h"

#include <set>
#include <string>

WABI_NAMESPACE_BEGIN


TF_DEFINE_ENV_SETTING(HIO_IMAGE_PLUGIN_RESTRICTION, "",
                  "Restricts HioImage plugin loading to the specified plugin");

TF_INSTANTIATE_SINGLETON(HioImageRegistry);

HioImageRegistry &HioImageRegistry::GetInstance()
{
  return TfSingleton<HioImageRegistry>::GetInstance();
}

HioImageRegistry::HioImageRegistry() : _typeMap(std::make_unique<HioRankedTypeMap>())
{
  // Register all image types using plugin metadata.
  _typeMap->Add(TfType::Find<HioImage>(),
                "imageTypes",
                HIO_DEBUG_TEXTURE_IMAGE_PLUGINS,
                TfGetEnvSetting(HIO_IMAGE_PLUGIN_RESTRICTION));
}

HioImageSharedPtr HioImageRegistry::_ConstructImage(std::string const &filename)
{
  TRACE_FUNCTION();

  // Lookup the plug-in type name based on the filename.
  const TfToken fileExtension(TfStringToLower(ArGetResolver().GetExtension(filename)));

  TfType const &pluginType = _typeMap->Find(fileExtension);

  if (!pluginType) {
    // Unknown prim type.
    TF_DEBUG(HIO_DEBUG_TEXTURE_IMAGE_PLUGINS)
      .Msg("[PluginLoad] Unknown image type '%s' for file '%s'\n",
           fileExtension.GetText(),
           filename.c_str());
    return nullptr;
  }

  PlugRegistry &plugReg = PlugRegistry::GetInstance();
  PlugPluginPtr const plugin = plugReg.GetPluginForType(pluginType);
  if (!plugin || !plugin->Load()) {
    TF_CODING_ERROR(
      "[PluginLoad] PlugPlugin could not be loaded for "
      "TfType '%s'\n",
      pluginType.GetTypeName().c_str());
    return nullptr;
  }

  HioImageFactoryBase *const factory = pluginType.GetFactory<HioImageFactoryBase>();
  if (!factory) {
    TF_CODING_ERROR(
      "[PluginLoad] Cannot manufacture type '%s' "
      "for image type '%s' for file '%s'\n",
      pluginType.GetTypeName().c_str(),
      fileExtension.GetText(),
      filename.c_str());

    return nullptr;
  }

  HioImageSharedPtr const instance = factory->New();
  if (!instance) {
    TF_CODING_ERROR(
      "[PluginLoad] Cannot construct instance of type '%s' "
      "for image type '%s' for file '%s'\n",
      pluginType.GetTypeName().c_str(),
      fileExtension.GetText(),
      filename.c_str());
    return nullptr;
  }

  TF_DEBUG(HIO_DEBUG_TEXTURE_IMAGE_PLUGINS)
    .Msg(
      "[PluginLoad] Loaded plugin '%s' for image type '%s' for "
      "file '%s'\n",
      pluginType.GetTypeName().c_str(),
      fileExtension.GetText(),
      filename.c_str());

  return instance;
}

bool HioImageRegistry::IsSupportedImageFile(std::string const &filename)
{
  // We support image files for which we can construct an image object.
  return _ConstructImage(filename) != 0;
}

WABI_NAMESPACE_END
