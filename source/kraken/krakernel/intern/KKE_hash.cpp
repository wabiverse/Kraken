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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_sys_types.h" /* for intptr_t support */
#include "KLI_utildefines.h"

#define RHASH_INTERNAL_API
#include "KKE_utils.h"

#include "KLI_strict_flags.h"

KRAKEN_NAMESPACE_BEGIN

#ifdef __BIG_ENDIAN__
/* Big Endian */
#  define MAKE_ID(a, b, c, d) ((int)(a) << 24 | (int)(b) << 16 | (c) << 8 | (d))
#  define MAKE_ID_8(a, b, c, d, e, f, g, h)                                              \
    ((int64_t)(a) << 56 | (int64_t)(b) << 48 | (int64_t)(c) << 40 | (int64_t)(d) << 32 | \
     (int64_t)(e) << 24 | (int64_t)(f) << 16 | (int64_t)(g) << 8 | (h))
#else
/* Little Endian */
#  define MAKE_ID(a, b, c, d) ((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))
#  define MAKE_ID_8(a, b, c, d, e, f, g, h)                                              \
    ((int64_t)(h) << 56 | (int64_t)(g) << 48 | (int64_t)(f) << 40 | (int64_t)(e) << 32 | \
     (int64_t)(d) << 24 | (int64_t)(c) << 16 | (int64_t)(b) << 8 | (a))
#endif

/**
 * Important that this value is an is _not_  aligned with `sizeof(void *)`.
 * So having a pointer to 2/4/8... aligned memory is enough to ensure
 * the `freeword` will never be used.
 * To be safe, use a word that's the same in both directions.
 */
#define FREEWORD                                                                            \
  ((sizeof(void *) > sizeof(int32_t)) ? MAKE_ID_8('e', 'e', 'r', 'f', 'f', 'r', 'e', 'e') : \
                                        MAKE_ID('e', 'f', 'f', 'e'))

/**
 * The 'used' word just needs to be set to something besides FREEWORD.
 */
#define USEDWORD MAKE_ID('u', 's', 'e', 'd')

/* optimize pool size */
#define USE_CHUNK_POW2

#ifndef NDEBUG
static bool mempool_debug_memset = false;
#endif

/** #KKE_mempool.flag */
enum
{
  KKE_MEMPOOL_NOP = 0,
  /** allow iterating on this mempool.
   *
   * \note this requires that the first four bytes of the elements
   * never begin with 'free' (#FREEWORD).
   * \note order of iteration is only assured to be the
   * order of allocation when no chunks have been freed.
   */
  KKE_MEMPOOL_ALLOW_ITER = (1 << 0),
};

struct KKE_freenode
{
  struct KKE_freenode *next;
  /** Used to identify this as a freed node. */
  intptr_t freeword;
};

struct KKE_mempool_chunk
{
  struct KKE_mempool_chunk *next;
};

/**
 * The mempool, stores and tracks memory \a chunks and elements within those chunks \a free.
 */
struct KKE_mempool
{
  /** Single linked list of allocated chunks. */
  KKE_mempool_chunk *chunks;
  /** Keep a pointer to the last, so we can append new chunks there
   * this is needed for iteration so we can loop over chunks in the order added. */
  KKE_mempool_chunk *chunk_tail;

  /** Element size in bytes. */
  uint esize;
  /** Chunk size in bytes. */
  uint csize;
  /** Number of elements per chunk. */
  uint pchunk;
  uint flag;
  /* keeps aligned to 16 bits */

  /** Free element list. Interleaved into chunk data. */
  KKE_freenode *free;
  /** Use to know how many chunks to keep for #KKE_mempool_clear. */
  uint maxchunks;
  /** Number of elements currently in use. */
  uint totused;
};

/* overhead for lockfree allocator (use to avoid slop-space) */
#define MEM_SIZE_OVERHEAD sizeof(size_t)
#define MEM_SIZE_OPTIMAL(size) ((size)-MEM_SIZE_OVERHEAD)

#define MEMPOOL_ELEM_SIZE_MIN (sizeof(void *) * 2)

#define CHUNK_DATA(chunk) \
  (CHECK_TYPE_INLINE(chunk, KKE_mempool_chunk *), (KKE_freenode *)((chunk) + 1))

