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
 *
 * Utility functions for variable size bit-masks.
 */

#include <limits.h>
#include <string.h>

#include "KLI_bitmap.h"
#include "KLI_math_bits.h"
#include "KLI_utildefines.h"

void KLI_bitmap_set_all(KLI_bitmap *bitmap, bool set, size_t bits)
{
  memset(bitmap, set ? UCHAR_MAX : 0, KLI_BITMAP_SIZE(bits));
}

void KLI_bitmap_flip_all(KLI_bitmap *bitmap, size_t bits)
{
  size_t blocks_num = _BITMAP_NUM_BLOCKS(bits);
  for (size_t i = 0; i < blocks_num; i++) {
    bitmap[i] ^= ~(KLI_bitmap)0;
  }
}

void KLI_bitmap_copy_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits)
{
  memcpy(dst, src, KLI_BITMAP_SIZE(bits));
}

void KLI_bitmap_and_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits)
{
  size_t blocks_num = _BITMAP_NUM_BLOCKS(bits);
  for (size_t i = 0; i < blocks_num; i++) {
    dst[i] &= src[i];
  }
}

void KLI_bitmap_or_all(KLI_bitmap *dst, const KLI_bitmap *src, size_t bits)
{
  size_t blocks_num = _BITMAP_NUM_BLOCKS(bits);
  for (size_t i = 0; i < blocks_num; i++) {
    dst[i] |= src[i];
  }
}

int KLI_bitmap_find_first_unset(const KLI_bitmap *bitmap, const size_t bits)
{
  const size_t blocks_num = _BITMAP_NUM_BLOCKS(bits);
  int result = -1;
  /* Skip over completely set blocks. */
  int index = 0;
  while (index < blocks_num && bitmap[index] == ~0u) {
    index++;
  }
  if (index < blocks_num) {
    /* Found a partially used block: find the lowest unused bit. */
    const uint m = ~bitmap[index];
    KLI_assert(m != 0);
    const uint bit_index = bitscan_forward_uint(m);
    result = bit_index + (index << _BITMAP_POWER);
  }
  return result;
}
