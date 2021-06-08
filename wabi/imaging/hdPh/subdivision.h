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
#ifndef WABI_IMAGING_HD_PH_SUBDIVISION_H
#define WABI_IMAGING_HD_PH_SUBDIVISION_H

#include "wabi/base/tf/token.h"
#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/computation.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/bufferResource.h"
#include "wabi/imaging/hdPh/meshTopology.h"
#include "wabi/imaging/hf/perfLog.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class Hd_Subdivision
///
/// Subdivision struct holding subdivision tables and patch tables.
///
/// This single struct can be used for cpu and gpu subdivision at the same time.
///
class HdPh_Subdivision {
 public:
  virtual ~HdPh_Subdivision();

  virtual int GetNumVertices() const               = 0;
  virtual int GetNumVarying() const                = 0;
  virtual int GetNumFaceVarying(int channel) const = 0;
  virtual int GetMaxNumFaceVarying() const         = 0;

  virtual VtIntArray GetRefinedFvarIndices(int channel) const = 0;

  virtual void RefineCPU(HdBufferSourceSharedPtr const &source,
                         void *vertexBuffer,
                         HdPh_MeshTopology::Interpolation interpolation,
                         int fvarChannel = 0) = 0;
  virtual void RefineGPU(HdBufferArrayRangeSharedPtr const &range,
                         TfToken const &name,
                         HdPh_MeshTopology::Interpolation interpolation,
                         int fvarChannel = 0) = 0;

  // computation factory methods
  virtual HdBufferSourceSharedPtr CreateTopologyComputation(HdPh_MeshTopology *topology,
                                                            bool adaptive,
                                                            int level,
                                                            SdfPath const &id) = 0;

  virtual HdBufferSourceSharedPtr CreateIndexComputation(
      HdPh_MeshTopology *topology,
      HdBufferSourceSharedPtr const &osdTopology) = 0;

  virtual HdBufferSourceSharedPtr CreateFvarIndexComputation(
      HdPh_MeshTopology *topology,
      HdBufferSourceSharedPtr const &osdTopology,
      int channel) = 0;

  virtual HdBufferSourceSharedPtr CreateRefineComputation(
      HdPh_MeshTopology *topology,
      HdBufferSourceSharedPtr const &source,
      HdBufferSourceSharedPtr const &osdTopology,
      HdPh_MeshTopology::Interpolation interpolation,
      int fvarChannel = 0) = 0;

  virtual HdComputationSharedPtr CreateRefineComputationGPU(
      HdPh_MeshTopology *topology,
      TfToken const &name,
      HdType type,
      HdPh_MeshTopology::Interpolation interpolation,
      int fvarChannel = 0) = 0;

  /// Returns true if the subdivision for \a scheme generates triangles,
  /// instead of quads.
  static bool RefinesToTriangles(TfToken const &scheme);

  /// Returns true if the subdivision for \a scheme generates bspline patches.
  static bool RefinesToBSplinePatches(TfToken const &scheme);

  /// Returns true if the subdivision for \a scheme generates box spline
  /// triangle patches.
  static bool RefinesToBoxSplineTrianglePatches(TfToken const &scheme);
};

// ---------------------------------------------------------------------------
/// \class Hd_OsdTopologyComputation
///
/// OpenSubdiv Topology Analysis.
/// Create Hd_Subdivision struct and sets it into HdPh_MeshTopology.
///
class HdPh_OsdTopologyComputation : public HdComputedBufferSource {
 public:
  HdPh_OsdTopologyComputation(HdPh_MeshTopology *topology, int level, SdfPath const &id);

  /// overrides
  void GetBufferSpecs(HdBufferSpecVector *specs) const override;

 protected:
  HdPh_MeshTopology *_topology;
  int _level;
  SdfPath const _id;
};

