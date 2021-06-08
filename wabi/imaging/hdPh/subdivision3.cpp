
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
#include "wabi/wabi.h"

#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/subdivision.h"
#include "wabi/imaging/hdPh/subdivision3.h"

#include "wabi/imaging/hd/bufferArrayRange.h"
#include "wabi/imaging/hd/bufferResource.h"
#include "wabi/imaging/hd/meshUtil.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/imaging/hf/perfLog.h"

#include "wabi/imaging/pxOsd/refinerFactory.h"
#include "wabi/imaging/pxOsd/tokens.h"

#include "wabi/base/gf/vec4i.h"

#include <opensubdiv/far/patchTable.h>
#include <opensubdiv/far/patchTableFactory.h>
#include <opensubdiv/far/stencilTable.h>
#include <opensubdiv/far/stencilTableFactory.h>
#include <opensubdiv/osd/cpuEvaluator.h>
#include <opensubdiv/osd/cpuVertexBuffer.h>
#include <opensubdiv/osd/glVertexBuffer.h>
#include <opensubdiv/osd/mesh.h>
#include <opensubdiv/version.h>

WABI_NAMESPACE_BEGIN

typedef OpenSubdiv::Osd::CpuVertexBuffer HdPh_OsdCpuVertexBuffer;

WABI_NAMESPACE_END

// There's a buffer synchronization bug in driver 331, and apparently fixed in 334.
// Don't enable compute shader kernel until driver updates.

#if OPENSUBDIV_HAS_GLSL_COMPUTE

#  include <opensubdiv/osd/glComputeEvaluator.h>
#  define HDPH_ENABLE_GPU_SUBDIVISION 1

WABI_NAMESPACE_BEGIN

typedef OpenSubdiv::Osd::GLStencilTableSSBO HdPh_OsdGpuStencilTable;
typedef OpenSubdiv::Osd::GLComputeEvaluator HdPh_OsdGpuEvaluator;

WABI_NAMESPACE_END

#elif OPENSUBDIV_HAS_GLSL_TRANSFORM_FEEDBACK

#  include <opensubdiv/osd/glXFBEvaluator.h>

#  define HDPH_ENABLE_GPU_SUBDIVISION 1

WABI_NAMESPACE_BEGIN

typedef OpenSubdiv::Osd::GLStencilTableTBO HdPh_OsdGpuStencilTable;
typedef OpenSubdiv::Osd::GLXFBEvaluator HdPh_OsdGpuEvaluator;

WABI_NAMESPACE_END

#else

#  define HDPH_ENABLE_GPU_SUBDIVISION 0

#endif

WABI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------

class HdPh_Osd3Subdivision : public HdPh_Subdivision {
 public:
  /// Construct HdPh_Subdivision. It takes an ownership of farmesh.
  HdPh_Osd3Subdivision();
  ~HdPh_Osd3Subdivision() override;

  int GetNumVertices() const override;

  void RefineCPU(HdBufferSourceSharedPtr const &source, bool varying, void *vertexBuffer) override;

  void RefineGPU(HdBufferArrayRangeSharedPtr const &range, TfToken const &name) override;

  // computation factory methods
  HdBufferSourceSharedPtr CreateTopologyComputation(HdPh_MeshTopology *topology,
                                                    bool adaptive,
                                                    int level,
                                                    SdfPath const &id) override;

  HdBufferSourceSharedPtr CreateIndexComputation(
      HdPh_MeshTopology *topology,
      HdBufferSourceSharedPtr const &osdTopology) override;

  HdBufferSourceSharedPtr CreateRefineComputation(
      HdPh_MeshTopology *topology,
      HdBufferSourceSharedPtr const &source,
      bool varying,
      HdBufferSourceSharedPtr const &osdTopology) override;

  HdComputationSharedPtr CreateRefineComputationGPU(HdPh_MeshTopology *topology,
                                                    TfToken const &name,
                                                    HdType dataType) override;

  void SetRefinementTables(OpenSubdiv::Far::StencilTable const *vertexStencils,
                           OpenSubdiv::Far::StencilTable const *varyingStencils,
                           OpenSubdiv::Far::PatchTable const *patchTable,
                           bool adaptive);

  bool IsAdaptive() const
  {
    return _adaptive;
  }

