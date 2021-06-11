//
// Copyright 2018 Pixar
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
#ifndef WABI_IMAGING_HD_SELECTION_H
#define WABI_IMAGING_HD_SELECTION_H

#include "wabi/base/gf/vec4f.h"
#include "wabi/base/vt/array.h"
#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

#include <memory>
#include <unordered_map>
#include <vector>

WABI_NAMESPACE_BEGIN

using HdSelectionSharedPtr = std::shared_ptr<class HdSelection>;

/// \class HdSelection
///
/// HdSelection holds a collection of selected items per selection mode.
/// The items may be rprims, instances of an rprim and subprimitives of an
/// rprim, such as elements (faces for meshes, individual curves for basis
/// curves), edges & points. Each item is referred to by the render index path.
///
/// It current supports active and rollover selection modes, and may be
/// inherited for customization.
///
class HdSelection {
 public:
  /// Selection modes allow differentiation in selection highlight behavior.
  enum HighlightMode {
    HighlightModeSelect = 0,  // Active selection
    HighlightModeLocate,      // Rollover selection

    HighlightModeCount
  };

  HdSelection() = default;

  HD_API
  virtual ~HdSelection();

  /// ------------------------ Population API --------------------------------
  HD_API
  void AddRprim(HighlightMode const &mode, SdfPath const &renderIndexPath);

  HD_API
  void AddInstance(HighlightMode const &mode,
                   SdfPath const &renderIndexPath,
                   VtIntArray const &instanceIndex = VtIntArray());

  HD_API
  void AddElements(HighlightMode const &mode,
                   SdfPath const &renderIndexPath,
                   VtIntArray const &elementIndices);

  HD_API
  void AddEdges(HighlightMode const &mode,
                SdfPath const &renderIndexPath,
                VtIntArray const &edgeIndices);

  HD_API
  void AddPoints(HighlightMode const &mode,
                 SdfPath const &renderIndexPath,
                 VtIntArray const &pointIndices);

  // Special handling for points: we allow a set of selected point indices to
  // also specify a color to use for highlighting.
  HD_API
  void AddPoints(HighlightMode const &mode,
                 SdfPath const &renderIndexPath,
                 VtIntArray const &pointIndices,
                 GfVec4f const &pointColor);

  // XXX: Ideally, this should be per instance, if we want to support
  // selection of subprims (faces/edges/points) per instance of an rprim.
  // By making this per rprim, all selected instances of the rprim will share
  // the same subprim highlighting.
  struct PrimSelectionState {
    PrimSelectionState() : fullySelected(false)
    {}

    bool fullySelected;
    // Use a vector of VtIntArray to avoid copying the indices data.
    // This way, we support multiple Add<Subprim> operations without
    // having to consolidate the indices each time.
    std::vector<VtIntArray> instanceIndices;
    std::vector<VtIntArray> elementIndices;
    std::vector<VtIntArray> edgeIndices;
    std::vector<VtIntArray> pointIndices;
    std::vector<int> pointColorIndices;
  };

  /// ---------------------------- Query API ---------------------------------

  // Returns a pointer to the selection state for the rprim path if selected.
  // Returns nullptr otherwise.
  HD_API
  PrimSelectionState const *GetPrimSelectionState(HighlightMode const &mode,
                                                  SdfPath const &renderIndexPath) const;

  // Returns the selected rprim render index paths for all the selection
  // modes. The vector returned may contain duplicates.
  HD_API
  SdfPathVector GetAllSelectedPrimPaths() const;

  // Returns the selected rprim render index paths for the given mode.
  HD_API
  SdfPathVector GetSelectedPrimPaths(HighlightMode const &mode) const;

  HD_API
  std::vector<GfVec4f> const &GetSelectedPointColors() const;

  // Returns true if nothing is selected.
  HD_API
  bool IsEmpty() const;

 private:
  void _AddPoints(HighlightMode const &mode,
                  SdfPath const &renderIndexPath,
                  VtIntArray const &pointIndices,
                  int pointColorIndex);

  void _GetSelectionPrimPathsForMode(HighlightMode const &mode, SdfPathVector *paths) const;

 protected:
  typedef std::unordered_map<SdfPath, PrimSelectionState, SdfPath::Hash> _PrimSelectionStateMap;
  // Keep track of selection per selection mode.
  _PrimSelectionStateMap _selMap[HighlightModeCount];

  // Track all colors used for point selection highlighting.
  std::vector<GfVec4f> _selectedPointColors;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_SELECTION_H
