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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#pragma once

#if defined(_MSC_VER)
#  define alloca _alloca
#endif

#if (defined(__GNUC__) || defined(__clang__)) && defined(__cplusplus)
extern "C++" {
/** Some magic to be sure we don't have reference in the type. */
template<typename T> static inline T decltype_helper(T x)
{
  return x;
}
#  define typeof(x) decltype(decltype_helper(x))
}
#endif

/* little macro so inline keyword works */
#if defined(_MSC_VER)
#  define KLI_INLINE static __forceinline
#else
#  define KLI_INLINE static inline __attribute__((always_inline)) __attribute__((__unused__))
#endif

#if defined(__GNUC__)
#  define KLI_NOINLINE __attribute__((noinline))
#else
#  define KLI_NOINLINE
#endif

#define STREQ(a, b) (strcmp(a, b) == 0)
#define STRCASEEQ(a, b) (strcasecmp(a, b) == 0)
#define STREQLEN(a, b, n) (strncmp(a, b, n) == 0)
#define STRCASEEQLEN(a, b, n) (strncasecmp(a, b, n) == 0)

#define STRPREFIX(a, b) (strncmp((a), (b), strlen(b)) == 0)