  OpenSubdiv::Far::PatchTable const *GetPatchTable() const
  {
    return _patchTable;
  }

 private:
  OpenSubdiv::Far::StencilTable const *_vertexStencils;
  OpenSubdiv::Far::StencilTable const *_varyingStencils;
  OpenSubdiv::Far::PatchTable const *_patchTable;
  bool _adaptive;

#if HDPH_ENABLE_GPU_SUBDIVISION
  /// Returns GPU stencil table. Creates it if not existed.
  /// A valid GL context has to be made to current before calling this method.
  HdPh_OsdGpuStencilTable *_GetGpuStencilTable();
  HdPh_OsdGpuStencilTable *_gpuStencilTable;
#endif
};

class HdPh_Osd3IndexComputation : public HdPh_OsdIndexComputation {
  struct PtexFaceInfo {
    int coarseFaceId;
    GfVec4i coarseEdgeIds;
  };

 public:
  HdPh_Osd3IndexComputation(HdPh_Osd3Subdivision *subdivision,
                            HdPh_MeshTopology *topology,
                            HdBufferSourceSharedPtr const &osdTopology);
  /// overrides
  bool Resolve() override;

 private:
  void _PopulateUniformPrimitiveBuffer(OpenSubdiv::Far::PatchTable const *patchTable);
  void _PopulatePatchPrimitiveBuffer(OpenSubdiv::Far::PatchTable const *patchTable);
  void _CreatePtexFaceToCoarseFaceInfoMapping(std::vector<PtexFaceInfo> *result);

  HdPh_Osd3Subdivision *_subdivision;
};

class HdPh_Osd3TopologyComputation : public HdPh_OsdTopologyComputation {
 public:
  HdPh_Osd3TopologyComputation(HdPh_Osd3Subdivision *subdivision,
                               HdPh_MeshTopology *topology,
                               bool adaptive,
                               int level,
                               SdfPath const &id);

  /// overrides
  bool Resolve() override;

 protected:
  bool _CheckValid() const override;

 private:
  HdPh_Osd3Subdivision *_subdivision;
  bool _adaptive;
};

// ---------------------------------------------------------------------------

HdPh_Osd3Subdivision::HdPh_Osd3Subdivision()
    : _vertexStencils(NULL),
      _varyingStencils(NULL),
      _patchTable(NULL),
      _adaptive(false)
{
#if HDPH_ENABLE_GPU_SUBDIVISION
  _gpuStencilTable = NULL;
#endif
}

HdPh_Osd3Subdivision::~HdPh_Osd3Subdivision()
{
  delete _vertexStencils;
  delete _varyingStencils;
  delete _patchTable;
#if HDPH_ENABLE_GPU_SUBDIVISION
  delete _gpuStencilTable;
#endif
}

void HdPh_Osd3Subdivision::SetRefinementTables(
    OpenSubdiv::Far::StencilTable const *vertexStencils,
    OpenSubdiv::Far::StencilTable const *varyingStencils,
    OpenSubdiv::Far::PatchTable const *patchTable,
    bool adaptive)
{
  if (_vertexStencils)
    delete _vertexStencils;
  if (_varyingStencils)
    delete _varyingStencils;
  if (_patchTable)
    delete _patchTable;

  _vertexStencils  = vertexStencils;
  _varyingStencils = varyingStencils;
  _patchTable      = patchTable;
  _adaptive        = adaptive;
}

/*virtual*/
int HdPh_Osd3Subdivision::GetNumVertices() const
{
  // returns the total number of vertices, including coarse and refined ones.
  if (!TF_VERIFY(_vertexStencils))
    return 0;

  return _vertexStencils->GetNumStencils() + _vertexStencils->GetNumControlVertices();
}

