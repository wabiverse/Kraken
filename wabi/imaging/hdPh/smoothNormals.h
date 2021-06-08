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
#ifndef WABI_IMAGING_HD_PH_SMOOTH_NORMALS_H
#define WABI_IMAGING_HD_PH_SMOOTH_NORMALS_H

#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/computation.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

WABI_NAMESPACE_BEGIN

class Hd_VertexAdjacency;

/// smooth normal computation GPU
///
///
class HdPh_SmoothNormalsComputationGPU : public HdComputation {
 public:
  /// Constructor
  HDPH_API
  HdPh_SmoothNormalsComputationGPU(Hd_VertexAdjacency const *adjacency,
                                   TfToken const &srcName,
                                   TfToken const &dstName,
                                   HdType srcDataType,
                                   bool packed);

  HDPH_API
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  HDPH_API
  virtual void Execute(HdBufferArrayRangeSharedPtr const &range,
                       HdResourceRegistry *resourceRegistry) override;

  /// This computation doesn't generate buffer source (i.e. 2nd phase)
  /// This is a gpu computation, but no need to resize the destination
  /// since it belongs the same range as src buffer.
  virtual int GetNumOutputElements() const override
  {
    return 0;
  }

 private:
  Hd_VertexAdjacency const *_adjacency;
  TfToken _srcName;
  TfToken _dstName;
  HdType _srcDataType;
  HdType _dstDataType;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_SMOOTH_NORMALS_H