#define NODE_STEP_NEXT(node) ((KKE_freenode *)((char *)(node) + esize))
#define NODE_STEP_PREV(node) ((KKE_freenode *)((char *)(node)-esize))

/** Extra bytes implicitly used for every chunk alloc. */
#define CHUNK_OVERHEAD (uint)(MEM_SIZE_OVERHEAD + sizeof(KKE_mempool_chunk))

static uint power_of_2_max_u(uint x)
{
  x -= 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}

KLI_INLINE KKE_mempool_chunk *mempool_chunk_find(KKE_mempool_chunk *head, uint index)
{
  while (index-- && head) {
    head = head->next;
  }
  return head;
}

/**
 * @return the number of chunks to allocate based on how many elements are needed.
 *
 * @note for small pools 1 is a good default, the elements need to be initialized,
 * adding overhead on creation which is redundant if they aren't used.
 */
KLI_INLINE uint mempool_maxchunks(const uint elem_num, const uint pchunk)
{
  return (elem_num <= pchunk) ? 1 : ((elem_num / pchunk) + 1);
}

static KKE_mempool_chunk *mempool_chunk_alloc(KKE_mempool *pool)
{
  return static_cast<KKE_mempool_chunk *>(MEM_mallocN(sizeof(KKE_mempool_chunk) + (size_t)pool->csize, "KKE_Mempool Chunk"));
}

/**
 * Initialize a chunk and add into \a pool->chunks
 *
 * \param pool: The pool to add the chunk into.
 * \param mpchunk: The new uninitialized chunk (can be malloc'd)
 * \param last_tail: The last element of the previous chunk
 * (used when building free chunks initially)
 * \return The last chunk,
 */
static KKE_freenode *mempool_chunk_add(KKE_mempool *pool,
                                       KKE_mempool_chunk *mpchunk,
                                       KKE_freenode *last_tail)
{
  const uint esize = pool->esize;
  KKE_freenode *curnode = CHUNK_DATA(mpchunk);
  uint j;

  /* append */
  if (pool->chunk_tail) {
    pool->chunk_tail->next = mpchunk;
  } else {
    KLI_assert(pool->chunks == NULL);
    pool->chunks = mpchunk;
  }

  mpchunk->next = NULL;
  pool->chunk_tail = mpchunk;

  if (ARCH_UNLIKELY(pool->free == NULL)) {
    pool->free = curnode;
  }

  /* loop through the allocated data, building the pointer structures */
  j = pool->pchunk;
  if (pool->flag & KKE_MEMPOOL_ALLOW_ITER) {
    while (j--) {
      curnode->next = NODE_STEP_NEXT(curnode);
      curnode->freeword = FREEWORD;
      curnode = curnode->next;
    }
  } else {
    while (j--) {
      curnode->next = NODE_STEP_NEXT(curnode);
      curnode = curnode->next;
    }
  }

  /* terminate the list (rewind one)
   * will be overwritten if 'curnode' gets passed in again as 'last_tail' */
  curnode = NODE_STEP_PREV(curnode);
  curnode->next = NULL;

#ifdef USE_TOTALLOC
  pool->totalloc += pool->pchunk;
#endif

  /* final pointer in the previously allocated chunk is wrong */
  if (last_tail) {
    last_tail->next = CHUNK_DATA(mpchunk);
  }

  return curnode;
}


