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
#ifndef WABI_IMAGING_HGI_ATTACHMENT_DESC_H
#define WABI_IMAGING_HGI_ATTACHMENT_DESC_H

#include "wabi/base/gf/vec4f.h"
#include "wabi/imaging/hgi/api.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgi/types.h"
#include "wabi/wabi.h"
#include <vector>

WABI_NAMESPACE_BEGIN

/// \struct HgiAttachmentDesc
///
/// Describes the properties of a framebuffer attachment.
///
/// <ul>
/// <li>format:
///   The format of the attachment.
///   Must match what is set in HgiTextureDesc.</li>
/// <li>usage:
///   Describes how the texture is intended to be used.
///   Must match what is set in HgiTextureDesc.</li>
/// <li>loadOp:
///   The operation to perform on the attachment pixel data prior to rendering.</li>
/// <li>storeOp:
///   The operation to perform on the attachment pixel data after rendering.</li>
/// <li>clearValue:
///   The value to clear the attachment with (r,g,b,a) or (depth,stencil,x,x)</li>
/// <li>blendEnabled:
///   Determines if a blend operation should be applied to the attachment.</li>
/// <li> ***BlendFactor:
///   The blend factors for source and destination.</li>
/// <li> ***BlendOp:
///   The blending operation.</li>
///
struct HgiAttachmentDesc {
  HgiAttachmentDesc()
      : format(HgiFormatInvalid),
        usage(0),
        loadOp(HgiAttachmentLoadOpLoad),
        storeOp(HgiAttachmentStoreOpStore),
        clearValue(0),
        blendEnabled(false),
        srcColorBlendFactor(HgiBlendFactorZero),
        dstColorBlendFactor(HgiBlendFactorZero),
        colorBlendOp(HgiBlendOpAdd),
        srcAlphaBlendFactor(HgiBlendFactorZero),
        dstAlphaBlendFactor(HgiBlendFactorZero),
        alphaBlendOp(HgiBlendOpAdd)
  {}

  HgiFormat format;
  HgiTextureUsage usage;
  HgiAttachmentLoadOp loadOp;
  HgiAttachmentStoreOp storeOp;
  GfVec4f clearValue;
  bool blendEnabled;
  HgiBlendFactor srcColorBlendFactor;
  HgiBlendFactor dstColorBlendFactor;
  HgiBlendOp colorBlendOp;
  HgiBlendFactor srcAlphaBlendFactor;
  HgiBlendFactor dstAlphaBlendFactor;
  HgiBlendOp alphaBlendOp;
};

using HgiAttachmentDescVector = std::vector<HgiAttachmentDesc>;

HGI_API
bool operator==(const HgiAttachmentDesc &lhs, const HgiAttachmentDesc &rhs);

HGI_API
bool operator!=(const HgiAttachmentDesc &lhs, const HgiAttachmentDesc &rhs);

HGI_API
std::ostream &operator<<(std::ostream &out, const HgiAttachmentDesc &attachment);

WABI_NAMESPACE_END

#endif
