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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/base/gf/vec4f.h"
#include "wabi/base/tf/diagnostic.h"

#include "wabi/imaging/hgiGL/conversions.h"
#include "wabi/imaging/hgiGL/diagnostic.h"
#include "wabi/imaging/hgiGL/sampler.h"

WABI_NAMESPACE_BEGIN

HgiGLSampler::HgiGLSampler(HgiSamplerDesc const &desc) : HgiSampler(desc), _samplerId(0)
{
  glCreateSamplers(1, &_samplerId);

  if (!_descriptor.debugName.empty()) {
    HgiGLObjectLabel(GL_SAMPLER, _samplerId, _descriptor.debugName);
  }

  glSamplerParameteri(
    _samplerId, GL_TEXTURE_WRAP_S, HgiGLConversions::GetSamplerAddressMode(desc.addressModeU));

  glSamplerParameteri(
    _samplerId, GL_TEXTURE_WRAP_T, HgiGLConversions::GetSamplerAddressMode(desc.addressModeV));

  glSamplerParameteri(
    _samplerId, GL_TEXTURE_WRAP_R, HgiGLConversions::GetSamplerAddressMode(desc.addressModeW));

  glSamplerParameteri(
    _samplerId, GL_TEXTURE_MIN_FILTER, HgiGLConversions::GetMinFilter(desc.minFilter, desc.mipFilter));

  glSamplerParameteri(_samplerId, GL_TEXTURE_MAG_FILTER, HgiGLConversions::GetMagFilter(desc.magFilter));

  static const GfVec4f borderColor(0);
  glSamplerParameterfv(_samplerId, GL_TEXTURE_BORDER_COLOR, borderColor.GetArray());

  static const float maxAnisotropy = 16.0;

  glSamplerParameterf(_samplerId, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

  HGIGL_POST_PENDING_GL_ERRORS();
}

HgiGLSampler::~HgiGLSampler()
{
  glDeleteSamplers(1, &_samplerId);
  _samplerId = 0;
  HGIGL_POST_PENDING_GL_ERRORS();
}

uint64_t HgiGLSampler::GetRawResource() const
{
  return (uint64_t)_samplerId;
}

uint32_t HgiGLSampler::GetSamplerId() const
{
  return _samplerId;
}

WABI_NAMESPACE_END