/*virtual*/
void HdPh_Osd3Subdivision::RefineCPU(HdBufferSourceSharedPtr const &source,
                                     bool varying,
                                     void *vertexBuffer)
{
  OpenSubdiv::Far::StencilTable const *stencilTable = varying ? _varyingStencils : _vertexStencils;

  if (!TF_VERIFY(stencilTable))
    return;

  OpenSubdiv::Osd::CpuVertexBuffer *osdVertexBuffer =
      static_cast<OpenSubdiv::Osd::CpuVertexBuffer *>(vertexBuffer);

  size_t numElements = source->GetNumElements();

  // Stride is measured here in components, not bytes.
  size_t stride = HdGetComponentCount(source->GetTupleType().type);

  // NOTE: in osd, GetNumElements() returns how many fields in a vertex
  //          (i.e.  3 for XYZ, and 4 for RGBA)
  //       in Phoenix, GetNumElements() returns how many vertices
  //       (or faces, etc) in a buffer. We basically follow the Phoenix
  //       convention in this file.
  TF_VERIFY(stride == (size_t)osdVertexBuffer->GetNumElements(),
            "%zu vs %i",
            stride,
            osdVertexBuffer->GetNumElements());

  // if the mesh has more vertices than that in use in topology (faceIndices),
  // we need to trim the buffer so that they won't overrun the coarse
  // vertex buffer which we allocated using the stencil table.
  // see HdPh_Osd3Subdivision::GetNumVertices()
  if (numElements > (size_t)stencilTable->GetNumControlVertices()) {
    numElements = stencilTable->GetNumControlVertices();
  }

  // filling coarse vertices
  osdVertexBuffer->UpdateData((const float *)source->GetData(),
                              /*offset=*/0,
                              numElements);

  // if there is no stencil (e.g. torus with adaptive refinement),
  // just return here
  if (stencilTable->GetNumStencils() == 0)
    return;

  // apply opensubdiv with CPU evaluator.
  OpenSubdiv::Osd::BufferDescriptor srcDesc(0, stride, stride);
  OpenSubdiv::Osd::BufferDescriptor dstDesc(numElements * stride, stride, stride);

  OpenSubdiv::Osd::CpuEvaluator::EvalStencils(
      osdVertexBuffer, srcDesc, osdVertexBuffer, dstDesc, stencilTable);
}

