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
#ifndef WABI_IMAGING_HD_PH_EXT_COMPUTATION_H
#define WABI_IMAGING_HD_PH_EXT_COMPUTATION_H

#include "wabi/base/vt/value.h"
#include "wabi/imaging/hd/extComputation.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

class HdSceneDelegate;
using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;

/// \class HdPhExtComputation
///
/// Specialization of HdExtComputation which manages inputs as GPU resources.
///
class HdPhExtComputation : public HdExtComputation {
 public:
  /// Construct a new ExtComputation identified by id.
  HDPH_API
  HdPhExtComputation(SdfPath const &id);

  HDPH_API
  ~HdPhExtComputation() override;

  HDPH_API
  void Sync(HdSceneDelegate *sceneDelegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits) override;

  HDPH_API
  void Finalize(HdRenderParam *renderParam) override;

  HDPH_API
  HdBufferArrayRangeSharedPtr const &GetInputRange() const
  {
    return _inputRange;
  }

 private:
  // No default construction or copying
  HdPhExtComputation()                           = delete;
  HdPhExtComputation(const HdPhExtComputation &) = delete;
  HdPhExtComputation &operator=(const HdPhExtComputation &) = delete;

  HdBufferArrayRangeSharedPtr _inputRange;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_EXT_COMPUTATION_H
