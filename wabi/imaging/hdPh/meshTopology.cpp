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
#include "wabi/wabi.h"

#include "wabi/imaging/hdPh/meshTopology.h"
#include "wabi/imaging/hdPh/quadrangulate.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subdivision.h"
#include "wabi/imaging/hdPh/subdivision3.h"
#include "wabi/imaging/hdPh/triangulate.h"

#include "wabi/imaging/hd/bufferArrayRange.h"
#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/meshUtil.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"
#include "wabi/base/tf/diagnostic.h"

WABI_NAMESPACE_BEGIN

// static
HdPh_MeshTopologySharedPtr HdPh_MeshTopology::New(const HdMeshTopology &src,
                                                  int refineLevel,
                                                  RefineMode refineMode)
{
  return HdPh_MeshTopologySharedPtr(new HdPh_MeshTopology(src, refineLevel, refineMode));
}

// explicit
HdPh_MeshTopology::HdPh_MeshTopology(const HdMeshTopology &src, int refineLevel, RefineMode refineMode)
  : HdMeshTopology(src, refineLevel),
    _quadInfo(nullptr),
    _quadrangulateTableRange(),
    _quadInfoBuilder(),
    _refineMode(refineMode),
    _subdivision(nullptr),
    _osdTopologyBuilder()
{}

HdPh_MeshTopology::~HdPh_MeshTopology()
{
  delete _quadInfo;
  delete _subdivision;
}

bool HdPh_MeshTopology::operator==(HdPh_MeshTopology const &other) const
{

  TRACE_FUNCTION();

  // no need to compare _adajency and _quadInfo
  return HdMeshTopology::operator==(other);
}

