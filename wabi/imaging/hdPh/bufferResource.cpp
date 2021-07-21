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

#include "wabi/imaging/glf/contextCaps.h"

#include "wabi/imaging/hdPh/bufferResource.h"

WABI_NAMESPACE_BEGIN

HdPhBufferResource::HdPhBufferResource(TfToken const &role, HdTupleType tupleType, int offset, int stride)
  : HdBufferResource(role, tupleType, offset, stride),
    _gpuAddr(0)
{
  /*NOTHING*/
}

HdPhBufferResource::~HdPhBufferResource()
{
  /*NOTHING*/
}

void HdPhBufferResource::SetAllocation(HgiBufferHandle const &handle, size_t size)
{
  _handle = handle;
  HdResource::SetSize(size);

  GlfContextCaps const &caps = GlfContextCaps::GetInstance();

  // note: gpu address remains valid until the buffer object is deleted,
  // or when the data store is respecified via BufferData/BufferStorage.
  // It doesn't change even when we make the buffer resident or non-resident.
  // https://www.opengl.org/registry/specs/NV/shader_buffer_load.txt
  if (handle && caps.bindlessBufferEnabled)
  {
    glGetNamedBufferParameterui64vNV(
      handle->GetRawResource(), GL_BUFFER_GPU_ADDRESS_NV, (GLuint64EXT *)&_gpuAddr);
  }
  else
  {
    _gpuAddr = 0;
  }
}

WABI_NAMESPACE_END