/*virtual*/
void HdPh_Osd3Subdivision::RefineGPU(HdBufferArrayRangeSharedPtr const &range, TfToken const &name)
{
#if HDPH_ENABLE_GPU_SUBDIVISION
  if (!TF_VERIFY(_vertexStencils))
    return;

  // filling coarse vertices has been done at resource registry.

  HdPhBufferArrayRangeSharedPtr range_ = std::static_pointer_cast<HdPhBufferArrayRange>(range);

  // vertex buffer wrapper for OpenSubdiv API
  HdPh_OsdRefineComputationGPU::VertexBuffer vertexBuffer(range_->GetResource(name));

  // vertex buffer is not interleaved, but aggregated.
  // we need an offset to locate the current range.
  size_t stride         = vertexBuffer.GetNumElements();
  int numCoarseVertices = _vertexStencils->GetNumControlVertices();

  OpenSubdiv::Osd::BufferDescriptor srcDesc(
      /*offset=*/range->GetElementOffset() * stride,
      /*length=*/stride,
      /*stride=*/stride);
  OpenSubdiv::Osd::BufferDescriptor dstDesc(
      /*offset=*/(range->GetElementOffset() + numCoarseVertices) * stride,
      /*length=*/stride,
      /*stride=*/stride);

  // GPU evaluator can be static, as long as it's called sequentially.
  static OpenSubdiv::Osd::EvaluatorCacheT<HdPh_OsdGpuEvaluator> evaluatorCache;

  HdPh_OsdGpuEvaluator const *instance = OpenSubdiv::Osd::GetEvaluator<HdPh_OsdGpuEvaluator>(
      &evaluatorCache, srcDesc, dstDesc, (void *)NULL /*deviceContext*/);

  instance->EvalStencils(&vertexBuffer, srcDesc, &vertexBuffer, dstDesc, _GetGpuStencilTable());
#else
  TF_CODING_ERROR("No GPU kernel available.\n");
#endif
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateTopologyComputation(
    HdPh_MeshTopology *topology,
    bool adaptive,
    int level,
    SdfPath const &id)
{
  return HdBufferSourceSharedPtr(
      new HdPh_Osd3TopologyComputation(this, topology, adaptive, level, id));
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateIndexComputation(
    HdPh_MeshTopology *topology,
    HdBufferSourceSharedPtr const &osdTopology)
{
  return HdBufferSourceSharedPtr(new HdPh_Osd3IndexComputation(this, topology, osdTopology));
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateRefineComputation(
    HdPh_MeshTopology *topology,
    HdBufferSourceSharedPtr const &source,
    bool varying,
    HdBufferSourceSharedPtr const &osdTopology)
{
  return HdBufferSourceSharedPtr(new HdPh_OsdRefineComputation<HdPh_OsdCpuVertexBuffer>(
      topology, source, varying, osdTopology));
}

/*virtual*/
HdComputationSharedPtr HdPh_Osd3Subdivision::CreateRefineComputationGPU(
    HdPh_MeshTopology *topology,
    TfToken const &name,
    HdType dataType)
{
  return HdComputationSharedPtr(new HdPh_OsdRefineComputationGPU(topology, name, dataType));
}

#if HDPH_ENABLE_GPU_SUBDIVISION
HdPh_OsdGpuStencilTable *HdPh_Osd3Subdivision::_GetGpuStencilTable()
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!_gpuStencilTable) {
    _gpuStencilTable = HdPh_OsdGpuStencilTable::Create(_vertexStencils, NULL);
  }

  return _gpuStencilTable;
}
#endif

// ---------------------------------------------------------------------------

HdPh_Osd3TopologyComputation::HdPh_Osd3TopologyComputation(HdPh_Osd3Subdivision *subdivision,
                                                           HdPh_MeshTopology *topology,
                                                           bool adaptive,
                                                           int level,
                                                           SdfPath const &id)
    : HdPh_OsdTopologyComputation(topology, level, id),
      _subdivision(subdivision),
      _adaptive(adaptive)
{}

bool HdPh_Osd3TopologyComputation::Resolve()
{
  using namespace OpenSubdiv;

  if (!_TryLock())
    return false;

  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  // do far analysis and set stencils and patch table into HdPh_Subdivision.

  // create topology refiner
  PxOsdTopologyRefinerSharedPtr refiner;

  if (!TF_VERIFY(_topology)) {
    _SetResolved();
    return true;
  }

  // for empty topology, we don't need to refine anything.
  // but still need to return the typed buffer for codegen
  if (_topology->GetFaceVertexCounts().size() == 0) {
    // leave refiner empty
  }
  else {
    refiner = PxOsdRefinerFactory::Create(_topology->GetPxOsdMeshTopology(),
                                          TfToken(_id.GetText()));
  }

  if (!TF_VERIFY(_subdivision)) {
    _SetResolved();
    return true;
  }

  // refine
  //  and
  // create stencil/patch table
  Far::StencilTable const *vertexStencils  = NULL;
  Far::StencilTable const *varyingStencils = NULL;
  Far::PatchTable const *patchTable        = NULL;

  if (refiner) {
    Far::PatchTableFactory::Options patchOptions(_level);
    if (_adaptive) {
      patchOptions.endCapType = Far::PatchTableFactory::Options::ENDCAP_BSPLINE_BASIS;
#if OPENSUBDIV_VERSION_NUMBER >= 30400
      // Improve fidelity when refining to limit surface patches
      // These options supported since v3.1.0 and v3.2.0 respectively.
      patchOptions.useInfSharpPatch                 = true;
      patchOptions.generateLegacySharpCornerPatches = false;
#endif
    }

    // split trace scopes.
    {
      HD_TRACE_SCOPE("refine");
      if (_adaptive) {
        Far::TopologyRefiner::AdaptiveOptions adaptiveOptions(_level);
#if OPENSUBDIV_VERSION_NUMBER >= 30400
        adaptiveOptions = patchOptions.GetRefineAdaptiveOptions();
#endif
        refiner->RefineAdaptive(adaptiveOptions);
      }
      else {
        refiner->RefineUniform(_level);
      }
    }
    {
      HD_TRACE_SCOPE("stencil factory");
      Far::StencilTableFactory::Options options;
      options.generateOffsets            = true;
      options.generateIntermediateLevels = _adaptive;
      options.interpolationMode          = Far::StencilTableFactory::INTERPOLATE_VERTEX;
      vertexStencils                     = Far::StencilTableFactory::Create(*refiner, options);

      options.interpolationMode = Far::StencilTableFactory::INTERPOLATE_VARYING;
      varyingStencils           = Far::StencilTableFactory::Create(*refiner, options);
    }
    {
      HD_TRACE_SCOPE("patch factory");
      patchTable = Far::PatchTableFactory::Create(*refiner, patchOptions);
    }
  }

  // merge endcap
  if (patchTable && patchTable->GetLocalPointStencilTable()) {
    // append stencils
    if (Far::StencilTable const *vertexStencilsWithLocalPoints =
            Far::StencilTableFactory::AppendLocalPointStencilTable(
                *refiner, vertexStencils, patchTable->GetLocalPointStencilTable())) {
      delete vertexStencils;
      vertexStencils = vertexStencilsWithLocalPoints;
    }
    if (Far::StencilTable const *varyingStencilsWithLocalPoints =
            Far::StencilTableFactory::AppendLocalPointStencilTable(
                *refiner, varyingStencils, patchTable->GetLocalPointStencilTable())) {
      delete varyingStencils;
      varyingStencils = varyingStencilsWithLocalPoints;
    }
  }

  // set tables to topology
  // HdPh_Subdivision takes an ownership of stencilTable and patchTable.
  _subdivision->SetRefinementTables(vertexStencils, varyingStencils, patchTable, _adaptive);

  _SetResolved();
  return true;
}

bool HdPh_Osd3TopologyComputation::_CheckValid() const
{
  return true;
}

// ---------------------------------------------------------------------------

HdPh_Osd3IndexComputation::HdPh_Osd3IndexComputation(HdPh_Osd3Subdivision *subdivision,
                                                     HdPh_MeshTopology *topology,
                                                     HdBufferSourceSharedPtr const &osdTopology)
    : HdPh_OsdIndexComputation(topology, osdTopology),
      _subdivision(subdivision)
{}

bool HdPh_Osd3IndexComputation::Resolve()
{
  using namespace OpenSubdiv;

  if (_osdTopology && !_osdTopology->IsResolved())
    return false;

  if (!_TryLock())
    return false;

  HdPh_Subdivision *subdivision = _topology->GetSubdivision();
  if (!TF_VERIFY(subdivision)) {
    _SetResolved();
    return true;
  }

  Far::PatchTable const *patchTable = _subdivision->GetPatchTable();

  Far::Index const *firstIndex = NULL;
  size_t ptableSize            = 0;
  if (patchTable) {
    ptableSize = patchTable->GetPatchControlVerticesTable().size();
    if (ptableSize > 0) {
      firstIndex = &patchTable->GetPatchControlVerticesTable()[0];
    }
  }

  TfToken const &scheme = _topology->GetScheme();

  if (_subdivision->IsAdaptive() &&
      (HdPh_Subdivision::RefinesToBSplinePatches(scheme) ||
       HdPh_Subdivision::RefinesToBoxSplineTrianglePatches(scheme))) {

    // Bundle groups of 12 or 16 patch control vertices.
    int arraySize = patchTable ? patchTable->GetPatchArrayDescriptor(0).GetNumControlVertices() :
                                 0;

    VtArray<int> indices(ptableSize);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    HdBufferSourceSharedPtr patchIndices(
        new HdVtBufferSource(HdTokens->indices, VtValue(indices), arraySize));

    _SetResult(patchIndices);

    _PopulatePatchPrimitiveBuffer(patchTable);
  }
  else if (HdPh_Subdivision::RefinesToTriangles(scheme)) {
    // populate refined triangle indices.
    VtArray<GfVec3i> indices(ptableSize / 3);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    HdBufferSourceSharedPtr triIndices(new HdVtBufferSource(HdTokens->indices, VtValue(indices)));
    _SetResult(triIndices);

    _PopulateUniformPrimitiveBuffer(patchTable);
  }
  else {
    // populate refined quad indices.
    VtArray<GfVec4i> indices(ptableSize / 4);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    // refined quads index buffer
    HdBufferSourceSharedPtr quadIndices(new HdVtBufferSource(HdTokens->indices, VtValue(indices)));
    _SetResult(quadIndices);

    _PopulateUniformPrimitiveBuffer(patchTable);
  }

  _SetResolved();
  return true;
}

void HdPh_Osd3IndexComputation::_CreatePtexFaceToCoarseFaceInfoMapping(
    std::vector<HdPh_Osd3IndexComputation::PtexFaceInfo> *result)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!TF_VERIFY(result))
    return;

  int const *numVertsPtr = _topology->GetFaceVertexCounts().cdata();
  int numAuthoredFaces   = _topology->GetFaceVertexCounts().size();
  int const *vertsPtr    = _topology->GetFaceVertexIndices().cdata();
  int numVertIndices     = _topology->GetFaceVertexIndices().size();
  result->clear();
  result->reserve(numAuthoredFaces);  // first guess at possible size

  // enumerate edges of the coarse topology to help compute the ptexFace's
  // coarse edge id's
  HdMeshUtil::EdgeMap authoredEdgeMap = HdMeshUtil::ComputeAuthoredEdgeMap(_topology);

  int regFaceSize = 4;
  if (HdPh_Subdivision::RefinesToTriangles(_topology->GetScheme())) {
    regFaceSize = 3;
  }

  // XXX: see comment below regarding flip
  // bool flip = (_topology->GetOrientation() != HdTokens->rightHanded);

  for (int faceId = 0, v = 0; faceId < numAuthoredFaces; ++faceId) {
    int nv = numVertsPtr[faceId];

    // hole faces shouldn't affect ptex id, i.e., ptex face id's are
    // assigned for hole faces.
    // note: this is inconsistent with quadrangulation
    // (HdMeshUtil::ComputeQuadIndices), but consistent with OpenSubdiv 3.x
    // (see ptexIndices.cpp)

    if (v + nv > numVertIndices)
      break;

    if (nv == regFaceSize) {
      // regular face => 1:1 mapping to a ptex face
      PtexFaceInfo info;
      info.coarseFaceId = faceId;

      GfVec4i coarseEdgeIds(-1, -1, -1, -1);

      // all edges of the regular face must exist in the authored edge map
      for (int e = 0; e < nv; ++e) {
        // XXX: don't we need to flip a face's vertex indices, like
        // we do in HdMeshUtil::Compute{Triangle,Quad}Indices?
        GfVec2i edge(vertsPtr[v + e], vertsPtr[v + (e + 1) % nv]);
        auto it = authoredEdgeMap.find(edge);
        TF_VERIFY(it != authoredEdgeMap.end());
        coarseEdgeIds[e] = it->second;
      }

      info.coarseEdgeIds = coarseEdgeIds;
      result->push_back(info);
    }
    else if (nv <= 2) {
      // Handling degenerated faces
      int numPtexFaces = (regFaceSize == 4) ? nv : nv - 2;
      for (int f = 0; f < numPtexFaces; ++f) {
        PtexFaceInfo info;
        info.coarseFaceId  = faceId;
        info.coarseEdgeIds = GfVec4i(-1, -1, -1, -1);
        result->push_back(info);
      }
    }
    else {
      // if we expect quad faces, non-quad n-gons are quadrangulated into
      // n-quads
      // if we expect tri faces, non-tri n-gons are triangulated into
      // n-2-tris. note: we don't currently support non-tri faces when
      // using loop (see pxOsd/refinerFactory.cpp)
      int numPtexFaces = (regFaceSize == 4) ? nv : nv - 2;
      for (int f = 0; f < numPtexFaces; ++f) {
        PtexFaceInfo info;
        info.coarseFaceId = faceId;

        GfVec4i coarseEdgeIds(-1, -1, -1, -1);

        if (regFaceSize == 4) {  // quadrangulation
          // only the first (index 0) and last (index 3) edges of the
          // quad are from the authored edges; the other 2 are the
          // result of quadrangulation.
          GfVec2i e0 = GfVec2i(vertsPtr[v + f], vertsPtr[v + (f + 1) % nv]);
          GfVec2i e3 = GfVec2i(vertsPtr[v + (f + nv - 1) % nv], vertsPtr[v + f]);
          auto it    = authoredEdgeMap.find(e0);
          TF_VERIFY(it != authoredEdgeMap.end());
          coarseEdgeIds[0] = it->second;

          it = authoredEdgeMap.find(e3);
          TF_VERIFY(it != authoredEdgeMap.end());
          coarseEdgeIds[3] = it->second;
        }
        else {  // triangular ptex
                // XXX: Add support
        }

        info.coarseEdgeIds = coarseEdgeIds;
        result->push_back(info);
      }
    }  // irregular face

    v += nv;
  }

  result->shrink_to_fit();
}

