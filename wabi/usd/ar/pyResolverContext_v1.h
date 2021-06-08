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
#ifndef INCLUDE_AR_PY_RESOLVER_CONTEXT
#  error This file should not be included directly. Include pyResolverContext.h instead
#endif

#ifndef WABI_USD_AR_PY_RESOLVER_CONTEXT_V1_H
#  define WABI_USD_AR_PY_RESOLVER_CONTEXT_V1_H

/// \file ar/pyResolverContext_v1.h
/// Macros for creating Python bindings for objects used with
/// ArResolverContext.

#  include "wabi/base/tf/pySafePython.h"

#  include <boost/python/extract.hpp>
#  include <boost/python/object.hpp>

#  include "wabi/usd/ar/api.h"
#  include "wabi/usd/ar/resolverContext.h"
#  include "wabi/wabi.h"

#  include "wabi/base/tf/pyLock.h"
#  include "wabi/base/tf/pyObjWrapper.h"

#  include <functional>

WABI_NAMESPACE_BEGIN

/// Register the specified type as a context object that may be
/// converted from a Python into a ArResolverContext object
/// in C++ and vice versa. This typically would be called in the
/// source file where the Python wrapping for the context object
/// is defined.
template<class Context> void ArWrapResolverContextForPython();

#  ifndef doxygen
// Private helper functions for converting ArResolverContext
// objects to and from Python.

template<class Context>
bool Ar_ConvertResolverContextFromPython(PyObject *obj, ArResolverContext *context)
{
  boost::python::extract<const Context &> x(obj);
  if (x.check()) {
    if (context) {
      *context = ArResolverContext(x());
    }
    return true;
  }
  return false;
}

template<class Context>
bool Ar_ConvertResolverContextToPython(const ArResolverContext &context, TfPyObjWrapper *obj)
{
  if (const Context *contextObj = context.Get<Context>()) {
    if (obj) {
      TfPyLock lock;
      *obj = boost::python::object(*contextObj);
    }
    return true;
  }
  return false;
}

typedef std::function<bool(PyObject *, ArResolverContext *)> Ar_MakeResolverContextFromPythonFn;
typedef std::function<bool(const ArResolverContext &, TfPyObjWrapper *)>
    Ar_ResolverContextToPythonFn;

AR_API
void Ar_RegisterResolverContextPythonConversion(
    const Ar_MakeResolverContextFromPythonFn &convertFunc,
    const Ar_ResolverContextToPythonFn &getObjectFunc);

AR_API
bool Ar_CanConvertResolverContextFromPython(PyObject *pyObj);

AR_API
ArResolverContext Ar_ConvertResolverContextFromPython(PyObject *pyObj);

AR_API
TfPyObjWrapper Ar_ConvertResolverContextToPython(const ArResolverContext &context);

template<class Context> void ArWrapResolverContextForPython()
{
  Ar_RegisterResolverContextPythonConversion(Ar_ConvertResolverContextFromPython<Context>,
                                             Ar_ConvertResolverContextToPython<Context>);
};

#  endif  // doxygen

WABI_NAMESPACE_END

#endif
