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
#include "wabi/usd/usdSkel/topology.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/wrapTypeHelpers.h"
#include "wabi/usd/usd/pyConversions.h"

#include <boost/python.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace
{

  tuple _Validate(const UsdSkelTopology &self)
  {
    std::string reason;
    bool success = self.Validate(&reason);
    return boost::python::make_tuple(success, reason);
  }

}  // namespace

void wrapUsdSkelTopology()
{
  using This = UsdSkelTopology;

  class_<This>("Topology", no_init)
    .def(init<const SdfPathVector &>())
    .def(init<const VtTokenArray &>())
    .def(init<VtIntArray>())

    .def("GetParent", &This::GetParent)

    .def("IsRoot", &This::IsRoot)

    .def("GetParentIndices", &This::GetParentIndices, return_value_policy<return_by_value>())

    .def("GetNumJoints", &This::GetNumJoints)

    .def("__len__", &This::size)

    .def("Validate", &_Validate);
}
