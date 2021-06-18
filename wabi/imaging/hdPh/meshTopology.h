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
#ifndef WABI_IMAGING_HD_ST_MESH_TOPOLOGY_H
#define WABI_IMAGING_HD_ST_MESH_TOPOLOGY_H

#include "wabi/imaging/hd/meshTopology.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
class HdPh_Subdivision;
struct HdQuadInfo;
class SdfPath;

using HdBufferSourceWeakPtr = std::weak_ptr<class HdBufferSource>;
using HdBufferSourceSharedPtr = std::shared_ptr<class HdBufferSource>;

using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;

using HdComputationSharedPtr = std::shared_ptr<class HdComputation>;

using HdPh_AdjacencyBuilderComputationPtr = std::weak_ptr<class HdPh_AdjacencyBuilderComputation>;

using HdPh_QuadInfoBuilderComputationPtr = std::weak_ptr<class HdPh_QuadInfoBuilderComputation>;
using HdPh_QuadInfoBuilderComputationSharedPtr = std::shared_ptr<class HdPh_QuadInfoBuilderComputation>;

using HdPh_MeshTopologySharedPtr = std::shared_ptr<class HdPh_MeshTopology>;

/// \class HdPh_MeshTopology
///
/// Phoenix implementation for mesh topology.
///
class HdPh_MeshTopology final : public HdMeshTopology
{
 public:
  /// Specifies how subdivision mesh topology is refined.
  enum RefineMode
  {
    RefineModeUniform = 0,
    RefineModePatches
  };

  /// Specifies type of interpolation to use in refinement
  enum Interpolation
  {
    INTERPOLATE_VERTEX,
    INTERPOLATE_VARYING,
    INTERPOLATE_FACEVARYING,
  };

  static HdPh_MeshTopologySharedPtr New(const HdMeshTopology &src,
                                        int refineLevel,
                                        RefineMode refineMode = RefineModeUniform);

  virtual ~HdPh_MeshTopology();

  /// Equality check between two mesh topologies.
  bool operator==(HdPh_MeshTopology const &other) const;

  /// \name Triangulation
  /// @{

  /// Returns the triangle indices (for drawing) buffer source computation.
  HdBufferSourceSharedPtr GetTriangleIndexBuilderComputation(SdfPath const &id);

  /// Returns the CPU face-varying triangulate computation
  HdBufferSourceSharedPtr GetTriangulateFaceVaryingComputation(HdBufferSourceSharedPtr const &source,
                                                               SdfPath const &id);

  /// @}

  ///
  /// \name Quadrangulation
  /// @{

  /// Returns the quadinfo computation for the use of primvar
  /// quadrangulation.
  /// If gpu is true, the quadrangulate table will be transferred to GPU
  /// via the resource registry.
  HdPh_QuadInfoBuilderComputationSharedPtr GetQuadInfoBuilderComputation(
    bool gpu,
    SdfPath const &id,
    HdPhResourceRegistry *resourceRegistry = nullptr);

  /// Returns the quad indices (for drawing) buffer source computation.
  HdBufferSourceSharedPtr GetQuadIndexBuilderComputation(SdfPath const &id);

  /// Returns the CPU quadrangulated buffer source.
  HdBufferSourceSharedPtr GetQuadrangulateComputation(HdBufferSourceSharedPtr const &source,
                                                      SdfPath const &id);

  /// Returns the GPU quadrangulate computation.
  HdComputationSharedPtr GetQuadrangulateComputationGPU(TfToken const &name,
                                                        HdType dataType,
                                                        SdfPath const &id);

  /// Returns the CPU face-varying quadrangulate computation
  HdBufferSourceSharedPtr GetQuadrangulateFaceVaryingComputation(HdBufferSourceSharedPtr const &source,
                                                                 SdfPath const &id);

  /// Returns the quadrangulation table range on GPU
  HdBufferArrayRangeSharedPtr const &GetQuadrangulateTableRange() const
  {
    return _quadrangulateTableRange;
  }

