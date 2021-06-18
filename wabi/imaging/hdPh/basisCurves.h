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
#ifndef WABI_IMAGING_HD_ST_BASIS_CURVES_H
#define WABI_IMAGING_HD_ST_BASIS_CURVES_H

#include "wabi/imaging/hd/basisCurves.h"
#include "wabi/imaging/hd/drawingCoord.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/vt/array.h"
#include "wabi/usd/sdf/path.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdPhDrawItem;
using HdPh_BasisCurvesTopologySharedPtr = std::shared_ptr<class HdPh_BasisCurvesTopology>;

/// \class HdPhBasisCurves
///
/// A collection of curves using a particular basis.
///
/// Render mode is dependent on both the HdBasisCurvesGeomStyle, refinement
/// level, and the authored primvars.
///
/// If style is set to HdBasisCurvesGeomStyleWire, the curves will always draw
/// as infinitely thin wires.  Cubic curves will be refined if complexity is
/// above 0, otherwise they draw the unrefined control points. (This may
/// provide a misleading representation for Catmull-Rom and Bspline curves.)
///
/// If style is set to HdBasisCurvesGeomStylePatch, the curves will draw as
/// patches ONLY if refinement level is above 0.  Otherwise, they draw
/// as the unrefined control points (see notes on HdBasisCurvesGeomStyleWire).
///
/// Curves rendered as patches may be rendered as ribbons or halftubes.
/// Curves with primvar authored normals will always render as ribbons.
/// Curves without primvar authored normals are assumed to be round and may be
/// rendered in one of three styles:
///   * if complexity is 1, a camera facing normal is used
///   * if complexity is 2, a fake "bumped" round normal is used
///   * if complexity is 3 or above, the patch is displaced into a half tube
/// We plan for future checkins will remove the need for the camera facing normal
/// mode, using the fake "bumped" round normal instead.
class HdPhBasisCurves final : public HdBasisCurves
{
 public:
  HF_MALLOC_TAG_NEW("new HdPhBasisCurves");

  HDPH_API
  HdPhBasisCurves(SdfPath const &id);

  HDPH_API
  ~HdPhBasisCurves() override;

  HDPH_API
  void Sync(HdSceneDelegate *delegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits,
            TfToken const &reprToken) override;

  HDPH_API
  void Finalize(HdRenderParam *renderParam) override;

  HDPH_API
  HdDirtyBits GetInitialDirtyBitsMask() const override;

 protected:
  HDPH_API
  void _InitRepr(TfToken const &reprToken, HdDirtyBits *dirtyBits) override;

  HDPH_API
  HdDirtyBits _PropagateDirtyBits(HdDirtyBits bits) const override;

  void _UpdateRepr(HdSceneDelegate *sceneDelegate,
                   HdRenderParam *renderParam,
                   TfToken const &reprToken,
                   HdDirtyBits *dirtyBitsState);

  void _PopulateTopology(HdSceneDelegate *sceneDelegate,
                         HdRenderParam *renderParam,
                         HdPhDrawItem *drawItem,
                         HdDirtyBits *dirtyBits,
                         const HdBasisCurvesReprDesc &desc);

  void _PopulateVertexPrimvars(HdSceneDelegate *sceneDelegate,
                               HdRenderParam *renderParam,
                               HdPhDrawItem *drawItem,
                               HdDirtyBits *dirtyBits);

  void _PopulateVaryingPrimvars(HdSceneDelegate *sceneDelegate,
                                HdRenderParam *renderParam,
                                HdPhDrawItem *drawItem,
                                HdDirtyBits *dirtyBits);

  void _PopulateElementPrimvars(HdSceneDelegate *sceneDelegate,
                                HdRenderParam *renderParam,
                                HdPhDrawItem *drawItem,
                                HdDirtyBits *dirtyBits);

 private:
  enum DrawingCoord
  {
    HullTopology = HdDrawingCoord::CustomSlotsBegin,
    PointsTopology,
    InstancePrimvar  // has to be at the very end
  };

  enum DirtyBits : HdDirtyBits
  {
    DirtyIndices = HdChangeTracker::CustomBitsBegin,
    DirtyHullIndices = (DirtyIndices << 1),
    DirtyPointsIndices = (DirtyHullIndices << 1)
  };

  // When processing primvars, these will get set to if we determine that
  // we should do cubic basis interpolation on the normals and widths.
  // NOTE: I worry that it may be possible for these to get out of sync.
  // The right long term fix is likely to maintain proper separation between
  // varying and vertex primvars throughout the HdPh rendering pipeline.
  bool _basisWidthInterpolation = false;
  bool _basisNormalInterpolation = false;

  bool _SupportsRefinement(int refineLevel);
  bool _SupportsUserWidths(HdPhDrawItem *drawItem);
  bool _SupportsUserNormals(HdPhDrawItem *drawItem);

  void _UpdateDrawItem(HdSceneDelegate *sceneDelegate,
                       HdRenderParam *renderParam,
                       HdPhDrawItem *drawItem,
                       HdDirtyBits *dirtyBits,
                       const HdBasisCurvesReprDesc &desc);

  void _UpdateDrawItemGeometricShader(HdSceneDelegate *sceneDelegate,
                                      HdRenderParam *renderParam,
                                      HdPhDrawItem *drawItem,
                                      const HdBasisCurvesReprDesc &desc);

  void _UpdateShadersForAllReprs(HdSceneDelegate *sceneDelegate,
                                 HdRenderParam *renderParam,
                                 bool updateMaterialShader,
                                 bool updateGeometricShader);

  HdPh_BasisCurvesTopologySharedPtr _topology;
  HdTopology::ID _topologyId;
  HdDirtyBits _customDirtyBitsInUse;
  int _refineLevel;  // XXX: could be moved into HdBasisCurveTopology.
  bool _displayOpacity                : 1;
  bool _occludedSelectionShowsThrough : 1;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_BASIS_CURVES_H
