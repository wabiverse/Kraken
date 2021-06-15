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

#include "wabi/usd/ar/resolvedPath.h"

#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/stringUtils.h"

#include <boost/python/class.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/return_value_policy.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

static std::string _Repr(const ArResolvedPath &p)
{
  return TfStringPrintf("%sResolvedPath(%s)",
                        TF_PY_REPR_PREFIX.c_str(),
                        !p ? "" : TfStringPrintf("'%s'", p.GetPathString().c_str()).c_str());
}

static bool _NonZero(const ArResolvedPath &p)
{
  return static_cast<bool>(p);
}

void wrapResolvedPath()
{
  using This = ArResolvedPath;

  class_<This>("ResolvedPath")
    .def(init<>())
    .def(init<const std::string &>())

    .def(self == self)
    .def(self != self)
    .def(self < self)
    .def(self > self)
    .def(self <= self)
    .def(self >= self)

    .def(self == std::string())
    .def(self != std::string())
    .def(self < std::string())
    .def(self > std::string())
    .def(self <= std::string())
    .def(self >= std::string())

    .def(TfPyBoolBuiltinFuncName, _NonZero)
    .def("__hash__", &This::GetHash)
    .def("__repr__", &_Repr)
    .def("__str__", &This::GetPathString, return_value_policy<return_by_value>())

    .def("GetPathString", &This::GetPathString, return_value_policy<return_by_value>());

  implicitly_convertible<This, std::string>();
}
