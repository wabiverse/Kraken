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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#pragma once

/* std */
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#  include <uchar.h>
#  include <stdbool.h>
typedef unsigned __int64 size_t;
#endif

/* wabi */
#include <wabi/wabi.h>

/* base */
#include <wabi/base/arch/export.h>

/* kraklib */
#include "KLI_compiler_attrs.h"

/* assert. */
#include "KLI_assert.h"

namespace fs = std::filesystem;

/* -------------------------------------------------------------------- */
/** \name Min/Max Macros
 * \{ */

/* useful for finding bad use of min/max */
#if 0
/* gcc only */
#  define _TYPECHECK(a, b) ((void)(((typeof(a) *)0) == ((typeof(b) *)0)))
#  define MIN2(x, y) (_TYPECHECK(x, y), (((x) < (y) ? (x) : (y))))
#  define MAX2(x, y) (_TYPECHECK(x, y), (((x) > (y) ? (x) : (y))))
#endif

/* min/max */
#if defined(__GNUC__) || defined(__clang__)

#  define MIN2(a, b) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      ((a_) < (b_) ? (a_) : (b_)); \
    })

#  define MAX2(a, b) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      ((a_) > (b_) ? (a_) : (b_)); \
    })

#  define MIN3(a, b, c) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      typeof(c) c_ = (c); \
      ((a_ < b_) ? ((a_ < c_) ? a_ : c_) : ((b_ < c_) ? b_ : c_)); \
    })

#  define MAX3(a, b, c) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      typeof(c) c_ = (c); \
      ((a_ > b_) ? ((a_ > c_) ? a_ : c_) : ((b_ > c_) ? b_ : c_)); \
    })

#  define MIN4(a, b, c, d) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      typeof(c) c_ = (c); \
      typeof(d) d_ = (d); \
      ((a_ < b_) ? ((a_ < c_) ? ((a_ < d_) ? a_ : d_) : ((c_ < d_) ? c_ : d_)) : \
                   ((b_ < c_) ? ((b_ < d_) ? b_ : d_) : ((c_ < d_) ? c_ : d_))); \
    })

#  define MAX4(a, b, c, d) \
    __extension__({ \
      typeof(a) a_ = (a); \
      typeof(b) b_ = (b); \
      typeof(c) c_ = (c); \
      typeof(d) d_ = (d); \
      ((a_ > b_) ? ((a_ > c_) ? ((a_ > d_) ? a_ : d_) : ((c_ > d_) ? c_ : d_)) : \
                   ((b_ > c_) ? ((b_ > d_) ? b_ : d_) : ((c_ > d_) ? c_ : d_))); \
    })

#else
#  define MIN2(a, b) ((a) < (b) ? (a) : (b))
#  define MAX2(a, b) ((a) > (b) ? (a) : (b))

#  define MIN3(a, b, c) (MIN2(MIN2((a), (b)), (c)))
#  define MIN4(a, b, c, d) (MIN2(MIN2((a), (b)), MIN2((c), (d))))

#  define MAX3(a, b, c) (MAX2(MAX2((a), (b)), (c)))
#  define MAX4(a, b, c, d) (MAX2(MAX2((a), (b)), MAX2((c), (d))))
#endif

/* min/max that return a value of our choice */
#define MAX3_PAIR(cmp_a, cmp_b, cmp_c, ret_a, ret_b, ret_c) \
  ((cmp_a > cmp_b) ? ((cmp_a > cmp_c) ? ret_a : ret_c) : ((cmp_b > cmp_c) ? ret_b : ret_c))

#define MIN3_PAIR(cmp_a, cmp_b, cmp_c, ret_a, ret_b, ret_c) \
  ((cmp_a < cmp_b) ? ((cmp_a < cmp_c) ? ret_a : ret_c) : ((cmp_b < cmp_c) ? ret_b : ret_c))

#define INIT_MINMAX(min, max) \
  { \
    (min)[0] = (min)[1] = (min)[2] = 1.0e30f; \
    (max)[0] = (max)[1] = (max)[2] = -1.0e30f; \
  } \
  (void)0
#define INIT_MINMAX2(min, max) \
  { \
    (min)[0] = (min)[1] = 1.0e30f; \
    (max)[0] = (max)[1] = -1.0e30f; \
  } \
  (void)0
#define DO_MIN(vec, min) \
  { \
    if ((min)[0] > (vec)[0]) { \
      (min)[0] = (vec)[0]; \
    } \
    if ((min)[1] > (vec)[1]) { \
      (min)[1] = (vec)[1]; \
    } \
    if ((min)[2] > (vec)[2]) { \
      (min)[2] = (vec)[2]; \
    } \
  } \
  (void)0
#define DO_MAX(vec, max) \
  { \
    if ((max)[0] < (vec)[0]) { \
      (max)[0] = (vec)[0]; \
    } \
    if ((max)[1] < (vec)[1]) { \
      (max)[1] = (vec)[1]; \
    } \
    if ((max)[2] < (vec)[2]) { \
      (max)[2] = (vec)[2]; \
    } \
  } \
  (void)0