KKE_mempool *KKE_mempool_create(uint esize, uint elem_num, uint pchunk, uint flag)
{
  KKE_mempool *pool;
  KKE_freenode *last_tail = NULL;
  uint i, maxchunks;

  /* allocate the pool structure */
  pool = static_cast<KKE_mempool *>(MEM_mallocN(sizeof(KKE_mempool), "memory pool"));

  /* set the elem size */
  if (esize < (int)MEMPOOL_ELEM_SIZE_MIN) {
    esize = (int)MEMPOOL_ELEM_SIZE_MIN;
  }

  if (flag & KKE_MEMPOOL_ALLOW_ITER) {
    esize = MAX2(esize, (uint)sizeof(KKE_freenode));
  }

  maxchunks = mempool_maxchunks(elem_num, pchunk);

  pool->chunks = NULL;
  pool->chunk_tail = NULL;
  pool->esize = esize;

  /* Optimize chunk size to powers of 2, accounting for slop-space. */
  {
    KLI_assert(power_of_2_max_u(pchunk * esize) > CHUNK_OVERHEAD);
    pchunk = (power_of_2_max_u(pchunk * esize) - CHUNK_OVERHEAD) / esize;
  }

  pool->csize = esize * pchunk;

  /* Ensure this is a power of 2, minus the rounding by element size. */
#if !defined(NDEBUG)
  {
    uint final_size = (uint)MEM_SIZE_OVERHEAD + (uint)sizeof(KKE_mempool_chunk) + pool->csize;
    KLI_assert(((uint)power_of_2_max_u(final_size) - final_size) < pool->esize);
  }
#endif

  pool->pchunk = pchunk;
  pool->flag = flag;
  pool->free = NULL; /* mempool_chunk_add assigns */
  pool->maxchunks = maxchunks;
#ifdef USE_TOTALLOC
  pool->totalloc = 0;
#endif
  pool->totused = 0;

  if (elem_num) {
    /* Allocate the actual chunks. */
    for (i = 0; i < maxchunks; i++) {
      KKE_mempool_chunk *mpchunk = mempool_chunk_alloc(pool);
      last_tail = mempool_chunk_add(pool, mpchunk, last_tail);
    }
  }

#ifdef WITH_MEM_VALGRIND
  VALGRIND_CREATE_MEMPOOL(pool, 0, false);
#endif

  return pool;
}

void *KKE_mempool_alloc(KKE_mempool *pool)
{
  KKE_freenode *free_pop;

  if (ARCH_UNLIKELY(pool->free == NULL)) {
    /* Need to allocate a new chunk. */
    KKE_mempool_chunk *mpchunk = mempool_chunk_alloc(pool);
    mempool_chunk_add(pool, mpchunk, NULL);
  }

  free_pop = pool->free;

  KLI_assert(pool->chunk_tail->next == NULL);

  if (pool->flag & KKE_MEMPOOL_ALLOW_ITER) {
    free_pop->freeword = USEDWORD;
  }

  pool->free = free_pop->next;
  pool->totused++;

#ifdef WITH_MEM_VALGRIND
  VALGRIND_MEMPOOL_ALLOC(pool, free_pop, pool->esize);
#endif

  return (void *)free_pop;
}


/**
 * Next prime after `2^n` (skipping 2 & 3).
 *
 * \note Also used by: `KKE_edgehash` & `KKE_smallhash`.
 */
extern const uint KKE_rhash_hash_sizes[]; /* Quiet warning, this is only used by smallhash.c */
const uint KKE_rhash_hash_sizes[] = {
  5,       11,      17,      37,      67,       131,      257,      521,       1031,
  2053,    4099,    8209,    16411,   32771,    65537,    131101,   262147,    524309,
  1048583, 2097169, 4194319, 8388617, 16777259, 33554467, 67108879, 134217757, 268435459,
};
#define hashsizes KKE_rhash_hash_sizes

#define RHASH_MAX_SIZE 27
KLI_STATIC_ASSERT(ARRAY_SIZE(hashsizes) == RHASH_MAX_SIZE, "Invalid 'hashsizes' size");

#define RHASH_LIMIT_GROW(_nbkt) (((_nbkt)*3) / 4)
#define RHASH_LIMIT_SHRINK(_nbkt) (((_nbkt)*3) / 16)

struct Entry
{
  struct Entry *next;

  void *key;
  TfToken &token;
};

struct RHashEntry
{
  Entry e;

  void *val;
};

typedef Entry RSetEntry;

#define RHASH_ENTRY_SIZE(_is_rset) ((_is_rset) ? sizeof(RSetEntry) : sizeof(RHashEntry))

/* -------------------------------------------------------------------- */
/** \name Internal Utility API
 * \{ */

KLI_INLINE void rhash_entry_copy(RHash *gh_dst,
                                 Entry *dst,
                                 const RHash *gh_src,
                                 const Entry *src,
                                 RHashKeyCopyFP keycopyfp,
                                 RHashValCopyFP valcopyfp)
{
  dst->key = (keycopyfp) ? keycopyfp(src->key) : src->key;

  if ((gh_dst->flag & RHASH_FLAG_IS_RSET) == 0) {
    if ((gh_src->flag & RHASH_FLAG_IS_RSET) == 0) {
      ((RHashEntry *)dst)->val = (valcopyfp) ? valcopyfp(((RHashEntry *)src)->val) :
                                               ((RHashEntry *)src)->val;
    } else {
      ((RHashEntry *)dst)->val = NULL;
    }
  }
}

