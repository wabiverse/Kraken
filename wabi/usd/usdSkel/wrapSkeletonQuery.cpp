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
#include "wabi/usd/usdSkel/skeletonQuery.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/wrapTypeHelpers.h"
#include "wabi/usd/usd/pyConversions.h"

#include "wabi/usd/usdGeom/xformCache.h"
#include "wabi/usd/usdSkel/animQuery.h"
#include "wabi/usd/usdSkel/skeleton.h"
#include "wabi/usd/usdSkel/topology.h"

#include <boost/python.hpp>
#include <boost/python/extract.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace
{

VtMatrix4dArray _ComputeJointLocalTransforms(UsdSkelSkeletonQuery &self, UsdTimeCode time, bool atRest)
{
  VtMatrix4dArray xforms;
  self.ComputeJointLocalTransforms(&xforms, time, atRest);
  return xforms;
}

VtMatrix4dArray _ComputeJointSkelTransforms(UsdSkelSkeletonQuery &self, UsdTimeCode time, bool atRest)
{
  VtMatrix4dArray xforms;
  self.ComputeJointSkelTransforms(&xforms, time, atRest);
  return xforms;
}

VtMatrix4dArray _ComputeJointWorldTransforms(UsdSkelSkeletonQuery &self,
                                             UsdGeomXformCache &xfCache,
                                             bool atRest)
{
  VtMatrix4dArray xforms;
  self.ComputeJointWorldTransforms(&xforms, &xfCache, atRest);
  return xforms;
}

VtMatrix4dArray _ComputeSkinningTransforms(UsdSkelSkeletonQuery &self, UsdTimeCode time)
{
  VtMatrix4dArray xforms;
  self.ComputeSkinningTransforms(&xforms, time);
  return xforms;
}

VtMatrix4dArray _GetJointWorldBindTransforms(UsdSkelSkeletonQuery &self)
{
  VtMatrix4dArray xforms;
  self.GetJointWorldBindTransforms(&xforms);
  return xforms;
}

VtMatrix4dArray _ComputeJointRestRelativeTransforms(UsdSkelSkeletonQuery &self, UsdTimeCode time)
{
  VtMatrix4dArray xforms;
  self.ComputeJointRestRelativeTransforms(&xforms, time);
  return xforms;
}

}  // namespace

void wrapUsdSkelSkeletonQuery()
{
  using This = UsdSkelSkeletonQuery;

  class_<This>("SkeletonQuery", no_init)

    .def(!self)
    .def(self == self)
    .def(self != self)

    .def("__str__", &This::GetDescription)

    .def("GetPrim", &This::GetPrim, return_value_policy<return_by_value>())

    .def("GetSkeleton", &This::GetSkeleton, return_value_policy<return_by_value>())

    .def("GetAnimQuery", &This::GetAnimQuery, return_value_policy<return_by_value>())

    .def("GetTopology", &This::GetTopology, return_value_policy<return_by_value>())

    .def("GetMapper", &This::GetMapper, return_value_policy<return_by_value>())

    .def("GetJointOrder", &This::GetJointOrder)

    .def("GetJointWorldBindTransforms", &_GetJointWorldBindTransforms)

    .def("ComputeJointLocalTransforms",
         &_ComputeJointLocalTransforms,
         (arg("time") = UsdTimeCode::Default(), arg("atRest") = false))

    .def("ComputeJointSkelTransforms",
         &_ComputeJointSkelTransforms,
         (arg("time") = UsdTimeCode::Default(), arg("atRest") = false))

    .def("ComputeJointWorldTransforms",
         &_ComputeJointWorldTransforms,
         (arg("time") = UsdTimeCode::Default(), arg("atRest") = false))

    .def("ComputeSkinningTransforms", &_ComputeSkinningTransforms, (arg("time") = UsdTimeCode::Default()))

    .def("ComputeJointRestRelativeTransforms",
         &_ComputeJointRestRelativeTransforms,
         (arg("time") = UsdTimeCode::Default()))

    .def("HasBindPose", &This::HasBindPose)

    .def("HasRestPose", &This::HasRestPose)

    ;
}
