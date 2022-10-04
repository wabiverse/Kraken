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
 * @ingroup KRAKEN Library.
 * Gadget Vault.
 *
 * Shared logic for #KLI_task_parallel_mempool to create a threaded iterator,
 * without exposing the these functions publicly.
 */

#include "KLI_compiler_attrs.h"

#include "KLI_mempool.h"
#include "KLI_task.h"

typedef struct KLI_mempool_threadsafe_iter
{
  KLI_mempool_iter iter;
  struct KLI_mempool_chunk **curchunk_threaded_shared;
} KLI_mempool_threadsafe_iter;

typedef struct ParallelMempoolTaskData
{
  KLI_mempool_threadsafe_iter ts_iter;
  TaskParallelTLS tls;
} ParallelMempoolTaskData;

/**
 * Initialize an array of mempool iterators, #KLI_MEMPOOL_ALLOW_ITER flag must be set.
 *
 * This is used in threaded code, to generate as much iterators as needed
 * (each task should have its own),
 * such that each iterator goes over its own single chunk,
 * and only getting the next chunk to iterate over has to be
 * protected against concurrency (which can be done in a lock-less way).
 *
 * To be used when creating a task for each single item in the pool is totally overkill.
 *
 * See #KLI_task_parallel_mempool implementation for detailed usage example.
 */
ParallelMempoolTaskData *mempool_iter_threadsafe_create(KLI_mempool *pool,
                                                        size_t iter_num) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();
void mempool_iter_threadsafe_destroy(ParallelMempoolTaskData *iter_arr) ATTR_NONNULL();

/**
 * A version of #KLI_mempool_iterstep that uses
 * #KLI_mempool_threadsafe_iter.curchunk_threaded_shared for threaded iteration support.
 * (threaded section noted in comments).
 */
void *mempool_iter_threadsafe_step(KLI_mempool_threadsafe_iter *iter);

#ifdef __cplusplus
}
#endif
