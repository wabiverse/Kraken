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
#include <wabi/base/tf/stringUtils.h>

/* kraklib */
#include "KLI_compiler_attrs.h"

/* assert. */
#include "KLI_assert.h"

#ifndef doxygen

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

#  define MIN2(a, b)               \
    __extension__({                \
      typeof(a) a_ = (a);          \
      typeof(b) b_ = (b);          \
      ((a_) < (b_) ? (a_) : (b_)); \
    })

#  define MAX2(a, b)               \
    __extension__({                \
      typeof(a) a_ = (a);          \
      typeof(b) b_ = (b);          \
      ((a_) > (b_) ? (a_) : (b_)); \
    })

#  define MIN3(a, b, c)                                            \
    __extension__({                                                \
      typeof(a) a_ = (a);                                          \
      typeof(b) b_ = (b);                                          \
      typeof(c) c_ = (c);                                          \
      ((a_ < b_) ? ((a_ < c_) ? a_ : c_) : ((b_ < c_) ? b_ : c_)); \
    })

#  define MAX3(a, b, c)                                            \
    __extension__({                                                \
      typeof(a) a_ = (a);                                          \
      typeof(b) b_ = (b);                                          \
      typeof(c) c_ = (c);                                          \
      ((a_ > b_) ? ((a_ > c_) ? a_ : c_) : ((b_ > c_) ? b_ : c_)); \
    })

#  define MIN4(a, b, c, d)                                                       \
    __extension__({                                                              \
      typeof(a) a_ = (a);                                                        \
      typeof(b) b_ = (b);                                                        \
      typeof(c) c_ = (c);                                                        \
      typeof(d) d_ = (d);                                                        \
      ((a_ < b_) ? ((a_ < c_) ? ((a_ < d_) ? a_ : d_) : ((c_ < d_) ? c_ : d_)) : \
                   ((b_ < c_) ? ((b_ < d_) ? b_ : d_) : ((c_ < d_) ? c_ : d_))); \
    })

