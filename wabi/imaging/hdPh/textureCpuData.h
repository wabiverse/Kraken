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
#ifndef WABI_IMAGING_HD_PH_TEXTURE_CPU_DATA_H
#define WABI_IMAGING_HD_PH_TEXTURE_CPU_DATA_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

struct HgiTextureDesc;

/// \class HdPhTextureCpuData
///
/// Represents CPU data that can be stored in a HdPhUvTextureObject, mostly,
/// likely during the load phase to be committed to the GPU.
///
class HdPhTextureCpuData {
 public:
  HDPH_API
  virtual ~HdPhTextureCpuData();

  /// The metadata of the texture (width, height, ...) including a
  /// pointer to the CPU data (as initialData).
  virtual const HgiTextureDesc &GetTextureDesc() const = 0;

  /// Make GPU generate mipmaps. The number of mipmaps is specified
  /// in the texture descriptor and the mipmaps are generate from
  /// the mip level 0 data.
  virtual bool GetGenerateMipmaps() const = 0;

  /// Are the data valid (e.g., false if file could not be found).
  virtual bool IsValid() const = 0;
};

WABI_NAMESPACE_END

#endif
