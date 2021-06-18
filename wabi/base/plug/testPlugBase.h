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
#ifndef WABI_BASE_PLUG_TEST_PLUG_BASE_H
#define WABI_BASE_PLUG_TEST_PLUG_BASE_H

#include "wabi/base/plug/api.h"
#include "wabi/base/tf/refBase.h"
#include "wabi/base/tf/stringUtils.h"
#include "wabi/base/tf/type.h"
#include "wabi/base/tf/weakBase.h"
#include "wabi/wabi.h"

#include <string>

WABI_NAMESPACE_BEGIN

template<int M>
class _TestPlugBase : public TfRefBase, public TfWeakBase
{
 public:
  typedef _TestPlugBase This;
  typedef TfRefPtr<This> RefPtr;
  typedef TfWeakPtr<This> Ptr;
  constexpr static int N = M;

  virtual ~_TestPlugBase()
  {}

  virtual std::string GetTypeName()
  {
    return TfType::Find(this).GetTypeName();
  }

  static RefPtr New()
  {
    return TfCreateRefPtr(new This());
  }

  PLUG_API
  static RefPtr Manufacture(const std::string &subclass);

 protected:
  _TestPlugBase()
  {}
};

template<int N>
class _TestPlugFactoryBase : public TfType::FactoryBase
{
 public:
  virtual TfRefPtr<_TestPlugBase<N>> New() const = 0;
};

template<typename T>
class _TestPlugFactory : public _TestPlugFactoryBase<T::N>
{
 public:
  virtual TfRefPtr<_TestPlugBase<T::N>> New() const
  {
    return T::New();
  }
};

typedef _TestPlugBase<1> _TestPlugBase1;
typedef _TestPlugBase<2> _TestPlugBase2;
typedef _TestPlugBase<3> _TestPlugBase3;
typedef _TestPlugBase<4> _TestPlugBase4;

WABI_NAMESPACE_END

#endif  // WABI_BASE_PLUG_TEST_PLUG_BASE_H