void HdPh_Osd3IndexComputation::_PopulateUniformPrimitiveBuffer(
    OpenSubdiv::Far::PatchTable const *patchTable)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  // primitiveParam from patchtable contains a map of
  // gl_PrimitiveID to PtexIndex. It should be reinterpreted
  // to face index if necessary.
  std::vector<PtexFaceInfo> ptexIndexToCoarseFaceInfoMapping;
  _CreatePtexFaceToCoarseFaceInfoMapping(&ptexIndexToCoarseFaceInfoMapping);

  // store faceIndex, ptexIndex and edgeFlag(=0)
  size_t numPatches = patchTable ? patchTable->GetPatchParamTable().size() : 0;
  VtVec3iArray primitiveParam(numPatches);
  VtVec4iArray edgeIndices(numPatches);

  // ivec3
  for (size_t i = 0; i < numPatches; ++i) {
    OpenSubdiv::Far::PatchParam const &patchParam = patchTable->GetPatchParamTable()[i];

    int ptexIndex            = patchParam.GetFaceId();
    PtexFaceInfo const &info = ptexIndexToCoarseFaceInfoMapping[ptexIndex];
    int faceIndex            = info.coarseFaceId;

    unsigned int field0  = patchParam.field0;
    unsigned int field1  = patchParam.field1;
    primitiveParam[i][0] = HdMeshUtil::EncodeCoarseFaceParam(faceIndex, 0);
    primitiveParam[i][1] = *((int *)&field0);
    primitiveParam[i][2] = *((int *)&field1);

    edgeIndices[i] = info.coarseEdgeIds;
  }

  _primitiveBuffer.reset(new HdVtBufferSource(HdTokens->primitiveParam, VtValue(primitiveParam)));

  _edgeIndicesBuffer.reset(new HdVtBufferSource(HdTokens->edgeIndices, VtValue(edgeIndices)));
}

