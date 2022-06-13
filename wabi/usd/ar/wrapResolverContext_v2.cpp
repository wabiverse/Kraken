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
#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/list.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/python/object.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/tuple.hpp>

#include "wabi/usd/ar/resolverContext.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/stringUtils.h"

using namespace boost::python;

WABI_NAMESPACE_USING

namespace
{

  // Helper struct to allow implicit conversions from None or a
  // sequence of context objects to an Ar.ResolverContext object.
  struct Ar_ResolverContextFromPython
  {
    Ar_ResolverContextFromPython()
    {
      converter::registry::push_back(&_convertible,
                                     &_construct,
                                     boost::python::type_id<ArResolverContext>());
    }

    static void *_convertible(PyObject *obj_ptr)
    {
      object obj(handle<>(borrowed(obj_ptr)));
      if (obj.is_none() || extract<std::vector<ArResolverContext>>(obj).check()) {
        return obj_ptr;
      }
      return 0;
    }

    static void _construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
    {
      void *storage =
        ((converter::rvalue_from_python_storage<ArResolverContext> *)data)->storage.bytes;

      ArResolverContext context;

      object obj(handle<>(borrowed(obj_ptr)));
      if (!obj.is_none()) {
        context = ArResolverContext(extract<std::vector<ArResolverContext>>(obj)());
      }

      new (storage) ArResolverContext(context);
      data->convertible = storage;
    }
  };

}  // anonymous namespace

WABI_NAMESPACE_BEGIN

class Ar_ResolverContextPythonAccess
{
 public:

  static boost::python::list GetAsList(const ArResolverContext &ctx)
  {
    boost::python::list l;
    for (const auto &data : ctx._contexts) {
      l.append(data->GetPythonObj().Get());
    }
    return l;
  }

  static std::string GetRepr(const ArResolverContext &ctx)
  {
    std::vector<std::string> objReprs;
    for (const auto &data : ctx._contexts) {
      objReprs.push_back(TfPyObjectRepr(data->GetPythonObj().Get()));
    }
    return TF_PY_REPR_PREFIX +
           TfStringPrintf("ResolverContext(%s)", TfStringJoin(objReprs, ", ").c_str());
  }
};

WABI_NAMESPACE_END

static ArResolverContext *_Create(const boost::python::object &obj)
{
  extract<ArResolverContext> convertToContext(obj);
  return new ArResolverContext(convertToContext());
}

static size_t _Hash(const ArResolverContext &ctx)
{
  return hash_value(ctx);
}

static ArResolverContext _TestImplicitConversion(const ArResolverContext &ctx)
{
  return ctx;
}

void wrapResolverContext()
{
  Ar_ResolverContextFromPython();

  class_<ArResolverContext>("ResolverContext", no_init)
    .def(init<>())
    .def("__init__", make_constructor(&_Create))

    .def("IsEmpty", &ArResolverContext::IsEmpty)
    .def("Get", &Ar_ResolverContextPythonAccess::GetAsList)
    .def("GetDebugString", &ArResolverContext::GetDebugString)

    .def(self == self)
    .def(self != self)
    .def(self < self)

    .def("__hash__", &_Hash)
    .def("__repr__", &Ar_ResolverContextPythonAccess::GetRepr);

  TfPyContainerConversions::from_python_sequence<
    std::vector<ArResolverContext>,
    TfPyContainerConversions::variable_capacity_policy>();

  // Helper function for unit tests to exercise implicit conversion of
  // context objects into an ArResolverContext.
  def("_TestImplicitConversion", &_TestImplicitConversion);
}
