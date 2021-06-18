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

#include "wabi/imaging/pxOsd/meshTopology.h"
#include "wabi/imaging/pxOsd/meshTopologyValidation.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyEnum.h"
#include "wabi/base/tf/pyUtils.h"

#include <boost/python/class.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/operators.hpp>

#include <sstream>
#include <vector>

using namespace boost::python;

WABI_NAMESPACE_USING

static std::string _InvalidationRepr(PxOsdMeshTopologyValidation::Invalidation const &invalidation)
{
  return TfStringPrintf("PxOsd.MeshTopologyValidation.Invalidation('%s', '%s')",
                        TfPyRepr(invalidation.code).c_str(),
                        invalidation.message.c_str());
}

static std::string _ValidationRepr(PxOsdMeshTopologyValidation const &validation)
{
  std::ostringstream repr(std::ostringstream::ate);
  if (validation)
  {
    repr << "PxOsd.MeshTopologyValidation()";
  }
  else
  {
    repr << "PxOsd.MeshTopologyValidation<";
    for (auto const &element : validation)
    {
      repr << TfPyRepr(element.code) << ", " << element.message << "), ";
    }
    repr << ">";
  }
  return repr.str();
}

static PxOsdMeshTopologyValidation::Invalidation _InvalidationInit(PxOsdMeshTopologyValidation::Code code,
                                                                   std::string const &message)
{
  return {code, message};
}

void wrapMeshTopologyValidation()
{
  using This = PxOsdMeshTopologyValidation;

  class_<This> cls("MeshTopologyValidation", init<>());
  cls.def(!self);
  {
    scope obj = cls;
    TfPyWrapEnum<This::Code, true>();
    class_<This::Invalidation>("Invalidation", no_init)
      .def("__init__", &::_InvalidationInit)
      .def_readwrite("code", &This::Invalidation::code)
      .def_readwrite("message", &This::Invalidation::message)
      .def("__repr__", &::_InvalidationRepr);
  }
  cls.def("__repr__", &::_ValidationRepr);
  cls.def("__iter__", iterator<This>());
}