void HdPh_Osd3IndexComputation::_PopulatePatchPrimitiveBuffer(
    OpenSubdiv::Far::PatchTable const *patchTable)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  std::vector<PtexFaceInfo> ptexIndexToCoarseFaceInfoMapping;
  _CreatePtexFaceToCoarseFaceInfoMapping(&ptexIndexToCoarseFaceInfoMapping);

  // BSPLINES
  size_t numPatches = patchTable ? patchTable->GetPatchParamTable().size() : 0;
  VtVec4iArray primitiveParam(numPatches);
  VtVec4iArray edgeIndices(numPatches);

  // ivec4
  for (size_t i = 0; i < numPatches; ++i) {
    OpenSubdiv::Far::PatchParam const &patchParam = patchTable->GetPatchParamTable()[i];

    float sharpness = 0.0;
    if (i < patchTable->GetSharpnessIndexTable().size()) {
      OpenSubdiv::Far::Index sharpnessIndex = patchTable->GetSharpnessIndexTable()[i];
      if (sharpnessIndex >= 0)
        sharpness = patchTable->GetSharpnessValues()[sharpnessIndex];
    }

    int ptexIndex            = patchParam.GetFaceId();
    PtexFaceInfo const &info = ptexIndexToCoarseFaceInfoMapping[ptexIndex];
    int faceIndex            = info.coarseFaceId;
    unsigned int field0      = patchParam.field0;
    unsigned int field1      = patchParam.field1;
    primitiveParam[i][0]     = HdMeshUtil::EncodeCoarseFaceParam(faceIndex, 0);
    primitiveParam[i][1]     = *((int *)&field0);
    primitiveParam[i][2]     = *((int *)&field1);

    int sharpnessAsInt   = static_cast<int>(sharpness);
    primitiveParam[i][3] = sharpnessAsInt;

    edgeIndices[i] = info.coarseEdgeIds;
  }
  _primitiveBuffer.reset(new HdVtBufferSource(HdTokens->primitiveParam, VtValue(primitiveParam)));

  _edgeIndicesBuffer.reset(new HdVtBufferSource(HdTokens->edgeIndices, VtValue(edgeIndices)));
}

// ---------------------------------------------------------------------------

HdPh_Subdivision *HdPh_Osd3Factory::CreateSubdivision()
{
  return new HdPh_Osd3Subdivision();
}

WABI_NAMESPACE_END