#  define MAX4(a, b, c, d)                                                       \
    __extension__({                                                              \
      typeof(a) a_ = (a);                                                        \
      typeof(b) b_ = (b);                                                        \
      typeof(c) c_ = (c);                                                        \
      typeof(d) d_ = (d);                                                        \
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

#define INIT_MINMAX(min, max)                  \
  {                                            \
    (min)[0] = (min)[1] = (min)[2] = 1.0e30f;  \
    (max)[0] = (max)[1] = (max)[2] = -1.0e30f; \
  }                                            \
  (void)0
#define INIT_MINMAX2(min, max)      \
  {                                 \
    (min)[0] = (min)[1] = 1.0e30f;  \
    (max)[0] = (max)[1] = -1.0e30f; \
  }                                 \
  (void)0
#define DO_MIN(vec, min)       \
  {                            \
    if ((min)[0] > (vec)[0]) { \
      (min)[0] = (vec)[0];     \
    }                          \
    if ((min)[1] > (vec)[1]) { \
      (min)[1] = (vec)[1];     \
    }                          \
    if ((min)[2] > (vec)[2]) { \
      (min)[2] = (vec)[2];     \
    }                          \
  }                            \
  (void)0
#define DO_MAX(vec, max)       \
  {                            \
    if ((max)[0] < (vec)[0]) { \
      (max)[0] = (vec)[0];     \
    }                          \
    if ((max)[1] < (vec)[1]) { \
      (max)[1] = (vec)[1];     \
    }                          \
    if ((max)[2] < (vec)[2]) { \
      (max)[2] = (vec)[2];     \
    }                          \
  }                            \
  (void)0
#define DO_MINMAX(vec, min, max) \
  {                              \
    if ((min)[0] > (vec)[0]) {   \
      (min)[0] = (vec)[0];       \
    }                            \
    if ((min)[1] > (vec)[1]) {   \
      (min)[1] = (vec)[1];       \
    }                            \
    if ((min)[2] > (vec)[2]) {   \
      (min)[2] = (vec)[2];       \
    }                            \
    if ((max)[0] < (vec)[0]) {   \
      (max)[0] = (vec)[0];       \
    }                            \
    if ((max)[1] < (vec)[1]) {   \
      (max)[1] = (vec)[1];       \
    }                            \
    if ((max)[2] < (vec)[2]) {   \
      (max)[2] = (vec)[2];       \
    }                            \
  }                              \
  (void)0
#define DO_MINMAX2(vec, min, max) \
  {                               \
    if ((min)[0] > (vec)[0]) {    \
      (min)[0] = (vec)[0];        \
    }                             \
    if ((min)[1] > (vec)[1]) {    \
      (min)[1] = (vec)[1];        \
    }                             \
    if ((max)[0] < (vec)[0]) {    \
      (max)[0] = (vec)[0];        \
    }                             \
    if ((max)[1] < (vec)[1]) {    \
      (max)[1] = (vec)[1];        \
    }                             \
  }                               \
  (void)0

/** \} */

/* -------------------------------------------------------------------- */
/** \name Clamp Macros
 * \{ */

#define CLAMPIS(a, b, c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

#define CLAMP(a, b, c)      \
  {                         \
    if ((a) < (b)) {        \
      (a) = (b);            \
    } else if ((a) > (c)) { \
      (a) = (c);            \
    }                       \
  }                         \
  (void)0

#define CLAMP_MAX(a, c) \
  {                     \
    if ((a) > (c)) {    \
      (a) = (c);        \
    }                   \
  }                     \
  (void)0

#define CLAMP_MIN(a, b) \
  {                     \
    if ((a) < (b)) {    \
      (a) = (b);        \
    }                   \
  }                     \
  (void)0

#define CLAMP2(vec, b, c)  \
  {                        \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
  }                        \
  (void)0

#define CLAMP2_MIN(vec, b)  \
  {                         \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
  }                         \
  (void)0

#define CLAMP2_MAX(vec, b)  \
  {                         \
    CLAMP_MAX((vec)[0], b); \
    CLAMP_MAX((vec)[1], b); \
  }                         \
  (void)0

#define CLAMP3(vec, b, c)  \
  {                        \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
    CLAMP((vec)[2], b, c); \
  }                        \
  (void)0

#define CLAMP3_MIN(vec, b)  \
  {                         \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
    CLAMP_MIN((vec)[2], b); \
  }                         \
  (void)0

#define CLAMP3_MAX(vec, b)  \
  {                         \
    CLAMP_MAX((vec)[0], b); \
    CLAMP_MAX((vec)[1], b); \
    CLAMP_MAX((vec)[2], b); \
  }                         \
  (void)0

#define CLAMP4(vec, b, c)  \
  {                        \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
    CLAMP((vec)[2], b, c); \
    CLAMP((vec)[3], b, c); \
  }                        \
  (void)0

#define CLAMP4_MIN(vec, b)  \
  {                         \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
    CLAMP_MIN((vec)[2], b); \
    CLAMP_MIN((vec)[3], b); \
  }                         \
  (void)0

#define CLAMP4_MAX(vec, b)  \
  {                         \
    CLAMP_MAX((vec)[0], b); \
    CLAMP_MAX((vec)[1], b); \
    CLAMP_MAX((vec)[2], b); \
    CLAMP_MAX((vec)[3], b); \
  }                         \
  (void)0

/** \} */

/**
 *  Macro to convert a value to string in the pre-processor: */
#define STRINGIFY_ARG(x) "" #x
#define STRINGIFY_APPEND(a, b) "" a #b
#define STRINGIFY(x) STRINGIFY_APPEND("", x)

/* assuming a static array */
#if defined(__GNUC__) && !defined(__cplusplus) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#  define ARRAY_SIZE(arr)                                                             \
    ((sizeof(struct { int isnt_array : ((const void *)&(arr) == &(arr)[0]); }) * 0) + \
     (sizeof(arr) / sizeof(*(arr))))
#else
#  define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
#endif

#define _VA_NARGS_GLUE(x, y) x y
#define _VA_NARGS_RETURN_COUNT(_1_,   \
                               _2_,   \
                               _3_,   \
                               _4_,   \
                               _5_,   \
                               _6_,   \
                               _7_,   \
                               _8_,   \
                               _9_,   \
                               _10_,  \
                               _11_,  \
                               _12_,  \
                               _13_,  \
                               _14_,  \
                               _15_,  \
                               _16_,  \
                               _17_,  \
                               _18_,  \
                               _19_,  \
                               _20_,  \
                               _21_,  \
                               _22_,  \
                               _23_,  \
                               _24_,  \
                               _25_,  \
                               _26_,  \
                               _27_,  \
                               _28_,  \
                               _29_,  \
                               _30_,  \
                               _31_,  \
                               _32_,  \
                               _33_,  \
                               _34_,  \
                               _35_,  \
                               _36_,  \
                               _37_,  \
                               _38_,  \
                               _39_,  \
                               _40_,  \
                               _41_,  \
                               _42_,  \
                               _43_,  \
                               _44_,  \
                               _45_,  \
                               _46_,  \
                               _47_,  \
                               _48_,  \
                               _49_,  \
                               _50_,  \
                               _51_,  \
                               _52_,  \
                               _53_,  \
                               _54_,  \
                               _55_,  \
                               _56_,  \
                               _57_,  \
                               _58_,  \
                               _59_,  \
                               _60_,  \
                               _61_,  \
                               _62_,  \
                               _63_,  \
                               _64_,  \
                               count, \
                               ...)   \
  count
#define _VA_NARGS_EXPAND(args) _VA_NARGS_RETURN_COUNT args
#define _VA_NARGS_OVERLOAD_MACRO2(name, count) name##count
#define _VA_NARGS_OVERLOAD_MACRO1(name, count) _VA_NARGS_OVERLOAD_MACRO2(name, count)
#define _VA_NARGS_OVERLOAD_MACRO(name, count) _VA_NARGS_OVERLOAD_MACRO1(name, count)
/* --- expose for re-use --- */
/* 64 args max */
#define VA_NARGS_COUNT(...)      \
  _VA_NARGS_EXPAND((__VA_ARGS__, \
                    64,          \
                    63,          \
                    62,          \
                    61,          \
                    60,          \
                    59,          \
                    58,          \
                    57,          \
                    56,          \
                    55,          \
                    54,          \
                    53,          \
                    52,          \
                    51,          \
                    50,          \
                    49,          \
                    48,          \
                    47,          \
                    46,          \
                    45,          \
                    44,          \
                    43,          \
                    42,          \
                    41,          \
                    40,          \
                    39,          \
                    38,          \
                    37,          \
                    36,          \
                    35,          \
                    34,          \
                    33,          \
                    32,          \
                    31,          \
                    30,          \
                    29,          \
                    28,          \
                    27,          \
                    26,          \
                    25,          \
                    24,          \
                    23,          \
                    22,          \
                    21,          \
                    20,          \
                    19,          \
                    18,          \
                    17,          \
                    16,          \
                    15,          \
                    14,          \
                    13,          \
                    12,          \
                    11,          \
                    10,          \
                    9,           \
                    8,           \
                    7,           \
                    6,           \
                    5,           \
                    4,           \
                    3,           \
                    2,           \
                    1,           \
                    0))
#define VA_NARGS_CALL_OVERLOAD(name, ...) \
  _VA_NARGS_GLUE(_VA_NARGS_OVERLOAD_MACRO(name, VA_NARGS_COUNT(__VA_ARGS__)), (__VA_ARGS__))

/* ARRAY_SET_ITEMS#(v, ...): set indices of array 'v' */
/* internal helpers */
#define _VA_ARRAY_SET_ITEMS2(v, a) ((v)[0] = (a))
#define _VA_ARRAY_SET_ITEMS3(v, a, b) \
  _VA_ARRAY_SET_ITEMS2(v, a);         \
  ((v)[1] = (b))
#define _VA_ARRAY_SET_ITEMS4(v, a, b, c) \
  _VA_ARRAY_SET_ITEMS3(v, a, b);         \
  ((v)[2] = (c))
#define _VA_ARRAY_SET_ITEMS5(v, a, b, c, d) \
  _VA_ARRAY_SET_ITEMS4(v, a, b, c);         \
  ((v)[3] = (d))
#define _VA_ARRAY_SET_ITEMS6(v, a, b, c, d, e) \
  _VA_ARRAY_SET_ITEMS5(v, a, b, c, d);         \
  ((v)[4] = (e))
#define _VA_ARRAY_SET_ITEMS7(v, a, b, c, d, e, f) \
  _VA_ARRAY_SET_ITEMS6(v, a, b, c, d, e);         \
  ((v)[5] = (f))
#define _VA_ARRAY_SET_ITEMS8(v, a, b, c, d, e, f, g) \
  _VA_ARRAY_SET_ITEMS7(v, a, b, c, d, e, f);         \
  ((v)[6] = (g))
#define _VA_ARRAY_SET_ITEMS9(v, a, b, c, d, e, f, g, h) \
  _VA_ARRAY_SET_ITEMS8(v, a, b, c, d, e, f, g);         \
  ((v)[7] = (h))
#define _VA_ARRAY_SET_ITEMS10(v, a, b, c, d, e, f, g, h, i) \
  _VA_ARRAY_SET_ITEMS9(v, a, b, c, d, e, f, g, h);          \
  ((v)[8] = (i))
#define _VA_ARRAY_SET_ITEMS11(v, a, b, c, d, e, f, g, h, i, j) \
  _VA_ARRAY_SET_ITEMS10(v, a, b, c, d, e, f, g, h, i);         \
  ((v)[9] = (j))
#define _VA_ARRAY_SET_ITEMS12(v, a, b, c, d, e, f, g, h, i, j, k) \
  _VA_ARRAY_SET_ITEMS11(v, a, b, c, d, e, f, g, h, i, j);         \
  ((v)[10] = (k))
#define _VA_ARRAY_SET_ITEMS13(v, a, b, c, d, e, f, g, h, i, j, k, l) \
  _VA_ARRAY_SET_ITEMS12(v, a, b, c, d, e, f, g, h, i, j, k);         \
  ((v)[11] = (l))
#define _VA_ARRAY_SET_ITEMS14(v, a, b, c, d, e, f, g, h, i, j, k, l, m) \
  _VA_ARRAY_SET_ITEMS13(v, a, b, c, d, e, f, g, h, i, j, k, l);         \
  ((v)[12] = (m))
#define _VA_ARRAY_SET_ITEMS15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
  _VA_ARRAY_SET_ITEMS14(v, a, b, c, d, e, f, g, h, i, j, k, l, m);         \
  ((v)[13] = (n))
#define _VA_ARRAY_SET_ITEMS16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
  _VA_ARRAY_SET_ITEMS15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n);         \
  ((v)[14] = (o))
