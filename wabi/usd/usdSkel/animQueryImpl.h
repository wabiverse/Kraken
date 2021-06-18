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
#ifndef WABI_USD_USD_SKEL_ANIM_QUERY_IMPL_H
#define WABI_USD_USD_SKEL_ANIM_QUERY_IMPL_H

/// \file UsdSkel_AnimQueryImpl

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/refBase.h"
#include "wabi/base/vt/types.h"

#include "wabi/usd/usdGeom/xformable.h"

#include "wabi/usd/sdf/path.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

class UsdAttribute;
class UsdPrim;
class UsdTimeCode;

TF_DECLARE_REF_PTRS(UsdSkel_AnimQueryImpl);

/// \class UsdSkel_AnimQueryImpl
///
/// Internal implementation of anim animation query.
/// Subclassing of animation queries is supported out of an expectation
/// that additional core animation prim types may be added in the future.
class UsdSkel_AnimQueryImpl : public TfRefBase
{
 public:
  /// Create an anim query for \p prim, if the prim is a valid type.
  static UsdSkel_AnimQueryImplRefPtr New(const UsdPrim &prim);

  virtual ~UsdSkel_AnimQueryImpl()
  {}

  virtual UsdPrim GetPrim() const = 0;

  virtual bool ComputeJointLocalTransforms(VtMatrix4dArray *xforms, UsdTimeCode time) const = 0;

  virtual bool ComputeJointLocalTransforms(VtMatrix4fArray *xforms, UsdTimeCode time) const = 0;

  virtual bool ComputeJointLocalTransformComponents(VtVec3fArray *translations,
                                                    VtQuatfArray *rotations,
                                                    VtVec3hArray *scales,
                                                    UsdTimeCode time) const = 0;

  virtual bool GetJointTransformTimeSamples(const GfInterval &interval,
                                            std::vector<double> *times) const = 0;

  virtual bool GetJointTransformAttributes(std::vector<UsdAttribute> *attrs) const = 0;

  virtual bool JointTransformsMightBeTimeVarying() const = 0;

  virtual bool ComputeBlendShapeWeights(VtFloatArray *weights,
                                        UsdTimeCode time = UsdTimeCode::Default()) const = 0;

  virtual bool GetBlendShapeWeightTimeSamples(const GfInterval &interval,
                                              std::vector<double> *times) const = 0;

  virtual bool GetBlendShapeWeightAttributes(std::vector<UsdAttribute> *attrs) const = 0;

  virtual bool BlendShapeWeightsMightBeTimeVarying() const = 0;

  const VtTokenArray &GetJointOrder() const
  {
    return _jointOrder;
  }

  const VtTokenArray &GetBlendShapeOrder() const
  {
    return _blendShapeOrder;
  }

 protected:
  VtTokenArray _jointOrder, _blendShapeOrder;
};

WABI_NAMESPACE_END

#endif  // USDSKEL_ANIMQUERY_IMPL