#define DO_MINMAX(vec, min, max) \
  { \
    if ((min)[0] > (vec)[0]) { \
      (min)[0] = (vec)[0]; \
    } \
    if ((min)[1] > (vec)[1]) { \
      (min)[1] = (vec)[1]; \
    } \
    if ((min)[2] > (vec)[2]) { \
      (min)[2] = (vec)[2]; \
    } \
    if ((max)[0] < (vec)[0]) { \
      (max)[0] = (vec)[0]; \
    } \
    if ((max)[1] < (vec)[1]) { \
      (max)[1] = (vec)[1]; \
    } \
    if ((max)[2] < (vec)[2]) { \
      (max)[2] = (vec)[2]; \
    } \
  } \
  (void)0
#define DO_MINMAX2(vec, min, max) \
  { \
    if ((min)[0] > (vec)[0]) { \
      (min)[0] = (vec)[0]; \
    } \
    if ((min)[1] > (vec)[1]) { \
      (min)[1] = (vec)[1]; \
    } \
    if ((max)[0] < (vec)[0]) { \
      (max)[0] = (vec)[0]; \
    } \
    if ((max)[1] < (vec)[1]) { \
      (max)[1] = (vec)[1]; \
    } \
  } \
  (void)0

/** \} */

/**
 *  Macro to convert a value to string in the pre-processor: */
#define STRINGIFY_ARG(x) "" #x
#define STRINGIFY_APPEND(a, b) "" a #b
#define STRINGIFY(x) STRINGIFY_APPEND("", x)

/* assuming a static array */
#if defined(__GNUC__) && !defined(__cplusplus) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#  define ARRAY_SIZE(arr) \
    ((sizeof(struct { int isnt_array : ((const void *)&(arr) == &(arr)[0]); }) * 0) + \
     (sizeof(arr) / sizeof(*(arr))))
#else
#  define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
#endif

/**
 * UNUSED macro, for function argument */
#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_##x
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_##x
#else
#  define UNUSED_FUNCTION(x) UNUSED_##x
#endif

#define IFACE_(msgid) msgid
#define N_(msgid) msgid

/* hint to mark function arguments expected to be non-null
 * if no arguments are given to the macro, all of pointer
 * arguments would be expected to be non-null
 */
#ifdef __GNUC__
#  define ATTR_NONNULL(args...) __attribute__((nonnull(args)))
#else
#  define ATTR_NONNULL(...)
#endif

/* never returns NULL */
#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 409 /* gcc4.9+ only */
#  define ATTR_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#  define ATTR_RETURNS_NONNULL
#endif

/* hint to mark function as it wouldn't return */
#if defined(__GNUC__) || defined(__clang__)
#  define ATTR_NORETURN __attribute__((noreturn))
#else
#  define ATTR_NORETURN
#endif

/* hint to treat any non-null function return value cannot alias any other pointer */
#if (defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 403))
#  define ATTR_MALLOC __attribute__((malloc))
#else
#  define ATTR_MALLOC
#endif

/* ensures a NULL terminating argument as the n'th last argument of a variadic function */
#ifdef __GNUC__
#  define ATTR_SENTINEL(arg_pos) __attribute__((sentinel(arg_pos)))
#else
#  define ATTR_SENTINEL(arg_pos)
#endif

/* hint to compiler that function uses printf-style format string */
#ifdef __GNUC__
#  define ATTR_PRINTF_FORMAT(format_param, dots_param) \
    __attribute__((format(printf, format_param, dots_param)))
#else
#  define ATTR_PRINTF_FORMAT(format_param, dots_param)
#endif

#define FIND_TOKEN(i) (TfToken::Find(i))

#define STRINGALL(x) TfStringify(x)
#define CHARALL(x) TfStringify(x).c_str()

#define CONCAT(a, b) TfStringCatPaths(a, b).c_str()
#define STRCAT(a, b) TfStringCatPaths(a, b)

#define CHARSTR(a) a.c_str()

/* -------------------------------------------------------------------- */
/** \name Pointer Macros
 * \{ */

#if defined(__GNUC__) || defined(__clang__)
#  define POINTER_OFFSET(v, ofs) ((typeof(v))((char *)(v) + (ofs)))
#else
#  define POINTER_OFFSET(v, ofs) ((void *)((char *)(v) + (ofs)))
#endif

/* Warning-free macros for storing ints in pointers. Use these _only_
 * for storing an int in a pointer, not a pointer in an int (64bit)! */
#define POINTER_FROM_INT(i) ((void *)(intptr_t)(i))
#define POINTER_AS_INT(i) ((void)0, ((int)(intptr_t)(i)))

#define POINTER_FROM_UINT(i) ((void *)(uintptr_t)(i))
#define POINTER_AS_UINT(i) ((void)0, ((unsigned int)(uintptr_t)(i)))

/** \} */

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;

/* clang-format off */
#define VALUE_ZERO 0
#define ARRAY_ZERO {0}
#define POINTER_ZERO nullptr
#define EMPTY
/* clang-format on */

#ifdef _WIN32
#define setenv(x, y, z) _putenv(CHARALL(STRINGALL(x) + "=" + y))
#endif