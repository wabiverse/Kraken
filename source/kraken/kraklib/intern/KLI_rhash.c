/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2001-2002 NaN Holding BV. All rights reserved. */

/** \file
 * \ingroup bli
 *
 * A general (pointer -> pointer) chaining hash table
 * for 'Abstract Data Types' (known as an ADT Hash Table).
 */

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_mempool.h"
#include "KLI_sys_types.h" /* for intptr_t support */
#include "KLI_utildefines.h"

#define RHASH_INTERNAL_API
#include "KLI_rhash.h" /* own include */

/* keep last */
#include "KLI_strict_flags.h"

/* -------------------------------------------------------------------- */
/** \name Structs & Constants
 * \{ */

#define RHASH_USE_MODULO_BUCKETS

/**
 * Next prime after `2^n` (skipping 2 & 3).
 *
 * \note Also used by: `KLI_edgehash` & `KLI_smallhash`.
 */
extern const uint KLI_rhash_hash_sizes[]; /* Quiet warning, this is only used by smallhash.c */
const uint KLI_rhash_hash_sizes[] = {
    5,       11,      17,      37,      67,       131,      257,      521,       1031,
    2053,    4099,    8209,    16411,   32771,    65537,    131101,   262147,    524309,
    1048583, 2097169, 4194319, 8388617, 16777259, 33554467, 67108879, 134217757, 268435459,
};
#define hashsizes KLI_rhash_hash_sizes

#ifdef RHASH_USE_MODULO_BUCKETS
#  define RHASH_MAX_SIZE 27
KLI_STATIC_ASSERT(ARRAY_SIZE(hashsizes) == RHASH_MAX_SIZE, "Invalid 'hashsizes' size");
#else
#  define RHASH_BUCKET_BIT_MIN 2
#  define RHASH_BUCKET_BIT_MAX 28 /* About 268M of buckets... */
#endif

/**
 * \note Max load #RHASH_LIMIT_GROW used to be 3. (pre 2.74).
 * Python uses 0.6666, tommyhashlib even goes down to 0.5.
 * Reducing our from 3 to 0.75 gives huge speedup
 * (about twice quicker pure RHash insertions/lookup,
 * about 25% - 30% quicker 'dynamic-topology' stroke drawing e.g.).
 * Min load #RHASH_LIMIT_SHRINK is a quarter of max load, to avoid resizing to quickly.
 */
#define RHASH_LIMIT_GROW(_nbkt) (((_nbkt)*3) / 4)
#define RHASH_LIMIT_SHRINK(_nbkt) (((_nbkt)*3) / 16)

/* WARNING! Keep in sync with ugly _gh_Entry in header!!! */
typedef struct Entry {
  struct Entry *next;

  void *key;
} Entry;

typedef struct RHashEntry {
  Entry e;

  void *val;
} RHashEntry;

typedef Entry RSetEntry;

#define RHASH_ENTRY_SIZE(_is_rset) ((_is_rset) ? sizeof(RSetEntry) : sizeof(RHashEntry))

struct RHash {
  RHashHashFP hashfp;
  RHashCmpFP cmpfp;

  Entry **buckets;
  struct KLI_mempool *entrypool;
  uint nbuckets;
  uint limit_grow, limit_shrink;
#ifdef RHASH_USE_MODULO_BUCKETS
  uint cursize, size_min;
#else
  uint bucket_mask, bucket_bit, bucket_bit_min;
#endif

  uint nentries;
  uint flag;
};

/** \} */

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
    }
    else {
      ((RHashEntry *)dst)->val = NULL;
    }
  }
}

/**
 * Get the full hash for a key.
 */
KLI_INLINE uint rhash_keyhash(const RHash *gh, const void *key)
{
  return gh->hashfp(key);
}

/**
 * Get the full hash for an entry.
 */
KLI_INLINE uint rhash_entryhash(const RHash *gh, const Entry *e)
{
  return gh->hashfp(e->key);
}

/**
 * Get the bucket-index for an already-computed full hash.
 */
KLI_INLINE uint rhash_bucket_index(const RHash *gh, const uint hash)
{
#ifdef RHASH_USE_MODULO_BUCKETS
  return hash % gh->nbuckets;
#else
  return hash & gh->bucket_mask;
#endif
}

/**
 * Find the index of next used bucket, starting from \a curr_bucket (\a gh is assumed non-empty).
 */
