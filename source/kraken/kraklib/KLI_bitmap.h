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
 */

#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int KLI_bitmap;

/* WARNING: the bitmap does not keep track of its own size or check
 * for out-of-bounds access */

/* internal use */
/* 2^5 = 32 (bits) */
#define _BITMAP_POWER 5
/* 0b11111 */
#define _BITMAP_MASK 31

/**
 * Number of blocks needed to hold '_num' bits.
 */
#define _BITMAP_NUM_BLOCKS(_num) (((_num) + _BITMAP_MASK) >> _BITMAP_POWER)

/**
 * Size (in bytes) used to hold '_num' bits.
 */
#define KLI_BITMAP_SIZE(_num) ((size_t)(_BITMAP_NUM_BLOCKS(_num)) * sizeof(KLI_bitmap))

/**
 * Allocate memory for a bitmap with '_num' bits; free with MEM_freeN().
 */
#define KLI_BITMAP_NEW(_num, _alloc_string) \
  ((KLI_bitmap *)MEM_callocN(KLI_BITMAP_SIZE(_num), _alloc_string))

/**
 * Allocate a bitmap on the stack.
 */
#define KLI_BITMAP_NEW_ALLOCA(_num) \
  ((KLI_bitmap *)memset(alloca(KLI_BITMAP_SIZE(_num)), 0, KLI_BITMAP_SIZE(_num)))

/**
 * Allocate using given MemArena.
 */
#define KLI_BITMAP_NEW_MEMARENA(_mem, _num) \
  (CHECK_TYPE_INLINE(_mem, MemArena *), \
   ((KLI_bitmap *)KLI_memarena_calloc(_mem, KLI_BITMAP_SIZE(_num))))

/**
 * Declares a bitmap as a variable.
 */
#define KLI_BITMAP_DECLARE(_name, _num) KLI_bitmap _name[_BITMAP_NUM_BLOCKS(_num)] = {}

/**
 * Get the value of a single bit at '_index'.
 */
#define KLI_BITMAP_TEST(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   ((_bitmap)[(_index) >> _BITMAP_POWER] & (1u << ((_index)&_BITMAP_MASK))))

#define KLI_BITMAP_TEST_AND_SET_ATOMIC(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   (atomic_fetch_and_or_uint32((uint32_t *)&(_bitmap)[(_index) >> _BITMAP_POWER], \
                               (1u << ((_index)&_BITMAP_MASK))) & \
    (1u << ((_index)&_BITMAP_MASK))))

#define KLI_BITMAP_TEST_BOOL(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   (KLI_BITMAP_TEST(_bitmap, _index) != 0))

/**
 * Set the value of a single bit at '_index'.
 */
#define KLI_BITMAP_ENABLE(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   ((_bitmap)[(_index) >> _BITMAP_POWER] |= (1u << ((_index)&_BITMAP_MASK))))

/**
 * Clear the value of a single bit at '_index'.
 */
#define KLI_BITMAP_DISABLE(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   ((_bitmap)[(_index) >> _BITMAP_POWER] &= ~(1u << ((_index)&_BITMAP_MASK))))

/**
 * Flip the value of a single bit at '_index'.
 */
#define KLI_BITMAP_FLIP(_bitmap, _index) \
  (CHECK_TYPE_ANY(_bitmap, KLI_bitmap *, const KLI_bitmap *), \
   ((_bitmap)[(_index) >> _BITMAP_POWER] ^= (1u << ((_index)&_BITMAP_MASK))))

/**
 * Set or clear the value of a single bit at '_index'.
 */
#define KLI_BITMAP_SET(_bitmap, _index, _set) \
  { \
    CHECK_TYPE(_bitmap, KLI_bitmap *); \
    if (_set) { \
      KLI_BITMAP_ENABLE(_bitmap, _index); \
    } \
    else { \
      KLI_BITMAP_DISABLE(_bitmap, _index); \
    } \
  } \
  (void)0

/**
 * Resize bitmap to have space for '_num' bits.
 */
#define KLI_BITMAP_RESIZE(_bitmap, _num) \
  { \
    CHECK_TYPE(_bitmap, KLI_bitmap *); \
    (_bitmap) = MEM_recallocN(_bitmap, KLI_BITMAP_SIZE(_num)); \
  } \
  (void)0

/**
 * Set or clear all bits in the bitmap.
 */
void KLI_bitmap_set_all(KLI_bitmap *bitmap, bool set, size_t bits);
/**
 * Invert all bits in the bitmap.
 */
void KLI_bitmap_flip_all(KLI_bitmap *bitmap, size_t bits);
/**
 * Copy all bits from one bitmap to another.
 */
void KLI_bitmap_copy_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits);
/**
 * Combine two bitmaps with boolean AND.
 */
void KLI_bitmap_and_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits);
/**
 * Combine two bitmaps with boolean OR.
 */
void KLI_bitmap_or_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits);

/**
 * Find index of the lowest unset bit.
 * Returns -1 if all the bits are set.
 */
int KLI_bitmap_find_first_unset(const KLI_bitmap *bitmap, size_t bits);

#ifdef __cplusplus
}
#endif