/**
 * Get the full hash for a token.
 */
KLI_INLINE uint rhash_keyhash(const RHash *rh, const TfToken &key)
{
  return rh->hashtfp(key);
}

/**
 * Get the full hash for a key.
 */
KLI_INLINE uint rhash_keyhash(const RHash *rh, const void *key)
{
  return rh->hashfp(key);
}

/**
 * Get the full hash for an entry.
 */
KLI_INLINE uint rhash_entryhash(const RHash *rh, const Entry *e)
{
  return rh->hashfp(e->key);
}

/**
 * Get the bucket-index for an already-computed full hash.
 */
KLI_INLINE uint rhash_bucket_index(const RHash *rh, const uint hash)
{
  return hash % rh->nbuckets;
}

/**
 * Internal lookup function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE Entry *rhash_lookup_entry_ex(const RHash *rh, const TfToken &key, const uint bucket_index)
{
  Entry *e;
  /* If we do not store RHash, not worth computing it for each entry here!
   * Typically, comparison function will be quicker, and since it's needed in the end anyway... */
  for (e = rh->buckets[bucket_index]; e; e = e->next) {
    if (ARCH_UNLIKELY(rh->cmptfp(key, e->token) == false)) {
      return e;
    }
  }

  return NULL;
}

/**
 * Internal lookup function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE Entry *rhash_lookup_entry_ex(const RHash *rh, const void *key, const uint bucket_index)
{
  Entry *e;
  /* If we do not store RHash, not worth computing it for each entry here!
   * Typically, comparison function will be quicker, and since it's needed in the end anyway... */
  for (e = rh->buckets[bucket_index]; e; e = e->next) {
    if (ARCH_UNLIKELY(rh->cmpfp(key, e->key) == false)) {
      return e;
    }
  }

  return NULL;
}

/**
 * Internal lookup function. Only wraps #rhash_lookup_entry_ex
 */
KLI_INLINE Entry *rhash_lookup_entry(const RHash *rh, const TfToken &key)
{
  const uint hash = rhash_keyhash(rh, key);
  const uint bucket_index = rhash_bucket_index(rh, hash);
  return rhash_lookup_entry_ex(rh, key, bucket_index);
}

/**
 * Internal lookup function. Only wraps #rhash_lookup_entry_ex
 */
KLI_INLINE Entry *rhash_lookup_entry(const RHash *rh, const void *key)
{
  const uint hash = rhash_keyhash(rh, key);
  const uint bucket_index = rhash_bucket_index(rh, hash);
  return rhash_lookup_entry_ex(rh, key, bucket_index);
}

bool KKE_rhash_haskey(const RHash *rh, const TfToken &key)
{
  return (rhash_lookup_entry(rh, key) != NULL);
}

bool KKE_rhash_haskey(const RHash *rh, const void *key)
{
  return (rhash_lookup_entry(rh, key) != NULL);
}

/**
 * Expand buckets to the next size up or down.
 */
static void rhash_buckets_resize(RHash *rh, const uint nbuckets)
{
  Entry **buckets_old = rh->buckets;
  Entry **buckets_new;
  const uint nbuckets_old = rh->nbuckets;
  uint i;

  KLI_assert((rh->nbuckets != nbuckets) || !rh->buckets);
  //  printf("%s: %d -> %d\n", __func__, nbuckets_old, nbuckets);

  rh->nbuckets = nbuckets;

  buckets_new = (Entry **)MEM_callocN(sizeof(*rh->buckets) * rh->nbuckets, __func__);

  if (buckets_old) {
    if (nbuckets > nbuckets_old) {
      for (i = 0; i < nbuckets_old; i++) {
        for (Entry *e = buckets_old[i], *e_next; e; e = e_next) {
          const uint hash = rhash_entryhash(rh, e);
          const uint bucket_index = rhash_bucket_index(rh, hash);
          e_next = e->next;
          e->next = buckets_new[bucket_index];
          buckets_new[bucket_index] = e;
        }
      }
    } else {
      for (i = 0; i < nbuckets_old; i++) {
        for (Entry *e = buckets_old[i], *e_next; e; e = e_next) {
          const uint hash = rhash_entryhash(rh, e);
          const uint bucket_index = rhash_bucket_index(rh, hash);
          e_next = e->next;
          e->next = buckets_new[bucket_index];
          buckets_new[bucket_index] = e;
        }
      }
    }
  }

  rh->buckets = buckets_new;
  if (buckets_old) {
    MEM_freeN(buckets_old);
  }
}

