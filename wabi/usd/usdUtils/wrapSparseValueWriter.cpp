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
#include "wabi/wabi.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/usd/usd/pyConversions.h"
#include "wabi/usd/usdUtils/sparseValueWriter.h"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/make_constructor.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

static UsdUtilsSparseAttrValueWriter *__init__(const UsdAttribute &attr,
                                               const object &defaultValue)
{
  return new UsdUtilsSparseAttrValueWriter(attr,
                                           UsdPythonToSdfType(defaultValue, attr.GetTypeName()));
}

static bool _WrapSetTimeSample(UsdUtilsSparseAttrValueWriter &vc,
                               const object &value,
                               const UsdTimeCode time)
{
  return vc.SetTimeSample(UsdPythonToSdfType(value, vc.GetAttr().GetTypeName()), time);
}

static bool _WrapSetAttribute(UsdUtilsSparseValueWriter &vc,
                              const UsdAttribute &attr,
                              const object &value,
                              const UsdTimeCode time)
{
  return vc.SetAttribute(attr, UsdPythonToSdfType(value, attr.GetTypeName()), time);
}

static std::vector<UsdUtilsSparseAttrValueWriter> _WrapGetSparseAttrValueWriters(
    UsdUtilsSparseValueWriter &vc)
{
  return vc.GetSparseAttrValueWriters();
}

void wrapSparseValueWriter()
{
  class_<UsdUtilsSparseAttrValueWriter>("SparseAttrValueWriter", no_init)
      .def("__init__",
           make_constructor(
               __init__, default_call_policies(), (arg("attr"), arg("defaultValue") = object())))

      .def("SetTimeSample", _WrapSetTimeSample, (arg("value"), arg("time")));

  class_<UsdUtilsSparseValueWriter>("SparseValueWriter", init<>())
      .def("SetAttribute",
           _WrapSetAttribute,
           (arg("attr"), arg("value"), arg("time") = UsdTimeCode::Default()))

      .def("GetSparseAttrValueWriters", _WrapGetSparseAttrValueWriters);

  // Register to and from vector conversions.
  boost::python::to_python_converter<
      std::vector<UsdUtilsSparseAttrValueWriter>,
      TfPySequenceToPython<std::vector<UsdUtilsSparseAttrValueWriter>>>();
}
