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

#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pySingleton.h"
#include "wabi/usd/gdr/geomNode.h"
#include "wabi/usd/gdr/registry.h"
#include "wabi/usd/ndr/discoveryPlugin.h"
#include "wabi/usd/ndr/registry.h"
#include "wabi/wabi.h"

#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

void wrapRegistry()
{
  typedef GdrRegistry This;
  typedef TfWeakPtr<GdrRegistry> ThisPtr;

  class_<std::vector<GdrGeomNodeConstPtr>>("GeomNodeList")
      .def(vector_indexing_suite<std::vector<GdrGeomNodeConstPtr>>());

  class_<This, ThisPtr, bases<NdrRegistry>, boost::noncopyable>("Registry", no_init)
      .def(TfPySingleton())
      .def("GetGeomNodeByIdentifier",
           &This::GetGeomNodeByIdentifier,
           (args("identifier"), args("typePriority") = NdrTokenVec()),
           return_internal_reference<>())

      .def("GetGeomNodeByIdentifierAndType",
           &This::GetGeomNodeByIdentifierAndType,
           (args("identifier"), args("nodeType")),
           return_internal_reference<>())

      .def("GetGeomNodeFromAsset",
           &This::GetGeomNodeFromAsset,
           (arg("geomAsset"),
            arg("metadata")      = NdrTokenMap(),
            arg("subIdentifier") = TfToken(),
            arg("sourceType")    = TfToken()),
           return_internal_reference<>())

      .def("GetGeomNodeFromSourceCode",
           &This::GetGeomNodeFromSourceCode,
           (arg("sourceCode"), arg("sourceType"), arg("metadata") = NdrTokenMap()),
           return_internal_reference<>())

      .def("GetGeomNodeByName",
           &This::GetGeomNodeByName,
           (args("name"),
            args("typePriority") = NdrTokenVec(),
            args("filter")       = NdrVersionFilterDefaultOnly),
           return_internal_reference<>())

      .def("GetGeomNodeByNameAndType",
           &This::GetGeomNodeByNameAndType,
           (args("name"), args("nodeType"), args("filter") = NdrVersionFilterDefaultOnly),
           return_internal_reference<>())

      .def("GetGeomNodesByIdentifier", &This::GetGeomNodesByIdentifier, (args("identifier")))

      .def("GetGeomNodesByName",
           &This::GetGeomNodesByName,
           (args("name"), args("filter") = NdrVersionFilterDefaultOnly))

      .def("GetGeomNodesByFamily",
           &This::GetGeomNodesByFamily,
           (args("family") = TfToken(), args("filter") = NdrVersionFilterDefaultOnly));
}
