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

typedef struct _GSQueue GSQueue;

GSQueue *KLI_gsqueue_new(size_t elem_size);
/**
 * Returns true if the queue is empty, false otherwise.
 */
bool KLI_gsqueue_is_empty(const GSQueue *queue);
size_t KLI_gsqueue_len(const GSQueue *queue);
/**
 * Retrieves and removes the first element from the queue.
 * The value is copies to @a r_item, which must be at least @a elem_size bytes.
 *
 * Does not reduce amount of allocated memory.
 */
void KLI_gsqueue_pop(GSQueue *queue, void *r_item);
/**
 * Copies the source value onto the end of the queue
 *
 * @note This copies #GSQueue.elem_size bytes from @a item,
 * (the pointer itself is not stored).
 *
 * @param item: source data to be copied to the queue.
 */
void KLI_gsqueue_push(GSQueue *queue, const void *item);
/**
 * Free the queue's data and the queue itself.
 */
void KLI_gsqueue_free(GSQueue *queue);

#ifdef __cplusplus
}
#endif
