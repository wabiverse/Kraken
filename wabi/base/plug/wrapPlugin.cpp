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

#include "wabi/base/js/converter.h"
#include "wabi/base/plug/plugin.h"
#include "wabi/base/tf/iterator.h"
#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyPtrHelpers.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/wabi.h"

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <string>

using namespace boost::python;
using std::string;
using std::vector;

WABI_NAMESPACE_USING

namespace
{

  static dict _ConvertDict(const JsObject &dictionary)
  {
    dict result;
    TF_FOR_ALL (i, dictionary) {
      const string &key = i->first;
      const JsValue &val = i->second;

      result[key] = JsConvertToContainerType<object, dict>(val);
    }
    return result;
  }

  static dict _GetMetadata(PlugPluginPtr plugin)
  {
    return _ConvertDict(plugin->GetMetadata());
  }

  static dict _GetMetadataForType(PlugPluginPtr plugin, const TfType &type)
  {
    return _ConvertDict(plugin->GetMetadataForType(type));
  }

}  // anonymous namespace

void wrapPlugin()
{
  typedef PlugPlugin This;
  typedef PlugPluginPtr ThisPtr;

  class_<This, ThisPtr, boost::noncopyable>("Plugin", no_init)
    .def(TfPyWeakPtr())
    .def("Load", &This::Load)

    .add_property("isLoaded", &This::IsLoaded)
    .add_property("isPythonModule", &This::IsPythonModule)
    .add_property("isResource", &This::IsResource)

    .add_property("metadata", _GetMetadata)

    .add_property("name", make_function(&This::GetName, return_value_policy<return_by_value>()))
    .add_property("path", make_function(&This::GetPath, return_value_policy<return_by_value>()))
    .add_property("resourcePath",
                  make_function(&This::GetResourcePath, return_value_policy<return_by_value>()))

    .def("GetMetadataForType", _GetMetadataForType)
    .def("DeclaresType", &This::DeclaresType, (arg("type"), arg("includeSubclasses") = false))

    .def("MakeResourcePath", &This::MakeResourcePath)
    .def("FindPluginResource", &This::FindPluginResource, (arg("path"), arg("verify") = true));

  // The call to JsConvertToContainerType in _ConvertDict creates
  // vectors of boost::python::objects for array values, so register
  // a converter that turns that vector into a Python list.
  boost::python::to_python_converter<std::vector<object>,
                                     TfPySequenceToPython<std::vector<object>>>();
}

TF_REFPTR_CONST_VOLATILE_GET(PlugPlugin)
