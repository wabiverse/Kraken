
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

#include "wabi/base/arch/defines.h"

#ifdef ARCH_OS_WINDOWS
# include <winrt/base.h>
#endif /* ARCH_OS_WINDOWS */

#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/subdivision.h"
#include "wabi/imaging/hdPh/subdivision3.h"
#include "wabi/imaging/hdPh/tokens.h"

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
#include <opensubdiv/osd/mesh.h>
#include <opensubdiv/version.h>

WABI_NAMESPACE_BEGIN

typedef OpenSubdiv::Osd::CpuVertexBuffer HdPh_OsdCpuVertexBuffer;

WABI_NAMESPACE_END

// There's a buffer synchronization bug in driver 331, and apparently fixed in 334.
// Don't enable compute shader kernel until driver updates.

#if OPENSUBDIV_HAS_GLSL_COMPUTE

#  include <opensubdiv/osd/glVertexBuffer.h>
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

class HdPh_Osd3Subdivision : public HdPh_Subdivision
{
 public:
  /// Construct HdPh_Subdivision. It takes an ownership of farmesh.
  HdPh_Osd3Subdivision();
  ~HdPh_Osd3Subdivision() override;

  int GetNumVertices() const override;
  int GetNumVarying() const override;
  int GetNumFaceVarying(int channel) const override;
  int GetMaxNumFaceVarying() const override;

  VtIntArray GetRefinedFvarIndices(int channel) const override;

  void RefineCPU(HdBufferSourceSharedPtr const &source,
                 void *vertexBuffer,
                 HdPh_MeshTopology::Interpolation interpolation,
                 int fvarChannel = 0) override;

  void RefineGPU(HdBufferArrayRangeSharedPtr const &range,
                 TfToken const &name,
                 HdPh_MeshTopology::Interpolation interpolation,
                 int fvarChannel = 0) override;

  // computation factory methods
  HdBufferSourceSharedPtr CreateTopologyComputation(HdPh_MeshTopology *topology,
                                                    bool adaptive,
                                                    int level,
                                                    SdfPath const &id) override;

  HdBufferSourceSharedPtr CreateIndexComputation(HdPh_MeshTopology *topology,
                                                 HdBufferSourceSharedPtr const &osdTopology) override;

  HdBufferSourceSharedPtr CreateFvarIndexComputation(HdPh_MeshTopology *topology,
                                                     HdBufferSourceSharedPtr const &osdTopology,
                                                     int channel) override;

  HdBufferSourceSharedPtr CreateRefineComputation(HdPh_MeshTopology *topology,
                                                  HdBufferSourceSharedPtr const &source,
                                                  HdBufferSourceSharedPtr const &osdTopology,
                                                  HdPh_MeshTopology::Interpolation interpolation,
                                                  int fvarChannel = 0) override;

  HdComputationSharedPtr CreateRefineComputationGPU(HdPh_MeshTopology *topology,
                                                    TfToken const &name,
                                                    HdType dataType,
                                                    HdPh_MeshTopology::Interpolation interpolation,
                                                    int fvarChannel = 0) override;

  void SetRefinementTables(OpenSubdiv::Far::StencilTable const *vertexStencils,
                           OpenSubdiv::Far::StencilTable const *varyingStencils,
                           std::vector<OpenSubdiv::Far::StencilTable const *> faceVaryingStencils,
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
  std::vector<OpenSubdiv::Far::StencilTable const *> _faceVaryingStencils;
  OpenSubdiv::Far::PatchTable const *_patchTable;
  bool _adaptive;
  int _maxNumFaceVarying;  // calculated during SetRefinementTables()

#if HDPH_ENABLE_GPU_SUBDIVISION
  /// Returns GPU stencil table. Creates it if not existed.
  /// A valid GL context has to be made to current before calling this method.
  HdPh_OsdGpuStencilTable *_GetGpuVertexStencilTable();
  HdPh_OsdGpuStencilTable *_GetGpuVaryingStencilTable();
  HdPh_OsdGpuStencilTable *_GetGpuFaceVaryingStencilTable(int channel);
  HdPh_OsdGpuStencilTable *_gpuVertexStencilTable;
  HdPh_OsdGpuStencilTable *_gpuVaryingStencilTable;
  std::vector<HdPh_OsdGpuStencilTable *> _gpuFaceVaryingStencilTables;
#endif
};

class HdPh_Osd3IndexComputation : public HdPh_OsdIndexComputation
{
  struct BaseFaceInfo
  {
    int baseFaceParam;
    GfVec2i baseFaceEdgeIndices;
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
  void _CreateBaseFaceMapping(std::vector<BaseFaceInfo> *result);

