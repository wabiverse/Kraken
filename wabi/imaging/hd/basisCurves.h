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
#ifndef WABI_IMAGING_HD_BASIS_CURVES_H
#define WABI_IMAGING_HD_BASIS_CURVES_H

#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/rprim.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

#define HD_BASISCURVES_REPR_DESC_TOKENS (surfaceShader)(hullColor)(pointColor)

TF_DECLARE_PUBLIC_TOKENS(HdBasisCurvesReprDescTokens, HD_API, HD_BASISCURVES_REPR_DESC_TOKENS);

/// \class HdBasisCurvesReprDesc
///
/// Descriptor to configure a drawItem for a repr.
///
struct HdBasisCurvesReprDesc {
  HdBasisCurvesReprDesc(HdBasisCurvesGeomStyle geomStyle = HdBasisCurvesGeomStyleInvalid,
                        TfToken shadingTerminal = HdBasisCurvesReprDescTokens->surfaceShader)
      : geomStyle(geomStyle),
        shadingTerminal(shadingTerminal)
  {}

  bool IsEmpty() const
  {
    return geomStyle == HdBasisCurvesGeomStyleInvalid;
  }

  HdBasisCurvesGeomStyle geomStyle;
  /// Specifies how the fragment color should be computed from primvar;
  /// this can be used to render heatmap highlighting etc.
  TfToken shadingTerminal;
};

/// Hydra Schema for a collection of curves using a particular basis.
///
class HdBasisCurves : public HdRprim {
 public:
  HD_API
  virtual ~HdBasisCurves();

  ///
  /// Topology
  ///
  inline HdBasisCurvesTopology GetBasisCurvesTopology(HdSceneDelegate *delegate) const;
  inline HdDisplayStyle GetDisplayStyle(HdSceneDelegate *delegate) const;

  HD_API
  TfTokenVector const &GetBuiltinPrimvarNames() const override;

  /// Configure geometric style of drawItems for \p reprName
  HD_API
  static void ConfigureRepr(TfToken const &reprName, HdBasisCurvesReprDesc desc);

  /// Returns whether refinement is always on or not.
  HD_API
  static bool IsEnabledForceRefinedCurves();

 protected:
  HD_API
  HdBasisCurves(SdfPath const &id);

  typedef _ReprDescConfigs<HdBasisCurvesReprDesc> _BasisCurvesReprConfig;

  HD_API
  static _BasisCurvesReprConfig::DescArray _GetReprDesc(TfToken const &reprName);

 private:
  // Class can not be default constructed or copied.
  HdBasisCurves()                      = delete;
  HdBasisCurves(const HdBasisCurves &) = delete;
  HdBasisCurves &operator=(const HdBasisCurves &) = delete;

  static _BasisCurvesReprConfig _reprDescConfig;
};

inline HdBasisCurvesTopology HdBasisCurves::GetBasisCurvesTopology(HdSceneDelegate *delegate) const
{
  return delegate->GetBasisCurvesTopology(GetId());
}

inline HdDisplayStyle HdBasisCurves::GetDisplayStyle(HdSceneDelegate *delegate) const
{
  return delegate->GetDisplayStyle(GetId());
}

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_BASIS_CURVES_H
