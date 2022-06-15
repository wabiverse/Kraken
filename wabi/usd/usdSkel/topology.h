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
#ifndef WABI_USD_USD_SKEL_TOPOLOGY_H
#define WABI_USD_USD_SKEL_TOPOLOGY_H

/// \file usdSkel/topology.h

#include "wabi/usd/usdSkel/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/span.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

/// \class UsdSkelTopology
///
/// Object holding information describing skeleton topology.
/// This provides the hierarchical information needed to reason about joint
/// relationships in a manner suitable to computations.
class UsdSkelTopology
{
 public:

  /// Construct an empty topology.
  UsdSkelTopology() = default;

  /// Construct a skel topology from \p paths, an array holding ordered joint
  /// paths as tokens.
  /// Internally, each token must be converted to an SdfPath. If SdfPath
  /// objects are already accessible, it is more efficient to use the
  /// construct taking an SdfPath array.
  USDSKEL_API
  UsdSkelTopology(TfSpan<const TfToken> paths);

  /// Construct a skel topology from \p paths, an array of joint paths.
  USDSKEL_API
  UsdSkelTopology(TfSpan<const SdfPath> paths);

  /// Construct a skel topology from an array of parent indices.
  /// For each joint, this provides the parent index of that
  /// joint, or -1 if none.
  USDSKEL_API
  UsdSkelTopology(const VtIntArray &parentIndices);

  /// Validate the topology.
  /// If validation is unsuccessful, a reason
  /// why will be written to \p reason, if provided.
  USDSKEL_API
  bool Validate(std::string *reason = nullptr) const;

  const VtIntArray &GetParentIndices() const
  {
    return _parentIndices;
  }

  size_t GetNumJoints() const
  {
    return size();
  }

  size_t size() const
  {
    return _parentIndices.size();
  }

  /// Returns the parent joint of the \p index'th joint,
  /// Returns -1 for joints with no parent (roots).
  inline int GetParent(size_t index) const;

  /// Returns true if the \p index'th joint is a root joint.
  bool IsRoot(size_t index) const
  {
    return GetParent(index) < 0;
  }

  bool operator==(const UsdSkelTopology &o) const;

  bool operator!=(const UsdSkelTopology &o) const
  {
    return !(*this == o);
  }

 private:

  VtIntArray _parentIndices;
};

int UsdSkelTopology::GetParent(size_t index) const
{
  TF_DEV_AXIOM(index < _parentIndices.size());
  return _parentIndices[index];
}

WABI_NAMESPACE_END

#endif  // WABI_USD_USD_SKEL_TOPOLOGY_H
