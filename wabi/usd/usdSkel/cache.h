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
#ifndef WABI_USD_USD_SKEL_CACHE_H
#define WABI_USD_USD_SKEL_CACHE_H

/// \file usdSkel/cache.h

#include "wabi/usd/usdSkel/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/primFlags.h"

#include "wabi/usd/usdSkel/animQuery.h"
#include "wabi/usd/usdSkel/binding.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class UsdSkelRoot;
class UsdSkelAnimation;
class UsdSkelSkeleton;
class UsdSkelSkeletonQuery;
class UsdSkelSkinningQuery;

TF_DECLARE_REF_PTRS(UsdSkelBinding);

/// Thread-safe cache for accessing query objects for evaluating skeletal data.
///
/// This provides caching of major structural components, such as skeletal
/// topology. In a streaming context, this cache is intended to persist.
class UsdSkelCache
{
 public:
  USDSKEL_API
  UsdSkelCache();

  USDSKEL_API
  void Clear();

  /// Populate the cache for the skeletal data beneath prim \p root,
  /// as traversed using \p predicate.
  ///
  /// Population resolves inherited skel bindings set using the
  /// UsdSkelBindingAPI, making resolved bindings available through
  /// GetSkinningQuery(), ComputeSkelBdining() and ComputeSkelBindings().
  USDSKEL_API
  bool Populate(const UsdSkelRoot &root, Usd_PrimFlagsPredicate predicate) const;

  /// Get a skel query for computing properties of \p skel.
  ///
  /// This does not require Populate() to be called on the cache.
  USDSKEL_API
  UsdSkelSkeletonQuery GetSkelQuery(const UsdSkelSkeleton &skel) const;

  /// Get an anim query corresponding to \p anim.
  ///
  /// This does not require Populate() to be called on the cache.
  USDSKEL_API
  UsdSkelAnimQuery GetAnimQuery(const UsdSkelAnimation &anim) const;

  /// \overload
  /// \deprecated
  USDSKEL_API
  UsdSkelAnimQuery GetAnimQuery(const UsdPrim &prim) const;

  /// Get a skinning query at \p prim.
  ///
  /// Skinning queries are defined at any skinnable prims (I.e., boundable
  /// prims with fully defined joint influences).
  ///
  /// The caller must first Populate() the cache with the skel root containing
  /// \p prim, with a predicate that will visit \p prim, in order for a
  /// skinning query to be discoverable.
  USDSKEL_API
  UsdSkelSkinningQuery GetSkinningQuery(const UsdPrim &prim) const;

  /// Compute the set of skeleton bindings beneath \p skelRoot,
  /// as discovered through a traversal using \p predicate.
  ///
  /// Skinnable prims are only discoverable by this method if Populate()
  /// has already been called for \p skelRoot, with an equivalent predicate.
  USDSKEL_API
  bool ComputeSkelBindings(const UsdSkelRoot &skelRoot,
                           std::vector<UsdSkelBinding> *bindings,
                           Usd_PrimFlagsPredicate predicate) const;

  /// Compute the bindings corresponding to a single skeleton, bound beneath
  /// \p skelRoot, as discovered through a traversal using \p predicate.
  ///
  /// Skinnable prims are only discoverable by this method if Populate()
  /// has already been called for \p skelRoot, with an equivalent predicate.
  USDSKEL_API
  bool ComputeSkelBinding(const UsdSkelRoot &skelRoot,
                          const UsdSkelSkeleton &skel,
                          UsdSkelBinding *binding,
                          Usd_PrimFlagsPredicate predicate) const;

 private:
  std::shared_ptr<class UsdSkel_CacheImpl> _impl;

  friend class UsdSkelAnimQuery;
  friend class UsdSkelSkeletonQuery;
};

WABI_NAMESPACE_END

#endif  // USDSKEL_EVALCACHE_H
