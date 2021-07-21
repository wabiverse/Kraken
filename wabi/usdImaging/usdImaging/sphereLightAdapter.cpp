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
#include "wabi/usdImaging/usdImaging/sphereLightAdapter.h"
#include "wabi/usdImaging/usdImaging/delegate.h"
#include "wabi/usdImaging/usdImaging/indexProxy.h"
#include "wabi/usdImaging/usdImaging/tokens.h"

#include "wabi/imaging/hd/tokens.h"

WABI_NAMESPACE_BEGIN

TF_REGISTRY_FUNCTION(TfType)
{
  typedef UsdImagingSphereLightAdapter Adapter;
  TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter>>();
  t.SetFactory<UsdImagingPrimAdapterFactory<Adapter>>();
}

UsdImagingSphereLightAdapter::~UsdImagingSphereLightAdapter()
{}

bool UsdImagingSphereLightAdapter::IsSupported(UsdImagingIndexProxy const *index) const
{
  return UsdImagingLightAdapter::IsEnabledSceneLights() &&
         index->IsSprimTypeSupported(HdPrimTypeTokens->sphereLight);
}

SdfPath UsdImagingSphereLightAdapter::Populate(UsdPrim const &prim,
                                               UsdImagingIndexProxy *index,
                                               UsdImagingInstancerContext const *instancerContext)
{
  index->InsertSprim(HdPrimTypeTokens->sphereLight, prim.GetPath(), prim);
  HD_PERF_COUNTER_INCR(UsdImagingTokens->usdPopulatedPrimCount);

  return prim.GetPath();
}

void UsdImagingSphereLightAdapter::_RemovePrim(SdfPath const &cachePath, UsdImagingIndexProxy *index)
{
  index->RemoveSprim(HdPrimTypeTokens->sphereLight, cachePath);
}

WABI_NAMESPACE_END