/**
 * Check if the number of items in the GHash is large enough to require more buckets,
 * or small enough to require less buckets, and resize \a gh accordingly.
 */
static void rhash_buckets_expand(RHash *rh, const uint nentries, const bool user_defined)
{
  uint new_nbuckets;

  if (ARCH_LIKELY(rh->buckets && (nentries < rh->limit_grow))) {
    return;
  }

  new_nbuckets = rh->nbuckets;

  while ((nentries > rh->limit_grow) && (rh->cursize < RHASH_MAX_SIZE - 1)) {
    new_nbuckets = hashsizes[++rh->cursize];
    rh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  }

  if (user_defined) {
    rh->size_min = rh->cursize;
  }

  if ((new_nbuckets == rh->nbuckets) && rh->buckets) {
    return;
  }

  rh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  rh->limit_shrink = RHASH_LIMIT_SHRINK(new_nbuckets);
  rhash_buckets_resize(rh, new_nbuckets);
}

/**
 * Internal insert function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE void rhash_insert_ex(RHash *rh, const TfToken &key, void *val, const uint bucket_index)
{
  RHashEntry *e = static_cast<RHashEntry *>(KKE_mempool_alloc(rh->entrypool));

  KLI_assert((rh->flag & RHASH_FLAG_ALLOW_DUPES) || (KKE_rhash_haskey(rh, key) == 0));
  KLI_assert(!(rh->flag & RHASH_FLAG_IS_RSET));

  e->e.next = rh->buckets[bucket_index];
  e->e.token = key;
  e->val = val;
  rh->buckets[bucket_index] = (Entry *)e;

  rhash_buckets_expand(rh, ++rh->nentries, false);
}

/**
 * Internal insert function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE void rhash_insert_ex(RHash *gh, void *key, void *val, const uint bucket_index)
{
  RHashEntry *e = static_cast<RHashEntry *>(KKE_mempool_alloc(gh->entrypool));

  KLI_assert((gh->flag & RHASH_FLAG_ALLOW_DUPES) || (KKE_rhash_haskey(gh, key) == 0));
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));

  e->e.next = gh->buckets[bucket_index];
  e->e.key = key;
  e->val = val;
  gh->buckets[bucket_index] = (Entry *)e;

  rhash_buckets_expand(gh, ++gh->nentries, false);
}

KLI_INLINE bool rhash_insert_safe(RHash *rh,
                                  TfToken &key,
                                  void *val,
                                  const bool override,
                                  RHashKeyFreeFP keyfreefp,
                                  RHashValFreeFP valfreefp)
{
  const uint hash = rhash_keyhash(rh, key);
  const uint bucket_index = rhash_bucket_index(rh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(rh, key, bucket_index);

  KLI_assert(!(rh->flag & RHASH_FLAG_IS_RSET));

  if (e) {
    if (override) {
      if (keyfreefp) {
        keyfreefp(e->e.key);
      }
      if (valfreefp) {
        valfreefp(e->val);
      }
      e->e.token = key;
      e->val = val;
    }
    return false;
  }
  rhash_insert_ex(rh, key, val, bucket_index);
  return true;
}

KLI_INLINE bool rhash_insert_safe(RHash *rh,
                                  void *key,
                                  void *val,
                                  const bool override,
                                  RHashKeyFreeFP keyfreefp,
                                  RHashValFreeFP valfreefp)
{
  const uint hash = rhash_keyhash(rh, key);
  const uint bucket_index = rhash_bucket_index(rh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(rh, key, bucket_index);

  KLI_assert(!(rh->flag & RHASH_FLAG_IS_RSET));

  if (e) {
    if (override) {
      if (keyfreefp) {
        keyfreefp(e->e.key);
      }
      if (valfreefp) {
        valfreefp(e->val);
      }
      e->e.key = key;
      e->val = val;
    }
    return false;
  }
  rhash_insert_ex(rh, key, val, bucket_index);
  return true;
}

/**
 * Clear and reset @a rh buckets, reserve again buckets for given number of entries.
 */
