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
#include "wabi/base/tf/makePyConstructor.h"
#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyFunction.h"
#include "wabi/base/tf/pyPtrHelpers.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/usd/ndr/declare.h"
#include "wabi/usd/ndr/discoveryPlugin.h"
#include "wabi/usd/ndr/filesystemDiscovery.h"
#include "wabi/wabi.h"

#include <boost/python.hpp>

using namespace boost::python;
WABI_NAMESPACE_USING
using namespace TfPyContainerConversions;

namespace
{

  static _NdrFilesystemDiscoveryPluginRefPtr New()
  {
    return TfCreateRefPtr(new _NdrFilesystemDiscoveryPlugin());
  }

  static _NdrFilesystemDiscoveryPluginRefPtr NewWithFilter(_NdrFilesystemDiscoveryPlugin::Filter filter)
  {
    return TfCreateRefPtr(new _NdrFilesystemDiscoveryPlugin(std::move(filter)));
  }

  // This is testing discovery from Python.  We need a discovery context
  // but Python can't normally create one.  We implement a dummy context
  // for just that purpose.
  class _Context : public NdrDiscoveryPluginContext
  {
   public:
    ~_Context() override = default;

    TfToken GetSourceType(const TfToken &discoveryType) const override
    {
      return discoveryType;
    }

    static TfRefPtr<_Context> New()
    {
      return TfCreateRefPtr(new _Context);
    }
  };

  void wrapFilesystemDiscoveryContext()
  {
    typedef _Context This;
    typedef TfWeakPtr<_Context> ThisPtr;

    class_<This, ThisPtr, bases<NdrDiscoveryPluginContext>, boost::noncopyable>("Context", no_init)
      .def(TfPyRefAndWeakPtr())
      .def(TfMakePyConstructor(This::New));
  }

}  // namespace

void wrapFilesystemDiscovery()
{
  typedef _NdrFilesystemDiscoveryPlugin This;
  typedef _NdrFilesystemDiscoveryPluginPtr ThisPtr;

  return_value_policy<copy_const_reference> copyRefPolicy;
  from_python_sequence<std::vector<ThisPtr>, variable_capacity_policy>();

  TfPyFunctionFromPython<bool(NdrNodeDiscoveryResult &)>();

  scope s = class_<This, ThisPtr, bases<NdrDiscoveryPlugin>, boost::noncopyable>(
              "_FilesystemDiscoveryPlugin",
              no_init)
              .def(TfPyRefAndWeakPtr())
              .def(TfMakePyConstructor(New))
              .def(TfMakePyConstructor(NewWithFilter))
              .def("DiscoverNodes", &This::DiscoverNodes, return_value_policy<TfPySequenceToList>())
              .def("GetSearchURIs", &This::GetSearchURIs, copyRefPolicy);

  wrapFilesystemDiscoveryContext();
}