KLI_INLINE uint rhash_find_next_bucket_index(const RHash *gh, uint curr_bucket)
{
  if (curr_bucket >= gh->nbuckets) {
    curr_bucket = 0;
  }
  if (gh->buckets[curr_bucket]) {
    return curr_bucket;
  }
  for (; curr_bucket < gh->nbuckets; curr_bucket++) {
    if (gh->buckets[curr_bucket]) {
      return curr_bucket;
    }
  }
  for (curr_bucket = 0; curr_bucket < gh->nbuckets; curr_bucket++) {
    if (gh->buckets[curr_bucket]) {
      return curr_bucket;
    }
  }
  KLI_assert_unreachable();
  return 0;
}

/**
 * Expand buckets to the next size up or down.
 */
static void rhash_buckets_resize(RHash *gh, const uint nbuckets)
{
  Entry **buckets_old = gh->buckets;
  Entry **buckets_new;
  const uint nbuckets_old = gh->nbuckets;
  uint i;

  KLI_assert((gh->nbuckets != nbuckets) || !gh->buckets);
  //  printf("%s: %d -> %d\n", __func__, nbuckets_old, nbuckets);

  gh->nbuckets = nbuckets;
#ifdef RHASH_USE_MODULO_BUCKETS
#else
  gh->bucket_mask = nbuckets - 1;
#endif

  buckets_new = (Entry **)MEM_callocN(sizeof(*gh->buckets) * gh->nbuckets, __func__);

  if (buckets_old) {
    if (nbuckets > nbuckets_old) {
      for (i = 0; i < nbuckets_old; i++) {
        for (Entry *e = buckets_old[i], *e_next; e; e = e_next) {
          const uint hash = rhash_entryhash(gh, e);
          const uint bucket_index = rhash_bucket_index(gh, hash);
          e_next = e->next;
          e->next = buckets_new[bucket_index];
          buckets_new[bucket_index] = e;
        }
      }
    }
    else {
      for (i = 0; i < nbuckets_old; i++) {
#ifdef RHASH_USE_MODULO_BUCKETS
        for (Entry *e = buckets_old[i], *e_next; e; e = e_next) {
          const uint hash = rhash_entryhash(gh, e);
          const uint bucket_index = rhash_bucket_index(gh, hash);
          e_next = e->next;
          e->next = buckets_new[bucket_index];
          buckets_new[bucket_index] = e;
        }
#else
        /* No need to recompute hashes in this case, since our mask is just smaller,
         * all items in old bucket 'i' will go in same new bucket (i & new_mask)! */
        const uint bucket_index = rhash_bucket_index(gh, i);
        KLI_assert(!buckets_old[i] ||
                   (bucket_index == rhash_bucket_index(gh, rhash_entryhash(gh, buckets_old[i]))));
        Entry *e;
        for (e = buckets_old[i]; e && e->next; e = e->next) {
          /* pass */
        }
        if (e) {
          e->next = buckets_new[bucket_index];
          buckets_new[bucket_index] = buckets_old[i];
        }
#endif
      }
    }
  }

  gh->buckets = buckets_new;
  if (buckets_old) {
    MEM_freeN(buckets_old);
  }
}

/**
 * Check if the number of items in the RHash is large enough to require more buckets,
 * or small enough to require less buckets, and resize \a gh accordingly.
 */
static void rhash_buckets_expand(RHash *gh, const uint nentries, const bool user_defined)
{
  uint new_nbuckets;

  if (LIKELY(gh->buckets && (nentries < gh->limit_grow))) {
    return;
  }

  new_nbuckets = gh->nbuckets;

#ifdef RHASH_USE_MODULO_BUCKETS
  while ((nentries > gh->limit_grow) && (gh->cursize < RHASH_MAX_SIZE - 1)) {
    new_nbuckets = hashsizes[++gh->cursize];
    gh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  }
#else
  while ((nentries > gh->limit_grow) && (gh->bucket_bit < RHASH_BUCKET_BIT_MAX)) {
    new_nbuckets = 1u << ++gh->bucket_bit;
    gh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  }
#endif

  if (user_defined) {
#ifdef RHASH_USE_MODULO_BUCKETS
    gh->size_min = gh->cursize;
#else
    gh->bucket_bit_min = gh->bucket_bit;
#endif
  }

  if ((new_nbuckets == gh->nbuckets) && gh->buckets) {
    return;
  }

  gh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  gh->limit_shrink = RHASH_LIMIT_SHRINK(new_nbuckets);
  rhash_buckets_resize(gh, new_nbuckets);
}

