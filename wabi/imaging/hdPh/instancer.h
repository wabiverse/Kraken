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
#ifndef WABI_IMAGING_HD_ST_INSTANCER_H
#define WABI_IMAGING_HD_ST_INSTANCER_H

#include "wabi/base/tf/hashmap.h"
#include "wabi/base/vt/array.h"
#include "wabi/imaging/hd/changeTracker.h"
#include "wabi/imaging/hd/instancer.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class HdRprim;
class HdPhDrawItem;
struct HdRprimSharedData;

using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;

/// \class HdPhInstancer
///
/// HdPh implements instancing by drawing each proto multiple times with
/// a single draw call.  Application of instance primvars (like transforms)
/// is done in shaders. Instance transforms in particular are computed in
/// ApplyInstanceTransform in instancing.glslfx.
///
/// If this instancer is nested, instance indices will be computed
/// recursively by ascending the hierarchy. HdPhInstancer computes a flattened
/// index structure for each prototype by taking the cartesian product of the
/// instance indices at each level.
///
/// For example:
///   - InstancerA draws instances [ProtoX, InstancerB, ProtoX, InstancerB]
///   - InstancerB draws instances [ProtoY, ProtoZ, ProtoY]
/// The flattened index for Proto Y is:
/// [0, 0, 1]; [1, 0, 3]; [2, 2, 1]; [3, 2, 3];
/// where the first tuple element is the position in the flattened index;
/// the second tuple element is the position in Instancer B;
/// and the last tuple element is the position in Instancer A.
///
/// The flattened index gives the number of times the proto is drawn, and the
/// index tuple can be passed to the shader so that each instance can look up
/// its instance primvars in the bound primvar arrays.

class HdPhInstancer : public HdInstancer
{
 public:
  /// Constructor.
  HDPH_API
  HdPhInstancer(HdSceneDelegate *delegate, SdfPath const &id);

  // Updates the instance primvar buffers.
  // XXX: Note, this is currently called from rprimUtils instead of the
  // render index sync phase, so it needs to take a mutex.
  HDPH_API
  void Sync(HdSceneDelegate *sceneDelegate, HdRenderParam *renderParam, HdDirtyBits *dirtyBits) override;

  HdBufferArrayRangeSharedPtr GetInstancePrimvarRange() const
  {
    return _instancePrimvarRange;
  }

  /// Populates the instance index indirection buffer for \p prototypeId and
  /// returns a flat array of instance index tuples.
  HDPH_API
  VtIntArray GetInstanceIndices(SdfPath const &prototypeId);

 protected:
  HDPH_API
  void _GetInstanceIndices(SdfPath const &prototypeId, std::vector<VtIntArray> *instanceIndicesArray);

  HDPH_API
  void _SyncPrimvars(HdSceneDelegate *sceneDelegate, HdDirtyBits *dirtyBits);

 private:
  // # of entries in an instance primvar.  This should be consistent between
  // all primvars, and also consistent with the instance indices (meaning
  // no instance index is out-of-range).
  size_t _instancePrimvarNumElements;

  // The BAR of the instance primvars for this instancer.
  // (Note: instance indices are computed per prototype and the rprim owns
  // the bar).
  HdBufferArrayRangeSharedPtr _instancePrimvarRange;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_INSTANCER_H