  /// Clears the quadrangulation table range
  void ClearQuadrangulateTableRange()
  {
    _quadrangulateTableRange.reset();
  }

  /// Sets the quadrangulation struct. HdMeshTopology takes an
  /// ownership of quadInfo (caller shouldn't free)
  void SetQuadInfo(HdQuadInfo const *quadInfo);

  /// Returns the quadrangulation struct.
  HdQuadInfo const *GetQuadInfo() const
  {
    return _quadInfo;
  }

  /// @}

  ///
  /// \name Points
  /// @{

  /// Returns the point indices buffer source computation.
  HdBufferSourceSharedPtr GetPointsIndexBuilderComputation();

  /// @}

  ///
  /// \name Subdivision
  /// @{

  /// Returns the subdivision struct.
  HdPh_Subdivision const *GetSubdivision() const
  {
    return _subdivision;
  }

  /// Returns the subdivision struct (non-const).
  HdPh_Subdivision *GetSubdivision()
  {
    return _subdivision;
  }

  /// Returns true if the subdivision on this mesh produces
  /// triangles (otherwise quads)
  bool RefinesToTriangles() const;

  /// Returns true if the subdivision of this mesh produces bspline patches
  bool RefinesToBSplinePatches() const;

  /// Returns true if the subdivision of this mesh produces box spline
  /// triangle patches
  bool RefinesToBoxSplineTrianglePatches() const;

  /// Returns the subdivision topology computation. It computes
  /// far mesh and produces refined quad-indices buffer.
  HdBufferSourceSharedPtr GetOsdTopologyComputation(SdfPath const &debugId);

  /// Returns the refined indices builder computation.
  /// This just returns index and primitive buffer, and should be preceded by
  /// topology computation.
  HdBufferSourceSharedPtr GetOsdIndexBuilderComputation();

  /// Returns the refined face-varying indices builder computation.
  /// This just returns the index and patch param buffer for a face-varying
  /// channel, and should be preceded by topology computation.
  HdBufferSourceSharedPtr GetOsdFvarIndexBuilderComputation(int channel);

  /// Returns the subdivision primvar refine computation on CPU.
  HdBufferSourceSharedPtr GetOsdRefineComputation(HdBufferSourceSharedPtr const &source,
                                                  Interpolation interpolation,
                                                  int fvarChannel = 0);

  /// Returns the subdivision primvar refine computation on GPU.
  HdComputationSharedPtr GetOsdRefineComputationGPU(TfToken const &name,
                                                    HdType dataType,
                                                    Interpolation interpolation,
                                                    int fvarChannel = 0);

  /// Sets the face-varying topologies.
  void SetFvarTopologies(std::vector<VtIntArray> const &fvarTopologies)
  {
    _fvarTopologies = fvarTopologies;
  }

  /// Returns the face-varying topologies.
  std::vector<VtIntArray> const &GetFvarTopologies()
  {
    return _fvarTopologies;
  }

  /// @}

 private:
  // quadrangulation info on CPU
  HdQuadInfo const *_quadInfo;

  // quadrangulation info on GPU
  HdBufferArrayRangeSharedPtr _quadrangulateTableRange;

  HdPh_QuadInfoBuilderComputationPtr _quadInfoBuilder;

  // OpenSubdiv
  RefineMode _refineMode;
  HdPh_Subdivision *_subdivision;
  HdBufferSourceWeakPtr _osdTopologyBuilder;

  std::vector<VtIntArray> _fvarTopologies;

  // Must be created through factory
  explicit HdPh_MeshTopology(const HdMeshTopology &src, int refineLevel, RefineMode refineMode);

  // No default construction or copying.
  HdPh_MeshTopology() = delete;
  HdPh_MeshTopology(const HdPh_MeshTopology &) = delete;
  HdPh_MeshTopology &operator=(const HdPh_MeshTopology &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_MESH_TOPOLOGY_H
