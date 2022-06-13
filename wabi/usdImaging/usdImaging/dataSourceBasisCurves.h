//
// Copyright 2022 Pixar
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
#ifndef WABI_USD_IMAGING_USD_IMAGING_DATA_SOURCE_BASISCURVES_H
#define WABI_USD_IMAGING_USD_IMAGING_DATA_SOURCE_BASISCURVES_H

#include "wabi/usdImaging/usdImaging/dataSourceGprim.h"
#include "wabi/usdImaging/usdImaging/dataSourceStageGlobals.h"

#include "wabi/usd/usdGeom/basisCurves.h"

#include "wabi/imaging/hd/dataSource.h"

WABI_NAMESPACE_BEGIN


/// \class UsdImagingDataSourceBasisCurvesTopology
///
/// A container data source representing basis curves topology information.
///
class UsdImagingDataSourceBasisCurvesTopology : public HdContainerDataSource
{
 public:

  HD_DECLARE_DATASOURCE(UsdImagingDataSourceBasisCurvesTopology);

  bool Has(const TfToken &name) override;
  TfTokenVector GetNames() override;
  HdDataSourceBaseHandle Get(const TfToken &name) override;

 private:

  UsdImagingDataSourceBasisCurvesTopology(const SdfPath &sceneIndexPath,
                                          UsdGeomBasisCurves usdBasisCurves,
                                          const UsdImagingDataSourceStageGlobals &stageGlobals);

 private:

  const SdfPath _sceneIndexPath;
  UsdGeomBasisCurves _usdBasisCurves;
  const UsdImagingDataSourceStageGlobals &_stageGlobals;
};

HD_DECLARE_DATASOURCE_HANDLES(UsdImagingDataSourceBasisCurvesTopology);

// ----------------------------------------------------------------------------

/// \class UsdImagingDataSourceBasisCurves
///
/// A container data source representing data unique to basis curves
///
class UsdImagingDataSourceBasisCurves : public HdContainerDataSource
{
 public:

  HD_DECLARE_DATASOURCE(UsdImagingDataSourceBasisCurves);

  bool Has(const TfToken &name) override;
  TfTokenVector GetNames() override;
  HdDataSourceBaseHandle Get(const TfToken &name) override;

 private:

  UsdImagingDataSourceBasisCurves(const SdfPath &sceneIndexPath,
                                  UsdGeomBasisCurves usdBasisCurves,
                                  const UsdImagingDataSourceStageGlobals &stageGlobals);

 private:

  const SdfPath _sceneIndexPath;
  UsdGeomBasisCurves _usdBasisCurves;
  const UsdImagingDataSourceStageGlobals &_stageGlobals;
};

HD_DECLARE_DATASOURCE_HANDLES(UsdImagingDataSourceBasisCurves);

// ----------------------------------------------------------------------------

/// \class UsdImagingDataSourceBasisCurvesPrim
///
/// A prim data source representing a UsdGeomBasisCurves prim.
///
class UsdImagingDataSourceBasisCurvesPrim : public UsdImagingDataSourceGprim
{
 public:

  HD_DECLARE_DATASOURCE(UsdImagingDataSourceBasisCurvesPrim);

  bool Has(const TfToken &name) override;
  TfTokenVector GetNames() override;
  HdDataSourceBaseHandle Get(const TfToken &name) override;

 private:

  UsdImagingDataSourceBasisCurvesPrim(const SdfPath &sceneIndexPath,
                                      UsdPrim usdPrim,
                                      const UsdImagingDataSourceStageGlobals &stageGlobals);
};

HD_DECLARE_DATASOURCE_HANDLES(UsdImagingDataSourceBasisCurvesPrim);

WABI_NAMESPACE_END

#endif  // WABI_USD_IMAGING_USD_IMAGING_DATA_SOURCE_BASISCURVES_H
