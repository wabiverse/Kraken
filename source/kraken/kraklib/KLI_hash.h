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

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 *
 * This file contains code that can be shared between different hash table implementations.
 */

#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Jenkins Lookup3 Hash Functions.
 * Source: http://burtleburtle.net/bob/c/lookup3.c
 */

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))
#define final(a, b, c) \
  { \
    c ^= b; \
    c -= rot(b, 14); \
    a ^= c; \
    a -= rot(c, 11); \
    b ^= a; \
    b -= rot(a, 25); \
    c ^= b; \
    c -= rot(b, 16); \
    a ^= c; \
    a -= rot(c, 4); \
    b ^= a; \
    b -= rot(a, 14); \
    c ^= b; \
    c -= rot(b, 24); \
  } \
  ((void)0)

KLI_INLINE unsigned int KLI_hash_int_3d(unsigned int kx, unsigned int ky, unsigned int kz)
{
  unsigned int a, b, c;
  a = b = c = 0xdeadbeef + (3 << 2) + 13;

  c += kz;
  b += ky;
  a += kx;
  final(a, b, c);

  return c;
}

KLI_INLINE unsigned int KLI_hash_int_2d(unsigned int kx, unsigned int ky)
{
  unsigned int a, b, c;

  a = b = c = 0xdeadbeef + (2 << 2) + 13;
  a += kx;
  b += ky;

  final(a, b, c);

  return c;
}

#undef final
#undef rot

KLI_INLINE unsigned int KLI_hash_string(const char *str)
{
  unsigned int i = 0, c;

  while ((c = *str++)) {
    i = i * 37 + c;
  }
  return i;
}

KLI_INLINE float KLI_hash_int_2d_to_float(uint32_t kx, uint32_t ky)
{
  return (float)KLI_hash_int_2d(kx, ky) / (float)0xFFFFFFFFu;
}

KLI_INLINE float KLI_hash_int_3d_to_float(uint32_t kx, uint32_t ky, uint32_t kz)
{
  return (float)KLI_hash_int_3d(kx, ky, kz) / (float)0xFFFFFFFFu;
}

KLI_INLINE unsigned int KLI_hash_int(unsigned int k)
{
  return KLI_hash_int_2d(k, 0);
}

KLI_INLINE float KLI_hash_int_01(unsigned int k)
{
  return (float)KLI_hash_int(k) * (1.0f / (float)0xFFFFFFFF);
}

KLI_INLINE void KLI_hash_pointer_to_color(const void *ptr, int *r, int *g, int *b)
{
  size_t val = (size_t)ptr;
  const size_t hash_a = KLI_hash_int(val & 0x0000ffff);
  const size_t hash_b = KLI_hash_int((uint)((val & 0xffff0000) >> 16));
  const size_t hash = hash_a ^ (hash_b + 0x9e3779b9 + (hash_a << 6) + (hash_a >> 2));
  *r = (hash & 0xff0000) >> 16;
  *g = (hash & 0x00ff00) >> 8;
  *b = hash & 0x0000ff;
}

#ifdef __cplusplus
}
#endif
