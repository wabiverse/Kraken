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
#ifndef WABI_BASE_TF_OSTREAM_METHODS_H
#define WABI_BASE_TF_OSTREAM_METHODS_H

/// \file tf/ostreamMethods.h
/// \ingroup group_tf_DebuggingOutput
///
/// Handy ostream output for various lib/tf and STL containers.
///
/// These functions are useful when you need to quickly output various STL
/// containers.  The stream operators are only available if the contained
/// types have stream operators.
///
/// This facility should \e not be used to output data for later input: this
/// is essentially a "write-only" facility meant for diagnostics or
/// human-readable display; the formats described herein are subject to change
/// without notice.

#include "wabi/base/tf/hashmap.h"
#include "wabi/wabi.h"

#include <list>
#include <map>
#include <ostream>
#include <set>
#include <type_traits>
#include <vector>

#include <boost/type_traits/has_left_shift.hpp>

WABI_NAMESPACE_BEGIN

template<class T> constexpr bool Tf_IsOstreamable()
{
  return boost::has_left_shift<std::ostream &, /* << */ T, /* -> */ std::ostream &>::value;
}

WABI_NAMESPACE_END

// These operator<< overloads need to go in the std namespace for
// Koenig lookup to work.
namespace std {

/// Output an STL vector using [ ] as delimiters.
/// \ingroup group_tf_DebuggingOutput
template<class T>
typename std::enable_if<WABI_NS::Tf_IsOstreamable<T>(), std::ostream &>::type operator<<(
    std::ostream &out,
    const std::vector<T> &v)
{
  out << "[ ";
  for (auto const &obj : v)
    out << obj << " ";
  out << "]";

  return out;
}

/// Output an STL set using ( ) as delimiters.
/// \ingroup group_tf_DebuggingOutput
template<class T>
typename std::enable_if<WABI_NS::Tf_IsOstreamable<T>(), std::ostream &>::type operator<<(
    std::ostream &out,
    const std::set<T> &v)
{
  out << "( ";
  for (auto const &obj : v)
    out << obj << " ";
  out << ")";

  return out;
}

/// Output an STL list using { } as delimiters.
/// \ingroup group_tf_DebuggingOutput
template<class T>
typename std::enable_if<WABI_NS::Tf_IsOstreamable<T>(), std::ostream &>::type operator<<(
    std::ostream &out,
    const std::list<T> &l)
{
  out << "{ ";
  for (auto const &obj : l)
    out << obj << " ";
  out << "}";

  return out;
}

/// Output an TfHashMap using < > as delimiters.
/// \ingroup group_tf_DebuggingOutput
template<class K, class M, class H, class C, class A>
typename std::enable_if<WABI_NS::Tf_IsOstreamable<K>() && WABI_NS::Tf_IsOstreamable<M>(),
                        std::ostream &>::type
operator<<(std::ostream &out, const WABI_NS::TfHashMap<K, M, H, C, A> &h)
{
  out << "< ";
  for (auto const &p : h)
    out << "<" << p.first << ": " << p.second << "> ";
  out << ">";
  return out;
}

/// Output an STL map using < > as delimiters.
/// \ingroup group_tf_DebuggingOutput
template<class K, class M>
typename std::enable_if<WABI_NS::Tf_IsOstreamable<K>() && WABI_NS::Tf_IsOstreamable<M>(),
                        std::ostream &>::type
operator<<(std::ostream &out, const std::map<K, M> &h)
{
  out << "< ";
  for (auto const &p : h)
    out << "<" << p.first << ": " << p.second << "> ";
  out << ">";
  return out;
}

}  // namespace std

#endif  // WABI_BASE_TF_OSTREAM_METHODS_H
