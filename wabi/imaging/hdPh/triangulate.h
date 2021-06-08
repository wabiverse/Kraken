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
#ifndef WABI_IMAGING_HD_PH_TRIANGULATE_H
#define WABI_IMAGING_HD_PH_TRIANGULATE_H

#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/path.h"

WABI_NAMESPACE_BEGIN

class HdPh_MeshTopology;

/// \class HdPh_TriangleIndexBuilderComputation
///
/// Triangle indices computation CPU.
///

// Index triangulation generates a mapping from triangle ID to authored
// face index domain, called primitiveParams. The primitive params are stored
// alongside topology index buffers, so that the same aggregation locators can
// be used for such an additional buffer as well. This change transforms index
// buffer from int array to int[3] array or int[4] array at first. Thanks to
// the heterogenius non-interleaved buffer aggregation ability in hd,
// we'll get this kind of buffer layout:
//
// ----+--------+--------+------
// ... |i0 i1 i2|i3 i4 i5| ...   index buffer (for triangles)
// ----+--------+--------+------
// ... |   m0   |   m1   | ...   primitive param buffer (coarse face index)
// ----+--------+--------+------

class HdPh_TriangleIndexBuilderComputation : public HdComputedBufferSource {
 public:
  HdPh_TriangleIndexBuilderComputation(HdPh_MeshTopology *topology, SdfPath const &id);
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  virtual bool Resolve() override;

  virtual bool HasChainedBuffer() const override;
  virtual HdBufferSourceSharedPtrVector GetChainedBuffers() const override;

 protected:
  virtual bool _CheckValid() const override;

 private:
  SdfPath const _id;
  HdPh_MeshTopology *_topology;
  HdBufferSourceSharedPtr _primitiveParam;
  HdBufferSourceSharedPtr _trianglesEdgeIndices;
};

//

/// \class HdPh_TriangulateFaceVaryingComputation
///
/// CPU face-varying triangulation.
///
class HdPh_TriangulateFaceVaryingComputation : public HdComputedBufferSource {
 public:
  HdPh_TriangulateFaceVaryingComputation(HdPh_MeshTopology *topolgoy,
                                         HdBufferSourceSharedPtr const &source,
                                         SdfPath const &id);

  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  virtual bool Resolve() override;

 protected:
  virtual bool _CheckValid() const override;

 private:
  SdfPath const _id;
  HdPh_MeshTopology *_topology;
  HdBufferSourceSharedPtr _source;
};

WABI_NAMESPACE_END

#endif  // HdPh_TRIANGULATE_H
