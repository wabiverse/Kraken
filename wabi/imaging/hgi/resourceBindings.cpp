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
#include "wabi/imaging/hgi/resourceBindings.h"

WABI_NAMESPACE_BEGIN

HgiResourceBindings::HgiResourceBindings(HgiResourceBindingsDesc const &desc) : _descriptor(desc)
{}

HgiResourceBindings::~HgiResourceBindings() = default;

HgiResourceBindingsDesc const &HgiResourceBindings::GetDescriptor() const
{
  return _descriptor;
}

HgiBufferBindDesc::HgiBufferBindDesc() : bindingIndex(0), stageUsage(HgiShaderStageVertex)
{}

bool operator==(const HgiBufferBindDesc &lhs, const HgiBufferBindDesc &rhs)
{
  return lhs.buffers == rhs.buffers && lhs.resourceType == rhs.resourceType &&
         lhs.offsets == rhs.offsets && lhs.bindingIndex == rhs.bindingIndex &&
         lhs.stageUsage == rhs.stageUsage;
}

bool operator!=(const HgiBufferBindDesc &lhs, const HgiBufferBindDesc &rhs)
{
  return !(lhs == rhs);
}

HgiTextureBindDesc::HgiTextureBindDesc()
    : resourceType(HgiBindResourceTypeCombinedSamplerImage),
      bindingIndex(0),
      stageUsage(HgiShaderStageFragment)
{}

bool operator==(const HgiTextureBindDesc &lhs, const HgiTextureBindDesc &rhs)
{
  return lhs.textures == rhs.textures && lhs.resourceType == rhs.resourceType &&
         lhs.bindingIndex == rhs.bindingIndex && lhs.stageUsage == rhs.stageUsage &&
         lhs.samplers == rhs.samplers;
}

bool operator!=(const HgiTextureBindDesc &lhs, const HgiTextureBindDesc &rhs)
{
  return !(lhs == rhs);
}

HgiResourceBindingsDesc::HgiResourceBindingsDesc() = default;

bool operator==(const HgiResourceBindingsDesc &lhs, const HgiResourceBindingsDesc &rhs)
{
  return lhs.debugName == rhs.debugName && lhs.buffers == rhs.buffers &&
         lhs.textures == rhs.textures;
}

bool operator!=(const HgiResourceBindingsDesc &lhs, const HgiResourceBindingsDesc &rhs)
{
  return !(lhs == rhs);
}

WABI_NAMESPACE_END
