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

#ifndef WABI_BASE_TRACE_DYNAMIC_KEY_H
#define WABI_BASE_TRACE_DYNAMIC_KEY_H

#include "wabi/base/tf/token.h"
#include "wabi/base/trace/staticKeyData.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
/// \class TraceDynamicKey
///
/// This class stores data used to create dynamic keys which can be referenced
/// in TraceEvent instances.
///
/// Is a key is known at compile time, it is preferable to use
/// a static constexpr TraceStaticKeyData instance instead.
class TraceDynamicKey
{
 public:
  /// Constructor for TfToken.
  TraceDynamicKey(TfToken name)
    : _key(std::move(name))
  {
    _data._name = _key.GetText();
  }

  /// Constructor for string.
  TraceDynamicKey(const std::string &name)
    : _key(name)
  {
    _data._name = _key.GetText();
  }

  /// Constructor for C string.
  TraceDynamicKey(const char *name)
    : _key(name)
  {
    _data._name = _key.GetText();
  }

  /// Equality operator.
  bool operator==(const TraceDynamicKey &other) const
  {
    return _key == other._key;
  }

  /// Return a cached hash code for this key.
  size_t Hash() const
  {
    return _key.Hash();
  }

  /// A Hash functor which uses the cached hash which may be used to store
  /// keys in a TfHashMap.
  struct HashFunctor
  {
    size_t operator()(const TraceDynamicKey &key) const
    {
      return key.Hash();
    }
  };

  /// Returns a reference to TraceStaticKeyData.
  const TraceStaticKeyData &GetData() const
  {
    return _data;
  }

 private:
  TraceStaticKeyData _data;
  TfToken _key;
};

WABI_NAMESPACE_END

#endif  // WABI_BASE_TRACE_DYNAMIC_KEY_H