  HdPh_Osd3Subdivision *_subdivision;
};

class HdPh_Osd3FvarIndexComputation : public HdComputedBufferSource
{

 public:
  HdPh_Osd3FvarIndexComputation(HdPh_Osd3Subdivision *subdivision,
                                HdPh_MeshTopology *topology,
                                HdBufferSourceSharedPtr const &osdTopology,
                                int channel);
  /// overrides
  bool HasChainedBuffer() const override;
  bool Resolve() override;
  void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  HdBufferSourceSharedPtrVector GetChainedBuffers() const override;

 protected:
  bool _CheckValid() const override;

 private:
  void _PopulateFvarPatchParamBuffer(OpenSubdiv::Far::PatchTable const *patchTable);

  HdPh_MeshTopology *_topology;
  HdBufferSourceSharedPtr _osdTopology;
  HdPh_Osd3Subdivision *_subdivision;
  HdBufferSourceSharedPtr _fvarPatchParamBuffer;
  int _channel;
  TfToken _indicesName;
  TfToken _patchParamName;
};

class HdPh_Osd3TopologyComputation : public HdPh_OsdTopologyComputation
{
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
  : _vertexStencils(nullptr),
    _varyingStencils(nullptr),
    _patchTable(nullptr),
    _adaptive(false),
    _maxNumFaceVarying(0)
{
#if HDPH_ENABLE_GPU_SUBDIVISION
  _gpuVertexStencilTable = nullptr;
  _gpuVaryingStencilTable = nullptr;
#endif
}

HdPh_Osd3Subdivision::~HdPh_Osd3Subdivision()
{
  delete _vertexStencils;
  delete _varyingStencils;
  for (size_t i = 0; i < _faceVaryingStencils.size(); ++i)
  {
    delete _faceVaryingStencils[i];
  }
  _faceVaryingStencils.clear();
  delete _patchTable;
#if HDPH_ENABLE_GPU_SUBDIVISION
  delete _gpuVertexStencilTable;
  delete _gpuVaryingStencilTable;
  for (size_t i = 0; i < _gpuFaceVaryingStencilTables.size(); ++i)
  {
    delete _gpuFaceVaryingStencilTables[i];
  }
  _gpuFaceVaryingStencilTables.clear();
#endif
}

void HdPh_Osd3Subdivision::SetRefinementTables(
  OpenSubdiv::Far::StencilTable const *vertexStencils,
  OpenSubdiv::Far::StencilTable const *varyingStencils,
  std::vector<OpenSubdiv::Far::StencilTable const *> faceVaryingStencils,
  OpenSubdiv::Far::PatchTable const *patchTable,
  bool adaptive)
{
  if (_vertexStencils)
  {
    delete _vertexStencils;
  }
  if (_varyingStencils)
  {
    delete _varyingStencils;
  }
  for (size_t i = 0; i < _faceVaryingStencils.size(); i++)
  {
    if (_faceVaryingStencils[i])
    {
      delete _faceVaryingStencils[i];
    }
  }
  _faceVaryingStencils.clear();
  if (_patchTable)
    delete _patchTable;

  _vertexStencils = vertexStencils;
  _varyingStencils = varyingStencils;
  _faceVaryingStencils = faceVaryingStencils;
  _patchTable = patchTable;
  _adaptive = adaptive;

  _maxNumFaceVarying = 0;
  for (size_t i = 0; i < _faceVaryingStencils.size(); ++i)
  {
    _maxNumFaceVarying = std::max(_maxNumFaceVarying, GetNumFaceVarying(i));
  }
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
int HdPh_Osd3Subdivision::GetNumVarying() const
{
  // returns the total number of vertices, including coarse and refined ones.
  if (!TF_VERIFY(_varyingStencils))
    return 0;

  return _varyingStencils->GetNumStencils() + _varyingStencils->GetNumControlVertices();
}

/*virtual*/
int HdPh_Osd3Subdivision::GetNumFaceVarying(int channel) const
{
  // returns the total number of vertices, including coarse and refined ones.
  if (!TF_VERIFY(_faceVaryingStencils[channel]))
    return 0;

  return _faceVaryingStencils[channel]->GetNumStencils() +
         _faceVaryingStencils[channel]->GetNumControlVertices();
}

/*virtual*/
int HdPh_Osd3Subdivision::GetMaxNumFaceVarying() const
{
  // returns the largest total number of face-varying values (coarse and
  // refined) for all the face-varying channels
  return _maxNumFaceVarying;
}

/*virtual*/
VtIntArray HdPh_Osd3Subdivision::GetRefinedFvarIndices(int channel) const
{
  VtIntArray fvarIndices;
  if (_patchTable && _patchTable->GetNumFVarChannels() > channel)
  {
    OpenSubdiv::Far::ConstIndexArray indices = _patchTable->GetFVarValues(channel);
    for (int i = 0; i < indices.size(); ++i)
    {
      fvarIndices.push_back(indices[i]);
    }
  }
  return fvarIndices;
}

/*virtual*/
void HdPh_Osd3Subdivision::RefineCPU(HdBufferSourceSharedPtr const &source,
                                     void *vertexBuffer,
                                     HdPh_MeshTopology::Interpolation interpolation,
                                     int fvarChannel)
{
  if (interpolation == HdPh_MeshTopology::INTERPOLATE_FACEVARYING)
  {
    if (!TF_VERIFY(fvarChannel >= 0))
    {
      return;
    }

    if (!TF_VERIFY(fvarChannel < (int)_faceVaryingStencils.size()))
    {
      return;
    }
  }

  OpenSubdiv::Far::StencilTable const *stencilTable =
    (interpolation == HdPh_MeshTopology::INTERPOLATE_VERTEX)  ? _vertexStencils :
    (interpolation == HdPh_MeshTopology::INTERPOLATE_VARYING) ? _varyingStencils :
                                                                _faceVaryingStencils[fvarChannel];

  if (!TF_VERIFY(stencilTable))
    return;

  OpenSubdiv::Osd::CpuVertexBuffer *osdVertexBuffer = static_cast<OpenSubdiv::Osd::CpuVertexBuffer *>(
    vertexBuffer);

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
  if (numElements > (size_t)stencilTable->GetNumControlVertices())
  {
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

  OpenSubdiv::Osd::CpuEvaluator::EvalStencils(osdVertexBuffer,
                                              srcDesc,
                                              osdVertexBuffer,
                                              dstDesc,
                                              stencilTable);
}

/*virtual*/
void HdPh_Osd3Subdivision::RefineGPU(HdBufferArrayRangeSharedPtr const &range,
                                     TfToken const &name,
                                     HdPh_MeshTopology::Interpolation interpolation,
                                     int fvarChannel)
{
#if HDPH_ENABLE_GPU_SUBDIVISION
  if (interpolation == HdPh_MeshTopology::INTERPOLATE_FACEVARYING)
  {
    if (!TF_VERIFY(fvarChannel >= 0))
    {
      return;
    }

    if (!TF_VERIFY(fvarChannel < (int)_faceVaryingStencils.size()))
    {
      return;
    }
  }

  OpenSubdiv::Far::StencilTable const *stencilTable =
    (interpolation == HdPh_MeshTopology::INTERPOLATE_VERTEX)  ? _vertexStencils :
    (interpolation == HdPh_MeshTopology::INTERPOLATE_VARYING) ? _varyingStencils :
                                                                _faceVaryingStencils[fvarChannel];

  if (!TF_VERIFY(stencilTable))
  {
    return;
  }

  // filling coarse vertices has been done at resource registry.

  HdPhBufferArrayRangeSharedPtr range_ = std::static_pointer_cast<HdPhBufferArrayRange>(range);

  // vertex buffer wrapper for OpenSubdiv API
  HdPh_OsdRefineComputationGPU::VertexBuffer vertexBuffer(range_->GetResource(name));

  // vertex buffer is not interleaved, but aggregated.
  // we need an offset to locate the current range.
  size_t stride = vertexBuffer.GetNumElements();
  int numCoarseVertices = stencilTable->GetNumControlVertices();

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
    &evaluatorCache,
    srcDesc,
    dstDesc,
    (void *)NULL /*deviceContext*/);

  HdPh_OsdGpuStencilTable const *gpuStencilTable = (interpolation == HdPh_MeshTopology::INTERPOLATE_VERTEX) ?
                                                     _GetGpuVertexStencilTable() :
                                                   (interpolation ==
                                                    HdPh_MeshTopology::INTERPOLATE_VARYING) ?
                                                     _GetGpuVaryingStencilTable() :
                                                     _GetGpuFaceVaryingStencilTable(fvarChannel);

  instance->EvalStencils(&vertexBuffer, srcDesc, &vertexBuffer, dstDesc, gpuStencilTable);
#else
  TF_CODING_ERROR("No GPU kernel available.\n");
#endif
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateTopologyComputation(HdPh_MeshTopology *topology,
                                                                        bool adaptive,
                                                                        int level,
                                                                        SdfPath const &id)
{
  return std::make_shared<HdPh_Osd3TopologyComputation>(this, topology, adaptive, level, id);
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateIndexComputation(
  HdPh_MeshTopology *topology,
  HdBufferSourceSharedPtr const &osdTopology)
{
  return std::make_shared<HdPh_Osd3IndexComputation>(this, topology, osdTopology);
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateFvarIndexComputation(
  HdPh_MeshTopology *topology,
  HdBufferSourceSharedPtr const &osdTopology,
  int channel)
{
  return std::make_shared<HdPh_Osd3FvarIndexComputation>(this, topology, osdTopology, channel);
}

/*virtual*/
HdBufferSourceSharedPtr HdPh_Osd3Subdivision::CreateRefineComputation(
  HdPh_MeshTopology *topology,
  HdBufferSourceSharedPtr const &source,
  HdBufferSourceSharedPtr const &osdTopology,
  HdPh_MeshTopology::Interpolation interpolation,
  int fvarChannel)
{
  return std::make_shared<HdPh_OsdRefineComputation<HdPh_OsdCpuVertexBuffer>>(topology,
                                                                              source,
                                                                              osdTopology,
                                                                              interpolation,
                                                                              fvarChannel);
}

/*virtual*/
HdComputationSharedPtr HdPh_Osd3Subdivision::CreateRefineComputationGPU(
  HdPh_MeshTopology *topology,
  TfToken const &name,
  HdType dataType,
  HdPh_MeshTopology::Interpolation interpolation,
  int fvarChannel)
{
  return std::make_shared<HdPh_OsdRefineComputationGPU>(topology,
                                                        name,
                                                        dataType,
                                                        interpolation,
                                                        fvarChannel);
}

#if HDPH_ENABLE_GPU_SUBDIVISION

HdPh_OsdGpuStencilTable *HdPh_Osd3Subdivision::_GetGpuVertexStencilTable()
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!_gpuVertexStencilTable)
  {
    _gpuVertexStencilTable = HdPh_OsdGpuStencilTable::Create(_vertexStencils, NULL);
  }

  return _gpuVertexStencilTable;
}

HdPh_OsdGpuStencilTable *HdPh_Osd3Subdivision::_GetGpuVaryingStencilTable()
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!_gpuVaryingStencilTable)
  {
    _gpuVaryingStencilTable = HdPh_OsdGpuStencilTable::Create(_varyingStencils, NULL);
  }

  return _gpuVaryingStencilTable;
}

HdPh_OsdGpuStencilTable *HdPh_Osd3Subdivision::_GetGpuFaceVaryingStencilTable(int channel)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (_gpuFaceVaryingStencilTables.empty())
  {
    _gpuFaceVaryingStencilTables.resize(_faceVaryingStencils.size(), nullptr);
  }

