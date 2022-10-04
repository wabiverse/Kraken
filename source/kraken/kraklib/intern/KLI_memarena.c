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
 * @ingroup KRAKEN library.
 * Gadget Vault.
 *
 * @brief Efficient memory allocation for many small chunks.
 * @section aboutmemarena Memory Arena
 *
 * Memory arena's are commonly used when the program
 * needs to quickly allocate lots of little bits of data,
 * which are all freed at the same moment.
 *
 * @note Memory can't be freed during the arenas lifetime.
 */

#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_asan.h"
#include "KLI_memarena.h"
#include "KLI_strict_flags.h"
#include "KLI_utildefines.h"

#ifdef WITH_MEM_VALGRIND
#  include "valgrind/memcheck.h"
#else
#  define VALGRIND_CREATE_MEMPOOL(pool, rzB, is_zeroed) UNUSED_VARS(pool, rzB, is_zeroed)
#  define VALGRIND_DESTROY_MEMPOOL(pool) UNUSED_VARS(pool)
#  define VALGRIND_MEMPOOL_ALLOC(pool, addr, size) UNUSED_VARS(pool, addr, size)
#  define VALGRIND_MOVE_MEMPOOL(pool_a, pool_b) UNUSED_VARS(pool_a, pool_b)
#endif

struct MemBuf
{
  struct MemBuf *next;
  uchar data[0];
};

struct MemArena
{
  uchar *curbuf;
  const char *name;
  struct MemBuf *bufs;

  size_t bufsize, cursize;
  size_t align;

  bool use_calloc;
};

static void memarena_buf_free_all(struct MemBuf *mb)
{
  while (mb != NULL) {
    struct MemBuf *mb_next = mb->next;

    /* Unpoison memory because #MEM_freeN might overwrite it. */
    KLI_asan_unpoison(mb, (uint)MEM_allocN_len(mb));

    MEM_freeN(mb);
    mb = mb_next;
  }
}

MemArena *KLI_memarena_new(const size_t bufsize, const char *name)
{
  MemArena *ma = MEM_callocN(sizeof(*ma), "memarena");
  ma->bufsize = bufsize;
  ma->align = 8;
  ma->name = name;

  VALGRIND_CREATE_MEMPOOL(ma, 0, false);

  return ma;
}

void KLI_memarena_use_calloc(MemArena *ma)
{
  ma->use_calloc = 1;
}

void KLI_memarena_use_malloc(MemArena *ma)
{
  ma->use_calloc = 0;
}

void KLI_memarena_use_align(struct MemArena *ma, const size_t align)
{
  /* Align must be a power of two. */
  KLI_assert((align & (align - 1)) == 0);

  ma->align = align;
}

void KLI_memarena_free(MemArena *ma)
{
  memarena_buf_free_all(ma->bufs);

  VALGRIND_DESTROY_MEMPOOL(ma);

  MEM_freeN(ma);
}

/** Pad num up by \a amt (must be power of two). */
#define PADUP(num, amt) (((num) + ((amt)-1)) & ~((amt)-1))

/** Align alloc'ed memory (needed if `align > 8`). */
static void memarena_curbuf_align(MemArena *ma)
{
  uchar *tmp;

  tmp = (uchar *)PADUP((intptr_t)ma->curbuf, (int)ma->align);
  ma->cursize -= (size_t)(tmp - ma->curbuf);
  ma->curbuf = tmp;
}

void *KLI_memarena_alloc(MemArena *ma, size_t size)
{
  void *ptr;

  /* Ensure proper alignment by rounding size up to multiple of 8. */
  size = PADUP(size, ma->align);

  if (UNLIKELY(size > ma->cursize)) {
    if (size > ma->bufsize - (ma->align - 1)) {
      ma->cursize = PADUP(size + 1, ma->align);
    } else {
      ma->cursize = ma->bufsize;
    }

    struct MemBuf *mb = (ma->use_calloc ? MEM_callocN : MEM_mallocN)(sizeof(*mb) + ma->cursize,
                                                                     ma->name);
    ma->curbuf = mb->data;
    mb->next = ma->bufs;
    ma->bufs = mb;

    KLI_asan_poison(ma->curbuf, ma->cursize);

    memarena_curbuf_align(ma);
  }

  ptr = ma->curbuf;
  ma->curbuf += size;
  ma->cursize -= size;

  VALGRIND_MEMPOOL_ALLOC(ma, ptr, size);

  KLI_asan_unpoison(ptr, size);

  return ptr;
}

void *KLI_memarena_calloc(MemArena *ma, size_t size)
{
  void *ptr;

  /* No need to use this function call if we're calloc'ing by default. */
  KLI_assert(ma->use_calloc == false);

  ptr = KLI_memarena_alloc(ma, size);
  KLI_assert(ptr != NULL);
  memset(ptr, 0, size);

  return ptr;
}

void KLI_memarena_merge(MemArena *ma_dst, MemArena *ma_src)
{
  /* Memory arenas must be compatible. */
  KLI_assert(ma_dst != ma_src);
  KLI_assert(ma_dst->align == ma_src->align);
  KLI_assert(ma_dst->use_calloc == ma_src->use_calloc);
  KLI_assert(ma_dst->bufsize == ma_src->bufsize);

  if (ma_src->bufs == NULL) {
    return;
  }

  if (UNLIKELY(ma_dst->bufs == NULL)) {
    KLI_assert(ma_dst->curbuf == NULL);
    ma_dst->bufs = ma_src->bufs;
    ma_dst->curbuf = ma_src->curbuf;
    ma_dst->cursize = ma_src->cursize;
  } else {
    /* Keep the 'ma_dst->curbuf' for simplicity.
     * Insert buffers after the first. */
    if (ma_dst->bufs->next != NULL) {
      /* Loop over `ma_src` instead of `ma_dst` since it's likely the destination is larger
       * when used for accumulating from multiple sources. */
      struct MemBuf *mb_src = ma_src->bufs;
      mb_src = ma_src->bufs;
      while (mb_src && mb_src->next) {
        mb_src = mb_src->next;
      }
      mb_src->next = ma_dst->bufs->next;
    }
    ma_dst->bufs->next = ma_src->bufs;
  }

  ma_src->bufs = NULL;
  ma_src->curbuf = NULL;
  ma_src->cursize = 0;

  VALGRIND_MOVE_MEMPOOL(ma_src, ma_dst);
  VALGRIND_CREATE_MEMPOOL(ma_src, 0, false);
}

void KLI_memarena_clear(MemArena *ma)
{
  if (ma->bufs) {
    uchar *curbuf_prev;
    size_t curbuf_used;

    if (ma->bufs->next) {
      memarena_buf_free_all(ma->bufs->next);
      ma->bufs->next = NULL;
    }

    curbuf_prev = ma->curbuf;
    ma->curbuf = ma->bufs->data;
    memarena_curbuf_align(ma);

    /* restore to original size */
    curbuf_used = (size_t)(curbuf_prev - ma->curbuf);
    ma->cursize += curbuf_used;

    if (ma->use_calloc) {
      memset(ma->curbuf, 0, curbuf_used);
    }
    KLI_asan_poison(ma->curbuf, ma->cursize);
  }

  VALGRIND_DESTROY_MEMPOOL(ma);
  VALGRIND_CREATE_MEMPOOL(ma, 0, false);
}
