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
#include "wabi/usdImaging/usdImaging/capsuleAdapter.h"

#include "wabi/usdImaging/usdImaging/delegate.h"
#include "wabi/usdImaging/usdImaging/implicitSurfaceMeshUtils.h"
#include "wabi/usdImaging/usdImaging/indexProxy.h"
#include "wabi/usdImaging/usdImaging/tokens.h"

#include "wabi/imaging/hd/mesh.h"
#include "wabi/imaging/hd/meshTopology.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/usd/usdGeom/capsule.h"
#include "wabi/usd/usdGeom/xformCache.h"

#include "wabi/base/tf/type.h"

#include <cmath>

WABI_NAMESPACE_BEGIN

TF_REGISTRY_FUNCTION(TfType)
{
  typedef UsdImagingCapsuleAdapter Adapter;
  TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter>>();
  t.SetFactory<UsdImagingPrimAdapterFactory<Adapter>>();
}

UsdImagingCapsuleAdapter::~UsdImagingCapsuleAdapter()
{}

bool UsdImagingCapsuleAdapter::IsSupported(UsdImagingIndexProxy const *index) const
{
  return index->IsRprimTypeSupported(HdPrimTypeTokens->mesh);
}

SdfPath UsdImagingCapsuleAdapter::Populate(UsdPrim const &prim,
                                           UsdImagingIndexProxy *index,
                                           UsdImagingInstancerContext const *instancerContext)

{
  return _AddRprim(
      HdPrimTypeTokens->mesh, prim, index, GetMaterialUsdPath(prim), instancerContext);
}

void UsdImagingCapsuleAdapter::TrackVariability(
    UsdPrim const &prim,
    SdfPath const &cachePath,
    HdDirtyBits *timeVaryingBits,
    UsdImagingInstancerContext const *instancerContext) const
{
  BaseAdapter::TrackVariability(prim, cachePath, timeVaryingBits, instancerContext);

  // Check DirtyPoints before doing variability checks, in case we can skip
  // any of them...
  if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
    _IsVarying(prim,
               UsdGeomTokens->height,
               HdChangeTracker::DirtyPoints,
               UsdImagingTokens->usdVaryingPrimvar,
               timeVaryingBits,
               /*inherited*/ false);
  }
  if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
    _IsVarying(prim,
               UsdGeomTokens->radius,
               HdChangeTracker::DirtyPoints,
               UsdImagingTokens->usdVaryingPrimvar,
               timeVaryingBits,
               /*inherited*/ false);
  }
  if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
    _IsVarying(prim,
               UsdGeomTokens->axis,
               HdChangeTracker::DirtyPoints,
               UsdImagingTokens->usdVaryingPrimvar,
               timeVaryingBits,
               /*inherited*/ false);
  }
}

HdDirtyBits UsdImagingCapsuleAdapter::ProcessPropertyChange(UsdPrim const &prim,
                                                            SdfPath const &cachePath,
                                                            TfToken const &propertyName)
{
  if (propertyName == UsdGeomTokens->height || propertyName == UsdGeomTokens->radius ||
      propertyName == UsdGeomTokens->axis) {
    return HdChangeTracker::DirtyPoints;
  }

  // Allow base class to handle change processing.
  return BaseAdapter::ProcessPropertyChange(prim, cachePath, propertyName);
}

/*virtual*/
VtValue UsdImagingCapsuleAdapter::GetPoints(UsdPrim const &prim, UsdTimeCode time) const
{
  return GetMeshPoints(prim, time);
}

/*static*/
VtValue UsdImagingCapsuleAdapter::GetMeshPoints(UsdPrim const &prim, UsdTimeCode time)
{
  UsdGeomCapsule capsule(prim);
  double height = 1.0;
  double radius = 0.5;
  TfToken axis  = UsdGeomTokens->z;
  TF_VERIFY(capsule.GetHeightAttr().Get(&height, time));
  TF_VERIFY(capsule.GetRadiusAttr().Get(&radius, time));
  TF_VERIFY(capsule.GetAxisAttr().Get(&axis, time));

  return VtValue(UsdImagingGenerateCapsuleMeshPoints(height, radius, axis));
}

/*static*/
VtValue UsdImagingCapsuleAdapter::GetMeshTopology()
{
  // Topology is constant and identical for all capsules.
  return VtValue(HdMeshTopology(UsdImagingGetCapsuleMeshTopology()));
}

/*virtual*/
VtValue UsdImagingCapsuleAdapter::GetTopology(UsdPrim const &prim,
                                              SdfPath const &cachePath,
                                              UsdTimeCode time) const
{
  TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();
  return GetMeshTopology();
}

WABI_NAMESPACE_END