// ---------------------------------------------------------------------------
/// \class HdPh_OsdIndexComputation
///
/// OpenSubdiv refined index buffer computation.
///
/// computes index buffer and primitiveParam
///
/// primitiveParam : refined quads to coarse faces mapping buffer
///
/// ----+-----------+-----------+------
/// ... |i0 i1 i2 i3|i4 i5 i6 i7| ...    index buffer (for quads)
/// ----+-----------+-----------+------
/// ... |           |           | ...    primitive param[0] (coarse face index)
/// ... |     p0    |     p1    | ...    primitive param[1] (patch param 0)
/// ... |           |           | ...    primitive param[2] (patch param 1)
/// ----+-----------+-----------+------
///
class HdPh_OsdIndexComputation : public HdComputedBufferSource {
 public:
  /// overrides
  bool HasChainedBuffer() const override;
  void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  HdBufferSourceSharedPtrVector GetChainedBuffers() const override;

 protected:
  HdPh_OsdIndexComputation(HdPh_MeshTopology *topology,
                           HdBufferSourceSharedPtr const &osdTopology);

  bool _CheckValid() const override;

  HdPh_MeshTopology *_topology;
  HdBufferSourceSharedPtr _osdTopology;
  HdBufferSourceSharedPtr _primitiveBuffer;
  HdBufferSourceSharedPtr _edgeIndicesBuffer;
};

// ---------------------------------------------------------------------------
/// \class Hd_OsdRefineComputation
///
/// OpenSubdiv CPU Refinement.
/// This class isn't inherited from HdComputedBufferSource.
/// GetData() returns the internal buffer of Hd_OsdCpuVertexBuffer,
/// so that reducing data copy between osd buffer and HdBufferSource.
///
template<typename VERTEX_BUFFER> class HdPh_OsdRefineComputation final : public HdBufferSource {
 public:
  HdPh_OsdRefineComputation(HdPh_MeshTopology *topology,
                            HdBufferSourceSharedPtr const &source,
                            HdBufferSourceSharedPtr const &osdTopology,
                            HdPh_MeshTopology::Interpolation interpolation,
                            int fvarChannel = 0);
  ~HdPh_OsdRefineComputation() override;
  TfToken const &GetName() const override;
  size_t ComputeHash() const override;
  void const *GetData() const override;
  HdTupleType GetTupleType() const override;
  size_t GetNumElements() const override;
  void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  bool Resolve() override;
  bool HasPreChainedBuffer() const override;
  HdBufferSourceSharedPtr GetPreChainedBuffer() const override;
  HdPh_MeshTopology::Interpolation GetInterpolation() const;

 protected:
  bool _CheckValid() const override;

 private:
  HdPh_MeshTopology *_topology;
  HdBufferSourceSharedPtr _source;
  HdBufferSourceSharedPtr _osdTopology;
  VERTEX_BUFFER *_cpuVertexBuffer;
  HdPh_MeshTopology::Interpolation _interpolation;
  int _fvarChannel;
};

// ---------------------------------------------------------------------------
/// \class HdPh_OsdRefineComputationGPU
///
/// OpenSubdiv GPU Refinement.
///
class HdPh_OsdRefineComputationGPU : public HdComputation {
 public:
  HdPh_OsdRefineComputationGPU(HdPh_MeshTopology *topology,
                               TfToken const &name,
                               HdType type,
                               HdPh_MeshTopology::Interpolation interpolation,
                               int fvarChannel = 0);

  void Execute(HdBufferArrayRangeSharedPtr const &range,
               HdResourceRegistry *resourceRegistry) override;
  void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  int GetNumOutputElements() const override;
  HdPh_MeshTopology::Interpolation GetInterpolation() const;

  // A wrapper class to bridge between HdBufferResource and OpenSubdiv
  // vertex buffer API.
  //
  class VertexBuffer {
   public:
    VertexBuffer(HdBufferResourceSharedPtr const &resource)
    {
      _resource = std::static_pointer_cast<HdPhBufferResource>(resource);
    }

    // bit confusing, osd expects 'GetNumElements()' returns the num
    // components, in Phoenix sense
    size_t GetNumElements() const
    {
      return HdGetComponentCount(_resource->GetTupleType().type);
    }
    uint64_t BindVBO()
    {
      return _resource->GetHandle()->GetRawResource();
    }
    HdPhBufferResourceSharedPtr _resource;
  };

 private:
  HdPh_MeshTopology *_topology;
  TfToken _name;
  HdPh_MeshTopology::Interpolation _interpolation;
  int _fvarChannel;
};