  if (!_gpuFaceVaryingStencilTables[channel])
  {
    _gpuFaceVaryingStencilTables[channel] = HdPh_OsdGpuStencilTable::Create(_faceVaryingStencils[channel],
                                                                            NULL);
  }

  return _gpuFaceVaryingStencilTables[channel];
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

  if (!TF_VERIFY(_topology))
  {
    _SetResolved();
    return true;
  }

  // for empty topology, we don't need to refine anything.
  // but still need to return the typed buffer for codegen
  int numFvarChannels = 0;
  if (_topology->GetFaceVertexCounts().size() == 0)
  {
    // leave refiner empty
  } else
  {
    refiner = PxOsdRefinerFactory::Create(_topology->GetPxOsdMeshTopology(),
                                          _topology->GetFvarTopologies(),
                                          TfToken(_id.GetText()));
    numFvarChannels = refiner->GetNumFVarChannels();
  }

  if (!TF_VERIFY(_subdivision))
  {
    _SetResolved();
    return true;
  }

  // refine
  //  and
  // create stencil/patch table
  Far::StencilTable const *vertexStencils = nullptr;
  Far::StencilTable const *varyingStencils = nullptr;
  std::vector<Far::StencilTable const *> faceVaryingStencils(numFvarChannels);
  Far::PatchTable const *patchTable = nullptr;

