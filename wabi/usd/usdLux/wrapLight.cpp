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
#include "wabi/usd/usdLux/light.h"

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

static UsdAttribute _CreateIntensityAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateIntensityAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static UsdAttribute _CreateExposureAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateExposureAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static UsdAttribute _CreateDiffuseAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateDiffuseAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static UsdAttribute _CreateSpecularAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateSpecularAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static UsdAttribute _CreateNormalizeAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateNormalizeAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static UsdAttribute _CreateColorAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateColorAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Color3f), writeSparsely);
}

static UsdAttribute _CreateEnableColorTemperatureAttr(UsdLuxLight &self,
                                                      object defaultVal,
                                                      bool writeSparsely)
{
  return self.CreateEnableColorTemperatureAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool),
                                               writeSparsely);
}

static UsdAttribute _CreateColorTemperatureAttr(UsdLuxLight &self, object defaultVal, bool writeSparsely)
{
  return self.CreateColorTemperatureAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float),
                                         writeSparsely);
}

static std::string _Repr(const UsdLuxLight &self)
{
  std::string primRepr = TfPyRepr(self.GetPrim());
  return TfStringPrintf("UsdLux.Light(%s)", primRepr.c_str());
}

}  // anonymous namespace

void wrapUsdLuxLight()
{
  typedef UsdLuxLight This;

  class_<This, bases<UsdGeomXformable>> cls("Light");

  cls.def(init<UsdPrim>(arg("prim")))
    .def(init<UsdSchemaBase const &>(arg("schemaObj")))
    .def(TfTypePythonClass())

    .def("Get", &This::Get, (arg("stage"), arg("path")))
    .staticmethod("Get")

    .def("GetSchemaAttributeNames",
         &This::GetSchemaAttributeNames,
         arg("includeInherited") = true,
         return_value_policy<TfPySequenceToList>())
    .staticmethod("GetSchemaAttributeNames")

    .def("GetStaticTfType", (TfType const &(*)())TfType::Find<This>, return_value_policy<return_by_value>())
    .staticmethod("GetStaticTfType")

    .def(!self)

    .def("GetIntensityAttr", &This::GetIntensityAttr)
    .def("CreateIntensityAttr",
         &_CreateIntensityAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetExposureAttr", &This::GetExposureAttr)
    .def("CreateExposureAttr",
         &_CreateExposureAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetDiffuseAttr", &This::GetDiffuseAttr)
    .def("CreateDiffuseAttr",
         &_CreateDiffuseAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetSpecularAttr", &This::GetSpecularAttr)
    .def("CreateSpecularAttr",
         &_CreateSpecularAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetNormalizeAttr", &This::GetNormalizeAttr)
    .def("CreateNormalizeAttr",
         &_CreateNormalizeAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetColorAttr", &This::GetColorAttr)
    .def(
      "CreateColorAttr", &_CreateColorAttr, (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetEnableColorTemperatureAttr", &This::GetEnableColorTemperatureAttr)
    .def("CreateEnableColorTemperatureAttr",
         &_CreateEnableColorTemperatureAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetColorTemperatureAttr", &This::GetColorTemperatureAttr)
    .def("CreateColorTemperatureAttr",
         &_CreateColorTemperatureAttr,
         (arg("defaultValue") = object(), arg("writeSparsely") = false))

    .def("GetFiltersRel", &This::GetFiltersRel)
    .def("CreateFiltersRel", &This::CreateFiltersRel)
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

#include "wabi/usd/usdShade/connectableAPI.h"

namespace
{

WRAP_CUSTOM
{
  _class.def(init<UsdShadeConnectableAPI>(arg("connectable")))
    .def("ConnectableAPI", &UsdLuxLight::ConnectableAPI)

    .def("CreateOutput", &UsdLuxLight::CreateOutput, (arg("name"), arg("type")))
    .def("GetOutput", &UsdLuxLight::GetOutput, arg("name"))
    .def("GetOutputs",
         &UsdLuxLight::GetOutputs,
         (arg("onlyAuthored") = true),
         return_value_policy<TfPySequenceToList>())

    .def("CreateInput", &UsdLuxLight::CreateInput, (arg("name"), arg("type")))
    .def("GetInput", &UsdLuxLight::GetInput, arg("name"))
    .def("GetInputs",
         &UsdLuxLight::GetInputs,
         (arg("onlyAuthored") = true),
         return_value_policy<TfPySequenceToList>())

    .def("ComputeBaseEmission", &UsdLuxLight::ComputeBaseEmission)
    .def("GetLightLinkCollectionAPI", &UsdLuxLight::GetLightLinkCollectionAPI)
    .def("GetShadowLinkCollectionAPI", &UsdLuxLight::GetShadowLinkCollectionAPI);
}

}  // namespace
