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
#ifndef WABI_IMAGING_HD_ST_BUFFER_ARRAY_RANGE_H
#define WABI_IMAGING_HD_ST_BUFFER_ARRAY_RANGE_H

#include "wabi/base/tf/token.h"
#include "wabi/imaging/hd/bufferArrayRange.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdBufferArrayGL;
class HdPhResourceRegistry;

using HdPhBufferArrayRangeSharedPtr = std::shared_ptr<class HdPhBufferArrayRange>;

class HdPhBufferResource;

using HdPhBufferResourceSharedPtr = std::shared_ptr<class HdPhBufferResource>;
using HdPhBufferResourceNamedList = std::vector<std::pair<TfToken, HdPhBufferResourceSharedPtr>>;

/// \class HdPhBufferArrayRange
///
/// Interface class for representing range (subset) locator of HdBufferArray.
///
/// Each memory management strategy defines a specialized range class which is
/// inherited of this interface so that client (drawItem) can be agnostic about
/// the implementation detail of aggregation.
///
class HdPhBufferArrayRange : public HdBufferArrayRange {
 public:
  HdPhBufferArrayRange(HdPhResourceRegistry *resourceRegistry);

  /// Destructor (do nothing).
  /// The specialized range class may want to do something for garbage
  /// collection in its destructor. However, be careful not do any
  /// substantial work here (obviously including any kind of GL calls),
  /// since the destructor gets called frequently on various contexts.
  HDPH_API
  virtual ~HdPhBufferArrayRange();

  /// Returns the GPU resource. If the buffer array contains more than one
  /// resource, this method raises a coding error.
  virtual HdPhBufferResourceSharedPtr GetResource() const = 0;

  /// Returns the named GPU resource.
  virtual HdPhBufferResourceSharedPtr GetResource(TfToken const &name) = 0;

  /// Returns the list of all named GPU resources for this bufferArrayRange.
  virtual HdPhBufferResourceNamedList const &GetResources() const = 0;

  /// Sets the bufferSpecs for all resources.
  HDPH_API
  virtual void GetBufferSpecs(HdBufferSpecVector *bufferSpecs) const override;

 protected:
  HdPhResourceRegistry *GetResourceRegistry();

 private:
  HdPhResourceRegistry *_resourceRegistry;
};

HDPH_API
std::ostream &operator<<(std::ostream &out, const HdPhBufferArrayRange &self);

/// \class HdPhBufferArrayRangeContainer
///
/// A resizable container of HdBufferArrayRanges.
///
class HdPhBufferArrayRangeContainer {
 public:
  /// Constructor
  HdPhBufferArrayRangeContainer(int size) : _ranges(size)
  {}

  /// Set \p range into the container at \p index.
  /// If the size of container is smaller than index, resize it.
  HDPH_API
  void Set(int index, HdPhBufferArrayRangeSharedPtr const &range);

  /// Returns the bar at \p index. returns null if either the index
  // is out of range or not yet set.
  HDPH_API
  HdPhBufferArrayRangeSharedPtr const &Get(int index) const;

 private:
  std::vector<HdPhBufferArrayRangeSharedPtr> _ranges;
};

WABI_NAMESPACE_END

#endif  // HD_BUFFER_ARRAY_RANGE_GL_H