static void rhash_buckets_contract(RHash *gh,
                                   const uint nentries,
                                   const bool user_defined,
                                   const bool force_shrink)
{
  uint new_nbuckets;

  if (!(force_shrink || (gh->flag & RHASH_FLAG_ALLOW_SHRINK))) {
    return;
  }

  if (LIKELY(gh->buckets && (nentries > gh->limit_shrink))) {
    return;
  }

  new_nbuckets = gh->nbuckets;

#ifdef RHASH_USE_MODULO_BUCKETS
  while ((nentries < gh->limit_shrink) && (gh->cursize > gh->size_min)) {
    new_nbuckets = hashsizes[--gh->cursize];
    gh->limit_shrink = RHASH_LIMIT_SHRINK(new_nbuckets);
  }
#else
  while ((nentries < gh->limit_shrink) && (gh->bucket_bit > gh->bucket_bit_min)) {
    new_nbuckets = 1u << --gh->bucket_bit;
    gh->limit_shrink = RHASH_LIMIT_SHRINK(new_nbuckets);
  }
#endif

  if (user_defined) {
#ifdef RHASH_USE_MODULO_BUCKETS
    gh->size_min = gh->cursize;
#else
    gh->bucket_bit_min = gh->bucket_bit;
#endif
  }

  if ((new_nbuckets == gh->nbuckets) && gh->buckets) {
    return;
  }

  gh->limit_grow = RHASH_LIMIT_GROW(new_nbuckets);
  gh->limit_shrink = RHASH_LIMIT_SHRINK(new_nbuckets);
  rhash_buckets_resize(gh, new_nbuckets);
}

/**
 * Clear and reset \a gh buckets, reserve again buckets for given number of entries.
 */
KLI_INLINE void rhash_buckets_reset(RHash *gh, const uint nentries)
{
  MEM_SAFE_FREE(gh->buckets);

#ifdef RHASH_USE_MODULO_BUCKETS
  gh->cursize = 0;
  gh->size_min = 0;
  gh->nbuckets = hashsizes[gh->cursize];
#else
  gh->bucket_bit = RHASH_BUCKET_BIT_MIN;
  gh->bucket_bit_min = RHASH_BUCKET_BIT_MIN;
  gh->nbuckets = 1u << gh->bucket_bit;
  gh->bucket_mask = gh->nbuckets - 1;
#endif

  gh->limit_grow = RHASH_LIMIT_GROW(gh->nbuckets);
  gh->limit_shrink = RHASH_LIMIT_SHRINK(gh->nbuckets);

  gh->nentries = 0;

  rhash_buckets_expand(gh, nentries, (nentries != 0));
}

/**
 * Internal lookup function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE Entry *rhash_lookup_entry_ex(const RHash *gh, const void *key, const uint bucket_index)
{
  Entry *e;
  /* If we do not store RHash, not worth computing it for each entry here!
   * Typically, comparison function will be quicker, and since it's needed in the end anyway... */
  for (e = gh->buckets[bucket_index]; e; e = e->next) {
    if (UNLIKELY(gh->cmpfp(key, e->key) == false)) {
      return e;
    }
  }

  return NULL;
}

/**
 * Internal lookup function, returns previous entry of target one too.
 * Takes bucket_index argument to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 * Useful when modifying buckets somehow (like removing an entry...).
 */
KLI_INLINE Entry *rhash_lookup_entry_prev_ex(RHash *gh,
                                             const void *key,
                                             Entry **r_e_prev,
                                             const uint bucket_index)
{
  /* If we do not store RHash, not worth computing it for each entry here!
   * Typically, comparison function will be quicker, and since it's needed in the end anyway... */
  for (Entry *e_prev = NULL, *e = gh->buckets[bucket_index]; e; e_prev = e, e = e->next) {
    if (UNLIKELY(gh->cmpfp(key, e->key) == false)) {
      *r_e_prev = e_prev;
      return e;
    }
  }

  *r_e_prev = NULL;
  return NULL;
}

/**
 * Internal lookup function. Only wraps #rhash_lookup_entry_ex
 */
KLI_INLINE Entry *rhash_lookup_entry(const RHash *gh, const void *key)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  return rhash_lookup_entry_ex(gh, key, bucket_index);
}

static RHash *rhash_new(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        const uint nentries_reserve,
                        const uint flag)
{
  RHash *gh = MEM_mallocN(sizeof(*gh), info);

  gh->hashfp = hashfp;
  gh->cmpfp = cmpfp;

  gh->buckets = NULL;
  gh->flag = flag;

  rhash_buckets_reset(gh, nentries_reserve);
  gh->entrypool = KLI_mempool_create(
      RHASH_ENTRY_SIZE(flag & RHASH_FLAG_IS_RSET), 64, 64, KLI_MEMPOOL_NOP);

  return gh;
}

/**
 * Internal insert function.
 * Takes hash and bucket_index arguments to avoid calling #rhash_keyhash and #rhash_bucket_index
 * multiple times.
 */
