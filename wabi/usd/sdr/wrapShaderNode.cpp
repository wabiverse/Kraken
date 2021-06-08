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

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/pyPtrHelpers.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pyStaticTokens.h"
#include "wabi/base/tf/weakPtr.h"
#include "wabi/usd/sdr/shaderNode.h"
#include "wabi/usd/sdr/shaderProperty.h"
#include "wabi/wabi.h"

#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

// Boost treats a const ptr differently than a non-const ptr, so a custom
// converter is needed to deal with the const-ness
struct SdrShaderNodeConstPtrToPythonConverter {
  static PyObject *convert(SdrShaderNodeConstPtr shaderNode)
  {
    object shaderNodeObject(ptr(shaderNode));

    return incref(shaderNodeObject.ptr());
  }
};

void wrapShaderNode()
{
  typedef SdrShaderNode This;
  typedef SdrShaderNodePtr ThisPtr;

  TF_PY_WRAP_PUBLIC_TOKENS("NodeMetadata", SdrNodeMetadata, SDR_NODE_METADATA_TOKENS);
  TF_PY_WRAP_PUBLIC_TOKENS("NodeContext", SdrNodeContext, SDR_NODE_CONTEXT_TOKENS);

  TF_PY_WRAP_PUBLIC_TOKENS("NodeRole", SdrNodeRole, SDR_NODE_ROLE_TOKENS);

  return_value_policy<copy_const_reference> copyRefPolicy;
  to_python_converter<SdrShaderNodeConstPtr, SdrShaderNodeConstPtrToPythonConverter>();

  class_<This, ThisPtr, bases<NdrNode>, boost::noncopyable>("ShaderNode", no_init)
      .def("GetShaderInput", &This::GetShaderInput, return_internal_reference<>())
      .def("GetShaderOutput", &This::GetShaderOutput, return_internal_reference<>())
      .def("GetAssetIdentifierInputNames",
           &This::GetAssetIdentifierInputNames,
           return_value_policy<TfPySequenceToList>())
      .def("GetDefaultInput", &This::GetDefaultInput, return_internal_reference<>())
      .def("GetLabel", &This::GetLabel, copyRefPolicy)
      .def("GetCategory", &This::GetCategory, copyRefPolicy)
      .def("GetHelp", &This::GetHelp)
      .def("GetDepartments", &This::GetDepartments, copyRefPolicy)
      .def("GetPages", &This::GetPages, copyRefPolicy)
      .def("GetPrimvars", &This::GetPrimvars, copyRefPolicy)
      .def("GetAdditionalPrimvarProperties", &This::GetAdditionalPrimvarProperties, copyRefPolicy)
      .def("GetImplementationName", &This::GetImplementationName)
      .def("GetRole", &This::GetRole)
      .def("GetPropertyNamesForPage", &This::GetPropertyNamesForPage)
      .def("GetAllVstructNames", &This::GetAllVstructNames);
}