// ---------------------------------------------------------------------------
// template implementations
template<typename VERTEX_BUFFER>
HdPh_OsdRefineComputation<VERTEX_BUFFER>::HdPh_OsdRefineComputation(
    HdPh_MeshTopology *topology,
    HdBufferSourceSharedPtr const &source,
    HdBufferSourceSharedPtr const &osdTopology,
    HdPh_MeshTopology::Interpolation interpolation,
    int fvarChannel)
    : _topology(topology),
      _source(source),
      _osdTopology(osdTopology),
      _interpolation(interpolation),
      _fvarChannel(fvarChannel)
{}

template<typename VERTEX_BUFFER>
HdPh_OsdRefineComputation<VERTEX_BUFFER>::~HdPh_OsdRefineComputation()
{
  delete _cpuVertexBuffer;
}

template<typename VERTEX_BUFFER>
TfToken const &HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetName() const
{
  return _source->GetName();
}

template<class HashState, typename VERTEX_BUFFER>
void TfHashAppend(HashState &h, HdPh_OsdRefineComputation<VERTEX_BUFFER> const &bs)
{
  h.Append(bs.GetInterpolation());
}

template<typename VERTEX_BUFFER>
size_t HdPh_OsdRefineComputation<VERTEX_BUFFER>::ComputeHash() const
{
  return TfHash()(*this);
}

template<typename VERTEX_BUFFER>
void const *HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetData() const
{
  return _cpuVertexBuffer->BindCpuBuffer();
}

template<typename VERTEX_BUFFER>
HdTupleType HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetTupleType() const
{
  return _source->GetTupleType();
}

template<typename VERTEX_BUFFER>
size_t HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetNumElements() const
{
  return _cpuVertexBuffer->GetNumVertices();
}

template<typename VERTEX_BUFFER>
HdPh_MeshTopology::Interpolation HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetInterpolation() const
{
  return _interpolation;
}

template<typename VERTEX_BUFFER> bool HdPh_OsdRefineComputation<VERTEX_BUFFER>::Resolve()
{
  if (_source && !_source->IsResolved())
    return false;
  if (_osdTopology && !_osdTopology->IsResolved())
    return false;

  if (!_TryLock())
    return false;

  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPh_Subdivision *subdivision = _topology->GetSubdivision();
  if (!TF_VERIFY(subdivision)) {
    _SetResolved();
    return true;
  }

  // prepare cpu vertex buffer including refined vertices
  TF_VERIFY(!_cpuVertexBuffer);
  _cpuVertexBuffer = VERTEX_BUFFER::Create(
      HdGetComponentCount(_source->GetTupleType().type),
      _interpolation == HdPh_MeshTopology::INTERPOLATE_VERTEX ?
          subdivision->GetNumVertices() :
      _interpolation == HdPh_MeshTopology::INTERPOLATE_VARYING ?
          subdivision->GetNumVarying() :
          subdivision->GetMaxNumFaceVarying());

  subdivision->RefineCPU(_source, _cpuVertexBuffer, _interpolation, _fvarChannel);

  HD_PERF_COUNTER_INCR(HdPerfTokens->subdivisionRefineCPU);

  _SetResolved();
  return true;
}

template<typename VERTEX_BUFFER> bool HdPh_OsdRefineComputation<VERTEX_BUFFER>::_CheckValid() const
{
  bool valid = _source->IsValid();

  // _osdTopology is optional
  valid &= _osdTopology ? _osdTopology->IsValid() : true;

  return valid;
}

template<typename VERTEX_BUFFER>
void HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  // produces same spec buffer as source
  _source->GetBufferSpecs(specs);
}

template<typename VERTEX_BUFFER>
bool HdPh_OsdRefineComputation<VERTEX_BUFFER>::HasPreChainedBuffer() const
{
  return true;
}

template<typename VERTEX_BUFFER>
HdBufferSourceSharedPtr HdPh_OsdRefineComputation<VERTEX_BUFFER>::GetPreChainedBuffer() const
{
  return _source;
}

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_SUBDIVISION_H
