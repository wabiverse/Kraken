//
// Copyright 2017 Pixar
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
#ifndef WABI_IMAGING_HD_ST_EXT_COMP_GPU_COMPUTATION_BUFFER_SOURCE_H
#define WABI_IMAGING_HD_ST_EXT_COMP_GPU_COMPUTATION_BUFFER_SOURCE_H

#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/extCompGpuComputationResource.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/path.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/vt/value.h"

WABI_NAMESPACE_BEGIN

/// \class HdPhExtCompGpuComputationBufferSource
///
/// A Buffer Source that represents input processing for a GPU implementation
/// of an ExtComputation.
///
/// The source is responsible for resolving the inputs that are directed
/// at the computation itself rather than coming from the HdRprim the
/// computation is attached to. All the inputs bound through this source are
/// reflected in the compute kernel as read-only accessors accessible
/// via HdGet_<name>.
///
/// A GLSL example kernel using an input from a primvar computation would be:
/// \code
/// void compute(int index) {
///   // assumes the input buffer is named 'sourcePoints'
///   vec3 point = HdGet_sourcePoints(index);
///   // 'points' is an rprim primvar (HdToken->points)
///   HdSet_points(index, point * 2.0);
/// }
/// \endcode
///
/// In the example above a buffer source was given a input source named
/// 'sourcePoints' of type vec3. HdPhCodeGen generated the corresponding
/// accessor allowing the kernel to use it.
/// \see HdPhExtCompGpuComputation
class HdPhExtCompGpuComputationBufferSource final : public HdNullBufferSource {
 public:
  /// Constructs a GPU ExtComputation buffer source.
  /// \param[in] inputs the vector of HdBufferSource that are inputs to the
  /// computation only. This should not include inputs that are already
  /// assigned to an HdRprim that the computation is executing on.
  /// \param[inout] resource the GPU resident resource that will contain the data
  /// in the inputs after Resolve is called.
  /// \see HdExtComputation
  HdPhExtCompGpuComputationBufferSource(
      HdBufferSourceSharedPtrVector const &inputs,
      HdPhExtCompGpuComputationResourceSharedPtr const &resource);

  HDPH_API
  virtual ~HdPhExtCompGpuComputationBufferSource() = default;

  /// Resolves the source and populates the HdPhExtCompGpuComputationResource.
  /// This in effect commits resources to the GPU for use in one or more
  /// computations.
  /// As with all other sources this is called by the HdResourceRegistry
  /// during the Resolve phase of HdResourceRegistry::Commit
  HDPH_API
  virtual bool Resolve() override;

  /// Returns the vector of HdBufferSource inputs that this source intends
  /// to commit to GPU.
  virtual HdBufferSourceSharedPtrVector const &GetInputs() const
  {
    return _inputs;
  }

 protected:
  virtual bool _CheckValid() const override;

 private:
  HdBufferSourceSharedPtrVector _inputs;
  HdPhExtCompGpuComputationResourceSharedPtr _resource;

  HdPhExtCompGpuComputationBufferSource()                                              = delete;
  HdPhExtCompGpuComputationBufferSource(const HdPhExtCompGpuComputationBufferSource &) = delete;
  HdPhExtCompGpuComputationBufferSource &operator=(const HdPhExtCompGpuComputationBufferSource &) =
      delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_EXT_COMP_GPU_COMPUTATION_BUFFER_SOURCE_H
