/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2001-2002 NaN Holding BV. All rights reserved. */

#pragma once

/** \file
 * \ingroup bli
 *
 * RHash is a hash-map implementation (unordered key, value pairs).
 *
 * This is also used to implement a 'set' (see #RSet below).
 */

#include "KLI_compiler_attrs.h"
#include "KLI_compiler_compat.h"
#include "KLI_sys_types.h" /* for bool */

#ifdef __cplusplus
extern "C" {
#endif

#define _RHASH_INTERNAL_ATTR
#ifndef RHASH_INTERNAL_API
#  ifdef __GNUC__
#    undef _RHASH_INTERNAL_ATTR
#    define _RHASH_INTERNAL_ATTR __attribute__((deprecated)) /* not deprecated, just private. */
#  endif
#endif

/* -------------------------------------------------------------------- */
/** \name RHash Types
 * \{ */

typedef unsigned int (*RHashHashFP)(const void *key);
/** returns false when equal */
typedef bool (*RHashCmpFP)(const void *a, const void *b);
typedef void (*RHashKeyFreeFP)(void *key);
typedef void (*RHashValFreeFP)(void *val);
typedef void *(*RHashKeyCopyFP)(const void *key);
typedef void *(*RHashValCopyFP)(const void *val);

typedef struct RHash RHash;

typedef struct RHashIterator
{
  RHash *gh;
  struct Entry *curEntry;
  unsigned int curBucket;
} RHashIterator;

typedef struct RHashIterState
{
  unsigned int curr_bucket _RHASH_INTERNAL_ATTR;
} RHashIterState;

enum
{
  RHASH_FLAG_ALLOW_DUPES = (1 << 0),  /* Only checked for in debug mode */
  RHASH_FLAG_ALLOW_SHRINK = (1 << 1), /* Allow to shrink buckets' size. */

#ifdef RHASH_INTERNAL_API
  /* Internal usage only */
  /* Whether the RHash is actually used as RSet (no value storage). */
  RHASH_FLAG_IS_RSET = (1 << 16),
#endif
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash API
 *
 * Defined in `KLI_rhash.c`
 * \{ */

/**
 * Creates a new, empty RHash.
 *
 * \param hashfp: Hash callback.
 * \param cmpfp: Comparison callback.
 * \param info: Identifier string for the RHash.
 * \param nentries_reserve: Optionally reserve the number of members that the hash will hold.
 * Use this to avoid resizing buckets if the size is known or can be closely approximated.
 * \return  An empty RHash.
 */
RHash *KLI_rhash_new_ex(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
/**
 * Wraps #KLI_rhash_new_ex with zero entries reserved.
 */
RHash *KLI_rhash_new(RHashHashFP hashfp,
                     RHashCmpFP cmpfp,
                     const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
/**
 * Copy given RHash. Keys and values are also copied if relevant callback is provided,
 * else pointers remain the same.
 */
RHash *KLI_rhash_copy(const RHash *gh,
                      RHashKeyCopyFP keycopyfp,
                      RHashValCopyFP valcopyfp) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
/**
 * Frees the RHash and its members.
 *
 * \param gh: The RHash to free.
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 */
void KLI_rhash_free(RHash *gh, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp);
/**
 * Reserve given amount of entries (resize \a gh accordingly if needed).
 */
void KLI_rhash_reserve(RHash *gh, unsigned int nentries_reserve);
/**
 * Insert a key/value pair into the \a gh.
 *
 * \note Duplicates are not checked,
 * the caller is expected to ensure elements are unique unless
 * RHASH_FLAG_ALLOW_DUPES flag is set.
 */
void KLI_rhash_insert(RHash *gh, void *key, void *val);
/**
 * Inserts a new value to a key that may already be in rhash.
 *
 * Avoids #KLI_rhash_remove, #KLI_rhash_insert calls (double lookups)
 *
 * \returns true if a new key has been added.
 */
bool KLI_rhash_reinsert(RHash *gh,
                        void *key,
                        void *val,
                        RHashKeyFreeFP keyfreefp,
                        RHashValFreeFP valfreefp);
/**
 * Replaces the key of an item in the \a gh.
 *
 * Use when a key is re-allocated or its memory location is changed.
 *
 * \returns The previous key or NULL if not found, the caller may free if it's needed.
 */
void *KLI_rhash_replace_key(RHash *gh, void *key);
/**
 * Lookup the value of \a key in \a gh.
 *
 * \param key: The key to lookup.
 * \returns the value for \a key or NULL.
 *
 * \note When NULL is a valid value, use #KLI_rhash_lookup_p to differentiate a missing key
 * from a key with a NULL value. (Avoids calling #KLI_rhash_haskey before #KLI_rhash_lookup)
 */
void *KLI_rhash_lookup(const RHash *gh, const void *key) ATTR_WARN_UNUSED_RESULT;
/**
 * A version of #KLI_rhash_lookup which accepts a fallback argument.
 */
void *KLI_rhash_lookup_default(const RHash *gh,
                               const void *key,
                               void *val_default) ATTR_WARN_UNUSED_RESULT;
/**
 * Lookup a pointer to the value of \a key in \a gh.
 *
 * \param key: The key to lookup.
 * \returns the pointer to value for \a key or NULL.
 *
 * \note This has 2 main benefits over #KLI_rhash_lookup.
 * - A NULL return always means that \a key isn't in \a gh.
 * - The value can be modified in-place without further function calls (faster).
 */
void **KLI_rhash_lookup_p(RHash *gh, const void *key) ATTR_WARN_UNUSED_RESULT;
/**
 * Ensure \a key is exists in \a gh.
 *
 * This handles the common situation where the caller needs ensure a key is added to \a gh,
 * constructing a new value in the case the key isn't found.
 * Otherwise use the existing value.
 *
 * Such situations typically incur multiple lookups, however this function
 * avoids them by ensuring the key is added,
 * returning a pointer to the value so it can be used or initialized by the caller.
 *
 * \returns true when the value didn't need to be added.
 * (when false, the caller _must_ initialize the value).
 */
bool KLI_rhash_ensure_p(RHash *gh, void *key, void ***r_val) ATTR_WARN_UNUSED_RESULT;
/**
 * A version of #KLI_rhash_ensure_p that allows caller to re-assign the key.
 * Typically used when the key is to be duplicated.
 *
 * \warning Caller _must_ write to \a r_key when returning false.
 */
bool KLI_rhash_ensure_p_ex(RHash *gh, const void *key, void ***r_key, void ***r_val)
  ATTR_WARN_UNUSED_RESULT;
/**
 * Remove \a key from \a gh, or return false if the key wasn't found.
 *
 * \param key: The key to remove.
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 * \return true if \a key was removed from \a gh.
 */
bool KLI_rhash_remove(RHash *gh,
                      const void *key,
                      RHashKeyFreeFP keyfreefp,
                      RHashValFreeFP valfreefp);
/**
 * Wraps #KLI_rhash_clear_ex with zero entries reserved.
 */
void KLI_rhash_clear(RHash *gh, RHashKeyFreeFP keyfreefp, RHashValFreeFP valfreefp);
/**
 * Reset \a gh clearing all entries.
 *
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 * \param nentries_reserve: Optionally reserve the number of members that the hash will hold.
 */
void KLI_rhash_clear_ex(RHash *gh,
                        RHashKeyFreeFP keyfreefp,
                        RHashValFreeFP valfreefp,
                        unsigned int nentries_reserve);
/**
 * Remove \a key from \a gh, returning the value or NULL if the key wasn't found.
 *
 * \param key: The key to remove.
 * \param keyfreefp: Optional callback to free the key.
 * \return the value of \a key int \a gh or NULL.
 */
void *KLI_rhash_popkey(RHash *gh,
                       const void *key,
                       RHashKeyFreeFP keyfreefp) ATTR_WARN_UNUSED_RESULT;
/**
 * \return true if the \a key is in \a gh.
 */
bool KLI_rhash_haskey(const RHash *gh, const void *key) ATTR_WARN_UNUSED_RESULT;
/**
 * Remove a random entry from \a gh, returning true
 * if a key/value pair could be removed, false otherwise.
 *
 * \param r_key: The removed key.
 * \param r_val: The removed value.
 * \param state: Used for efficient removal.
 * \return true if there was something to pop, false if rhash was already empty.
 */
bool KLI_rhash_pop(RHash *gh, RHashIterState *state, void **r_key, void **r_val)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
/**
 * \return size of the RHash.
 */
unsigned int KLI_rhash_len(const RHash *gh) ATTR_WARN_UNUSED_RESULT;
/**
 * Sets a RHash flag.
 */
void KLI_rhash_flag_set(RHash *gh, unsigned int flag);
/**
 * Clear a RHash flag.
 */
void KLI_rhash_flag_clear(RHash *gh, unsigned int flag);

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash Iterator
 * \{ */

/**
 * Create a new RHashIterator. The hash table must not be mutated
 * while the iterator is in use, and the iterator will step exactly
 * #KLI_rhash_len(gh) times before becoming done.
 *
 * \param gh: The RHash to iterate over.
 * \return Pointer to a new iterator.
 */
RHashIterator *KLI_rhashIterator_new(RHash *gh) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

/**
 * Init an already allocated RHashIterator. The hash table must not
 * be mutated while the iterator is in use, and the iterator will
 * step exactly #KLI_rhash_len(gh) times before becoming done.
 *
 * \param ghi: The RHashIterator to initialize.
 * \param gh: The RHash to iterate over.
 */
void KLI_rhashIterator_init(RHashIterator *ghi, RHash *gh);
/**
 * Free a RHashIterator.
 *
 * \param ghi: The iterator to free.
 */
void KLI_rhashIterator_free(RHashIterator *ghi);
/**
 * Steps the iterator to the next index.
 *
 * \param ghi: The iterator.
 */
void KLI_rhashIterator_step(RHashIterator *ghi);

KLI_INLINE void *KLI_rhashIterator_getKey(RHashIterator *ghi) ATTR_WARN_UNUSED_RESULT;
KLI_INLINE void *KLI_rhashIterator_getValue(RHashIterator *ghi) ATTR_WARN_UNUSED_RESULT;
KLI_INLINE void **KLI_rhashIterator_getValue_p(RHashIterator *ghi) ATTR_WARN_UNUSED_RESULT;
KLI_INLINE bool KLI_rhashIterator_done(const RHashIterator *ghi) ATTR_WARN_UNUSED_RESULT;

struct _gh_Entry
{
  void *next, *key, *val;
};
KLI_INLINE void *KLI_rhashIterator_getKey(RHashIterator *ghi)
{
  return ((struct _gh_Entry *)ghi->curEntry)->key;
}
KLI_INLINE void *KLI_rhashIterator_getValue(RHashIterator *ghi)
{
  return ((struct _gh_Entry *)ghi->curEntry)->val;
}
KLI_INLINE void **KLI_rhashIterator_getValue_p(RHashIterator *ghi)
{
  return &((struct _gh_Entry *)ghi->curEntry)->val;
}
KLI_INLINE bool KLI_rhashIterator_done(const RHashIterator *ghi)
{
  return !ghi->curEntry;
}
/* disallow further access */
#ifdef __GNUC__
#  pragma GCC poison _gh_Entry
#else
#  define _gh_Entry void
#endif

#define RHASH_ITER(gh_iter_, rhash_)                                                          \
  for (KLI_rhashIterator_init(&gh_iter_, rhash_); KLI_rhashIterator_done(&gh_iter_) == false; \
       KLI_rhashIterator_step(&gh_iter_))

#define RHASH_ITER_INDEX(gh_iter_, rhash_, i_)            \
  for (KLI_rhashIterator_init(&gh_iter_, rhash_), i_ = 0; \
       KLI_rhashIterator_done(&gh_iter_) == false;        \
       KLI_rhashIterator_step(&gh_iter_), i_++)

/** \} */

/* -------------------------------------------------------------------- */
/** \name RSet Types
 * A 'set' implementation (unordered collection of unique elements).
 *
 * Internally this is a 'RHash' without any keys,
 * which is why this API's are in the same header & source file.
 * \{ */

typedef struct RSet RSet;

typedef RHashHashFP RSetHashFP;
typedef RHashCmpFP RSetCmpFP;
typedef RHashKeyFreeFP RSetKeyFreeFP;
typedef RHashKeyCopyFP RSetKeyCopyFP;

typedef RHashIterState RSetIterState;

/** \} */

/** \name RSet Public API
 *
 * Use rhash API to give 'set' functionality
 * \{ */

RSet *KLI_rset_new_ex(RSetHashFP hashfp,
                      RSetCmpFP cmpfp,
                      const char *info,
                      unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_new(RSetHashFP hashfp,
                   RSetCmpFP cmpfp,
                   const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
/**
 * Copy given RSet. Keys are also copied if callback is provided, else pointers remain the same.
 */
RSet *KLI_rset_copy(const RSet *gs, RSetKeyCopyFP keycopyfp) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
unsigned int KLI_rset_len(const RSet *gs) ATTR_WARN_UNUSED_RESULT;
void KLI_rset_flag_set(RSet *gs, unsigned int flag);
void KLI_rset_flag_clear(RSet *gs, unsigned int flag);
void KLI_rset_free(RSet *gs, RSetKeyFreeFP keyfreefp);
/**
 * Adds the key to the set (no checks for unique keys!).
 * Matching #KLI_rhash_insert
 */
void KLI_rset_insert(RSet *gs, void *key);
/**
 * A version of KLI_rset_insert which checks first if the key is in the set.
 * \returns true if a new key has been added.
 *
 * \note RHash has no equivalent to this because typically the value would be different.
 */
bool KLI_rset_add(RSet *gs, void *key);
/**
 * Set counterpart to #KLI_rhash_ensure_p_ex.
 * similar to KLI_rset_add, except it returns the key pointer.
 *
 * \warning Caller _must_ write to \a r_key when returning false.
 */
bool KLI_rset_ensure_p_ex(RSet *gs, const void *key, void ***r_key);
/**
 * Adds the key to the set (duplicates are managed).
 * Matching #KLI_rhash_reinsert
 *
 * \returns true if a new key has been added.
 */
bool KLI_rset_reinsert(RSet *gh, void *key, RSetKeyFreeFP keyfreefp);
/**
 * Replaces the key to the set if it's found.
 * Matching #KLI_rhash_replace_key
 *
 * \returns The old key or NULL if not found.
 */
void *KLI_rset_replace_key(RSet *gs, void *key);
bool KLI_rset_haskey(const RSet *gs, const void *key) ATTR_WARN_UNUSED_RESULT;
/**
 * Remove a random entry from \a gs, returning true if a key could be removed, false otherwise.
 *
 * \param r_key: The removed key.
 * \param state: Used for efficient removal.
 * \return true if there was something to pop, false if rset was already empty.
 */
bool KLI_rset_pop(RSet *gs, RSetIterState *state, void **r_key) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();
bool KLI_rset_remove(RSet *gs, const void *key, RSetKeyFreeFP keyfreefp);
void KLI_rset_clear_ex(RSet *gs, RSetKeyFreeFP keyfreefp, unsigned int nentries_reserve);
void KLI_rset_clear(RSet *gs, RSetKeyFreeFP keyfreefp);

/* When set's are used for key & value. */
/**
 * Returns the pointer to the key if it's found.
 */
void *KLI_rset_lookup(const RSet *gs, const void *key) ATTR_WARN_UNUSED_RESULT;
/**
 * Returns the pointer to the key if it's found, removing it from the RSet.
 * \note Caller must handle freeing.
 */
void *KLI_rset_pop_key(RSet *gs, const void *key) ATTR_WARN_UNUSED_RESULT;

/** \} */

/* -------------------------------------------------------------------- */
/** \name RSet Iterator
 * \{ */

/* rely on inline api for now */

/** Use a RSet specific type so we can cast but compiler sees as different */
typedef struct RSetIterator
{
  RHashIterator _ghi
#if defined(__GNUC__) && !defined(__clang__)
    __attribute__((deprecated))
#endif
    ;
} RSetIterator;

KLI_INLINE RSetIterator *KLI_rsetIterator_new(RSet *gs)
{
  return (RSetIterator *)KLI_rhashIterator_new((RHash *)gs);
}
KLI_INLINE void KLI_rsetIterator_init(RSetIterator *gsi, RSet *gs)
{
  KLI_rhashIterator_init((RHashIterator *)gsi, (RHash *)gs);
}
KLI_INLINE void KLI_rsetIterator_free(RSetIterator *gsi)
{
  KLI_rhashIterator_free((RHashIterator *)gsi);
}
KLI_INLINE void *KLI_rsetIterator_getKey(RSetIterator *gsi)
{
  return KLI_rhashIterator_getKey((RHashIterator *)gsi);
}
KLI_INLINE void KLI_rsetIterator_step(RSetIterator *gsi)
{
  KLI_rhashIterator_step((RHashIterator *)gsi);
}
KLI_INLINE bool KLI_rsetIterator_done(const RSetIterator *gsi)
{
  return KLI_rhashIterator_done((const RHashIterator *)gsi);
}

#define RSET_ITER(gs_iter_, rset_)                                                         \
  for (KLI_rsetIterator_init(&gs_iter_, rset_); KLI_rsetIterator_done(&gs_iter_) == false; \
       KLI_rsetIterator_step(&gs_iter_))

#define RSET_ITER_INDEX(gs_iter_, rset_, i_)            \
  for (KLI_rsetIterator_init(&gs_iter_, rset_), i_ = 0; \
       KLI_rsetIterator_done(&gs_iter_) == false;       \
       KLI_rsetIterator_step(&gs_iter_), i_++)

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash/RSet Debugging API's
 * \{ */

/* For testing, debugging only */
#ifdef RHASH_INTERNAL_API
/**
 * \return number of buckets in the RHash.
 */
int KLI_rhash_buckets_len(const RHash *gh);
int KLI_rset_buckets_len(const RSet *gs);

/**
 * Measure how well the hash function performs (1.0 is approx as good as random distribution),
 * and return a few other stats like load,
 * variance of the distribution of the entries in the buckets, etc.
 *
 * Smaller is better!
 */
double KLI_rhash_calc_quality_ex(RHash *gh,
                                 double *r_load,
                                 double *r_variance,
                                 double *r_prop_empty_buckets,
                                 double *r_prop_overloaded_buckets,
                                 int *r_biggest_bucket);
double KLI_rset_calc_quality_ex(RSet *gs,
                                double *r_load,
                                double *r_variance,
                                double *r_prop_empty_buckets,
                                double *r_prop_overloaded_buckets,
                                int *r_biggest_bucket);
double KLI_rhash_calc_quality(RHash *gh);
double KLI_rset_calc_quality(RSet *gs);
#endif /* RHASH_INTERNAL_API */

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash/RSet Macros
 * \{ */

#define RHASH_FOREACH_BEGIN(type, var, what) \
  do {                                       \
    RHashIterator gh_iter##var;              \
    RHASH_ITER(gh_iter##var, what)           \
    {                                        \
      type var = (type)(KLI_rhashIterator_getValue(&gh_iter##var));

#define RHASH_FOREACH_END() \
  }                         \
  }                         \
  while (0)

#define RSET_FOREACH_BEGIN(type, var, what) \
  do {                                      \
    RSetIterator gh_iter##var;              \
    RSET_ITER(gh_iter##var, what)           \
    {                                       \
      type var = (type)(KLI_rsetIterator_getKey(&gh_iter##var));

#define RSET_FOREACH_END() \
  }                        \
  }                        \
  while (0)

/** \} */

/* -------------------------------------------------------------------- */
/** \name RHash/RSet Utils
 *
 * Defined in `KLI_rhash_utils.c`
 * \{ */

/**
 * Callbacks for RHash (`KLI_rhashutil_`)
 *
 * \note '_p' suffix denotes void pointer arg,
 * so we can have functions that take correctly typed args too.
 */

unsigned int KLI_rhashutil_ptrhash(const void *key);
bool KLI_rhashutil_ptrcmp(const void *a, const void *b);

/**
 * This function implements the widely used `djb` hash apparently posted
 * by Daniel Bernstein to `comp.lang.c` some time ago. The 32 bit
 * unsigned hash value starts at 5381 and for each byte 'c' in the
 * string, is updated: `hash = hash * 33 + c`.
 * This function uses the signed value of each byte.
 *
 * NOTE: this is the same hash method that glib 2.34.0 uses.
 */
unsigned int KLI_rhashutil_strhash_n(const char *key, size_t n);
#define KLI_rhashutil_strhash(key) \
  (CHECK_TYPE_ANY(key, char *, const char *, const char *const), KLI_rhashutil_strhash_p(key))
unsigned int KLI_rhashutil_strhash_p(const void *ptr);
unsigned int KLI_rhashutil_strhash_p_murmur(const void *ptr);
bool KLI_rhashutil_strcmp(const void *a, const void *b);

#define KLI_rhashutil_inthash(key) \
  (CHECK_TYPE_ANY(&(key), int *, const int *), KLI_rhashutil_uinthash((unsigned int)key))
unsigned int KLI_rhashutil_uinthash(unsigned int key);
unsigned int KLI_rhashutil_inthash_p(const void *ptr);
unsigned int KLI_rhashutil_inthash_p_murmur(const void *ptr);
unsigned int KLI_rhashutil_inthash_p_simple(const void *ptr);
bool KLI_rhashutil_intcmp(const void *a, const void *b);

size_t KLI_rhashutil_combine_hash(size_t hash_a, size_t hash_b);

unsigned int KLI_rhashutil_uinthash_v4(const unsigned int key[4]);
#define KLI_rhashutil_inthash_v4(key) \
  (CHECK_TYPE_ANY(key, int *, const int *), KLI_rhashutil_uinthash_v4((const unsigned int *)key))
#define KLI_rhashutil_inthash_v4_p ((RSetHashFP)KLI_rhashutil_uinthash_v4)
#define KLI_rhashutil_uinthash_v4_p ((RSetHashFP)KLI_rhashutil_uinthash_v4)
unsigned int KLI_rhashutil_uinthash_v4_murmur(const unsigned int key[4]);
#define KLI_rhashutil_inthash_v4_murmur(key) \
  (CHECK_TYPE_ANY(key, int *, const int *),  \
   KLI_rhashutil_uinthash_v4_murmur((const unsigned int *)key))
#define KLI_rhashutil_inthash_v4_p_murmur ((RSetHashFP)KLI_rhashutil_uinthash_v4_murmur)
#define KLI_rhashutil_uinthash_v4_p_murmur ((RSetHashFP)KLI_rhashutil_uinthash_v4_murmur)
bool KLI_rhashutil_uinthash_v4_cmp(const void *a, const void *b);
#define KLI_rhashutil_inthash_v4_cmp KLI_rhashutil_uinthash_v4_cmp

typedef struct RHashPair
{
  const void *first;
  const void *second;
} RHashPair;

RHashPair *KLI_rhashutil_pairalloc(const void *first, const void *second);
unsigned int KLI_rhashutil_pairhash(const void *ptr);
bool KLI_rhashutil_paircmp(const void *a, const void *b);
void KLI_rhashutil_pairfree(void *ptr);

/**
 * Wrapper RHash Creation Functions
 */

RHash *KLI_rhash_ptr_new_ex(const char *info,
                            unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_ptr_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_str_new_ex(const char *info,
                            unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_str_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_int_new_ex(const char *info,
                            unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_int_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_pair_new_ex(const char *info,
                             unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RHash *KLI_rhash_pair_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

RSet *KLI_rset_ptr_new_ex(const char *info,
                          unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_ptr_new(const char *info);
RSet *KLI_rset_str_new_ex(const char *info,
                          unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_str_new(const char *info);
RSet *KLI_rset_pair_new_ex(const char *info,
                           unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_pair_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_int_new_ex(const char *info,
                          unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
RSet *KLI_rset_int_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

/** \} */

#ifdef __cplusplus
}
#endif
