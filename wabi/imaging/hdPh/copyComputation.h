//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef WABI_IMAGING_HD_ST_COPY_COMPUTATION_H
#define WABI_IMAGING_HD_ST_COPY_COMPUTATION_H

#include "wabi/imaging/hd/computation.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class HdPhCopyComputationGPU
///
/// A GPU computation which transfers a vbo range specified by src and name to
/// the given range.
///
class HdPhCopyComputationGPU : public HdComputation
{
 public:
  HDPH_API
  HdPhCopyComputationGPU(HdBufferArrayRangeSharedPtr const &src, TfToken const &name);

  HDPH_API
  virtual void Execute(HdBufferArrayRangeSharedPtr const &range,
                       HdResourceRegistry *resourceRegistry) override;

  HDPH_API
  virtual int GetNumOutputElements() const override;

  HDPH_API
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;

 private:
  HdBufferArrayRangeSharedPtr _src;
  TfToken _name;
};

WABI_NAMESPACE_END

#endif  // HDPH_COMPUTATION_H
