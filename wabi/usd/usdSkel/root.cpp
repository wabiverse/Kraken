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
#include "wabi/usd/usdSkel/root.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdSkelRoot, TfType::Bases<UsdGeomBoundable>>();

  // Register the usd prim typename as an alias under UsdSchemaBase. This
  // enables one to call
  // TfType::Find<UsdSchemaBase>().FindDerivedByName("SkelRoot")
  // to find TfType<UsdSkelRoot>, which is how IsA queries are
  // answered.
  TfType::AddAlias<UsdSchemaBase, UsdSkelRoot>("SkelRoot");
}

/* virtual */
UsdSkelRoot::~UsdSkelRoot()
{}

/* static */
UsdSkelRoot UsdSkelRoot::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdSkelRoot();
  }
  return UsdSkelRoot(stage->GetPrimAtPath(path));
}

/* static */
UsdSkelRoot UsdSkelRoot::Define(const UsdStagePtr &stage, const SdfPath &path)
{
  static TfToken usdPrimTypeName("SkelRoot");
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdSkelRoot();
  }
  return UsdSkelRoot(stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdSkelRoot::GetSchemaKind() const
{
  return UsdSkelRoot::schemaKind;
}

/* static */
const TfType &UsdSkelRoot::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdSkelRoot>();
  return tfType;
}

/* static */
bool UsdSkelRoot::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdSkelRoot::GetTfType() const
{
  return GetStaticTfType();
}

/*static*/
const TfTokenVector &UsdSkelRoot::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames;
  static TfTokenVector allNames = UsdGeomBoundable::GetSchemaAttributeNames(true);

  if (includeInherited)
    return allNames;
  else
    return localNames;
}

WABI_NAMESPACE_END

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'WABI_NAMESPACE_BEGIN', 'WABI_NAMESPACE_END'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

#include "wabi/usd/usd/primRange.h"
#include "wabi/usd/usdGeom/boundableComputeExtent.h"
#include "wabi/usd/usdGeom/xformCache.h"
#include "wabi/usd/usdSkel/binding.h"
#include "wabi/usd/usdSkel/cache.h"
#include "wabi/usd/usdSkel/skeleton.h"
#include "wabi/usd/usdSkel/skeletonQuery.h"
#include "wabi/usd/usdSkel/skinningQuery.h"
#include "wabi/usd/usdSkel/utils.h"

WABI_NAMESPACE_BEGIN

UsdSkelRoot UsdSkelRoot::Find(const UsdPrim &prim)
{
  for (UsdPrim p = prim; p; p = p.GetParent())
  {
    if (p.IsA<UsdSkelRoot>())
    {
      return UsdSkelRoot(p);
    }
  }
  return UsdSkelRoot();
}

/// Plugin extent method.
static bool _ComputeExtent(const UsdGeomBoundable &boundable,
                           const UsdTimeCode &time,
                           const GfMatrix4d *transform,
                           VtVec3fArray *extent)
{
  const UsdSkelRoot skelRoot(boundable);
  if (!TF_VERIFY(skelRoot))
  {
    return false;
  }

  UsdSkelCache skelCache;
  skelCache.Populate(skelRoot, UsdTraverseInstanceProxies());

  std::vector<UsdSkelBinding> bindings;
  if (!skelCache.ComputeSkelBindings(skelRoot, &bindings, UsdTraverseInstanceProxies()) ||
      bindings.size() == 0)
  {

    // XXX: The extent of a SkelRoot is intended to bound the set of
    // skinnable prims only. If we have no bindings, then there are no
    // skinnable prims to bound, and the case can be treated as a failed
    // extent computation.
    // We could potentially look for the set of skeletons bound beneath
    // the SkelRoot and compute the union of their extents, but since
    // Skeleton prims are themselves boundable, this seems redundant.
    return false;
  }

  UsdGeomXformCache xfCache;

  GfRange3d bbox;
  VtVec3fArray skelExtent;

  for (const UsdSkelBinding &binding : bindings)
  {

    UsdSkelSkeletonQuery skelQuery = skelCache.GetSkelQuery(binding.GetSkeleton());
    if (!TF_VERIFY(skelQuery))
      return false;

    // Compute skel-space joint transforms.
    // The extent for this skel is based on the pivots of all bones,
    // with some additional padding.
    VtMatrix4dArray skelXforms;
    if (!skelQuery.ComputeJointSkelTransforms(&skelXforms, time))
      continue;

    // Pre-compute a constant padding metric across all prims
    // skinned by this skeleton.
    float padding = 0;
    VtMatrix4dArray skelRestXforms;
    if (skelQuery.ComputeJointSkelTransforms(&skelRestXforms, time, /*atRest*/ true))
    {

      for (const auto &skinningQuery : binding.GetSkinningTargets())
      {

        const UsdPrim &skinnedPrim = skinningQuery.GetPrim();

        float skelPadding = skinningQuery.ComputeExtentsPadding(skelRestXforms,
                                                                UsdGeomBoundable(skinnedPrim));
        padding = std::max(padding, skelPadding);
      }
    }

    // Compute the final, padded extents from the skel-space
    // transforms, in the space of the SkelRoot prim.
    bool resetXformStack = false;
    GfMatrix4d skelRootXform = xfCache.ComputeRelativeTransform(binding.GetSkeleton().GetPrim(),
                                                                skelRoot.GetPrim(),
                                                                &resetXformStack);
    if (!resetXformStack && transform)
    {
      skelRootXform *= *transform;
    }
    UsdSkelComputeJointsExtent(skelXforms, &skelExtent, padding, &skelRootXform);

    for (const auto &p : skelExtent)
      bbox.UnionWith(p);
  }

  extent->resize(2);
  (*extent)[0] = GfVec3f(bbox.GetMin());
  (*extent)[1] = GfVec3f(bbox.GetMax());

  return true;
}

TF_REGISTRY_FUNCTION(UsdGeomBoundable)
{
  UsdGeomRegisterComputeExtentFunction<UsdSkelRoot>(_ComputeExtent);
}

WABI_NAMESPACE_END
