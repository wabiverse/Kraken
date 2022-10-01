/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include <algorithm>
#include <cmath>
#include <type_traits>

#include "KLI_math_base_safe.h"
#include "KLI_utildefines.h"

namespace kraken::math {

template<typename T> inline constexpr bool is_math_float_type = std::is_floating_point_v<T>;
template<typename T> inline constexpr bool is_math_integral_type = std::is_integral_v<T>;

template<typename T> inline bool is_zero(const T &a)
{
  return a == T(0);
}

template<typename T> inline bool is_any_zero(const T &a)
{
  return is_zero(a);
}

template<typename T> inline T abs(const T &a)
{
  return std::abs(a);
}

template<typename T> inline T min(const T &a, const T &b)
{
  return std::min(a, b);
}

template<typename T> inline T max(const T &a, const T &b)
{
  return std::max(a, b);
}

template<typename T> inline void max_inplace(T &a, const T &b)
{
  a = math::max(a, b);
}

template<typename T> inline void min_inplace(T &a, const T &b)
{
  a = math::min(a, b);
}

template<typename T> inline T clamp(const T &a, const T &min, const T &max)
{
  return std::clamp(a, min, max);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))> inline T mod(const T &a, const T &b)
{
  return std::fmod(a, b);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))>
inline T safe_mod(const T &a, const T &b)
{
  return (b != 0) ? std::fmod(a, b) : 0;
}

template<typename T> inline void min_max(const T &value, T &min, T &max)
{
  min = math::min(value, min);
  max = math::max(value, max);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))>
inline T safe_divide(const T &a, const T &b)
{
  return (b != 0) ? a / b : T(0.0f);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))> inline T floor(const T &a)
{
  return std::floor(a);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))> inline T ceil(const T &a)
{
  return std::ceil(a);
}

template<typename T> inline T distance(const T &a, const T &b)
{
  return std::abs(a - b);
}

template<typename T, KLI_ENABLE_IF((is_math_float_type<T>))> inline T fract(const T &a)
{
  return a - std::floor(a);
}

template<typename T,
         typename FactorT,
         KLI_ENABLE_IF((std::is_arithmetic_v<T>)),
         KLI_ENABLE_IF((is_math_float_type<FactorT>))>
inline T interpolate(const T &a, const T &b, const FactorT &t)
{
  auto result = a * (1 - t) + b * t;
  if constexpr (std::is_integral_v<T> && std::is_floating_point_v<FactorT>) {
    result = std::round(result);
  }
  return result;
}

template<typename T> inline T midpoint(const T &a, const T &b)
{
  auto result = (a + b) * T(0.5);
  if constexpr (std::is_integral_v<T>) {
    result = std::round(result);
  }
  return result;
}

}  // namespace kraken::math