  if (refiner)
  {
    Far::PatchTableFactory::Options patchOptions(_level);
    if (numFvarChannels > 0)
    {
      patchOptions.generateFVarTables = true;
      patchOptions.includeFVarBaseLevelIndices = true;
      patchOptions.generateFVarLegacyLinearPatches = !_adaptive;
    }
    if (_adaptive)
    {
      patchOptions.endCapType = Far::PatchTableFactory::Options::ENDCAP_BSPLINE_BASIS;
#if OPENSUBDIV_VERSION_NUMBER >= 30400
      // Improve fidelity when refining to limit surface patches
      // These options supported since v3.1.0 and v3.2.0 respectively.
      patchOptions.useInfSharpPatch = true;
      patchOptions.generateLegacySharpCornerPatches = false;
#endif
    }

    // split trace scopes.
    {
      HD_TRACE_SCOPE("refine");
      if (_adaptive)
      {
        Far::TopologyRefiner::AdaptiveOptions adaptiveOptions(_level);
#if OPENSUBDIV_VERSION_NUMBER >= 30400
        adaptiveOptions = patchOptions.GetRefineAdaptiveOptions();
#endif
        refiner->RefineAdaptive(adaptiveOptions);
      } else
      {
        refiner->RefineUniform(_level);
      }
    }
    {
      HD_TRACE_SCOPE("stencil factory");
      Far::StencilTableFactory::Options options;
      options.generateOffsets = true;
      options.generateIntermediateLevels = _adaptive;
      options.interpolationMode = Far::StencilTableFactory::INTERPOLATE_VERTEX;
      vertexStencils = Far::StencilTableFactory::Create(*refiner, options);

      options.interpolationMode = Far::StencilTableFactory::INTERPOLATE_VARYING;
      varyingStencils = Far::StencilTableFactory::Create(*refiner, options);

      options.interpolationMode = Far::StencilTableFactory::INTERPOLATE_FACE_VARYING;
      for (int i = 0; i < numFvarChannels; ++i)
      {
        options.fvarChannel = i;
        faceVaryingStencils[i] = Far::StencilTableFactory::Create(*refiner, options);
      }
    }
    {
      HD_TRACE_SCOPE("patch factory");
      patchTable = Far::PatchTableFactory::Create(*refiner, patchOptions);
    }
  }

