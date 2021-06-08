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
#include "wabi/usd/usdGeom/xformCache.h"
#include "wabi/wabi.h"

#include <boost/python/class.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace {

static tuple _GetLocalTransformation(UsdGeomXformCache &self, const UsdPrim &prim)
{
  bool resetsXformStack;
  GfMatrix4d localXform = self.GetLocalTransformation(prim, &resetsXformStack);

  return make_tuple(localXform, resetsXformStack);
}

static tuple _ComputeRelativeTransform(UsdGeomXformCache &self,
                                       const UsdPrim &prim,
                                       const UsdPrim &ancestor)
{
  bool resetXformStack;
  GfMatrix4d xform = self.ComputeRelativeTransform(prim, ancestor, &resetXformStack);

  return make_tuple(xform, resetXformStack);
}

}  // anonymous namespace

void wrapUsdGeomXformCache()
{
  typedef UsdGeomXformCache XformCache;

  class_<XformCache>("XformCache")
      .def(init<UsdTimeCode>(arg("time")))
      .def("GetLocalToWorldTransform", &XformCache::GetLocalToWorldTransform, arg("prim"))
      .def("GetParentToWorldTransform", &XformCache::GetParentToWorldTransform, arg("prim"))
      .def("GetLocalTransformation", &_GetLocalTransformation, arg("prim"))
      .def("ComputeRelativeTransform", &_ComputeRelativeTransform, (arg("prim"), arg("ancestor")))
      .def("Clear", &XformCache::Clear)
      .def("SetTime", &XformCache::SetTime, arg("time"))
      .def("GetTime", &XformCache::GetTime)

      .def("Swap", &XformCache::Swap, arg("other"));
}