KLI_INLINE void rhash_buckets_reset(RHash *rh, const uint nentries)
{
  MEM_SAFE_FREE(rh->buckets);

  rh->cursize = 0;
  rh->size_min = 0;
  rh->nbuckets = hashsizes[rh->cursize];

  rh->limit_grow = RHASH_LIMIT_GROW(rh->nbuckets);
  rh->limit_shrink = RHASH_LIMIT_SHRINK(rh->nbuckets);

  rh->nentries = 0;

  rhash_buckets_expand(rh, nentries, (nentries != 0));
}

void *KKE_rhash_lookup(RHash *rh, const void *key)
{
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry(rh, key);
  KLI_assert(!(rh->flag & RHASH_FLAG_IS_RSET));
  return e ? e->val : NULL;
}

static RHash *rhash_new(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        const uint nentries_reserve,
                        const uint flag)
{
  RHash *rh = static_cast<RHash *>(MEM_mallocN(sizeof(*rh), info));

  rh->hashfp = hashfp;
  rh->cmpfp = cmpfp;

  rh->buckets = NULL;
  rh->flag = flag;

  rhash_buckets_reset(rh, nentries_reserve);
  rh->entrypool = KKE_mempool_create(RHASH_ENTRY_SIZE(flag & RHASH_FLAG_IS_RSET),
                                     64,
                                     64,
                                     KKE_MEMPOOL_NOP);

  return rh;
}

/**
 * Copy the RHash.
 */
static RHash *rhash_copy(const RHash *rh, RHashKeyCopyFP keycopyfp, RHashValCopyFP valcopyfp)
{
  RHash *rh_new;
  uint i;
  /* This allows us to be sure to get the same number of buckets in rh_new as in rhash. */
  const uint reserve_nentries_new = MAX2(RHASH_LIMIT_GROW(rh->nbuckets) - 1, rh->nentries);

  KLI_assert(!valcopyfp || !(rh->flag & RHASH_FLAG_IS_RSET));

  rh_new = rhash_new(rh->hashfp, rh->cmpfp, __func__, 0, rh->flag);
  rhash_buckets_expand(rh_new, reserve_nentries_new, false);

  KLI_assert(rh_new->nbuckets == rh->nbuckets);

  for (i = 0; i < rh->nbuckets; i++) {
    Entry *e;

    for (e = rh->buckets[i]; e; e = e->next) {
      Entry *e_new = (Entry *)KKE_mempool_alloc(rh_new->entrypool);
      rhash_entry_copy(rh_new, e_new, rh, e, keycopyfp, valcopyfp);

      /* Warning!
       * This means entries in buckets in new copy will be in reversed order!
       * This shall not be an issue though, since order should never be assumed in rhash. */

      /* NOTE: We can use 'i' here, since we are sure that
       * 'rh' and 'rh_new' have the same number of buckets! */
      e_new->next = rh_new->buckets[i];
      rh_new->buckets[i] = e_new;
    }
  }
  rh_new->nentries = rh->nentries;

  return rh_new;
}


uint KKE_rhashutil_uinthash(uint key)
{
  key += ~(key << 16);
  key ^= (key >> 5);
  key += (key << 3);
  key ^= (key >> 13);
  key += ~(key << 9);
  key ^= (key >> 17);

  return key;
}

uint KKE_rhashutil_inthash_p(const void *ptr)
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

uint KKE_rhashutil_strhash_p(const void *ptr)
{
  const signed char *p;
  uint h = 5381;

  for (p = (const signed char *)ptr; *p != '\0'; p++) {
    h = (uint)((h << 5) + h) + (uint)*p;
  }

  return h;
}

bool KKE_rhashutil_strcmp(const void *a, const void *b)
{
  return (a == b) ? false : !STREQ((const char *)a, (const char *)b);
}

bool KKE_rhashutil_intcmp(const void *a, const void *b)
{
  return (a != b);
}


void KKE_linklist_lockfree_init(LockfreeLinkList *list)
{
  list->dummy_node.next = NULL;
  list->head = list->tail = &list->dummy_node;
}

/* -------------------------------------------------------------------- */
/** \name RHash Public API
 * \{ */

RHash *KKE_rhash_new_ex(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        const uint nentries_reserve)
{
  return rhash_new(hashfp, cmpfp, info, nentries_reserve, 0);
}