  // merge endcap
  if (patchTable && patchTable->GetLocalPointStencilTable())
  {
    // append stencils
    if (Far::StencilTable const *vertexStencilsWithLocalPoints =
          Far::StencilTableFactory::AppendLocalPointStencilTable(*refiner,
                                                                 vertexStencils,
                                                                 patchTable->GetLocalPointStencilTable()))
    {
      delete vertexStencils;
      vertexStencils = vertexStencilsWithLocalPoints;
    }
  }
  if (patchTable && patchTable->GetLocalPointVaryingStencilTable())
  {
    // append stencils
    if (Far::StencilTable const *varyingStencilsWithLocalPoints =
          Far::StencilTableFactory::AppendLocalPointStencilTableVarying(
            *refiner,
            varyingStencils,
            patchTable->GetLocalPointVaryingStencilTable()))
    {
      delete varyingStencils;
      varyingStencils = varyingStencilsWithLocalPoints;
    }
  }
  for (int i = 0; i < numFvarChannels; ++i)
  {
    if (patchTable && patchTable->GetLocalPointFaceVaryingStencilTable(i))
    {
      // append stencils
      if (Far::StencilTable const *faceVaryingStencilsWithLocalPoints =
            Far::StencilTableFactory::AppendLocalPointStencilTableFaceVarying(
              *refiner,
              faceVaryingStencils[i],
              patchTable->GetLocalPointFaceVaryingStencilTable(i),
              i))
      {
        delete faceVaryingStencils[i];
        faceVaryingStencils[i] = faceVaryingStencilsWithLocalPoints;
      }
    }
  }

  // set tables to topology
  // HdPh_Subdivision takes an ownership of stencilTables and patchTable.
  _subdivision->SetRefinementTables(vertexStencils,
                                    varyingStencils,
                                    faceVaryingStencils,
                                    patchTable,
                                    _adaptive);

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
  if (!TF_VERIFY(subdivision))
  {
    _SetResolved();
    return true;
  }

  Far::PatchTable const *patchTable = _subdivision->GetPatchTable();

  Far::Index const *firstIndex = NULL;
  size_t ptableSize = 0;
  if (patchTable)
  {
    ptableSize = patchTable->GetPatchControlVerticesTable().size();
    if (ptableSize > 0)
    {
      firstIndex = &patchTable->GetPatchControlVerticesTable()[0];
    }
  }

