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
#ifndef WABI_IMAGING_HGI_GL_BUFFER_H
#define WABI_IMAGING_HGI_GL_BUFFER_H

#include "wabi/imaging/hgi/buffer.h"
#include "wabi/imaging/hgiGL/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class HgiGLBuffer
///
/// Represents an OpenGL GPU buffer resource.
///
class HgiGLBuffer final : public HgiBuffer
{
 public:

  HGIGL_API
  ~HgiGLBuffer() override;

  HGIGL_API
  size_t GetByteSizeOfResource() const override;

  HGIGL_API
  uint64_t GetRawResource() const override;

  HGIGL_API
  void *GetCPUStagingAddress() override;

  uint32_t GetBufferId() const
  {
    return _bufferId;
  }

 protected:

  friend class HgiGL;

  HGIGL_API
  HgiGLBuffer(HgiBufferDesc const &desc);

 private:

  HgiGLBuffer() = delete;
  HgiGLBuffer &operator=(const HgiGLBuffer &) = delete;
  HgiGLBuffer(const HgiGLBuffer &) = delete;

  uint32_t _bufferId;
  void *_mapped;
  void *_cpuStaging;
};

WABI_NAMESPACE_END

#endif