KLI_INLINE void rhash_insert_ex(RHash *gh, void *key, void *val, const uint bucket_index)
{
  RHashEntry *e = KLI_mempool_alloc(gh->entrypool);

  KLI_assert((gh->flag & RHASH_FLAG_ALLOW_DUPES) || (KLI_rhash_haskey(gh, key) == 0));
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));

  e->e.next = gh->buckets[bucket_index];
  e->e.key = key;
  e->val = val;
  gh->buckets[bucket_index] = (Entry *)e;

  rhash_buckets_expand(gh, ++gh->nentries, false);
}

/**
 * Insert function that takes a pre-allocated entry.
 */
KLI_INLINE void rhash_insert_ex_keyonly_entry(RHash *gh,
                                              void *key,
                                              const uint bucket_index,
                                              Entry *e)
{
  KLI_assert((gh->flag & RHASH_FLAG_ALLOW_DUPES) || (KLI_rhash_haskey(gh, key) == 0));

  e->next = gh->buckets[bucket_index];
  e->key = key;
  gh->buckets[bucket_index] = e;

  rhash_buckets_expand(gh, ++gh->nentries, false);
}

/**
 * Insert function that doesn't set the value (use for RSet)
 */
KLI_INLINE void rhash_insert_ex_keyonly(RHash *gh, void *key, const uint bucket_index)
{
  Entry *e = KLI_mempool_alloc(gh->entrypool);

  KLI_assert((gh->flag & RHASH_FLAG_ALLOW_DUPES) || (KLI_rhash_haskey(gh, key) == 0));
  KLI_assert((gh->flag & RHASH_FLAG_IS_RSET) != 0);

  e->next = gh->buckets[bucket_index];
  e->key = key;
  gh->buckets[bucket_index] = e;

  rhash_buckets_expand(gh, ++gh->nentries, false);
}

KLI_INLINE void rhash_insert(RHash *gh, void *key, void *val)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);

  rhash_insert_ex(gh, key, val, bucket_index);
}

KLI_INLINE bool rhash_insert_safe(RHash *gh,
                                  void *key,
                                  void *val,
                                  const bool override,
                                  RHashKeyFreeFP keyfreefp,
                                  RHashValFreeFP valfreefp)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(gh, key, bucket_index);

  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));

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
  rhash_insert_ex(gh, key, val, bucket_index);
  return true;
}

KLI_INLINE bool rhash_insert_safe_keyonly(RHash *gh,
                                          void *key,
                                          const bool override,
                                          RHashKeyFreeFP keyfreefp)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  Entry *e = rhash_lookup_entry_ex(gh, key, bucket_index);

  KLI_assert((gh->flag & RHASH_FLAG_IS_RSET) != 0);

  if (e) {
    if (override) {
      if (keyfreefp) {
        keyfreefp(e->key);
      }
      e->key = key;
    }
    return false;
  }
  rhash_insert_ex_keyonly(gh, key, bucket_index);
  return true;
}

/**
 * Remove the entry and return it, caller must free from gh->entrypool.
 */
static Entry *rhash_remove_ex(RHash *gh,
                              const void *key,
                              RHashKeyFreeFP keyfreefp,
                              RHashValFreeFP valfreefp,
                              const uint bucket_index)
{
  Entry *e_prev;
  Entry *e = rhash_lookup_entry_prev_ex(gh, key, &e_prev, bucket_index);

  KLI_assert(!valfreefp || !(gh->flag & RHASH_FLAG_IS_RSET));

  if (e) {
    if (keyfreefp) {
      keyfreefp(e->key);
    }
    if (valfreefp) {
      valfreefp(((RHashEntry *)e)->val);
    }

    if (e_prev) {
      e_prev->next = e->next;
    }
    else {
      gh->buckets[bucket_index] = e->next;
    }

    rhash_buckets_contract(gh, --gh->nentries, false, false);
  }

  return e;
}

/**
 * Remove a random entry and return it (or NULL if empty), caller must free from gh->entrypool.
 */
static Entry *rhash_pop(RHash *gh, RHashIterState *state)
{
  uint curr_bucket = state->curr_bucket;
  if (gh->nentries == 0) {
    return NULL;
  }

  /* NOTE: using first_bucket_index here allows us to avoid potential
   * huge number of loops over buckets,
   * in case we are popping from a large rhash with few items in it... */
  curr_bucket = rhash_find_next_bucket_index(gh, curr_bucket);

  Entry *e = gh->buckets[curr_bucket];
  KLI_assert(e);

  rhash_remove_ex(gh, e->key, NULL, NULL, curr_bucket);

  state->curr_bucket = curr_bucket;
  return e;
}

