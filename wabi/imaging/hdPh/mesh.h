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
#ifndef WABI_IMAGING_HD_PH_MESH_H
#define WABI_IMAGING_HD_PH_MESH_H

#include "wabi/imaging/hd/changeTracker.h"
#include "wabi/imaging/hd/drawingCoord.h"
#include "wabi/imaging/hd/mesh.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/vt/array.h"
#include "wabi/usd/sdf/path.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdPhDrawItem;
class HdSceneDelegate;

using Hd_VertexAdjacencySharedPtr = std::shared_ptr<class Hd_VertexAdjacency>;
using HdBufferSourceSharedPtr     = std::shared_ptr<class HdBufferSource>;
using HdPh_MeshTopologySharedPtr  = std::shared_ptr<class HdPh_MeshTopology>;

using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

/// A subdivision surface or poly-mesh object.
///
class HdPhMesh final : public HdMesh {
 public:
  HF_MALLOC_TAG_NEW("new HdPhMesh");

  HDPH_API
  HdPhMesh(SdfPath const &id);

  HDPH_API
  ~HdPhMesh() override;

  HDPH_API
  void Sync(HdSceneDelegate *delegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits,
            TfToken const &reprToken) override;

  HDPH_API
  void Finalize(HdRenderParam *renderParam) override;

  HDPH_API
  HdDirtyBits GetInitialDirtyBitsMask() const override;

  /// Topology (member) getter
  HDPH_API
  HdMeshTopologySharedPtr GetTopology() const override;

  /// Returns whether packed (10_10_10 bits) normals to be used
  HDPH_API
  static bool IsEnabledPackedNormals();

 protected:
  HDPH_API
  void _InitRepr(TfToken const &reprToken, HdDirtyBits *dirtyBits) override;

  HDPH_API
  HdDirtyBits _PropagateDirtyBits(HdDirtyBits bits) const override;

  void _UpdateRepr(HdSceneDelegate *sceneDelegate,
                   HdRenderParam *renderParam,
                   TfToken const &reprToken,
                   HdDirtyBits *dirtyBitsState);

  HdBufferArrayRangeSharedPtr _GetSharedPrimvarRange(
      uint64_t primvarId,
      HdBufferSpecVector const &updatedOrAddedSpecs,
      HdBufferSpecVector const &removedSpecs,
      HdBufferArrayRangeSharedPtr const &curRange,
      bool *isFirstInstance,
      HdPhResourceRegistrySharedPtr const &resourceRegistry) const;

  bool _UseQuadIndices(const HdRenderIndex &renderIndex,
                       HdPh_MeshTopologySharedPtr const &topology) const;

  bool _UseLimitRefinement(const HdRenderIndex &renderIndex) const;

  bool _UseSmoothNormals(HdPh_MeshTopologySharedPtr const &topology) const;

  bool _UseFlatNormals(const HdMeshReprDesc &desc) const;

  void _UpdateDrawItem(HdSceneDelegate *sceneDelegate,
                       HdRenderParam *renderParam,
                       HdPhDrawItem *drawItem,
                       HdDirtyBits *dirtyBits,
                       const HdMeshReprDesc &desc,
                       bool requireSmoothNormals,
                       bool requireFlatNormals);

  void _UpdateDrawItemGeometricShader(HdSceneDelegate *sceneDelegate,
                                      HdRenderParam *renderParam,
                                      HdPhDrawItem *drawItem,
                                      const HdMeshReprDesc &desc);

  void _UpdateShadersForAllReprs(HdSceneDelegate *sceneDelegate,
                                 HdRenderParam *renderParam,
                                 bool updateMaterialShader,
                                 bool updateGeometricShader);

  void _PopulateTopology(HdSceneDelegate *sceneDelegate,
                         HdRenderParam *renderParam,
                         HdPhDrawItem *drawItem,
                         HdDirtyBits *dirtyBits,
                         const HdMeshReprDesc &desc);

  void _PopulateAdjacency(HdPhResourceRegistrySharedPtr const &resourceRegistry);

  void _PopulateVertexPrimvars(HdSceneDelegate *sceneDelegate,
                               HdRenderParam *renderParam,
                               HdPhDrawItem *drawItem,
                               HdDirtyBits *dirtyBits,
                               bool requireSmoothNormals);

  void _PopulateFaceVaryingPrimvars(HdSceneDelegate *sceneDelegate,
                                    HdRenderParam *renderParam,
                                    HdPhDrawItem *drawItem,
                                    HdDirtyBits *dirtyBits,
                                    const HdMeshReprDesc &desc);

  void _PopulateElementPrimvars(HdSceneDelegate *sceneDelegate,
                                HdRenderParam *renderParam,
                                HdPhDrawItem *drawItem,
                                HdDirtyBits *dirtyBits,
                                bool requireFlatNormals);

  int _GetRefineLevelForDesc(const HdMeshReprDesc &desc) const;

 private:
  enum DrawingCoord {
    HullTopology = HdDrawingCoord::CustomSlotsBegin,
    PointsTopology,
    InstancePrimvar  // has to be at the very end
  };

  enum DirtyBits : HdDirtyBits {
    DirtySmoothNormals = HdChangeTracker::CustomBitsBegin,
    DirtyFlatNormals   = (DirtySmoothNormals << 1),
    DirtyIndices       = (DirtyFlatNormals << 1),
    DirtyHullIndices   = (DirtyIndices << 1),
    DirtyPointsIndices = (DirtyHullIndices << 1)
  };

  HdPh_MeshTopologySharedPtr _topology;
  Hd_VertexAdjacencySharedPtr _vertexAdjacency;

  HdTopology::ID _topologyId;
  HdTopology::ID _vertexPrimvarId;
  HdDirtyBits _customDirtyBitsInUse;

  HdType _pointsDataType;
  HdInterpolation _sceneNormalsInterpolation;
  HdCullStyle _cullStyle;
  bool _hasMirroredTransform : 1;
  bool _doubleSided          : 1;
  bool _flatShadingEnabled   : 1;
  bool _displacementEnabled  : 1;
  bool _limitNormals         : 1;
  bool _sceneNormals         : 1;
  bool _hasVaryingTopology   : 1;  // The prim's topology has changed since
                                   // the prim was created
  bool _displayOpacity                : 1;
  bool _occludedSelectionShowsThrough : 1;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_MESH_H
