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
#include "wabi/imaging/hdPh/fallbackLightingShader.h"
#include "wabi/imaging/hdPh/package.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/hf/perfLog.h"

#include "wabi/imaging/hio/glslfx.h"

#include <boost/functional/hash.hpp>

#include <string>

WABI_NAMESPACE_BEGIN

HdPh_FallbackLightingShader::HdPh_FallbackLightingShader()
{
  _glslfx.reset(new HioGlslfx(HdPhPackageFallbackLightingShader()));
}

HdPh_FallbackLightingShader::~HdPh_FallbackLightingShader()
{
  // nothing
}

/* virtual */
HdPh_FallbackLightingShader::ID HdPh_FallbackLightingShader::ComputeHash() const
{
  TfToken glslfxFile = HdPhPackageFallbackLightingShader();

  size_t hash = glslfxFile.Hash();

  return (ID)hash;
}

/* virtual */
std::string HdPh_FallbackLightingShader::GetSource(TfToken const &shaderStageKey) const
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  return _glslfx->GetSource(shaderStageKey);
}

/* virtual */
void HdPh_FallbackLightingShader::SetCamera(GfMatrix4d const &worldToViewMatrix,
                                            GfMatrix4d const &projectionMatrix)
{
  // nothing
}

void HdPh_FallbackLightingShader::BindResources(const int program,
                                                HdPh_ResourceBinder const &binder,
                                                HdRenderPassState const &state)
{
  // nothing
}

void HdPh_FallbackLightingShader::UnbindResources(const int program,
                                                  HdPh_ResourceBinder const &binder,
                                                  HdRenderPassState const &state)
{
  // nothing
}

/*virtual*/
void HdPh_FallbackLightingShader::AddBindings(HdBindingRequestVector *customBindings)
{
  // no-op
}

WABI_NAMESPACE_END