/**
 * Run free callbacks for freeing entries.
 */
static void rhash_free_cb(RHash *gh, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp)
{
  uint i;

  KLI_assert(keyfreefp || valfreefp);
  KLI_assert(!valfreefp || !(gh->flag & RHASH_FLAG_IS_RSET));

  for (i = 0; i < gh->nbuckets; i++) {
    Entry *e;

    for (e = gh->buckets[i]; e; e = e->next) {
      if (keyfreefp) {
        keyfreefp(e->key);
      }
      if (valfreefp) {
        valfreefp(((RHashEntry *)e)->val);
      }
    }
  }
}

/**
 * Copy the RHash.
 */
static RHash *rhash_copy(const RHash *gh, RHashKeyCopyFP keycopyfp, RHashValCopyFP valcopyfp)
{
  RHash *gh_new;
  uint i;
  /* This allows us to be sure to get the same number of buckets in gh_new as in rhash. */
  const uint reserve_nentries_new = MAX2(RHASH_LIMIT_GROW(gh->nbuckets) - 1, gh->nentries);

  KLI_assert(!valcopyfp || !(gh->flag & RHASH_FLAG_IS_RSET));

  gh_new = rhash_new(gh->hashfp, gh->cmpfp, __func__, 0, gh->flag);
  rhash_buckets_expand(gh_new, reserve_nentries_new, false);

  KLI_assert(gh_new->nbuckets == gh->nbuckets);

  for (i = 0; i < gh->nbuckets; i++) {
    Entry *e;

    for (e = gh->buckets[i]; e; e = e->next) {
      Entry *e_new = KLI_mempool_alloc(gh_new->entrypool);
      rhash_entry_copy(gh_new, e_new, gh, e, keycopyfp, valcopyfp);

      /* Warning!
       * This means entries in buckets in new copy will be in reversed order!
       * This shall not be an issue though, since order should never be assumed in rhash. */

      /* NOTE: We can use 'i' here, since we are sure that
       * 'gh' and 'gh_new' have the same number of buckets! */
      e_new->next = gh_new->buckets[i];
      gh_new->buckets[i] = e_new;
    }
  }
  gh_new->nentries = gh->nentries;

  return gh_new;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash Public API
 * \{ */

RHash *KLI_rhash_new_ex(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        const uint nentries_reserve)
{
  return rhash_new(hashfp, cmpfp, info, nentries_reserve, 0);
}

RHash *KLI_rhash_new(RHashHashFP hashfp, RHashCmpFP cmpfp, const char *info)
{
  return KLI_rhash_new_ex(hashfp, cmpfp, info, 0);
}

RHash *KLI_rhash_copy(const RHash *gh, RHashKeyCopyFP keycopyfp, RHashValCopyFP valcopyfp)
{
  return rhash_copy(gh, keycopyfp, valcopyfp);
}

void KLI_rhash_reserve(RHash *gh, const uint nentries_reserve)
{
  rhash_buckets_expand(gh, nentries_reserve, true);
  rhash_buckets_contract(gh, nentries_reserve, true, false);
}

uint KLI_rhash_len(const RHash *gh)
{
  return gh->nentries;
}

void KLI_rhash_insert(RHash *gh, void *key, void *val)
{
  rhash_insert(gh, key, val);
}

bool KLI_rhash_reinsert(
    RHash *gh, void *key, void *val, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp)
{
  return rhash_insert_safe(gh, key, val, true, keyfreefp, valfreefp);
}

void *KLI_rhash_replace_key(RHash *gh, void *key)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(gh, key, bucket_index);
  if (e != NULL) {
    void *key_prev = e->e.key;
    e->e.key = key;
    return key_prev;
  }
  return NULL;
}

void *KLI_rhash_lookup(const RHash *gh, const void *key)
{
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry(gh, key);
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));
  return e ? e->val : NULL;
}

void *KLI_rhash_lookup_default(const RHash *gh, const void *key, void *val_default)
{
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry(gh, key);
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));
  return e ? e->val : val_default;
}

void **KLI_rhash_lookup_p(RHash *gh, const void *key)
{
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry(gh, key);
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));
  return e ? &e->val : NULL;
}

bool KLI_rhash_ensure_p(RHash *gh, void *key, void ***r_val)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(gh, key, bucket_index);
  const bool haskey = (e != NULL);

  if (!haskey) {
    e = KLI_mempool_alloc(gh->entrypool);
    rhash_insert_ex_keyonly_entry(gh, key, bucket_index, (Entry *)e);
  }

  *r_val = &e->val;
  return haskey;
}

