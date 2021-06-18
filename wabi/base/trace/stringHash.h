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

#ifndef WABI_BASE_TRACE_STRING_HASH_H
#define WABI_BASE_TRACE_STRING_HASH_H

#include "wabi/wabi.h"

#include <cstdint>

WABI_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
///
/// \class TraceStringHash
///
/// This class provides a function to compute compile time hashes for string
/// literals.
///
class TraceStringHash
{
 public:
  /// Computes a compile time hash of \p str.
  template<int N>
  static constexpr std::uint32_t Hash(const char (&str)[N])
  {
    return djb2HashStr<N - 1>(str);
  }

 private:
  // Recursive function computing the xor variant of the djb2 hash
  // function.
  template<int N>
  static constexpr std::uint32_t djb2HashStr(const char *str)
  {
    return (djb2HashStr<N - 1>(str) * 33) ^ str[N - 1];
  }
};

// Template recursion base case.
template<>
constexpr std::uint32_t TraceStringHash::djb2HashStr<0>(const char *str)
{
  return 5381;
}

WABI_NAMESPACE_END

#endif  // WABI_BASE_TRACE_STRING_HASH_H