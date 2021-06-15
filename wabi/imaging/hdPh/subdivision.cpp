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
#include "wabi/imaging/hdPh/subdivision.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/pxOsd/tokens.h"
#include "wabi/wabi.h"

#include <opensubdiv/version.h>

WABI_NAMESPACE_BEGIN

/*virtual*/
HdPh_Subdivision::~HdPh_Subdivision()
{}

bool HdPh_Subdivision::RefinesToTriangles(TfToken const &scheme)
{
  // XXX: Ideally we'd like to delegate this to the concrete class.
  if (scheme == PxOsdOpenSubdivTokens->loop) {
    return true;
  }
  return false;
}

bool HdPh_Subdivision::RefinesToBSplinePatches(TfToken const &scheme)
{
  return scheme == PxOsdOpenSubdivTokens->catmullClark;
}

bool HdPh_Subdivision::RefinesToBoxSplineTrianglePatches(TfToken const &scheme)
{
#if OPENSUBDIV_VERSION_NUMBER >= 30400
  // v3.4.0 added support for limit surface patches for loop meshes
  if (scheme == PxOsdOpenSubdivTokens->loop) {
    return true;
  }
#endif
  return false;
}

// ---------------------------------------------------------------------------
HdPh_OsdTopologyComputation::HdPh_OsdTopologyComputation(HdPh_MeshTopology *topology,
                                                         int level,
                                                         SdfPath const &id)
  : _topology(topology),
    _level(level),
    _id(id)
{}

/*virtual*/
void HdPh_OsdTopologyComputation::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  // nothing
}

// ---------------------------------------------------------------------------
HdPh_OsdIndexComputation::HdPh_OsdIndexComputation(HdPh_MeshTopology *topology,
                                                   HdBufferSourceSharedPtr const &osdTopology)
  : _topology(topology),
    _osdTopology(osdTopology)
{}

/*virtual*/
void HdPh_OsdIndexComputation::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  if (_topology->RefinesToBSplinePatches()) {
    // bi-cubic bspline patches
    specs->emplace_back(HdTokens->indices, HdTupleType{HdTypeInt32, 16});
    // 3+1 (includes sharpness)
    specs->emplace_back(HdTokens->primitiveParam, HdTupleType{HdTypeInt32Vec4, 1});
    specs->emplace_back(HdTokens->edgeIndices, HdTupleType{HdTypeInt32Vec2, 1});
  }
  else if (_topology->RefinesToBoxSplineTrianglePatches()) {
    // quartic box spline triangle patches
    specs->emplace_back(HdTokens->indices, HdTupleType{HdTypeInt32, 12});
    // 3+1 (includes sharpness)
    specs->emplace_back(HdTokens->primitiveParam, HdTupleType{HdTypeInt32Vec4, 1});
    // int will suffice, but this unifies it for all the cases
    specs->emplace_back(HdTokens->edgeIndices, HdTupleType{HdTypeInt32Vec2, 1});
  }
  else if (HdPh_Subdivision::RefinesToTriangles(_topology->GetScheme())) {
    // triangles (loop)
    specs->emplace_back(HdTokens->indices, HdTupleType{HdTypeInt32Vec3, 1});
    specs->emplace_back(HdTokens->primitiveParam, HdTupleType{HdTypeInt32Vec3, 1});
    // int will suffice, but this unifies it for all the cases
    specs->emplace_back(HdTokens->edgeIndices, HdTupleType{HdTypeInt32Vec2, 1});
  }
  else {
    // quads (catmark, bilinear)
    specs->emplace_back(HdTokens->indices, HdTupleType{HdTypeInt32Vec4, 1});
    specs->emplace_back(HdTokens->primitiveParam, HdTupleType{HdTypeInt32Vec3, 1});
    specs->emplace_back(HdTokens->edgeIndices, HdTupleType{HdTypeInt32Vec2, 1});
  }
}

/*virtual*/
bool HdPh_OsdIndexComputation::HasChainedBuffer() const
{
  return true;
}

/*virtual*/
HdBufferSourceSharedPtrVector HdPh_OsdIndexComputation::GetChainedBuffers() const
{
  return {_primitiveBuffer, _edgeIndicesBuffer};
}

/*virtual*/
bool HdPh_OsdIndexComputation::_CheckValid() const
{
  return true;
}

// ---------------------------------------------------------------------------
/// OpenSubdiv GPU Refinement
///
///
HdPh_OsdRefineComputationGPU::HdPh_OsdRefineComputationGPU(HdPh_MeshTopology *topology,
                                                           TfToken const &name,
                                                           HdType type,
                                                           HdPh_MeshTopology::Interpolation interpolation,
                                                           int fvarChannel)
  : _topology(topology),
    _name(name),
    _interpolation(interpolation),
    _fvarChannel(fvarChannel)
{}

void HdPh_OsdRefineComputationGPU::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  // nothing
  //
  // GPU subdivision requires the source data on GPU in prior to
  // execution, so no need to populate bufferspec on registration.
}

void HdPh_OsdRefineComputationGPU::Execute(HdBufferArrayRangeSharedPtr const &range,
                                           HdResourceRegistry *resourceRegistry)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPh_Subdivision *subdivision = _topology->GetSubdivision();
  if (!TF_VERIFY(subdivision))
    return;

  subdivision->RefineGPU(range, _name, _interpolation, _fvarChannel);

  HD_PERF_COUNTER_INCR(HdPerfTokens->subdivisionRefineGPU);
}

int HdPh_OsdRefineComputationGPU::GetNumOutputElements() const
{
  // returns the total number of vertices, including coarse and refined ones
  HdPh_Subdivision const *subdivision = _topology->GetSubdivision();
  if (!TF_VERIFY(subdivision))
    return 0;
  if (_interpolation == HdPh_MeshTopology::INTERPOLATE_VERTEX) {
    return subdivision->GetNumVertices();
  }
  else if (_interpolation == HdPh_MeshTopology::INTERPOLATE_VARYING) {
    return subdivision->GetNumVarying();
  }
  else {
    return subdivision->GetMaxNumFaceVarying();
  }
}

HdPh_MeshTopology::Interpolation HdPh_OsdRefineComputationGPU::GetInterpolation() const
{
  return _interpolation;
}

WABI_NAMESPACE_END