bool KLI_rhash_ensure_p_ex(RHash *gh, const void *key, void ***r_key, void ***r_val)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  RHashEntry *e = (RHashEntry *)rhash_lookup_entry_ex(gh, key, bucket_index);
  const bool haskey = (e != NULL);

  if (!haskey) {
    /* Pass 'key' in case we resize. */
    e = KLI_mempool_alloc(gh->entrypool);
    rhash_insert_ex_keyonly_entry(gh, (void *)key, bucket_index, (Entry *)e);
    e->e.key = NULL; /* caller must re-assign */
  }

  *r_key = &e->e.key;
  *r_val = &e->val;
  return haskey;
}

bool KLI_rhash_remove(RHash *gh,
                      const void *key,
                      RHashKeyFreeFP keyfreefp,
                      RHashValFreeFP valfreefp)
{
  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  Entry *e = rhash_remove_ex(gh, key, keyfreefp, valfreefp, bucket_index);
  if (e) {
    KLI_mempool_free(gh->entrypool, e);
    return true;
  }
  return false;
}

void *KLI_rhash_popkey(RHash *gh, const void *key, RHashKeyFreeFP keyfreefp)
{
  /* Same as above but return the value,
   * no free value argument since it will be returned. */

  const uint hash = rhash_keyhash(gh, key);
  const uint bucket_index = rhash_bucket_index(gh, hash);
  RHashEntry *e = (RHashEntry *)rhash_remove_ex(gh, key, keyfreefp, NULL, bucket_index);
  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));
  if (e) {
    void *val = e->val;
    KLI_mempool_free(gh->entrypool, e);
    return val;
  }
  return NULL;
}

bool KLI_rhash_haskey(const RHash *gh, const void *key)
{
  return (rhash_lookup_entry(gh, key) != NULL);
}

bool KLI_rhash_pop(RHash *gh, RHashIterState *state, void **r_key, void **r_val)
{
  RHashEntry *e = (RHashEntry *)rhash_pop(gh, state);

  KLI_assert(!(gh->flag & RHASH_FLAG_IS_RSET));

  if (e) {
    *r_key = e->e.key;
    *r_val = e->val;

    KLI_mempool_free(gh->entrypool, e);
    return true;
  }

  *r_key = *r_val = NULL;
  return false;
}

void KLI_rhash_clear_ex(RHash *gh,
                        RHashKeyFreeFP keyfreefp,
                        RHashValFreeFP valfreefp,
                        const uint nentries_reserve)
{
  if (keyfreefp || valfreefp) {
    rhash_free_cb(gh, keyfreefp, valfreefp);
  }

  rhash_buckets_reset(gh, nentries_reserve);
  KLI_mempool_clear_ex(gh->entrypool, nentries_reserve ? (int)nentries_reserve : -1);
}

void KLI_rhash_clear(RHash *gh, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp)
{
  KLI_rhash_clear_ex(gh, keyfreefp, valfreefp, 0);
}

void KLI_rhash_free(RHash *gh, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp)
{
  KLI_assert((int)gh->nentries == KLI_mempool_len(gh->entrypool));
  if (keyfreefp || valfreefp) {
    rhash_free_cb(gh, keyfreefp, valfreefp);
  }

  MEM_freeN(gh->buckets);
  KLI_mempool_destroy(gh->entrypool);
  MEM_freeN(gh);
}

void KLI_rhash_flag_set(RHash *gh, uint flag)
{
  gh->flag |= flag;
}

