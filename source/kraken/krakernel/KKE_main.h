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

#ifndef __KRAKEN_KERNEL_MAIN_H__
#define __KRAKEN_KERNEL_MAIN_H__

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "USD_listBase.h"

#include "KLI_compiler_attrs.h"
#include "KLI_sys_types.h"

#define FILE_MAXDIR 768
#define FILE_MAXFILE 256
#define FILE_MAX 1024

#ifdef __cplusplus
extern "C" {
#endif

struct ID;
struct kContext;
struct KLI_mempool;
struct KrakenThumbnail;
struct RHash;
struct RSet;
struct IDNameLib_Map;
struct ImBuf;
struct Library;
struct MainLock;
struct UniqueName_Map;

/* Kraken thumbnail, as written on file (width, height, and data as char RGBA). */
/* We pack pixel data after that struct. */
typedef struct KrakenThumbnail
{
  int width, height;
  char rect[0];
} KrakenThumbnail;

/* Structs caching relations between data-blocks in a given Main. */
typedef struct MainIDRelationsEntryItem
{
  struct MainIDRelationsEntryItem *next;

  union
  {
    /* For `from_ids` list, a user of the hashed ID. */
    struct ID *from;
    /* For `to_ids` list, an ID used by the hashed ID. */
    struct ID **to;
  } id_pointer;
  /* Session uuid of the `id_pointer`. */
  uint session_uuid;

  int usage_flag; /* Using IDWALK_ enums, defined in BKE_lib_query.h */
} MainIDRelationsEntryItem;

typedef struct MainIDRelationsEntry
{
  /* Linked list of IDs using that ID. */
  struct MainIDRelationsEntryItem *from_ids;
  /* Linked list of IDs used by that ID. */
  struct MainIDRelationsEntryItem *to_ids;

  /* Session uuid of the ID matching that entry. */
  uint session_uuid;

  /* Runtime tags, users should ensure those are reset after usage. */
  uint tags;
} MainIDRelationsEntry;

/** #MainIDRelationsEntry.tags */
typedef enum eMainIDRelationsEntryTags
{
  /* Generic tag marking the entry as to be processed. */
  MAINIDRELATIONS_ENTRY_TAGS_DOIT = 1 << 0,

  /* Generic tag marking the entry as processed in the `to` direction (i.e. we processed the IDs
   * used by this item). */
  MAINIDRELATIONS_ENTRY_TAGS_PROCESSED_TO = 1 << 1,
  /* Generic tag marking the entry as processed in the `from` direction (i.e. we processed the IDs
   * using by this item). */
  MAINIDRELATIONS_ENTRY_TAGS_PROCESSED_FROM = 1 << 2,
  /* Generic tag marking the entry as processed. */
  MAINIDRELATIONS_ENTRY_TAGS_PROCESSED = MAINIDRELATIONS_ENTRY_TAGS_PROCESSED_TO |
                                         MAINIDRELATIONS_ENTRY_TAGS_PROCESSED_FROM,
} eMainIDRelationsEntryTags;

typedef struct MainIDRelations
{
  /* Mapping from an ID pointer to all of its parents (IDs using it) and children (IDs it uses).
   * Values are `MainIDRelationsEntry` pointers. */
  struct RHash *relations_from_pointers;
  /* NOTE: we could add more mappings when needed (e.g. from session uuid?). */

  short flag;

  /* Private... */
  struct KLI_mempool *entry_items_pool;
} MainIDRelations;

enum
{
  /* Those bmain relations include pointers/usages from editors. */
  MAINIDRELATIONS_INCLUDE_UI = 1 << 0,
};

typedef struct Main
{
  struct Main *next, *prev;

  /** The file-path of this usd file, an empty string indicates an unsaved file. */
  char filepath[1024];
  short versionfile, subversionfile;
  short minversionfile, minsubversionfile;

  uint64_t build_commit_timestamp;
  char build_hash[16];

  bool recovered;
  bool is_memfile_undo_written;
  bool is_memfile_undo_flush_needed;
  bool use_memfile_full_barrier;
  bool is_locked_for_linking;

  /**
   * True if this main is the 'GMAIN' of current Kraken.
   *
   * @note There should always be only one global main, all others generated temporarily for
   * various data management process must have this property set to false..
   */
  bool is_global_main;

  KrakenThumbnail *kraken_thumb;

  char launch_time[80];

  /**
   * @note
   * Like Blender, we use doubly-linked lists for all linked lists
   * in the Kraken library system. This effectively allows Pixar Stage
   * traversal of all prim types in a way that makes it compatible with
   * C, and likewise, use the same API across C, Python, CXX, and Swift.
   */
  struct Library *curlib;
  ListBase scenes;
  ListBase libraries;
  ListBase objects;
  ListBase meshes;
  ListBase curves;
  ListBase materials;
  ListBase metaballs;
  ListBase textures;
  ListBase images;
  ListBase lattices;
  ListBase lights;
  ListBase cameras;
  ListBase shapekeys;
  ListBase worlds;
  ListBase screens;
  ListBase fonts;
  ListBase texts;
  ListBase speakers;
  ListBase lightprobes;
  ListBase sounds;
  ListBase collections;
  ListBase armatures;
  ListBase actions;
  ListBase nodetrees;
  ListBase brushes;
  ListBase particles;
  ListBase palettes;
  ListBase paintcurves;
  ListBase wm;
  ListBase gpencils;
  ListBase movieclips;
  ListBase masks;
  ListBase linestyles;
  ListBase cachefiles;
  ListBase workspaces;
  ListBase pointclouds;
  ListBase volumes;
  ListBase simulations;

  /**
   * Must be generated, used and freed by same code - never assume this is valid data unless you
   * know when, who and how it was created.
   * Used by code doing a lot of remapping etc. at once to speed things up.
   */
  struct MainIDRelations *relations;

  /* IDMap of IDs. Currently used when reading (expanding) libraries. */
  struct IDNameLib_Map *id_map;

  /* Used for efficient calculations of unique names. */
  struct UniqueName_Map *name_map;

  struct MainLock *lock;
} Main;

/* because it ends cool. */
#define KRAKEN_GODSPEED 0
enum kkeStatusCode
{
  KRAKEN_SUCCESS = 0,
  KRAKEN_ERROR,
};

struct Main *KKE_main_new(void);
void KKE_main_free(struct Main *mainvar);
void KKE_kraken_free(void);

void KKE_main_lock(struct Main *kmain);
void KKE_main_unlock(struct Main *kmain);

void KKE_kraken_atexit(void);
void KKE_kraken_atexit_register(void (*func)(void *user_data), void *user_data);
void KKE_kraken_atexit_unregister(void (*func)(void *user_data), const void *user_data);

void KKE_kraken_plugins_init(void);
void KKE_kraken_enable_debug_codes(void);

const char *KKE_kraken_version_string(void);

/* *** Generic utils to loop over whole Main database. *** */

#define FOREACH_MAIN_LISTBASE_ID_BEGIN(_lb, _id)              \
  {                                                           \
    ID *_id_next = (ID *)(_lb)->first;                        \
    for ((_id) = _id_next; (_id) != NULL; (_id) = _id_next) { \
      _id_next = (ID *)(_id)->next;

#define FOREACH_MAIN_LISTBASE_ID_END \
  }                                  \
  }                                  \
  ((void)0)

#define FOREACH_MAIN_LISTBASE_BEGIN(_bmain, _lb)       \
  {                                                    \
    ListBase *_lbarray[INDEX_ID_MAX];                  \
    int _i = set_listbasepointers((_bmain), _lbarray); \
    while (_i--) {                                     \
      (_lb) = _lbarray[_i];

#define FOREACH_MAIN_LISTBASE_END \
  }                               \
  }                               \
  ((void)0)

/**
 * Top level `foreach`-like macro allowing to loop over all IDs in a given #Main data-base.
 *
 * NOTE: Order tries to go from 'user IDs' to 'used IDs' (e.g. collections will be processed
 * before objects, which will be processed before obdata types, etc.).
 *
 * WARNING: DO NOT use break statement with that macro, use #FOREACH_MAIN_LISTBASE and
 * #FOREACH_MAIN_LISTBASE_ID instead if you need that kind of control flow. */
#define FOREACH_MAIN_ID_BEGIN(_bmain, _id)     \
  {                                            \
    ListBase *_lb;                             \
    FOREACH_MAIN_LISTBASE_BEGIN((_bmain), _lb) \
    {                                          \
      FOREACH_MAIN_LISTBASE_ID_BEGIN(_lb, (_id))

#define FOREACH_MAIN_ID_END     \
  FOREACH_MAIN_LISTBASE_ID_END; \
  }                             \
  FOREACH_MAIN_LISTBASE_END;    \
  }                             \
  ((void)0)

/**
 * Return file-path of given @a main.
 */
const char *KKE_main_usdfile_path(const struct Main *kmain);

/**
 * @return A pointer to the @a ListBase of given @a kmain for requested @a type ID type.
 */
struct ListBase *which_libbase(struct Main *kmain, short type);

//#define INDEX_ID_MAX 39
/**
 * Put the pointers to all the #ListBase structs in given `bmain` into the `*lb[INDEX_ID_MAX]`
 * array, and return the number of those for convenience.
 *
 * This is useful for generic traversal of all the blocks in a #Main (by traversing all the lists
 * in turn), without worrying about block types.
 *
 * \param lb: Array of lists #INDEX_ID_MAX in length.
 *
 * \note The order of each ID type #ListBase in the array is determined by the `INDEX_ID_<IDTYPE>`
 * enum definitions in `DNA_ID.h`. See also the #FOREACH_MAIN_ID_BEGIN macro in `BKE_main.h`
 */
int set_listbasepointers(struct Main *main, struct ListBase *lb[]);

#define MAIN_VERSION_ATLEAST(main, ver, subver) \
  ((main)->versionfile > (ver) ||               \
   ((main)->versionfile == (ver) && (main)->subversionfile >= (subver)))

#define MAIN_VERSION_OLDER(main, ver, subver) \
  ((main)->versionfile < (ver) ||             \
   ((main)->versionfile == (ver) && (main)->subversionfile < (subver)))

/**
 * The size of thumbnails (optionally) stored in the `.usd` files header.
 *
 * NOTE: This is kept small as it's stored uncompressed in the `.usd` file,
 * where a larger size would increase the size of every `.usd` file unreasonably.
 * If we wanted to increase the size, we'd want to use compression (JPEG or similar). */
#define KRAKEN_THUMB_SIZE 128

#define KRAKEN_THUMB_MEMSIZE(_x, _y) \
  (sizeof(KrakenThumbnail) + ((size_t)(_x) * (size_t)(_y)) * sizeof(int))
/** Protect against buffer overflow vulnerability & negative sizes. */
#define KRAKEN_THUMB_MEMSIZE_IS_VALID(_x, _y) \
  (((_x) > 0 && (_y) > 0) && ((uint64_t)(_x) * (uint64_t)(_y) < (SIZE_MAX / (sizeof(int) * 4))))

#ifdef __cplusplus
}
#endif

#endif /* __KRAKEN_KERNEL_MAIN_H__ */
