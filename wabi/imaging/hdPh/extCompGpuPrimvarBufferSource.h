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
#ifndef WABI_IMAGING_HD_PH_EXT_COMP_GPU_PRIMVAR_BUFFER_SOURCE_H
#define WABI_IMAGING_HD_PH_EXT_COMP_GPU_PRIMVAR_BUFFER_SOURCE_H

#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

#include "wabi/usd/sdf/path.h"

WABI_NAMESPACE_BEGIN

/// \class HdPhExtCompGpuPrimvarBufferSource
/// A buffer source mapped to an output of an ExtComp CPU computation.
///
class HdPhExtCompGpuPrimvarBufferSource final : public HdNullBufferSource {
 public:
  HdPhExtCompGpuPrimvarBufferSource(TfToken const &name,
                                    HdTupleType const &valueType,
                                    int numElements,
                                    SdfPath const &compId);

  HDPH_API
  virtual ~HdPhExtCompGpuPrimvarBufferSource() = default;

  HDPH_API
  virtual size_t ComputeHash() const override;

  HDPH_API
  virtual bool Resolve() override;

  HD_API
  virtual TfToken const &GetName() const override;

  HDPH_API
  virtual size_t GetNumElements() const override;

  HD_API
  virtual HdTupleType GetTupleType() const override;

  HDPH_API
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;

 protected:
  virtual bool _CheckValid() const override;

 private:
  TfToken _name;
  HdTupleType _tupleType;
  size_t _numElements;
  SdfPath _compId;

  HdPhExtCompGpuPrimvarBufferSource()                                          = delete;
  HdPhExtCompGpuPrimvarBufferSource(const HdPhExtCompGpuPrimvarBufferSource &) = delete;
  HdPhExtCompGpuPrimvarBufferSource &operator=(const HdPhExtCompGpuPrimvarBufferSource &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_EXT_COMP_GPU_PRIMVAR_BUFFER_SOURCE_H
