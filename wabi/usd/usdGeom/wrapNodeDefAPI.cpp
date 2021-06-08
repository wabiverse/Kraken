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
#include "wabi/usd/usdGeom/nodeDefAPI.h"

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

namespace {

#define WRAP_CUSTOM template<class Cls> static void _CustomWrapCode(Cls &_class)

/* fwd decl. */
WRAP_CUSTOM;

static UsdAttribute _CreateImplementationSourceAttr(UsdGeomNodeDefAPI &self,
                                                    object defaultVal,
                                                    bool writeSparsely)
{
  return self.CreateImplementationSourceAttr(
      UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Token), writeSparsely);
}

static UsdAttribute _CreateIdAttr(UsdGeomNodeDefAPI &self, object defaultVal, bool writeSparsely)
{
  return self.CreateIdAttr(UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Token),
                           writeSparsely);
}

static std::string _Repr(const UsdGeomNodeDefAPI &self)
{
  std::string primRepr = TfPyRepr(self.GetPrim());
  return TfStringPrintf("UsdGeom.GeomNodeDefAPI(%s)", primRepr.c_str());
}

}  // anonymous namespace

void wrapUsdGeomNodeDefAPI()
{
  typedef UsdGeomNodeDefAPI This;

  class_<This, bases<UsdAPISchemaBase>> cls("GeomNodeDefAPI");

  cls.def(init<UsdPrim>(arg("prim")))
      .def(init<UsdSchemaBase const &>(arg("schemaObj")))
      .def(TfTypePythonClass())

      .def("Get", &This::Get, (arg("stage"), arg("path")))
      .staticmethod("Get")

      .def("Apply", &This::Apply, (arg("prim")))
      .staticmethod("Apply")

      .def("GetSchemaAttributeNames",
           &This::GetSchemaAttributeNames,
           arg("includeInherited") = true,
           return_value_policy<TfPySequenceToList>())
      .staticmethod("GetSchemaAttributeNames")

      .def("_GetStaticTfType",
           (TfType const &(*)())TfType::Find<This>,
           return_value_policy<return_by_value>())
      .staticmethod("_GetStaticTfType")

      .def(!self)

      .def("GetImplementationSourceAttr", &This::GetImplementationSourceAttr)
      .def("CreateImplementationSourceAttr",
           &_CreateImplementationSourceAttr,
           (arg("defaultValue") = object(), arg("writeSparsely") = false))

      .def("GetIdAttr", &This::GetIdAttr)
      .def("CreateIdAttr",
           &_CreateIdAttr,
           (arg("defaultValue") = object(), arg("writeSparsely") = false))

      .def("__repr__", ::_Repr);

  _CustomWrapCode(cls);
}

/**
 * =====================================================================
 *  Feel free to add custom code below this line, it will be preserved
 *  by the code generator. The entry point for your custom code should
 *  look minimally like the following:
 *
 * WRAP_CUSTOM {
 *     _class
 *         .def("MyCustomMethod", ...)
 *     ;
 * }
 *
 *  Of course any other ancillary or support code may be provided.
 *
 *  Just remember to wrap code in the appropriate delimiters:
 *  'namespace {', '}'.
 *
 * =====================================================================
 * --(BEGIN CUSTOM CODE)-- */

namespace {

static object _WrapGetGeomId(const UsdGeomNodeDefAPI &geom)
{
  TfToken id;
  if (geom.GetGeomId(&id)) {
    return object(id);
  }
  return object();
}

static object _WrapGetSourceAsset(const UsdGeomNodeDefAPI &geom, const TfToken &sourceType)
{
  SdfAssetPath asset;
  if (geom.GetSourceAsset(&asset, sourceType)) {
    return object(asset);
  }
  return object();
}

static object _WrapGetSourceAssetSubIdentifier(const UsdGeomNodeDefAPI &geom,
                                               const TfToken &sourceType)
{
  TfToken subIdentifier;
  if (geom.GetSourceAssetSubIdentifier(&subIdentifier, sourceType)) {
    return object(subIdentifier);
  }
  return object();
}

static object _WrapGetSourceCode(const UsdGeomNodeDefAPI &geom, const TfToken &sourceType)
{
  std::string code;
  if (geom.GetSourceCode(&code, sourceType)) {
    return object(code);
  }
  return object();
}

WRAP_CUSTOM
{
  _class.def("GetImplementationSource", &UsdGeomNodeDefAPI::GetImplementationSource)
      .def("SetGeomId", &UsdGeomNodeDefAPI::SetGeomId)
      .def("SetSourceAsset",
           &UsdGeomNodeDefAPI::SetSourceAsset,
           (arg("sourceAsset"), arg("sourceType") = UsdGeomTokens->universalSourceType))
      .def("SetSourceAssetSubIdentifier",
           &UsdGeomNodeDefAPI::SetSourceAssetSubIdentifier,
           (arg("subIdentifier"), arg("sourceType") = UsdGeomTokens->universalSourceType))
      .def("SetSourceCode",
           &UsdGeomNodeDefAPI::SetSourceCode,
           (arg("sourceCode"), arg("sourceType") = UsdGeomTokens->universalSourceType))
      .def("GetGeomId", _WrapGetGeomId)
      .def("GetSourceAsset",
           _WrapGetSourceAsset,
           arg("sourceType") = UsdGeomTokens->universalSourceType)
      .def("GetSourceAssetSubIdentifier",
           _WrapGetSourceAssetSubIdentifier,
           arg("sourceType") = UsdGeomTokens->universalSourceType)
      .def("GetSourceCode",
           _WrapGetSourceCode,
           arg("sourceType") = UsdGeomTokens->universalSourceType)
      .def("GetGeomNodeForSourceType",
           &UsdGeomNodeDefAPI::GetGeomNodeForSourceType,
           (arg("sourceType")),
           return_internal_reference<>());
}

}  // namespace
