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
#include "wabi/imaging/hgi/graphicsCmdsDesc.h"

#include <ostream>

WABI_NAMESPACE_BEGIN

bool operator==(const HgiGraphicsCmdsDesc &lhs, const HgiGraphicsCmdsDesc &rhs)
{
  return lhs.depthAttachmentDesc == rhs.depthAttachmentDesc &&
         lhs.colorAttachmentDescs == rhs.colorAttachmentDescs && lhs.depthTexture == rhs.depthTexture &&
         lhs.depthResolveTexture == rhs.depthResolveTexture && lhs.colorTextures == rhs.colorTextures &&
         lhs.colorResolveTextures == rhs.colorResolveTextures;
}

bool operator!=(const HgiGraphicsCmdsDesc &lhs, const HgiGraphicsCmdsDesc &rhs)
{
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &out, const HgiGraphicsCmdsDesc &desc)
{
  out << "HgiGraphicsCmdsDesc: {";

  for (HgiAttachmentDesc const &a : desc.colorAttachmentDescs)
  {
    out << a;
  }

  for (size_t i = 0; i < desc.colorTextures.size(); i++)
  {
    out << "colorTexture" << i << " ";
    out << "dimensions:" << desc.colorTextures[i]->GetDescriptor().dimensions << ", ";
  }

  for (size_t i = 0; i < desc.colorResolveTextures.size(); i++)
  {
    out << "colorResolveTexture" << i << ", ";
  }

  if (desc.depthTexture)
  {
    out << desc.depthAttachmentDesc;
    out << "depthTexture ";
    out << "dimensions:" << desc.depthTexture->GetDescriptor().dimensions;
  }

  if (desc.depthResolveTexture)
  {
    out << "depthResolveTexture";
  }

  out << "}";
  return out;
}

WABI_NAMESPACE_END
