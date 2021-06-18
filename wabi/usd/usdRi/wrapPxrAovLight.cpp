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
#include "wabi/usd/usd/schemaBase.h"
#include "wabi/usd/usdRi/pxrAovLight.h"

#include "wabi/usd/sdf/primSpec.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/wrapTypeHelpers.h"
#include "wabi/usd/usd/pyConversions.h"

#include <boost/python.hpp>

#include <string>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace
{

#define WRAP_CUSTOM template<class Cls> \
static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

static UsdAttribute _CreateAovNameAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateAovNameAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}

static UsdAttribute _CreateInPrimaryHitAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateInPrimaryHitAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateInReflectionAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateInReflectionAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateInRefractionAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateInRefractionAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateInvertAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateInvertAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateOnVolumeBoundariesAttr(UsdRiPxrAovLight &self,
                                                  object defaultVal,
                                                  bool writeSparsely)
{
  return self.CreateOnVolumeBoundariesAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool),
                                           writeSparsely);
}

static UsdAttribute _CreateUseColorAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateUseColorAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateUseThroughputAttr(UsdRiPxrAovLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateUseThroughputAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool),
                                      writeSparsely);
}

static std::string _Repr(const UsdRiPxrAovLight &self)
{
  std::string primRepr = TfPyRepr(self.GetPrim());
  return TfStringPrintf("UsdRi.PxrAovLight(%s)", primRepr.c_str());
}

}  // anonymous namespace

void wrapUsdRiPxrAovLight()
{
  typedef UsdRiPxrAovLight This;

  class_<This, bases<UsdLuxLight>> cls("wabiAovLight");

  cls.def(init<UsdPrim>(arg("prim")))
    .def(init<UsdSchemaBase const &>(arg("schemaObj")))
    .def(TfTypePythonClass())

    .def("Get", &This::Get, (arg("stage"), arg("path")))
    .staticmethod("Get")

    .def("Define", &This::Define, (arg("stage"), arg("path")))
    .staticmethod("Define")

    .def("GetSchemaAttributeNames",
         &This::GetSchemaAttributeNames,
         arg("includeInherited") = true,
         return_value_policy<TfPySequenceToList>())
    .staticmethod("GetSchemaAttributeNames")

    .def("_GetStaticTfType", (TfType const &(*)())TfType::Find<This>, return_value_policy<return_by_value>())
    .staticmethod("_GetStaticTfType")

    .def(!self)

    .def("GetAovNameAttr", &This::GetAovNameAttr)
    .def("CreateAovNameAttr",
         &_CreateAovNameAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetInPrimaryHitAttr", &This::GetInPrimaryHitAttr)
    .def("CreateInPrimaryHitAttr",
         &_CreateInPrimaryHitAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetInReflectionAttr", &This::GetInReflectionAttr)
    .def("CreateInReflectionAttr",
         &_CreateInReflectionAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetInRefractionAttr", &This::GetInRefractionAttr)
    .def("CreateInRefractionAttr",
         &_CreateInRefractionAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetInvertAttr", &This::GetInvertAttr)
    .def(
      "CreateInvertAttr", &_CreateInvertAttr, (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetOnVolumeBoundariesAttr", &This::GetOnVolumeBoundariesAttr)
    .def("CreateOnVolumeBoundariesAttr",
         &_CreateOnVolumeBoundariesAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetUseColorAttr", &This::GetUseColorAttr)
    .def("CreateUseColorAttr",
         &_CreateUseColorAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetUseThroughputAttr", &This::GetUseThroughputAttr)
    .def("CreateUseThroughputAttr",
         &_CreateUseThroughputAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("__repr__", ::_Repr);

  _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
//
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace
{

WRAP_CUSTOM
{}

}  // namespace