  TfToken const &scheme = _topology->GetScheme();

  if (_subdivision->IsAdaptive() && (HdPh_Subdivision::RefinesToBSplinePatches(scheme) ||
                                     HdPh_Subdivision::RefinesToBoxSplineTrianglePatches(scheme)))
  {

    // Bundle groups of 12 or 16 patch control vertices.
    int arraySize = patchTable ? patchTable->GetPatchArrayDescriptor(0).GetNumControlVertices() : 0;

    VtArray<int> indices(ptableSize);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    HdBufferSourceSharedPtr patchIndices = std::make_shared<HdVtBufferSource>(HdTokens->indices,
                                                                              VtValue(indices),
                                                                              arraySize);

    _SetResult(patchIndices);

    _PopulatePatchPrimitiveBuffer(patchTable);
  } else if (HdPh_Subdivision::RefinesToTriangles(scheme))
  {
    // populate refined triangle indices.
    VtArray<GfVec3i> indices(ptableSize / 3);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    HdBufferSourceSharedPtr triIndices = std::make_shared<HdVtBufferSource>(HdTokens->indices,
                                                                            VtValue(indices));
    _SetResult(triIndices);

    _PopulateUniformPrimitiveBuffer(patchTable);
  } else
  {
    // populate refined quad indices.
    VtArray<GfVec4i> indices(ptableSize / 4);
    memcpy(indices.data(), firstIndex, ptableSize * sizeof(int));

    // refined quads index buffer
    HdBufferSourceSharedPtr quadIndices = std::make_shared<HdVtBufferSource>(HdTokens->indices,
                                                                             VtValue(indices));
    _SetResult(quadIndices);

    _PopulateUniformPrimitiveBuffer(patchTable);
  }

  _SetResolved();
  return true;
}

void HdPh_Osd3IndexComputation::_CreateBaseFaceMapping(
  std::vector<HdPh_Osd3IndexComputation::BaseFaceInfo> *result)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!TF_VERIFY(result))
    return;

  int const *numVertsPtr = _topology->GetFaceVertexCounts().cdata();
  int const numFaces = _topology->GetFaceVertexCounts().size();
  int const numVertIndices = _topology->GetFaceVertexIndices().size();

  result->clear();
  result->reserve(numFaces);

  int regFaceSize = 4;
  if (HdPh_Subdivision::RefinesToTriangles(_topology->GetScheme()))
  {
    regFaceSize = 3;
  }

  for (int i = 0, v = 0, ev = 0; i < numFaces; ++i)
  {
    int const nv = numVertsPtr[i];

    if (v + nv > numVertIndices)
      break;

    if (nv == regFaceSize)
    {
      BaseFaceInfo info;
      info.baseFaceParam = HdMeshUtil::EncodeCoarseFaceParam(i, /*edgeFlag=*/0);
      info.baseFaceEdgeIndices = GfVec2i(ev, 0);
      result->push_back(info);
    } else if (nv < 3)
    {
      int const numBaseFaces = (regFaceSize == 4) ? nv : nv - 2;
      for (int f = 0; f < numBaseFaces; ++f)
      {
        BaseFaceInfo info;
        info.baseFaceParam = HdMeshUtil::EncodeCoarseFaceParam(i, /*edgeFlag=*/0);
        info.baseFaceEdgeIndices = GfVec2i(-1, -1);
        result->push_back(info);
      }
    } else
    {
      for (int j = 0; j < nv; ++j)
      {
        int edgeFlag = 0;
        if (j == 0)
        {
          edgeFlag = 1;
        } else if (j == nv - 1)
        {
          edgeFlag = 2;
        } else
        {
          edgeFlag = 3;
        }

        BaseFaceInfo info;
        info.baseFaceParam = HdMeshUtil::EncodeCoarseFaceParam(i, edgeFlag);
        info.baseFaceEdgeIndices = GfVec2i(ev + j, ev + (j + nv - 1) % nv);
        result->push_back(info);
      }
    }

    v += nv;
    ev += nv;
  }

  result->shrink_to_fit();
}