RHash *KKE_rhash_new(RHashHashFP hashfp, RHashCmpFP cmpfp, const char *info)
{
  return KKE_rhash_new_ex(hashfp, cmpfp, info, 0);
}

RHash *KKE_rhash_str_new_ex(const char *info, unsigned int nentries_reserve)
{
  return KKE_rhash_new_ex(KKE_rhashutil_strhash_p, KKE_rhashutil_strcmp, info, nentries_reserve);
}

RHash *KKE_rhash_str_new(const char *info)
{
  return KKE_rhash_str_new_ex(info, 0);
}

RHash *KKE_rhash_int_new_ex(const char *info, const uint nentries_reserve)
{
  return KKE_rhash_new_ex(KKE_rhashutil_inthash_p, KKE_rhashutil_intcmp, info, nentries_reserve);
}
RHash *KKE_rhash_int_new(const char *info)
{
  return KKE_rhash_int_new_ex(info, 0);
}

RHash *KKE_rhash_copy(const RHash *rh, RHashKeyCopyFP keycopyfp, RHashValCopyFP valcopyfp)
{
  return rhash_copy(rh, keycopyfp, valcopyfp);
}

bool KKE_rhash_reinsert(RHash *rh,
                        TfToken &key,
                        void *val,
                        RHashKeyFreeFP keyfreefp,
                        RHashValFreeFP valfreefp)
{
  return rhash_insert_safe(rh, key, val, true, keyfreefp, valfreefp);
}

bool KKE_rhash_reinsert(RHash *rh,
                        void *key,
                        void *val,
                        RHashKeyFreeFP keyfreefp,
                        RHashValFreeFP valfreefp)
{
  return rhash_insert_safe(rh, key, val, true, keyfreefp, valfreefp);
}

bool KLI_rhash_haskey(const RHash *gh, const void *key)
{
  return (rhash_lookup_entry(gh, key) != NULL);
}

/**
 * Insert function that takes a pre-allocated entry.
 */
KLI_INLINE void rhash_insert_ex_keyonly_entry(RHash *rh,
                                              void *key,
                                              const uint bucket_index,
                                              Entry *e)
{
  KLI_assert((rh->flag & RHASH_FLAG_ALLOW_DUPES) || (KLI_rhash_haskey(rh, key) == 0));

  e->next = rh->buckets[bucket_index];
  e->key = key;
  rh->buckets[bucket_index] = e;

  rhash_buckets_expand(rh, ++rh->nentries, false);
}

bool KKE_rset_ensure_p_ex(RSet *rs, const void *key, void ***r_key)
{
  const uint hash = rhash_keyhash((RHash *)rs, key);
  const uint bucket_index = rhash_bucket_index((RHash *)rs, hash);
  RSetEntry *e = (RSetEntry *)rhash_lookup_entry_ex((const RHash *)rs, key, bucket_index);
  const bool haskey = (e != NULL);

  if (!haskey) {
    /* Pass 'key' in case we resize */
    e = (RSetEntry *)KKE_mempool_alloc(((RHash *)rs)->entrypool);
    rhash_insert_ex_keyonly_entry((RHash *)rs, (void *)key, bucket_index, (Entry *)e);
    e->key = NULL; /* caller must re-assign */
  }

  *r_key = &e->key;
  return haskey;
}

uint KKE_rhashutil_ptrhash(const void *key)
{
  /* Based Python3.7's pointer hashing function. */

  size_t y = (size_t)key;
  /* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
   * excessive hash collisions for dicts and sets */

  /* NOTE: Unlike Python 'sizeof(uint)' is used instead of 'sizeof(void *)',
   * Otherwise casting to 'uint' ignores the upper bits on 64bit platforms. */
  return (uint)(y >> 4) | ((uint)y << (sizeof(uint[8]) - 4));
}

/* -------------------------------------------------------------------- */
/** \name GSet Public API
 * \{ */

RSet *KKE_rset_new_ex(RSetHashFP hashfp,
                      RSetCmpFP cmpfp,
                      const char *info,
                      const uint nentries_reserve)
{
  return (RSet *)rhash_new(hashfp, cmpfp, info, nentries_reserve, RHASH_FLAG_IS_RSET);
}


KRAKEN_NAMESPACE_END