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

#pragma once

#include "wabi/base/arch/defines.h"

#include "USD_scene.h"

#include "KKE_api.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/token.h>

#define _RHASH_INTERNAL_ATTR
#ifndef RHASH_INTERNAL_API
#  ifdef __GNUC__
#    undef _RHASH_INTERNAL_ATTR
#    define _RHASH_INTERNAL_ATTR __attribute__((deprecated)) /* not deprecated, just private. */
#  endif
#endif

KRAKEN_NAMESPACE_BEGIN

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

struct RHash
{
  RHashHashFP hashfp;
  RHashCmpFP cmpfp;

  struct Entry **buckets;
  struct KKE_mempool *entrypool;
  uint nbuckets;
  uint limit_grow, limit_shrink;
  uint cursize, size_min;

  uint nentries;
  uint flag;
};

struct RHashIterator
{
  RHash *rh;
  struct Entry *curEntry;
  unsigned int curBucket;
};

struct RHashIterState
{
  unsigned int curr_bucket _RHASH_INTERNAL_ATTR;
};

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

struct LockfreeLinkNode
{
  struct LockfreeLinkNode *next;
  /* NOTE: "Subclass" this structure to add custom-defined data. */
};

struct LockfreeLinkList
{
  /* We keep a dummy node at the beginning of the list all the time.
   * This allows us to make sure head and tail pointers are always
   * valid, and saves from annoying exception cases in insert().
   */
  LockfreeLinkNode dummy_node;
  /* NOTE: This fields might point to a dummy node. */
  LockfreeLinkNode *head, *tail;
};

typedef void (*LockfreeeLinkNodeFreeFP)(void *link);


void KKE_linklist_lockfree_init(LockfreeLinkList *list);


/* -------------------------------------------------------------------- */
/** \name RHash API
 *
 * Defined in `KKE_hash.cpp`
 * \{ */

/**
 * Creates a new, empty GHash.
 *
 * \param hashfp: Hash callback.
 * \param cmpfp: Comparison callback.
 * \param info: Identifier string for the GHash.
 * \param nentries_reserve: Optionally reserve the number of members that the hash will hold.
 * Use this to avoid resizing buckets if the size is known or can be closely approximated.
 * \return  An empty GHash.
 */
RHash *KKE_rhash_new_ex(RHashHashFP hashfp,
                        RHashCmpFP cmpfp,
                        const char *info,
                        unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;
/**
 * Wraps #KKE_rhash_new_ex with zero entries reserved.
 */
RHash *KKE_rhash_new(RHashHashFP hashfp,
                     RHashCmpFP cmpfp,
                     const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

RHash *KKE_rhash_str_new_ex(const char *info,
                            unsigned int nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

RHash *KKE_rhash_str_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

RHash *KKE_rhash_int_new_ex(const char *info,
                            const uint nentries_reserve) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

RHash *KKE_rhash_int_new(const char *info) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

/**
 * Copy given GHash. Keys and values are also copied if relevant callback is provided,
 * else pointers remain the same.
 */
RHash *KKE_rhash_copy(const RHash *gh,
                      RHashKeyCopyFP keycopyfp,
                      RHashValCopyFP valcopyfp) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

void *KKE_rhash_lookup(RHash *rh, const void *key);
void *KKE_rhash_lookup(RHash *rh, const TfToken &key);
void KKE_rhash_insert(RHash *rh, const TfToken &key, void *value);

/** Aligned with #PropertyUnit and `kpyunits_ucategories_items` in `kpy_utils_units.cpp`. */
enum
{
  K_UNIT_NONE = 0,
  K_UNIT_LENGTH = 1,
  K_UNIT_AREA = 2,
  K_UNIT_VOLUME = 3,
  K_UNIT_MASS = 4,
  K_UNIT_ROTATION = 5,
  K_UNIT_TIME = 6,
  K_UNIT_TIME_ABSOLUTE = 7,
  K_UNIT_VELOCITY = 8,
  K_UNIT_ACCELERATION = 9,
  K_UNIT_CAMERA = 10,
  K_UNIT_POWER = 11,
  K_UNIT_TEMPERATURE = 12,
  K_UNIT_TYPE_TOT = 13,
};

std::string kraken_exe_path_init(void);
std::string kraken_system_tempdir_path(void);

std::string kraken_datafiles_path_init(void);
std::string kraken_fonts_path_init(void);
std::string kraken_python_path_init(void);
std::string kraken_icon_path_init(void);
std::string kraken_startup_file_init(void);
std::string kraken_ocio_file_init(void);

size_t KKE_unit_value_as_string(char *str,
                                int len_max,
                                double value,
                                int prec,
                                int type,
                                const struct UnitSettings *settings,
                                bool pad);

KRAKEN_NAMESPACE_END