void KLI_rhash_flag_clear(RHash *gh, uint flag)
{
  gh->flag &= ~flag;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash Iterator API
 * \{ */

RHashIterator *KLI_rhashIterator_new(RHash *gh)
{
  RHashIterator *ghi = MEM_mallocN(sizeof(*ghi), "rhash iterator");
  KLI_rhashIterator_init(ghi, gh);
  return ghi;
}

void KLI_rhashIterator_init(RHashIterator *ghi, RHash *gh)
{
  ghi->gh = gh;
  ghi->curEntry = NULL;
  ghi->curBucket = UINT_MAX; /* wraps to zero */
  if (gh->nentries) {
    do {
      ghi->curBucket++;
      if (UNLIKELY(ghi->curBucket == ghi->gh->nbuckets)) {
        break;
      }
      ghi->curEntry = ghi->gh->buckets[ghi->curBucket];
    } while (!ghi->curEntry);
  }
}

void KLI_rhashIterator_step(RHashIterator *ghi)
{
  if (ghi->curEntry) {
    ghi->curEntry = ghi->curEntry->next;
    while (!ghi->curEntry) {
      ghi->curBucket++;
      if (ghi->curBucket == ghi->gh->nbuckets) {
        break;
      }
      ghi->curEntry = ghi->gh->buckets[ghi->curBucket];
    }
  }
}

void KLI_rhashIterator_free(RHashIterator *ghi)
{
  MEM_freeN(ghi);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name RSet Public API
 * \{ */

RSet *KLI_rset_new_ex(RSetHashFP hashfp,
                      RSetCmpFP cmpfp,
                      const char *info,
                      const uint nentries_reserve)
{
  return (RSet *)rhash_new(hashfp, cmpfp, info, nentries_reserve, RHASH_FLAG_IS_RSET);
}

RSet *KLI_rset_new(RSetHashFP hashfp, RSetCmpFP cmpfp, const char *info)
{
  return KLI_rset_new_ex(hashfp, cmpfp, info, 0);
}

RSet *KLI_rset_copy(const RSet *gs, RHashKeyCopyFP keycopyfp)
{
  return (RSet *)rhash_copy((const RHash *)gs, keycopyfp, NULL);
}

uint KLI_rset_len(const RSet *gs)
{
  return ((RHash *)gs)->nentries;
}

void KLI_rset_insert(RSet *gs, void *key)
{
  const uint hash = rhash_keyhash((RHash *)gs, key);
  const uint bucket_index = rhash_bucket_index((RHash *)gs, hash);
  rhash_insert_ex_keyonly((RHash *)gs, key, bucket_index);
}

bool KLI_rset_add(RSet *gs, void *key)
{
  return rhash_insert_safe_keyonly((RHash *)gs, key, false, NULL);
}

bool KLI_rset_ensure_p_ex(RSet *gs, const void *key, void ***r_key)
{
  const uint hash = rhash_keyhash((RHash *)gs, key);
  const uint bucket_index = rhash_bucket_index((RHash *)gs, hash);
  RSetEntry *e = (RSetEntry *)rhash_lookup_entry_ex((const RHash *)gs, key, bucket_index);
  const bool haskey = (e != NULL);

  if (!haskey) {
    /* Pass 'key' in case we resize */
    e = KLI_mempool_alloc(((RHash *)gs)->entrypool);
    rhash_insert_ex_keyonly_entry((RHash *)gs, (void *)key, bucket_index, (Entry *)e);
    e->key = NULL; /* caller must re-assign */
  }

  *r_key = &e->key;
  return haskey;
}

bool KLI_rset_reinsert(RSet *gs, void *key, RSetKeyFreeFP keyfreefp)
{
  return rhash_insert_safe_keyonly((RHash *)gs, key, true, keyfreefp);
}

void *KLI_rset_replace_key(RSet *gs, void *key)
{
  return KLI_rhash_replace_key((RHash *)gs, key);
}

bool KLI_rset_remove(RSet *gs, const void *key, RSetKeyFreeFP keyfreefp)
{
  return KLI_rhash_remove((RHash *)gs, key, keyfreefp, NULL);
}

bool KLI_rset_haskey(const RSet *gs, const void *key)
{
  return (rhash_lookup_entry((const RHash *)gs, key) != NULL);
}

bool KLI_rset_pop(RSet *gs, RSetIterState *state, void **r_key)
{
  RSetEntry *e = (RSetEntry *)rhash_pop((RHash *)gs, (RHashIterState *)state);

  if (e) {
    *r_key = e->key;

    KLI_mempool_free(((RHash *)gs)->entrypool, e);
    return true;
  }

  *r_key = NULL;
  return false;
}

void KLI_rset_clear_ex(RSet *gs, RSetKeyFreeFP keyfreefp, const uint nentries_reserve)
{
  KLI_rhash_clear_ex((RHash *)gs, keyfreefp, NULL, nentries_reserve);
}

void KLI_rset_clear(RSet *gs, RSetKeyFreeFP keyfreefp)
{
  KLI_rhash_clear((RHash *)gs, keyfreefp, NULL);
}

void KLI_rset_free(RSet *gs, RSetKeyFreeFP keyfreefp)
{
  KLI_rhash_free((RHash *)gs, keyfreefp, NULL);
}

void KLI_rset_flag_set(RSet *gs, uint flag)
{
  ((RHash *)gs)->flag |= flag;
}

void KLI_rset_flag_clear(RSet *gs, uint flag)
{
  ((RHash *)gs)->flag &= ~flag;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name RSet Combined Key/Value Usage
 *
 * \note Not typical `set` use, only use when the pointer identity matters.
 * This can be useful when the key references data stored outside the RSet.
 * \{ */

void *KLI_rset_lookup(const RSet *gs, const void *key)
{
  Entry *e = rhash_lookup_entry((const RHash *)gs, key);
  return e ? e->key : NULL;
}

void *KLI_rset_pop_key(RSet *gs, const void *key)
{
  const uint hash = rhash_keyhash((RHash *)gs, key);
  const uint bucket_index = rhash_bucket_index((RHash *)gs, hash);
  Entry *e = rhash_remove_ex((RHash *)gs, key, NULL, NULL, bucket_index);
  if (e) {
    void *key_ret = e->key;
    KLI_mempool_free(((RHash *)gs)->entrypool, e);
    return key_ret;
  }
  return NULL;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Debugging & Introspection
 * \{ */

#include "KLI_math.h"

int KLI_rhash_buckets_len(const RHash *gh)
{
  return (int)gh->nbuckets;
}
int KLI_rset_buckets_len(const RSet *gs)
{
  return KLI_rhash_buckets_len((const RHash *)gs);
}

double KLI_rhash_calc_quality_ex(RHash *gh,
                                 double *r_load,
                                 double *r_variance,
                                 double *r_prop_empty_buckets,
                                 double *r_prop_overloaded_buckets,
                                 int *r_biggest_bucket)
{
  double mean;
  uint i;

  if (gh->nentries == 0) {
    if (r_load) {
      *r_load = 0.0;
    }
    if (r_variance) {
      *r_variance = 0.0;
    }
    if (r_prop_empty_buckets) {
      *r_prop_empty_buckets = 1.0;
    }
    if (r_prop_overloaded_buckets) {
      *r_prop_overloaded_buckets = 0.0;
    }
    if (r_biggest_bucket) {
      *r_biggest_bucket = 0;
    }

    return 0.0;
  }

  mean = (double)gh->nentries / (double)gh->nbuckets;
  if (r_load) {
    *r_load = mean;
  }
  if (r_biggest_bucket) {
    *r_biggest_bucket = 0;
  }

  if (r_variance) {
    /* We already know our mean (i.e. load factor), easy to compute variance.
     * See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Two-pass_algorithm
     */
    double sum = 0.0;
    for (i = 0; i < gh->nbuckets; i++) {
      int count = 0;
      Entry *e;
      for (e = gh->buckets[i]; e; e = e->next) {
        count++;
      }
      sum += ((double)count - mean) * ((double)count - mean);
    }
    *r_variance = sum / (double)(gh->nbuckets - 1);
  }

  {
    uint64_t sum = 0;
    uint64_t overloaded_buckets_threshold = (uint64_t)max_ii(RHASH_LIMIT_GROW(1), 1);
    uint64_t sum_overloaded = 0;
    uint64_t sum_empty = 0;

    for (i = 0; i < gh->nbuckets; i++) {
      uint64_t count = 0;
      Entry *e;
      for (e = gh->buckets[i]; e; e = e->next) {
        count++;
      }
      if (r_biggest_bucket) {
        *r_biggest_bucket = max_ii(*r_biggest_bucket, (int)count);
      }
      if (r_prop_overloaded_buckets && (count > overloaded_buckets_threshold)) {
        sum_overloaded++;
      }
      if (r_prop_empty_buckets && !count) {
        sum_empty++;
      }
      sum += count * (count + 1);
    }
    if (r_prop_overloaded_buckets) {
      *r_prop_overloaded_buckets = (double)sum_overloaded / (double)gh->nbuckets;
    }
    if (r_prop_empty_buckets) {
      *r_prop_empty_buckets = (double)sum_empty / (double)gh->nbuckets;
    }
    return ((double)sum * (double)gh->nbuckets /
            ((double)gh->nentries * (gh->nentries + 2 * gh->nbuckets - 1)));
  }
}
double KLI_rset_calc_quality_ex(RSet *gs,
                                double *r_load,
                                double *r_variance,
                                double *r_prop_empty_buckets,
                                double *r_prop_overloaded_buckets,
                                int *r_biggest_bucket)
{
  return KLI_rhash_calc_quality_ex((RHash *)gs,
                                   r_load,
                                   r_variance,
                                   r_prop_empty_buckets,
                                   r_prop_overloaded_buckets,
                                   r_biggest_bucket);
}

double KLI_rhash_calc_quality(RHash *gh)
{
  return KLI_rhash_calc_quality_ex(gh, NULL, NULL, NULL, NULL, NULL);
}
double KLI_rset_calc_quality(RSet *gs)
{
  return KLI_rhash_calc_quality_ex((RHash *)gs, NULL, NULL, NULL, NULL, NULL);
}

/** \} */
