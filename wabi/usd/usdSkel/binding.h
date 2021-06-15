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
#ifndef WABI_USD_USD_SKEL_BINDING_H
#define WABI_USD_USD_SKEL_BINDING_H

/// \file usdSkel/binding.h

#include "wabi/usd/usdSkel/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/usdSkel/skeleton.h"
#include "wabi/usd/usdSkel/skinningQuery.h"

WABI_NAMESPACE_BEGIN

/// \class UsdSkelBinding
///
/// Helper object that describes the binding of a skeleton to a set of
/// skinnable objects. The set of skinnable objects is given as
/// UsdSkelSkinningQuery prims, which can be used both to identify the
/// skinned prim as well compute skinning properties of the prim.
class UsdSkelBinding {
 public:
  UsdSkelBinding()
  {}

  UsdSkelBinding(const UsdSkelSkeleton &skel, const VtArray<UsdSkelSkinningQuery> &skinningQueries)
    : _skel(skel),
      _skinningQueries(skinningQueries)
  {}

  /// Returns the bound skeleton.
  const UsdSkelSkeleton &GetSkeleton() const
  {
    return _skel;
  }

  /// Returns the set skinning targets.
  const VtArray<UsdSkelSkinningQuery> &GetSkinningTargets() const
  {
    return _skinningQueries;
  }

 private:
  UsdSkelSkeleton _skel;
  VtArray<UsdSkelSkinningQuery> _skinningQueries;
};

WABI_NAMESPACE_END

#endif  // USDSKEL_SKINNING_MAP
