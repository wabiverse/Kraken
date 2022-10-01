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

#include "KLI_compiler_attrs.h"
#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KLI_mempool;
struct KLI_mempool_chunk;

typedef struct KLI_mempool KLI_mempool;

KLI_mempool *KLI_mempool_create(unsigned int esize,
                                unsigned int elem_num,
                                unsigned int pchunk,
                                unsigned int flag)
  ATTR_MALLOC ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
void *KLI_mempool_alloc(KLI_mempool *pool) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL
  ATTR_NONNULL(1);
void *KLI_mempool_calloc(KLI_mempool *pool)
  ATTR_MALLOC ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL ATTR_NONNULL(1);
/**
 * Free an element from the mempool.
 *
 * \note doesn't protect against double frees, take care!
 */
void KLI_mempool_free(KLI_mempool *pool, void *addr) ATTR_NONNULL(1, 2);
/**
 * Empty the pool, as if it were just created.
 *
 * \param pool: The pool to clear.
 * \param totelem_reserve: Optionally reserve how many items should be kept from clearing.
 */
void KLI_mempool_clear_ex(KLI_mempool *pool, int totelem_reserve) ATTR_NONNULL(1);
/**
 * Wrap #KLI_mempool_clear_ex with no reserve set.
 */
void KLI_mempool_clear(KLI_mempool *pool) ATTR_NONNULL(1);
/**
 * Free the mempool itself (and all elements).
 */
void KLI_mempool_destroy(KLI_mempool *pool) ATTR_NONNULL(1);
int KLI_mempool_len(const KLI_mempool *pool) ATTR_NONNULL(1);
void *KLI_mempool_findelem(KLI_mempool *pool, unsigned int index) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL(1);

/**
 * Fill in \a data with pointers to each element of the mempool,
 * to create lookup table.
 *
 * \param pool: Pool to create a table from.
 * \param data: array of pointers at least the size of 'pool->totused'
 */
void KLI_mempool_as_table(KLI_mempool *pool, void **data) ATTR_NONNULL(1, 2);
/**
 * A version of #KLI_mempool_as_table that allocates and returns the data.
 */
void **KLI_mempool_as_tableN(KLI_mempool *pool,
                             const char *allocstr) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL(1, 2);
/**
 * Fill in \a data with the contents of the mempool.
 */
void KLI_mempool_as_array(KLI_mempool *pool, void *data) ATTR_NONNULL(1, 2);
/**
 * A version of #KLI_mempool_as_array that allocates and returns the data.
 */
void *KLI_mempool_as_arrayN(KLI_mempool *pool,
                            const char *allocstr) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL(1, 2);

#ifndef NDEBUG
void KLI_mempool_set_memory_debug(void);
#endif

/**
 * Iteration stuff.
 * \note this may easy to produce bugs with.
 */

/**  \note Private structure. */
typedef struct KLI_mempool_iter
{
  KLI_mempool *pool;
  struct KLI_mempool_chunk *curchunk;
  unsigned int curindex;
} KLI_mempool_iter;

/** #KLI_mempool.flag */
enum
{
  KLI_MEMPOOL_NOP = 0,
  /** allow iterating on this mempool.
   *
   * \note this requires that the first four bytes of the elements
   * never begin with 'free' (#FREEWORD).
   * \note order of iteration is only assured to be the
   * order of allocation when no chunks have been freed.
   */
  KLI_MEMPOOL_ALLOW_ITER = (1 << 0),
};

/**
 * Initialize a new mempool iterator, #KLI_MEMPOOL_ALLOW_ITER flag must be set.
 */
void KLI_mempool_iternew(KLI_mempool *pool, KLI_mempool_iter *iter) ATTR_NONNULL();
/**
 * Step over the iterator, returning the mempool item or NULL.
 */
void *KLI_mempool_iterstep(KLI_mempool_iter *iter) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

#ifdef __cplusplus
}
#endif
