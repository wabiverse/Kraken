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

/**
 * @file
 * @ingroup KRAKEN Library.
 * Gadget Vault.
 *
 * Helper functions and implementations of standard data types for #RHash
 * (not its implementation).
 */

#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_rhash.h" /* own include */
#include "KLI_hash_mm2a.h"
#include "KLI_utildefines.h"

/* keep last */
#include "KLI_strict_flags.h"

/* -------------------------------------------------------------------- */
/** \name Generic Key Hash & Comparison Functions
 * \{ */

#if 0
/* works but slower */
uint KLI_rhashutil_ptrhash(const void *key)
{
  return (uint)(intptr_t)key;
}
#else
uint KLI_rhashutil_ptrhash(const void *key)
{
  /* Based Python3.7's pointer hashing function. */

  size_t y = (size_t)key;
  /* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
   * excessive hash collisions for dicts and sets */

  /* NOTE: Unlike Python 'sizeof(uint)' is used instead of 'sizeof(void *)',
   * Otherwise casting to 'uint' ignores the upper bits on 64bit platforms. */
  return (uint)(y >> 4) | ((uint)y << (sizeof(uint[8]) - 4));
}
#endif
bool KLI_rhashutil_ptrcmp(const void *a, const void *b)
{
  return (a != b);
}

uint KLI_rhashutil_uinthash_v4(const uint key[4])
{
  uint hash;
  hash = key[0];
  hash *= 37;
  hash += key[1];
  hash *= 37;
  hash += key[2];
  hash *= 37;
  hash += key[3];
  return hash;
}

uint KLI_rhashutil_uinthash_v4_murmur(const uint key[4])
{
  return KLI_hash_mm2((const uchar *)key, sizeof(int[4]) /* sizeof(key) */, 0);
}

bool KLI_rhashutil_uinthash_v4_cmp(const void *a, const void *b)
{
  return (memcmp(a, b, sizeof(uint[4])) != 0);
}

uint KLI_rhashutil_uinthash(uint key)
{
  key += ~(key << 16);
  key ^= (key >> 5);
  key += (key << 3);
  key ^= (key >> 13);
  key += ~(key << 9);
  key ^= (key >> 17);

  return key;
}

uint KLI_rhashutil_inthash_p(const void *ptr)
{
  uintptr_t key = (uintptr_t)ptr;

  key += ~(key << 16);
  key ^= (key >> 5);
  key += (key << 3);
  key ^= (key >> 13);
  key += ~(key << 9);
  key ^= (key >> 17);

  return (uint)(key & 0xffffffff);
}

uint KLI_rhashutil_inthash_p_murmur(const void *ptr)
{
  uintptr_t key = (uintptr_t)ptr;

  return KLI_hash_mm2((const uchar *)&key, sizeof(key), 0);
}

uint KLI_rhashutil_inthash_p_simple(const void *ptr)
{
  return POINTER_AS_UINT(ptr);
}

bool KLI_rhashutil_intcmp(const void *a, const void *b)
{
  return (a != b);
}

size_t KLI_rhashutil_combine_hash(size_t hash_a, size_t hash_b)
{
  return hash_a ^ (hash_b + 0x9e3779b9 + (hash_a << 6) + (hash_a >> 2));
}

uint KLI_rhashutil_strhash_n(const char *key, size_t n)
{
  const signed char *p;
  uint h = 5381;

  for (p = (const signed char *)key; n-- && *p != '\0'; p++) {
    h = (uint)((h << 5) + h) + (uint)*p;
  }

  return h;
}
uint KLI_rhashutil_strhash_p(const void *ptr)
{
  const signed char *p;
  uint h = 5381;

  for (p = ptr; *p != '\0'; p++) {
    h = (uint)((h << 5) + h) + (uint)*p;
  }

  return h;
}
uint KLI_rhashutil_strhash_p_murmur(const void *ptr)
{
  const uchar *key = ptr;

  return KLI_hash_mm2(key, strlen((const char *)key) + 1, 0);
}
bool KLI_rhashutil_strcmp(const void *a, const void *b)
{
  return (a == b) ? false : !STREQ(a, b);
}

RHashPair *KLI_rhashutil_pairalloc(const void *first, const void *second)
{
  RHashPair *pair = MEM_mallocN(sizeof(RHashPair), "RHashPair");
  pair->first = first;
  pair->second = second;
  return pair;
}

uint KLI_rhashutil_pairhash(const void *ptr)
{
  const RHashPair *pair = ptr;
  uint hash = KLI_rhashutil_ptrhash(pair->first);
  return hash ^ KLI_rhashutil_ptrhash(pair->second);
}

bool KLI_rhashutil_paircmp(const void *a, const void *b)
{
  const RHashPair *A = a;
  const RHashPair *B = b;

  return ((A->first != B->first) || (A->second != B->second));
}

void KLI_rhashutil_pairfree(void *ptr)
{
  MEM_freeN(ptr);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Convenience RHash Creation Functions
 * \{ */

RHash *KLI_rhash_ptr_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rhash_new_ex(KLI_rhashutil_ptrhash, KLI_rhashutil_ptrcmp, info, nentries_reserve);
}
RHash *KLI_rhash_ptr_new(const char *info)
{
  return KLI_rhash_ptr_new_ex(info, 0);
}

RHash *KLI_rhash_str_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rhash_new_ex(KLI_rhashutil_strhash_p, KLI_rhashutil_strcmp, info, nentries_reserve);
}
RHash *KLI_rhash_str_new(const char *info)
{
  return KLI_rhash_str_new_ex(info, 0);
}

RHash *KLI_rhash_int_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rhash_new_ex(KLI_rhashutil_inthash_p, KLI_rhashutil_intcmp, info, nentries_reserve);
}
RHash *KLI_rhash_int_new(const char *info)
{
  return KLI_rhash_int_new_ex(info, 0);
}

RHash *KLI_rhash_pair_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rhash_new_ex(KLI_rhashutil_pairhash, KLI_rhashutil_paircmp, info, nentries_reserve);
}
RHash *KLI_rhash_pair_new(const char *info)
{
  return KLI_rhash_pair_new_ex(info, 0);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Convenience RSet Creation Functions
 * \{ */

RSet *KLI_rset_ptr_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rset_new_ex(KLI_rhashutil_ptrhash, KLI_rhashutil_ptrcmp, info, nentries_reserve);
}
RSet *KLI_rset_ptr_new(const char *info)
{
  return KLI_rset_ptr_new_ex(info, 0);
}

RSet *KLI_rset_str_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rset_new_ex(KLI_rhashutil_strhash_p, KLI_rhashutil_strcmp, info, nentries_reserve);
}
RSet *KLI_rset_str_new(const char *info)
{
  return KLI_rset_str_new_ex(info, 0);
}

RSet *KLI_rset_pair_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rset_new_ex(KLI_rhashutil_pairhash, KLI_rhashutil_paircmp, info, nentries_reserve);
}
RSet *KLI_rset_pair_new(const char *info)
{
  return KLI_rset_pair_new_ex(info, 0);
}

RSet *KLI_rset_int_new_ex(const char *info, const uint nentries_reserve)
{
  return KLI_rset_new_ex(KLI_rhashutil_inthash_p, KLI_rhashutil_intcmp, info, nentries_reserve);
}
RSet *KLI_rset_int_new(const char *info)
{
  return KLI_rset_int_new_ex(info, 0);
}

/** \} */
