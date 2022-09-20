/*
 * KrakenOS/Kraken.OS.Defines.hpp
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 *
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
 */

#ifndef KRAKEN_OS_DEFINES_HPP
#define KRAKEN_OS_DEFINES_HPP

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __APPLE__
/*
 * To define OS types on macOS, we
 * must use the official NextSTEP
 * definition macros. */
#  include <Foundation/NSDefines.hpp>

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#  define KRKN_EXPORT _NS_EXPORT
#  define KRKN_IMPORT
#  define KRKN_EXTERN_C extern "C"
#  define KRKN_API _NS_EXTERN
#  define KRKN_INLINE _NS_INLINE
#  define KRKN_PACKED(__Declaration__) __Declaration__ _NS_PACKED

#  define KRKN_CONST(type, name) _NS_CONST(type, name)
#  define KRKN_ENUM(type, name) _NS_ENUM(type, name)
#  define KRKN_OPTIONS(type, name) _NS_OPTIONS(type, name)

#  define _KRKN_CAST_TO_UINT(value) _NS_CAST_TO_UINT(value)
#  define KRKN_VALIDATE_SIZE(ns, name) _NS_VALIDATE_SIZE(ns, name)
#  define KRKN_VALIDATE_ENUM(ns, name) _NS_VALIDATE_ENUM(ns, name)

#elif defined(__linux__) && (defined(__GNUC__) && __GNUC__ >= 4 || defined(__clang__))

#  define KRKN_EXPORT __attribute__((visibility("default")))
#  define KRKN_IMPORT
#  define KRKN_EXTERN_C extern "C"
#  define KRKN_API KRKN_EXTERN_C __attribute__((visibility("default")))
#  define KRKN_INLINE inline __attribute__((always_inline))
#  define KRKN_PACKED(__Declaration__) __Declaration__ __attribute__((packed))

#  define KRKN_CONST(type, name) KRKN_API type const name
#  define KRKN_ENUM(type, name) enum name : type
#  define KRKN_OPTIONS(type, name) \
    using name = type;             \
    enum : name

#  define _KRKN_CAST_TO_UINT(value) static_cast<std::uintptr_t>(value)
#  define KRKN_VALIDATE_SIZE(ns, name) \
    static_assert(sizeof(ns::name) == sizeof(ns##name), "size mismatch " #ns "::" #name)
#  define KRKN_VALIDATE_ENUM(ns, name)                                          \
    static_assert(_KRKN_CAST_TO_UINT(ns::name) == _KRKN_CAST_TO_UINT(ns##name), \
                  "value mismatch " #ns "::" #name)

#elif (defined(_WIN32) || defined(_WIN64)) && \
  (defined(__GNUC__) && __GNUC__ >= 4 || defined(__clang__))

#  define KRKN_EXPORT __attribute__((dllexport))
#  define KRKN_IMPORT __attribute__((dllimport))
#  define KRKN_EXTERN_C extern "C"
#  define KRKN_API KRKN_EXTERN_C __attribute__(dllexport)
#  define KRKN_INLINE inline __attribute__((always_inline))
#  define KRKN_PACKED(__Declaration__) __Declaration__ __attribute__((packed))

#  define KRKN_CONST(type, name) KRKN_API type const name
#  define KRKN_ENUM(type, name) enum name : type
#  define KRKN_OPTIONS(type, name) \
    using name = type;             \
    enum : name

#  define _KRKN_CAST_TO_UINT(value) static_cast<std::uintptr_t>(value)
#  define KRKN_VALIDATE_SIZE(ns, name) \
    static_assert(sizeof(ns::name) == sizeof(ns##name), "size mismatch " #ns "::" #name)
#  define KRKN_VALIDATE_ENUM(ns, name)                                          \
    static_assert(_KRKN_CAST_TO_UINT(ns::name) == _KRKN_CAST_TO_UINT(ns##name), \
                  "value mismatch " #ns "::" #name)

#elif defined(_WIN32) || defined(_WIN64) /* MSVC */

#  define KRKN_EXPORT __declspec(dllexport)
#  define KRKN_IMPORT __declspec(dllimport)
#  define KRKN_EXTERN_C extern "C"
#  define KRKN_API KRKN_EXTERN_C __declspec(dllexport)
#  define KRKN_INLINE inline
#  define KRKN_PACKED(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))

#  define KRKN_CONST(type, name) KRKN_API type const name
#  define KRKN_ENUM(type, name) enum name : type
#  define KRKN_OPTIONS(type, name) \
    using name = type;             \
    enum : name

#  define _KRKN_CAST_TO_UINT(value) static_cast<std::uintptr_t>(value)
#  define KRKN_VALIDATE_SIZE(ns, name) \
    static_assert(sizeof(ns::name) == sizeof(ns##name), "size mismatch " #ns "::" #name)
#  define KRKN_VALIDATE_ENUM(ns, name)                                          \
    static_assert(_KRKN_CAST_TO_UINT(ns::name) == _KRKN_CAST_TO_UINT(ns##name), \
                  "value mismatch " #ns "::" #name)

#endif /* __APPLE__ */

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* KRAKEN_OS_DEFINES_HPP */
