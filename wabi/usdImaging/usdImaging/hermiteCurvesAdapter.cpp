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
#include "wabi/usdImaging/usdImaging/hermiteCurvesAdapter.h"

#include "wabi/usdImaging/usdImaging/delegate.h"
#include "wabi/usdImaging/usdImaging/indexProxy.h"
#include "wabi/usdImaging/usdImaging/tokens.h"

#include "wabi/imaging/hd/basisCurves.h"
#include "wabi/imaging/hd/perfLog.h"

#include "wabi/usd/usdGeom/hermiteCurves.h"
#include "wabi/usd/usdGeom/primvarsAPI.h"
#include "wabi/usd/usdGeom/xformCache.h"

#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

TF_REGISTRY_FUNCTION(TfType)
{
  typedef UsdImagingHermiteCurvesAdapter Adapter;
  TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter>>();
  t.SetFactory<UsdImagingPrimAdapterFactory<Adapter>>();
}

UsdImagingHermiteCurvesAdapter::~UsdImagingHermiteCurvesAdapter()
{}

bool UsdImagingHermiteCurvesAdapter::IsSupported(UsdImagingIndexProxy const *index) const
{
  return index->IsRprimTypeSupported(HdPrimTypeTokens->basisCurves);
}

SdfPath UsdImagingHermiteCurvesAdapter::Populate(
    UsdPrim const &prim,
    UsdImagingIndexProxy *index,
    UsdImagingInstancerContext const *instancerContext)
{
  return _AddRprim(
      HdPrimTypeTokens->basisCurves, prim, index, GetMaterialUsdPath(prim), instancerContext);
}

void UsdImagingHermiteCurvesAdapter::TrackVariability(
    UsdPrim const &prim,
    SdfPath const &cachePath,
    HdDirtyBits *timeVaryingBits,
    UsdImagingInstancerContext const *instancerContext) const
{
  BaseAdapter::TrackVariability(prim, cachePath, timeVaryingBits, instancerContext);

  // Discover time-varying points.
  _IsVarying(prim,
             UsdGeomTokens->points,
             HdChangeTracker::DirtyPoints,
             UsdImagingTokens->usdVaryingPrimvar,
             timeVaryingBits,
             /*isInherited*/ false);

  // Discover time-varying topology.
  //
  // Note that basis, wrap and type are all uniform attributes, so they can't
  // vary over time.
  _IsVarying(prim,
             UsdGeomTokens->curveVertexCounts,
             HdChangeTracker::DirtyTopology,
             UsdImagingTokens->usdVaryingTopology,
             timeVaryingBits,
             /*isInherited*/ false);
}

bool UsdImagingHermiteCurvesAdapter::_IsBuiltinPrimvar(TfToken const &primvarName) const
{
  return (primvarName == HdTokens->normals || primvarName == HdTokens->widths) ||
         UsdImagingGprimAdapter::_IsBuiltinPrimvar(primvarName);
}

HdDirtyBits UsdImagingHermiteCurvesAdapter::ProcessPropertyChange(UsdPrim const &prim,
                                                                  SdfPath const &cachePath,
                                                                  TfToken const &propertyName)
{
  if (propertyName == UsdGeomTokens->points) {
    return HdChangeTracker::DirtyPoints;
  }

  else if (propertyName == UsdGeomTokens->curveVertexCounts) {
    return HdChangeTracker::DirtyTopology;
  }

  // Allow base class to handle change processing.
  return BaseAdapter::ProcessPropertyChange(prim, cachePath, propertyName);
}

// -------------------------------------------------------------------------- //

VtValue UsdImagingHermiteCurvesAdapter::GetTopology(UsdPrim const &prim,
                                                    SdfPath const &cachePath,
                                                    UsdTimeCode time) const
{
  TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdBasisCurvesTopology topology(HdTokens->linear,
                                 HdTokens->bezier,
                                 HdTokens->nonperiodic,
                                 _Get<VtIntArray>(prim, UsdGeomTokens->curveVertexCounts, time),
                                 VtIntArray());
  return VtValue(topology);
}

WABI_NAMESPACE_END
