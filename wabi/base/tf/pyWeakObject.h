//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef WABI_BASE_TF_PY_WEAK_OBJECT_H
#define WABI_BASE_TF_PY_WEAK_OBJECT_H

#include "wabi/wabi.h"

#include "wabi/base/tf/api.h"
#include "wabi/base/tf/pyIdentity.h"

#include "wabi/base/tf/hash.h"
#include "wabi/base/tf/singleton.h"
#include "wabi/base/tf/weakBase.h"
#include "wabi/base/tf/weakPtr.h"

#include <boost/python/handle.hpp>
#include <boost/python/object.hpp>

#include "wabi/base/tf/hashmap.h"

WABI_NAMESPACE_BEGIN

typedef TfWeakPtr<struct Tf_PyWeakObject> Tf_PyWeakObjectPtr;

// A weak pointable weak reference to a python object.
struct Tf_PyWeakObject : public TfWeakBase
{
 public:

  typedef Tf_PyWeakObject This;

  static Tf_PyWeakObjectPtr GetOrCreate(boost::python::object const &obj);
  boost::python::object GetObject() const;
  void Delete();

 private:

  explicit Tf_PyWeakObject(boost::python::object const &obj);

  boost::python::handle<> _weakRef;
};

WABI_NAMESPACE_END

#endif  // WABI_BASE_TF_PY_WEAK_OBJECT_H
