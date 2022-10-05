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

#include "KLI_compiler_attrs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KLI_mempool;
struct MemArena;

typedef void (*LinkNodeFreeFP)(void *link);
typedef void (*LinkNodeApplyFP)(void *link, void *userdata);

typedef struct LinkNode
{
  struct LinkNode *next;
  void *link;
} LinkNode;

/**
 * Use for append (single linked list, storing the last element).
 *
 * \note list manipulation functions don't operate on this struct.
 * This is only to be used while appending.
 */
typedef struct LinkNodePair
{
  LinkNode *list, *last_node;
} LinkNodePair;

int KLI_linklist_count(const LinkNode *list) ATTR_WARN_UNUSED_RESULT;
int KLI_linklist_index(const LinkNode *list, void *ptr) ATTR_WARN_UNUSED_RESULT;

LinkNode *KLI_linklist_find(LinkNode *list, int index) ATTR_WARN_UNUSED_RESULT;
LinkNode *KLI_linklist_find_last(LinkNode *list) ATTR_WARN_UNUSED_RESULT;

void KLI_linklist_reverse(LinkNode **listp) ATTR_NONNULL(1);

/**
 * Move an item from its current position to a new one inside a single-linked list.
 * \note `*listp` may be modified.
 */
void KLI_linklist_move_item(LinkNode **listp, int curr_index, int new_index) ATTR_NONNULL(1);

/**
 * A version of #KLI_linklist_prepend that takes the allocated link.
 */
void KLI_linklist_prepend_nlink(LinkNode **listp, void *ptr, LinkNode *nlink) ATTR_NONNULL(1, 3);
void KLI_linklist_prepend(LinkNode **listp, void *ptr) ATTR_NONNULL(1);
void KLI_linklist_prepend_arena(LinkNode **listp, void *ptr, struct MemArena *ma)
  ATTR_NONNULL(1, 3);
void KLI_linklist_prepend_pool(LinkNode **listp, void *ptr, struct KLI_mempool *mempool)
  ATTR_NONNULL(1, 3);

/* Use #LinkNodePair to avoid full search. */

/**
 * A version of append that takes the allocated link.
 */
void KLI_linklist_append_nlink(LinkNodePair *list_pair, void *ptr, LinkNode *nlink)
  ATTR_NONNULL(1, 3);
void KLI_linklist_append(LinkNodePair *list_pair, void *ptr) ATTR_NONNULL(1);
void KLI_linklist_append_arena(LinkNodePair *list_pair, void *ptr, struct MemArena *ma)
  ATTR_NONNULL(1, 3);
void KLI_linklist_append_pool(LinkNodePair *list_pair, void *ptr, struct KLI_mempool *mempool)
  ATTR_NONNULL(1, 3);

void *KLI_linklist_pop(LinkNode **listp) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1);
void *KLI_linklist_pop_pool(LinkNode **listp, struct KLI_mempool *mempool) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL(1, 2);
void KLI_linklist_insert_after(LinkNode **listp, void *ptr) ATTR_NONNULL(1);

void KLI_linklist_free(LinkNode *list, LinkNodeFreeFP freefunc);
void KLI_linklist_freeN(LinkNode *list);
void KLI_linklist_free_pool(LinkNode *list, LinkNodeFreeFP freefunc, struct KLI_mempool *mempool);
void KLI_linklist_apply(LinkNode *list, LinkNodeApplyFP applyfunc, void *userdata);
LinkNode *KLI_linklist_sort(LinkNode *list,
                            int (*cmp)(const void *, const void *)) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL(2);
LinkNode *KLI_linklist_sort_r(LinkNode *list,
                              int (*cmp)(void *, const void *, const void *),
                              void *thunk) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(2);

#define KLI_linklist_prepend_alloca(listp, ptr) \
  KLI_linklist_prepend_nlink(listp, ptr, alloca(sizeof(LinkNode)))
#define KLI_linklist_append_alloca(list_pair, ptr) \
  KLI_linklist_append_nlink(list_pair, ptr, alloca(sizeof(LinkNode)))

#ifdef __cplusplus
}
#endif
