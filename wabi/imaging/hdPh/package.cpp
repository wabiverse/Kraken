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
#include "wabi/imaging/hdPh/package.h"

#include "wabi/base/plug/plugin.h"
#include "wabi/base/plug/thisPlugin.h"
#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/fileUtils.h"
#include "wabi/base/tf/stringUtils.h"

WABI_NAMESPACE_BEGIN

static TfToken _GetShaderPath(char const *shader)
{
  static PlugPluginPtr plugin = PLUG_THIS_PLUGIN;
  const std::string path = PlugFindPluginResource(plugin, TfStringCatPaths("shaders", shader));
  TF_VERIFY(!path.empty(), "Could not find shader: %s\n", shader);

  return TfToken(path);
}

TfToken HdPhPackageComputeShader()
{
  static TfToken s = _GetShaderPath("compute.glslfx");
  return s;
}

TfToken HdPhPackageDomeLightShader()
{
  static TfToken s = _GetShaderPath("domeLight.glslfx");
  return s;
}

TfToken HdPhPackagePtexTextureShader()
{
  static TfToken s = _GetShaderPath("ptexTexture.glslfx");
  return s;
}

TfToken HdPhPackageRenderPassShader()
{
  static TfToken s = _GetShaderPath("renderPassShader.glslfx");
  return s;
}

TfToken HdPhPackageFallbackLightingShader()
{
  static TfToken s = _GetShaderPath("fallbackLightingShader.glslfx");
  return s;
}

TfToken HdPhPackageFallbackSurfaceShader()
{
  static TfToken s = _GetShaderPath("fallbackSurface.glslfx");
  return s;
}

TfToken HdPhPackageFallbackVolumeShader()
{
  static TfToken s = _GetShaderPath("fallbackVolume.glslfx");
  return s;
}

TfToken HdPhPackageImageShader()
{
  static TfToken s = _GetShaderPath("imageShader.glslfx");
  return s;
}

TfToken HdPhPackageSimpleLightingShader()
{
  static TfToken simpleLightingShader = _GetShaderPath("simpleLightingShader.glslfx");
  return simpleLightingShader;
}

WABI_NAMESPACE_END
