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

#ifdef __cplusplus
extern "C" {
#endif

/* Utility functions. */
void _KLI_assert_print_pos(const char *file, const int line, const char *function, const char *id);
void _KLI_assert_print_backtrace(void);
void _KLI_assert_abort(void);
void _KLI_assert_unreachable_print(const char *file, const int line, const char *function);

#ifdef _WIN32
#  include <stdio.h>
void KLI_system_backtrace(FILE *fp);
#endif

#ifdef _MSC_VER
#  include <crtdbg.h> /* for _STATIC_ASSERT */
#endif

#ifndef NDEBUG
#  if defined(__GNUC__)
#    define _KLI_ASSERT_PRINT_POS(a) _KLI_assert_print_pos(__FILE__, __LINE__, __func__, #    a)
#  elif defined(_MSC_VER)
#    define _KLI_ASSERT_PRINT_POS(a) _KLI_assert_print_pos(__FILE__, __LINE__, __func__, #    a)
#  else
#    define _KLI_ASSERT_PRINT_POS(a) _KLI_assert_print_pos(__FILE__, __LINE__, "<?>", #    a)
#  endif
#  ifdef WITH_ASSERT_ABORT
#    define _KLI_ASSERT_ABORT _KLI_assert_abort
#  else
#    define _KLI_ASSERT_ABORT() (void)0
#  endif
#  define KLI_assert(a)                              \
    (void)((!(a)) ? ((_KLI_assert_print_backtrace(), \
                      _KLI_ASSERT_PRINT_POS(a),      \
                      _KLI_ASSERT_ABORT(),           \
                      NULL)) :                       \
                    NULL)
#else
#  define KLI_assert(a) ((void)0)
#endif

#if defined(__cplusplus)
#  define KLI_STATIC_ASSERT(a, msg) static_assert(a, msg);
#elif defined(_MSC_VER)
#  if (_MSC_VER > 1910) && !defined(__clang__)
#    define KLI_STATIC_ASSERT(a, msg) static_assert(a, msg);
#  else
#    define KLI_STATIC_ASSERT(a, msg) _STATIC_ASSERT(a);
#  endif
#elif defined(__COVERITY__)
#  define KLI_STATIC_ASSERT(a, msg)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define KLI_STATIC_ASSERT(a, msg) _Static_assert(a, msg);
#else
#  define KLI_STATIC_ASSERT(a, msg)
#endif

#define KLI_STATIC_ASSERT_ALIGN(st, align) \
  KLI_STATIC_ASSERT((sizeof(st) % (align) == 0), "Structure must be strictly aligned")

#define KLI_assert_unreachable()                                   \
  {                                                                \
    _KLI_assert_unreachable_print(__FILE__, __LINE__, __func__);   \
    KLI_assert(!"This line of code is marked to be unreachable."); \
  }                                                                \
  ((void)0)

#ifdef __cplusplus
}
#endif
