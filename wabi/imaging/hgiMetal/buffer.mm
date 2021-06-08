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
#include <Metal/Metal.h>

#include "wabi/base/arch/defines.h"

#include "wabi/imaging/hgiMetal/buffer.h"
#include "wabi/imaging/hgiMetal/capabilities.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/hgi.h"

WABI_NAMESPACE_BEGIN

HgiMetalBuffer::HgiMetalBuffer(HgiMetal *hgi, HgiBufferDesc const &desc)
    : HgiBuffer(desc),
      _bufferId(nil)
{

  if (desc.byteSize == 0) {
    TF_CODING_ERROR("Buffers must have a non-zero length");
  }

  MTLResourceOptions options = MTLResourceCPUCacheModeDefaultCache |
                               hgi->GetCapabilities().defaultStorageMode;

  if (desc.initialData) {
    _bufferId = [hgi->GetPrimaryDevice() newBufferWithBytes:desc.initialData
                                                     length:desc.byteSize
                                                    options:options];
  }
  else {
    _bufferId = [hgi->GetPrimaryDevice() newBufferWithLength:desc.byteSize options:options];
  }

  _descriptor.initialData = nullptr;

  HGIMETAL_DEBUG_LABEL(_bufferId, _descriptor.debugName.c_str());
}

HgiMetalBuffer::~HgiMetalBuffer()
{
  if (_bufferId != nil) {
    [_bufferId release];
    _bufferId = nil;
  }
}

size_t HgiMetalBuffer::GetByteSizeOfResource() const
{
  return _descriptor.byteSize;
}

uint64_t HgiMetalBuffer::GetRawResource() const
{
  return (uint64_t)_bufferId;
}

void *HgiMetalBuffer::GetCPUStagingAddress()
{
  if (_bufferId) {
    return [_bufferId contents];
  }

  return nullptr;
}
WABI_NAMESPACE_END
