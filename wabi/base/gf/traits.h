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
#ifndef WABI_BASE_GF_TRAITS_H
#define WABI_BASE_GF_TRAITS_H

#include "wabi/wabi.h"

#include <type_traits>

WABI_NAMESPACE_BEGIN

/// A metafunction with a static const bool member 'value' that is true for
/// GfVec types, like GfVec2i, GfVec4d, etc and false for all other types.
template<class T> struct GfIsGfVec {
  static const bool value = false;
};

/// A metafunction with a static const bool member 'value' that is true for
/// GfMatrix types, like GfMatrix3d, GfMatrix4f, etc and false for all other
/// types.
template<class T> struct GfIsGfMatrix {
  static const bool value = false;
};

/// A metafunction with a static const bool member 'value' that is true for
/// GfQuat types and false for all other types.
template<class T> struct GfIsGfQuat {
  static const bool value = false;
};

/// A metafunction with a static const bool member 'value' that is true for
/// GfRange types and false for all other types.
template<class T> struct GfIsGfRange {
  static const bool value = false;
};

/// A metafunction which is equivalent to std::is_floating_point but
/// allows for additional specialization for types like GfHalf
template<class T> struct GfIsFloatingPoint : public std::is_floating_point<T> {
};

/// A metafunction which is equivalent to std::arithmetic but
/// also includes any specializations from GfIsFloatingPoint (like GfHalf)
template<class T>
struct GfIsArithmetic
  : public std::integral_constant<bool, GfIsFloatingPoint<T>::value || std::is_arithmetic<T>::value> {
};

WABI_NAMESPACE_END

#endif  // WABI_BASE_GF_TRAITS_H