void HdPh_Osd3IndexComputation::_PopulateUniformPrimitiveBuffer(
  OpenSubdiv::Far::PatchTable const *patchTable)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  std::vector<BaseFaceInfo> patchFaceToBaseFaceMapping;
  _CreateBaseFaceMapping(&patchFaceToBaseFaceMapping);

  size_t numPatches = patchTable ? patchTable->GetPatchParamTable().size() : 0;
  VtVec3iArray primitiveParam(numPatches);
  VtVec2iArray edgeIndices(numPatches);

  for (size_t i = 0; i < numPatches; ++i)
  {
    OpenSubdiv::Far::PatchParam const &patchParam = patchTable->GetPatchParamTable()[i];

    int patchFaceIndex = patchParam.GetFaceId();
    BaseFaceInfo const &info = patchFaceToBaseFaceMapping[patchFaceIndex];

    unsigned int field0 = patchParam.field0;
    unsigned int field1 = patchParam.field1;
    primitiveParam[i][0] = info.baseFaceParam;
    primitiveParam[i][1] = *((int *)&field0);
    primitiveParam[i][2] = *((int *)&field1);

    edgeIndices[i] = info.baseFaceEdgeIndices;
  }

  _primitiveBuffer.reset(new HdVtBufferSource(HdTokens->primitiveParam, VtValue(primitiveParam)));

  _edgeIndicesBuffer.reset(new HdVtBufferSource(HdTokens->edgeIndices, VtValue(edgeIndices)));
}

void HdPh_Osd3IndexComputation::_PopulatePatchPrimitiveBuffer(OpenSubdiv::Far::PatchTable const *patchTable)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  std::vector<BaseFaceInfo> patchFaceToBaseFaceMapping;
  _CreateBaseFaceMapping(&patchFaceToBaseFaceMapping);

  size_t numPatches = patchTable ? patchTable->GetPatchParamTable().size() : 0;
  VtVec4iArray primitiveParam(numPatches);
  VtVec2iArray edgeIndices(numPatches);

  for (size_t i = 0; i < numPatches; ++i)
  {
    OpenSubdiv::Far::PatchParam const &patchParam = patchTable->GetPatchParamTable()[i];

    float sharpness = 0.0;
    if (i < patchTable->GetSharpnessIndexTable().size())
    {
      OpenSubdiv::Far::Index sharpnessIndex = patchTable->GetSharpnessIndexTable()[i];
      if (sharpnessIndex >= 0)
        sharpness = patchTable->GetSharpnessValues()[sharpnessIndex];
    }

    int patchFaceIndex = patchParam.GetFaceId();
    BaseFaceInfo const &info = patchFaceToBaseFaceMapping[patchFaceIndex];

    unsigned int field0 = patchParam.field0;
    unsigned int field1 = patchParam.field1;
    primitiveParam[i][0] = info.baseFaceParam;
    primitiveParam[i][1] = *((int *)&field0);
    primitiveParam[i][2] = *((int *)&field1);

    int sharpnessAsInt = static_cast<int>(sharpness);
    primitiveParam[i][3] = sharpnessAsInt;

    edgeIndices[i] = info.baseFaceEdgeIndices;
  }
  _primitiveBuffer.reset(new HdVtBufferSource(HdTokens->primitiveParam, VtValue(primitiveParam)));

  _edgeIndicesBuffer.reset(new HdVtBufferSource(HdTokens->edgeIndices, VtValue(edgeIndices)));
}

// ---------------------------------------------------------------------------

HdPh_Osd3FvarIndexComputation::HdPh_Osd3FvarIndexComputation(HdPh_Osd3Subdivision *subdivision,
                                                             HdPh_MeshTopology *topology,
                                                             HdBufferSourceSharedPtr const &osdTopology,
                                                             int channel)
  : _topology(topology),
    _osdTopology(osdTopology),
    _subdivision(subdivision),
    _channel(channel)
{
  _indicesName = TfToken(HdPhTokens->fvarIndices.GetString() + std::to_string(_channel));
  _patchParamName = TfToken(HdPhTokens->fvarPatchParam.GetString() + std::to_string(_channel));
}

