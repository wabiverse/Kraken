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

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/plane.h"
#include "wabi/base/gf/range3d.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/wrapTypeHelpers.h"

#include <boost/python/class.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/def.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/return_arg.hpp>

#include <string>

using namespace boost::python;

using std::string;

WABI_NAMESPACE_USING

namespace
{

  static string _Repr(GfPlane const &self)
  {
    return TF_PY_REPR_PREFIX + "Plane(" + TfPyRepr(self.GetNormal()) + ", " +
           TfPyRepr(self.GetDistanceFromOrigin()) + ")";
  }

  static object _FitPlaneToPoints(const std::vector<GfVec3d> &points)
  {
    GfPlane plane;
    return GfFitPlaneToPoints(points, &plane) ? object(plane) : object();
  }

}  // anonymous namespace

void wrapPlane()
{
  typedef GfPlane This;

  object getNormal = make_function(&This::GetNormal, return_value_policy<return_by_value>());

  def("FitPlaneToPoints", _FitPlaneToPoints);

  class_<This>("Plane", init<>())
    .def(init<const GfVec3d &, double>())
    .def(init<const GfVec3d &, const GfVec3d &>())
    .def(init<const GfVec3d &, const GfVec3d &, const GfVec3d &>())
    .def(init<const GfVec4d &>())

    .def(TfTypePythonClass())

    .def("Set", (void(This::*)(const GfVec3d &, double)) & This::Set, return_self<>())
    .def("Set", (void(This::*)(const GfVec3d &, const GfVec3d &)) & This::Set, return_self<>())
    .def("Set",
         (void(This::*)(const GfVec3d &, const GfVec3d &, const GfVec3d &)) & This::Set,
         return_self<>())
    .def("Set", (void(This::*)(const GfVec4d &)) & This::Set, return_self<>())

    .add_property("normal", getNormal)
    .add_property("distanceFromOrigin", &This::GetDistanceFromOrigin)

    .def("GetDistance", &This::GetDistance)
    .def("GetDistanceFromOrigin", &This::GetDistanceFromOrigin)
    .def("GetNormal", getNormal)
    .def("GetEquation", &This::GetEquation)
    .def("Project", &This::Project)

    .def("Transform", &This::Transform, return_self<>())

    .def("Reorient", &This::Reorient, return_self<>())

    .def("IntersectsPositiveHalfSpace",
         (bool(This::*)(const GfRange3d &) const) & This::IntersectsPositiveHalfSpace)

    .def("IntersectsPositiveHalfSpace",
         (bool(This::*)(const GfVec3d &) const) & This::IntersectsPositiveHalfSpace)

    .def(str(self))
    .def(self == self)
    .def(self != self)

    .def("__repr__", _Repr)

    ;
  to_python_converter<std::vector<This>, TfPySequenceToPython<std::vector<This>>>();
}
