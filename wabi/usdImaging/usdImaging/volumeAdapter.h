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
#ifndef WABI_USD_IMAGING_USD_IMAGING_VOLUME_ADAPTER_H
#define WABI_USD_IMAGING_USD_IMAGING_VOLUME_ADAPTER_H

/// \file usdImaging/volumeAdapter.h

#include "wabi/usd/usdVol/volume.h"
#include "wabi/usdImaging/usdImaging/api.h"
#include "wabi/usdImaging/usdImaging/gprimAdapter.h"
#include "wabi/usdImaging/usdImaging/primAdapter.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \class UsdImagingVolumeAdapter
///
/// Delegate support for UsdVolVolume.
///
class UsdImagingVolumeAdapter : public UsdImagingGprimAdapter {
 public:
  typedef UsdImagingGprimAdapter BaseAdapter;

  UsdImagingVolumeAdapter() : UsdImagingGprimAdapter()
  {}
  virtual ~UsdImagingVolumeAdapter();

  virtual SdfPath Populate(UsdPrim const &prim,
                           UsdImagingIndexProxy *index,
                           UsdImagingInstancerContext const *instancerContext = NULL) override;

  virtual bool IsSupported(UsdImagingIndexProxy const *index) const override;

  // ---------------------------------------------------------------------- //
  /// \name Parallel Setup and Resolve
  // ---------------------------------------------------------------------- //
  /// Thread Safe.
  virtual void TrackVariability(
      UsdPrim const &prim,
      SdfPath const &cachePath,
      HdDirtyBits *timeVaryingBits,
      UsdImagingInstancerContext const *instancerContext = NULL) const override;

  /// Thread Safe.
  virtual void UpdateForTime(
      UsdPrim const &prim,
      SdfPath const &cachePath,
      UsdTimeCode time,
      HdDirtyBits requestedBits,
      UsdImagingInstancerContext const *instancerContext = NULL) const override;

  virtual HdVolumeFieldDescriptorVector GetVolumeFieldDescriptors(UsdPrim const &usdPrim,
                                                                  SdfPath const &id,
                                                                  UsdTimeCode time) const override;

 private:
  bool _GatherVolumeData(UsdPrim const &prim, UsdVolVolume::FieldMap *fieldMap) const;
};

WABI_NAMESPACE_END

#endif  // WABI_USD_IMAGING_USD_IMAGING_VOLUME_ADAPTER_H