bool HdPh_Osd3FvarIndexComputation::Resolve()
{
  using namespace OpenSubdiv;

  if (_osdTopology && !_osdTopology->IsResolved())
    return false;

  if (!_TryLock())
    return false;

  HdPh_Subdivision *subdivision = _topology->GetSubdivision();
  if (!TF_VERIFY(subdivision))
  {
    _SetResolved();
    return true;
  }

  VtIntArray fvarIndices = _subdivision->GetRefinedFvarIndices(_channel);
  if (!TF_VERIFY(!fvarIndices.empty()))
  {
    _SetResolved();
    return true;
  }

  Far::Index const *firstIndex = fvarIndices.cdata();
  Far::PatchTable const *patchTable = _subdivision->GetPatchTable();
  size_t numPatches = patchTable ? patchTable->GetNumPatchesTotal() : 0;

  TfToken const &scheme = _topology->GetScheme();

  if (_subdivision->IsAdaptive() && (HdPh_Subdivision::RefinesToBSplinePatches(scheme) ||
                                     HdPh_Subdivision::RefinesToBoxSplineTrianglePatches(scheme)))
  {
    // Bundle groups of 12 or 16 patch control vertices
    int arraySize = patchTable ? patchTable->GetFVarPatchDescriptor(_channel).GetNumControlVertices() : 0;

    VtIntArray indices(arraySize * numPatches);
    memcpy(indices.data(), firstIndex, arraySize * numPatches * sizeof(int));

    HdBufferSourceSharedPtr patchIndices = std::make_shared<HdVtBufferSource>(_indicesName,
                                                                              VtValue(indices),
                                                                              arraySize);

    _SetResult(patchIndices);
    _PopulateFvarPatchParamBuffer(patchTable);
  } else if (HdPh_Subdivision::RefinesToTriangles(scheme))
  {
    // populate refined triangle indices.
    VtArray<GfVec3i> indices(numPatches);
    memcpy(indices.data(), firstIndex, 3 * numPatches * sizeof(int));

    HdBufferSourceSharedPtr triIndices = std::make_shared<HdVtBufferSource>(_indicesName, VtValue(indices));
    _SetResult(triIndices);
  } else
  {
    // populate refined quad indices.
    VtArray<GfVec4i> indices(numPatches);
    memcpy(indices.data(), firstIndex, 4 * numPatches * sizeof(int));

    HdBufferSourceSharedPtr quadIndices = std::make_shared<HdVtBufferSource>(_indicesName, VtValue(indices));
    _SetResult(quadIndices);
  }

  _SetResolved();
  return true;
}

void HdPh_Osd3FvarIndexComputation::_PopulateFvarPatchParamBuffer(
  OpenSubdiv::Far::PatchTable const *patchTable)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  VtVec2iArray fvarPatchParam(0);

  if (patchTable)
  {
    OpenSubdiv::Far::ConstPatchParamArray patchParamArray = patchTable->GetFVarPatchParams(_channel);
    size_t numPatches = patchParamArray.size();
    fvarPatchParam.resize(numPatches);

    for (size_t i = 0; i < numPatches; ++i)
    {
      OpenSubdiv::Far::PatchParam const &patchParam = patchParamArray[i];
      fvarPatchParam[i][0] = patchParam.field0;
      fvarPatchParam[i][1] = patchParam.field1;
    }
  }

  _fvarPatchParamBuffer.reset(new HdVtBufferSource(_patchParamName, VtValue(fvarPatchParam)));
}

void HdPh_Osd3FvarIndexComputation::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  if (_topology->RefinesToBSplinePatches())
  {
    // bi-cubic bspline patches
    specs->emplace_back(_indicesName, HdTupleType{HdTypeInt32, 16});
    specs->emplace_back(_patchParamName, HdTupleType{HdTypeInt32Vec2, 1});
  } else if (_topology->RefinesToBoxSplineTrianglePatches())
  {
    // quartic box spline triangle patches
    specs->emplace_back(_indicesName, HdTupleType{HdTypeInt32, 12});
    specs->emplace_back(_patchParamName, HdTupleType{HdTypeInt32Vec2, 1});
  } else if (HdPh_Subdivision::RefinesToTriangles(_topology->GetScheme()))
  {
    // triangles (loop)
    specs->emplace_back(_indicesName, HdTupleType{HdTypeInt32Vec3, 1});
  } else
  {
    // quads (catmark, bilinear)
    specs->emplace_back(_indicesName, HdTupleType{HdTypeInt32Vec4, 1});
  }
}

bool HdPh_Osd3FvarIndexComputation::HasChainedBuffer() const
{
  return (_topology->RefinesToBSplinePatches() || _topology->RefinesToBoxSplineTrianglePatches());
}

HdBufferSourceSharedPtrVector HdPh_Osd3FvarIndexComputation::GetChainedBuffers() const
{
  if (_topology->RefinesToBSplinePatches() || _topology->RefinesToBoxSplineTrianglePatches())
  {
    return {_fvarPatchParamBuffer};
  } else
  {
    return {};
  }
}

bool HdPh_Osd3FvarIndexComputation::_CheckValid() const
{
  return true;
}

// ---------------------------------------------------------------------------
HdPh_Subdivision *HdPh_Osd3Factory::CreateSubdivision()
{
  return new HdPh_Osd3Subdivision();
}

WABI_NAMESPACE_END