#define _VA_ARRAY_SET_ITEMS17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
  _VA_ARRAY_SET_ITEMS16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);         \
  ((v)[15] = (p))

/* reusable ARRAY_SET_ITEMS macro */
#define ARRAY_SET_ITEMS(...)                                  \
  {                                                           \
    VA_NARGS_CALL_OVERLOAD(_VA_ARRAY_SET_ITEMS, __VA_ARGS__); \
  }                                                           \
  (void)0

/** \} */

#ifdef __GNUC__
#  define CHECK_TYPE(var, type) \
    {                           \
      typeof(var) *__tmp;       \
      __tmp = (type *)NULL;     \
      (void)__tmp;              \
    }                           \
    (void)0

#  define CHECK_TYPE_PAIR(var_a, var_b) \
    {                                   \
      const typeof(var_a) *__tmp;       \
      __tmp = (typeof(var_b) *)NULL;    \
      (void)__tmp;                      \
    }                                   \
    (void)0

#  define CHECK_TYPE_PAIR_INLINE(var_a, var_b) \
    ((void)({                                  \
      const typeof(var_a) *__tmp;              \
      __tmp = (typeof(var_b) *)NULL;           \
      (void)__tmp;                             \
    }))

#else
#  define CHECK_TYPE(var, type) \
    {                           \
      EXPR_NOP(var);            \
    }                           \
    (void)0