void HdPh_MeshTopology::SetQuadInfo(HdQuadInfo const *quadInfo)
{
  delete _quadInfo;
  _quadInfo = quadInfo;
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetPointsIndexBuilderComputation()
{
  // this is simple enough to return the result right away.
  int numPoints = GetNumPoints();
  VtIntArray indices(numPoints);
  for (int i = 0; i < numPoints; ++i)
    indices[i] = i;

  return std::make_shared<HdVtBufferSource>(HdTokens->indices, VtValue(indices));
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetTriangleIndexBuilderComputation(SdfPath const &id)
{
  return std::make_shared<HdPh_TriangleIndexBuilderComputation>(this, id);
}

HdPh_QuadInfoBuilderComputationSharedPtr HdPh_MeshTopology::GetQuadInfoBuilderComputation(
  bool gpu,
  SdfPath const &id,
  HdPhResourceRegistry *resourceRegistry)
{
  HdPh_QuadInfoBuilderComputationSharedPtr builder = std::make_shared<HdPh_QuadInfoBuilderComputation>(this,
                                                                                                       id);

  // store as a weak ptr.
  _quadInfoBuilder = builder;

  if (gpu) {
    if (!TF_VERIFY(resourceRegistry)) {
      TF_CODING_ERROR(
        "resource registry must be non-null "
        "if gpu quadinfo is requested.");
      return builder;
    }

    HdBufferSourceSharedPtr quadrangulateTable = std::make_shared<HdPh_QuadrangulateTableComputation>(
      this, builder);

    // allocate quadrangulation table on GPU
    HdBufferSpecVector bufferSpecs;
    quadrangulateTable->GetBufferSpecs(&bufferSpecs);

    _quadrangulateTableRange = resourceRegistry->AllocateNonUniformBufferArrayRange(
      HdTokens->topology, bufferSpecs, HdBufferArrayUsageHint());

    resourceRegistry->AddSource(_quadrangulateTableRange, quadrangulateTable);
  }
  return builder;
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetQuadIndexBuilderComputation(SdfPath const &id)
{
  return std::make_shared<HdPh_QuadIndexBuilderComputation>(this, _quadInfoBuilder.lock(), id);
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetQuadrangulateComputation(HdBufferSourceSharedPtr const &source,
                                                                       SdfPath const &id)
{
  // check if the quad table is already computed as all-quads.
  if (_quadInfo && _quadInfo->IsAllQuads()) {
    // no need of quadrangulation.
    return HdBufferSourceSharedPtr();
  }

  // Make a dependency to quad info, in case if the topology
  // is chaging and the quad info hasn't been populated.
  //
  // It can be null for the second or later primvar animation.
  // Don't call GetQuadInfoBuilderComputation instead. It may result
  // unregisterd computation.
  HdBufferSourceSharedPtr quadInfo = _quadInfoBuilder.lock();

  return std::make_shared<HdPh_QuadrangulateComputation>(this, source, quadInfo, id);
}

HdComputationSharedPtr HdPh_MeshTopology::GetQuadrangulateComputationGPU(TfToken const &name,
                                                                         HdType dataType,
                                                                         SdfPath const &id)
{
  // check if the quad table is already computed as all-quads.
  if (_quadInfo && _quadInfo->IsAllQuads()) {
    // no need of quadrangulation.
    return HdComputationSharedPtr();
  }
  return std::make_shared<HdPh_QuadrangulateComputationGPU>(this, name, dataType, id);
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetQuadrangulateFaceVaryingComputation(
  HdBufferSourceSharedPtr const &source,
  SdfPath const &id)
{
  return std::make_shared<HdPh_QuadrangulateFaceVaryingComputation>(this, source, id);
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetTriangulateFaceVaryingComputation(
  HdBufferSourceSharedPtr const &source,
  SdfPath const &id)
{
  return std::make_shared<HdPh_TriangulateFaceVaryingComputation>(this, source, id);
}

bool HdPh_MeshTopology::RefinesToTriangles() const
{
  return HdPh_Subdivision::RefinesToTriangles(_topology.GetScheme());
}

bool HdPh_MeshTopology::RefinesToBSplinePatches() const
{
  return ((IsEnabledAdaptive() || (_refineMode == RefineModePatches)) &&
          HdPh_Subdivision::RefinesToBSplinePatches(_topology.GetScheme()));
}

bool HdPh_MeshTopology::RefinesToBoxSplineTrianglePatches() const
{
  return ((IsEnabledAdaptive() || (_refineMode == RefineModePatches)) &&
          HdPh_Subdivision::RefinesToBoxSplineTrianglePatches(_topology.GetScheme()));
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetOsdTopologyComputation(SdfPath const &id)
{
  if (HdBufferSourceSharedPtr builder = _osdTopologyBuilder.lock()) {
    return builder;
  }

  // this has to be the first instance.
  if (!TF_VERIFY(!_subdivision))
    return HdBufferSourceSharedPtr();

  // create HdPh_Subdivision
  _subdivision = HdPh_Osd3Factory::CreateSubdivision();

  if (!TF_VERIFY(_subdivision))
    return HdBufferSourceSharedPtr();

  bool adaptive = RefinesToBSplinePatches() || RefinesToBoxSplineTrianglePatches();

  // create a topology computation for HdPh_Subdivision
  HdBufferSourceSharedPtr builder = _subdivision->CreateTopologyComputation(
    this, adaptive, _refineLevel, id);
  _osdTopologyBuilder = builder;  // retain weak ptr
  return builder;
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetOsdIndexBuilderComputation()
{
  HdBufferSourceSharedPtr topologyBuilder = _osdTopologyBuilder.lock();
  return _subdivision->CreateIndexComputation(this, topologyBuilder);
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetOsdFvarIndexBuilderComputation(int channel)
{
  HdBufferSourceSharedPtr topologyBuilder = _osdTopologyBuilder.lock();
  return _subdivision->CreateFvarIndexComputation(this, topologyBuilder, channel);
}

HdBufferSourceSharedPtr HdPh_MeshTopology::GetOsdRefineComputation(HdBufferSourceSharedPtr const &source,
                                                                   Interpolation interpolation,
                                                                   int fvarChannel)
{
  // Make a dependency to far mesh.
  // (see comment on GetQuadrangulateComputation)
  //
  // It can be null for the second or later primvar animation.
  // Don't call GetOsdTopologyComputation instead. It may result
  // unregisterd computation.

  // for empty topology, we don't need to refine anything.
  // source will be scheduled at the caller
  if (_topology.GetFaceVertexCounts().size() == 0)
    return source;

  if (!TF_VERIFY(_subdivision)) {
    TF_CODING_ERROR(
      "GetOsdTopologyComputation should be called before "
      "GetOsdRefineComputation.");
    return source;
  }

  HdBufferSourceSharedPtr topologyBuilder = _osdTopologyBuilder.lock();

  return _subdivision->CreateRefineComputation(this, source, topologyBuilder, interpolation);
}

HdComputationSharedPtr HdPh_MeshTopology::GetOsdRefineComputationGPU(TfToken const &name,
                                                                     HdType dataType,
                                                                     Interpolation interpolation,
                                                                     int fvarChannel)
{
  // for empty topology, we don't need to refine anything.
  if (_topology.GetFaceVertexCounts().size() == 0) {
    return HdComputationSharedPtr();
  }

  if (!TF_VERIFY(_subdivision))
    return HdComputationSharedPtr();

  return _subdivision->CreateRefineComputationGPU(this, name, dataType, interpolation, fvarChannel);
}

WABI_NAMESPACE_END
