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
#ifndef WABI_IMAGING_HD_RESOURCE_H
#define WABI_IMAGING_HD_RESOURCE_H

#include "wabi/wabi.h"
#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/base/tf/token.h"

#include <cstddef>
#include <memory>

WABI_NAMESPACE_BEGIN


using HdResourceSharedPtr = std::shared_ptr<class HdResource>;

/// \class HdResource
///
/// Base class for all GPU resource objects.
///
class HdResource
{
 public:

  HD_API
  HdResource(TfToken const &role);
  HD_API
  virtual ~HdResource();

  /// Returns the role of the GPU data in this resource.
  TfToken const &GetRole() const
  {
    return _role;
  }

  /// Returns the size of the resource allocated in the GPU
  HD_API
  size_t GetSize() const
  {
    return _size;
  }

 protected:

  /// Stores the size of the resource allocated in the GPU
  HD_API
  void SetSize(size_t size);

 private:

  // Don't allow copies.
  HdResource(const HdResource &) = delete;
  HdResource &operator=(const HdResource &) = delete;


  const TfToken _role;
  size_t _size;
};


WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_RESOURCE_H