#  define CHECK_TYPE_PAIR(var_a, var_b)   \
    {                                     \
      (EXPR_NOP(var_a), EXPR_NOP(var_b)); \
    }                                     \
    (void)0
#  define CHECK_TYPE_PAIR_INLINE(var_a, var_b) (EXPR_NOP(var_a), EXPR_NOP(var_b))
#endif

/* -------------------------------------------------------------------- */
/** \name Swap/Shift Macros
 * \{ */

#define SWAP(type, a, b) \
  {                      \
    type sw_ap;          \
    CHECK_TYPE(a, type); \
    CHECK_TYPE(b, type); \
    sw_ap = (a);         \
    (a) = (b);           \
    (b) = sw_ap;         \
  }                      \
  (void)0

/* swap with a temp value */
#define SWAP_TVAL(tval, a, b) \
  {                           \
    CHECK_TYPE_PAIR(tval, a); \
    CHECK_TYPE_PAIR(tval, b); \
    (tval) = (a);             \
    (a) = (b);                \
    (b) = (tval);             \
  }                           \
  (void)0

/* shift around elements */
#define SHIFT3(type, a, b, c) \
  {                           \
    type tmp;                 \
    CHECK_TYPE(a, type);      \
    CHECK_TYPE(b, type);      \
    CHECK_TYPE(c, type);      \
    tmp = a;                  \
    a = c;                    \
    c = b;                    \
    b = tmp;                  \
  }                           \
  (void)0

#define SHIFT4(type, a, b, c, d) \
  {                              \
    type tmp;                    \
    CHECK_TYPE(a, type);         \
    CHECK_TYPE(b, type);         \
    CHECK_TYPE(c, type);         \
    CHECK_TYPE(d, type);         \
    tmp = a;                     \
    a = d;                       \
    d = c;                       \
    c = b;                       \
    b = tmp;                     \
  }                              \
  (void)0

/** No-op for expressions we don't want to instantiate, but must remain valid. */
#define EXPR_NOP(expr) (void)(0 ? ((void)(expr), 1) : 0)

/** \} */

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
#define TIP_(msgid) msgid
#define DATA_(msgid) msgid
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

#define STRINGALL(x) wabi::TfStringify(x)
#define CHARALL(x) wabi::TfStringify(x).c_str()

#define CONCAT(a, b) wabi::TfStringCatPaths(a, b).c_str()
#define STRCAT(a, b) wabi::TfStringCatPaths(a, b)

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
#  define setenv(x, y, z) _putenv(CHARALL(STRINGALL(x) + "=" + y))
#  define DebugOutput(w) OutputDebugString(TEXT(w))
#endif

#ifndef MAX_PATH
#  define MAX_PATH 260
#endif /* MAX_PATH */

#endif /* doxygen */