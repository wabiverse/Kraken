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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_utildefines.h"

#include "USD_ID.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AssetMetaData;
struct Key;
struct KrakenWriter;
struct RHash;
struct ID;
struct IDRemapper;
struct IDOverrideLibraryProperty;
struct IDOverrideLibraryPropertyOperation;
struct Library;
struct ListBase;
struct Main;
struct KrakenPRIM;
struct KrakenPROP;
struct kContext;
struct UniqueName_Map;
struct LibraryForeachIDData;

typedef enum eIDRemapType
{
  /** Remap an ID reference to a new reference. The new reference can also be null. */
  ID_REMAP_TYPE_REMAP = 0,

  /** Cleanup all IDs used by a specific one. */
  ID_REMAP_TYPE_CLEANUP = 1,
} eIDRemapType;

/* add any future new id property types here. */

/* Static ID override structs. */

typedef struct IDOverrideLibraryPropertyOperation
{
  struct IDOverrideLibraryPropertyOperation *next, *prev;

  /* Type of override. */
  short operation;
  short flag;

  /** Runtime, tags are common to both IDOverrideProperty and IDOverridePropertyOperation. */
  short tag;
  char _pad0[2];

  /* Sub-item references, if needed (for arrays or collections only).
   * We need both reference and local values to allow e.g. insertion into RNA collections
   * (constraints, modifiers...).
   * In RNA collection case, if names are defined, they are used in priority.
   * Names are pointers (instead of char[64]) to save some space, NULL or empty string when unset.
   * Indices are -1 when unset.
   *
   * NOTE: For insertion operations in RNA collections, reference may not actually exist in the
   * linked reference data. It is used to identify the anchor of the insertion operation (i.e. the
   * item after or before which the new local item should be inserted), in the local override. */
  char *subitem_reference_name;
  char *subitem_local_name;
  int subitem_reference_index;
  int subitem_local_index;
} IDOverrideLibraryPropertyOperation;

/**
 * Get allocation size of a given data-block type and optionally allocation name.
 */
size_t KKE_libblock_get_alloc_info(short type, const char **name);

/**
 * Allocates and returns memory of the right size for the specified block type,
 * initialized to zero.
 */
void *KKE_libblock_alloc_notest(short type) ATTR_WARN_UNUSED_RESULT;

/**
 * Allocates and returns a block of the specified type, with the specified name
 * (adjusted as necessary to ensure uniqueness), and appended to the specified list.
 * The user count is set to 1, all other content (apart from name and links) being
 * initialized to zero. */
void *KKE_libblock_alloc(struct Main *kmain, short type, const char *name, int flag)
  ATTR_WARN_UNUSED_RESULT;

/**
 * Initialize an ID of given type, such that it has valid 'empty' data.
 * ID is assumed to be just calloc'ed.
 */
void KKE_libblock_init_empty(struct ID *id) ATTR_NONNULL(1);

/**
 * Reset the runtime counters used by ID remapping.
 */
void KKE_libblock_runtime_reset_remapping_status(struct ID *id) ATTR_NONNULL(1);

bool KKE_lib_query_foreachid_iter_stop(struct LibraryForeachIDData *data);
void KKE_lib_query_foreachid_process(struct LibraryForeachIDData *data, ID **id_pp, int cb_flag);

void KKE_libblock_relink_ex(struct Main *kmain,
                            void *idv,
                            void *old_idv,
                            void *new_idv,
                            const short remap_flags);
void KKE_libblock_free_data(struct ID *id, const bool do_id_user);
void KKE_libblock_free_datablock(struct ID *id, const int flag);
void KKE_libblock_free_data_py(struct ID *id);
void KKE_libblock_relink_multiple(struct Main *kmain,
                                  struct LinkNode *ids,
                                  const eIDRemapType remap_type,
                                  struct IDRemapper *id_remapper,
                                  const short remap_flags);

void lib_override_library_property_operation_clear(
  struct IDOverrideLibraryPropertyOperation *opop);
void lib_override_library_property_clear(struct IDOverrideLibraryProperty *op);

void KKE_lib_override_library_clear(struct IDOverrideLibrary *override, const bool do_id_user);
void KKE_lib_override_library_free(struct IDOverrideLibrary **override, const bool do_id_user);
void KKE_asset_metadata_free(struct AssetMetaData **asset_data);


/**
 * When an ID's UUID is of that value, it is unset/invalid (e.g. for runtime IDs, etc.).
 */
#define MAIN_ID_SESSION_UUID_UNSET 0

/**
 * Return the owner ID of the given `id`, if any.
 *
 * @note This will only return non-NULL for embedded IDs (master collections etc.), and shape-keys.
 */
struct ID *KKE_id_owner_get(struct ID *id);

struct Key *KKE_key_from_id(ID *id);
struct Key **KKE_key_from_id_p(ID *id);

/**
 * Generic helper to create a new empty data-block of given type in given @a kmain database.
 *
 * @param name: can be NULL, in which case we get default name for this ID type.
 */
void *KKE_id_new(struct Main *kmain, short type, const char *name);

/**
 * Complete ID freeing, extended version for corner cases.
 * Can override default (and safe!) freeing process, to gain some speed up.
 *
 * At that point, given id is assumed to not be used by any other data-block already
 * (might not be actually true, in case e.g. several inter-related IDs get freed together...).
 * However, they might still be using (referencing) other IDs, this code takes care of it if
 * #LIB_TAG_NO_USER_REFCOUNT is not defined.
 *
 * @param kmain: #Main database containing the freed #ID,
 * can be NULL in case it's a temp ID outside of any #Main.
 * @param idv: Pointer to ID to be freed.
 * @param flag: Set of @a LIB_ID_FREE_... flags controlling/overriding usual freeing process,
 * 0 to get default safe behavior.
 * @param use_flag_from_idtag: Still use freeing info flags from given #ID data-block,
 * even if some overriding ones are passed in @a flag parameter.
 */
void KKE_id_free_ex(struct Main *kmain, void *idv, int flag, bool use_flag_from_idtag);

/**
 * Complete ID freeing, should be usable in most cases (even for out-of-Main IDs).
 *
 * See #KKE_id_free_ex description for full details.
 *
 * @param kmain: Main database containing the freed ID,
 * can be NULL in case it's a temp ID outside of any Main.
 * @param idv: Pointer to ID to be freed.
 */
void KKE_id_free(struct Main *kmain, void *idv);

/**
 * Ensures given ID has a unique name in given listbase.
 *
 * Only for local IDs (linked ones already have a unique ID in their library).
 *
 * @param do_linked_data: if true, also ensure a unique name in case the given @a id is linked
 * (otherwise, just ensure that it is properly sorted).
 *
 * @return true if a new name had to be created. */
bool KKE_id_new_name_validate(struct Main *kmain,
                              struct ListBase *lb,
                              struct ID *id,
                              const char *name,
                              bool do_linked_data) ATTR_NONNULL(1, 2, 3);

/**
 * New ID creation/copying options.
 */
enum
{
  /* *** Generic options (should be handled by all ID types copying, ID creation, etc.). *** */
  /** Create data-block outside of any main database -
   * similar to 'localize' functions of materials etc. */
  LIB_ID_CREATE_NO_MAIN = 1 << 0,
  /** Do not affect user refcount of data-blocks used by new one
   * (which also gets zero user-count then).
   * Implies LIB_ID_CREATE_NO_MAIN. */
  LIB_ID_CREATE_NO_USER_REFCOUNT = 1 << 1,
  /** Assume given 'newid' already points to allocated memory for whole data-block
   * (ID + data) - USE WITH CAUTION!
   * Implies LIB_ID_CREATE_NO_MAIN. */
  LIB_ID_CREATE_NO_ALLOCATE = 1 << 2,

  /** Do not tag new ID for update in depsgraph. */
  LIB_ID_CREATE_NO_DEG_TAG = 1 << 8,

  /** Very similar to #LIB_ID_CREATE_NO_MAIN, and should never be used with it (typically combined
   * with #LIB_ID_CREATE_LOCALIZE or #LIB_ID_COPY_LOCALIZE in fact).
   * It ensures that IDs created with it will get the #LIB_TAG_LOCALIZED tag, and uses some
   * specific code in some copy cases (mostly for node trees). */
  LIB_ID_CREATE_LOCAL = 1 << 9,

  /** Create for the depsgraph, when set #LIB_TAG_COPIED_ON_WRITE must be set.
   * Internally this is used to share some pointers instead of duplicating them. */
  LIB_ID_COPY_SET_COPIED_ON_WRITE = 1 << 10,

  /* *** Specific options to some ID types or usages. *** */
  /* *** May be ignored by unrelated ID copying functions. *** */
  /** Object only, needed by make_local code. */
  /* LIB_ID_COPY_NO_PROXY_CLEAR = 1 << 16, */ /* UNUSED */
  /** Do not copy preview data, when supported. */
  LIB_ID_COPY_NO_PREVIEW = 1 << 17,
  /** Copy runtime data caches. */
  LIB_ID_COPY_CACHES = 1 << 18,
  /** Don't copy `id->adt`, used by ID data-block localization routines. */
  LIB_ID_COPY_NO_ANIMDATA = 1 << 19,
  /** Mesh: Reference CD data layers instead of doing real copy - USE WITH CAUTION! */
  LIB_ID_COPY_CD_REFERENCE = 1 << 20,
  /** Do not copy id->override_library, used by ID data-block override routines. */
  LIB_ID_COPY_NO_LIB_OVERRIDE = 1 << 21,
  /** When copying local sub-data (like constraints or modifiers), do not set their "library
   * override local data" flag. */
  LIB_ID_COPY_NO_LIB_OVERRIDE_LOCAL_DATA_FLAG = 1 << 22,

  /* *** XXX Hackish/not-so-nice specific behaviors needed for some corner cases. *** */
  /* *** Ideally we should not have those, but we need them for now... *** */
  /** EXCEPTION! Deep-copy actions used by animation-data of copied ID. */
  LIB_ID_COPY_ACTIONS = 1 << 24,
  /** Keep the library pointer when copying data-block outside of kmain. */
  LIB_ID_COPY_KEEP_LIB = 1 << 25,
  /** EXCEPTION! Deep-copy shape-keys used by copied obdata ID. */
  LIB_ID_COPY_SHAPEKEY = 1 << 26,
  /** EXCEPTION! Specific deep-copy of node trees used e.g. for rendering purposes. */
  LIB_ID_COPY_NODETREE_LOCALIZE = 1 << 27,
  /**
   * EXCEPTION! Specific handling of RB objects regarding collections differs depending whether we
   * duplicate scene/collections, or objects.
   */
  LIB_ID_COPY_RIGID_BODY_NO_COLLECTION_HANDLING = 1 << 28,

  /* *** Helper 'defines' gathering most common flag sets. *** */
  /** Shape-keys are not real ID's, more like local data to geometry IDs. */
  LIB_ID_COPY_DEFAULT = LIB_ID_COPY_SHAPEKEY,

  /** Create a local, outside of kmain, data-block to work on. */
  LIB_ID_CREATE_LOCALIZE = LIB_ID_CREATE_NO_MAIN | LIB_ID_CREATE_NO_USER_REFCOUNT |
                           LIB_ID_CREATE_NO_DEG_TAG,
  /** Generate a local copy, outside of kmain, to work on (used by COW e.g.). */
  LIB_ID_COPY_LOCALIZE = LIB_ID_CREATE_LOCALIZE | LIB_ID_COPY_NO_PREVIEW | LIB_ID_COPY_CACHES |
                         LIB_ID_COPY_NO_LIB_OVERRIDE,
};

/**
 * Use after setting the ID's name
 * When name exists: call 'new_id'
 */
void KKE_libblock_ensure_unique_name(struct Main *kmain, const char *name);


/**
 * Duplicate (a.k.a. deep copy) common processing options.
 * See also eDupli_ID_Flags for options controlling what kind of IDs to duplicate.
 */
typedef enum eLibIDDuplicateFlags
{
  /**
   * This call to a duplicate function is part of another call for some parent ID.
   * Therefore, this sub-process should not clear `newid` pointers, nor handle remapping itself.
   * NOTE: In some cases (like Object one), the duplicate function may be called on the root ID
   * with this flag set, as remapping and/or other similar tasks need to be handled by the caller.
   */
  LIB_ID_DUPLICATE_IS_SUBPROCESS = 1 << 0,
  /** This call is performed on a 'root' ID, and should therefore perform some decisions regarding
   * sub-IDs (dependencies), check for linked vs. locale data, etc. */
  LIB_ID_DUPLICATE_IS_ROOT_ID = 1 << 1,
} eLibIDDuplicateFlags;

ENUM_OPERATORS(eLibIDDuplicateFlags, LIB_ID_DUPLICATE_IS_ROOT_ID)

/**
 * New freeing logic options.
 */
enum
{
  /* *** Generic options (should be handled by all ID types freeing). *** */
  /** Do not try to remove freed ID from given Main (passed Main may be NULL). */
  LIB_ID_FREE_NO_MAIN = 1 << 0,
  /**
   * Do not affect user refcount of data-blocks used by freed one.
   * Implies LIB_ID_FREE_NO_MAIN.
   */
  LIB_ID_FREE_NO_USER_REFCOUNT = 1 << 1,
  /**
   * Assume freed ID data-block memory is managed elsewhere, do not free it
   * (still calls relevant ID type's freeing function though) - USE WITH CAUTION!
   * Implies LIB_ID_FREE_NO_MAIN.
   */
  LIB_ID_FREE_NOT_ALLOCATED = 1 << 2,

  /** Do not tag freed ID for update in depsgraph. */
  LIB_ID_FREE_NO_DEG_TAG = 1 << 8,
  /** Do not attempt to remove freed ID from UI data/notifiers/... */
  LIB_ID_FREE_NO_UI_USER = 1 << 9,
  /** Do not remove freed ID's name from a potential runtime name-map. */
  LIB_ID_FREE_NO_NAMEMAP_REMOVE = 1 << 10,
};

/* Tips for the callback for cases it's gonna to modify the pointer. */
enum
{
  IDWALK_CB_NOP = 0,
  IDWALK_CB_NEVER_NULL = (1 << 0),
  IDWALK_CB_NEVER_SELF = (1 << 1),

  /**
   * Indicates whether this is direct (i.e. by local data) or indirect (i.e. by linked data) usage.
   */
  IDWALK_CB_INDIRECT_USAGE = (1 << 2),

  /**
   * That ID is used as mere sub-data by its owner (only case currently: those root nodetrees in
   * materials etc., and the Scene's master collections).
   * This means callback shall not *do* anything, only use this as informative data if it needs it.
   */
  IDWALK_CB_EMBEDDED = (1 << 3),

  /**
   * That ID is not really used by its owner, it's just an internal hint/helper.
   * This marks the 'from' pointers issue, like Key->from.
   * How to handle that kind of cases totally depends on what caller code is doing... */
  IDWALK_CB_LOOPBACK = (1 << 4),

  /** That ID is used as library override's reference by its owner. */
  IDWALK_CB_OVERRIDE_LIBRARY_REFERENCE = (1 << 5),

  /** That ID pointer is not overridable. */
  IDWALK_CB_OVERRIDE_LIBRARY_NOT_OVERRIDABLE = (1 << 6),

  /**
   * Indicates that this is an internal runtime ID pointer, like e.g. `ID.newid` or `ID.original`.
   * \note Those should be ignored in most cases, and won't be processed/generated anyway unless
   * `IDWALK_DO_INTERNAL_RUNTIME_POINTERS` option is enabled.
   */
  IDWALK_CB_INTERNAL = (1 << 7),

  /**
   * This ID usage is fully refcounted.
   * Callback is responsible to deal accordingly with #ID.us if needed.
   */
  IDWALK_CB_USER = (1 << 8),
  /**
   * This ID usage is not refcounted, but at least one user should be generated by it (to avoid
   * e.g. losing the used ID on save/reload).
   * Callback is responsible to deal accordingly with #ID.us if needed.
   */
  IDWALK_CB_USER_ONE = (1 << 9),
};

enum
{
  IDWALK_RET_NOP = 0,
  /** Completely stop iteration. */
  IDWALK_RET_STOP_ITER = 1 << 0,
  /** Stop recursion, that is, do not loop over ID used by current one. */
  IDWALK_RET_STOP_RECURSION = 1 << 1,
};

typedef struct LibraryIDLinkCallbackData
{
  void *user_data;
  /** Main database used to call `KKE_library_foreach_ID_link()`. */
  struct Main *kmain;
  /**
   * 'Real' ID, the one that might be in kmain, only differs from self_id when the later is an
   * embedded one.
   */
  struct ID *id_owner;
  /**
   * ID from which the current ID pointer is being processed. It may be an embedded ID like master
   * collection or root node tree.
   */
  struct ID *id_self;
  struct ID **id_pointer;
  int cb_flag;
} LibraryIDLinkCallbackData;

/**
 * Call a callback for each ID link which the given ID uses.
 *
 * \return a set of flags to control further iteration (0 to keep going).
 */
typedef int (*LibraryIDLinkCallback)(LibraryIDLinkCallbackData *cb_data);

/* Flags for the foreach function itself. */
enum
{
  IDWALK_NOP = 0,
  /** The callback will never modify the ID pointers it processes. */
  IDWALK_READONLY = (1 << 0),
  /** Recurse into 'descendant' IDs.
   * Each ID is only processed once. Order of ID processing is not guaranteed.
   *
   * Also implies IDWALK_READONLY, and excludes IDWALK_DO_INTERNAL_RUNTIME_POINTERS.
   *
   * NOTE: When enabled, embedded IDs are processed separately from their owner, as if they were
   * regular IDs. Owner ID is not available then in the #LibraryForeachIDData callback data.
   */
  IDWALK_RECURSE = (1 << 1),
  /** Include UI pointers (from WM and screens editors). */
  IDWALK_INCLUDE_UI = (1 << 2),
  /** Do not process ID pointers inside embedded IDs. Needed by depsgraph processing e.g. */
  IDWALK_IGNORE_EMBEDDED_ID = (1 << 3),

  /** Also process internal ID pointers like `ID.newid` or `ID.orig_id`.
   *  WARNING: Dangerous, use with caution. */
  IDWALK_DO_INTERNAL_RUNTIME_POINTERS = (1 << 9),
};

typedef void (*KKE_library_free_notifier_reference_cb)(const void *);
typedef void (*KKE_library_remap_editor_id_reference_cb)(const struct IDRemapper *mappings);

/* IDRemapper */
struct IDRemapper;
typedef enum IDRemapperApplyResult
{
  /** No remapping rules available for the source. */
  ID_REMAP_RESULT_SOURCE_UNAVAILABLE,
  /** Source isn't mappable (e.g. NULL). */
  ID_REMAP_RESULT_SOURCE_NOT_MAPPABLE,
  /** Source has been remapped to a new pointer. */
  ID_REMAP_RESULT_SOURCE_REMAPPED,
  /** Source has been set to NULL. */
  ID_REMAP_RESULT_SOURCE_UNASSIGNED,
} IDRemapperApplyResult;

typedef enum IDRemapperApplyOptions
{
  /**
   * Update the user count of the old and new ID data-block.
   *
   * For remapping the old ID users will be decremented and the new ID users will be
   * incremented. When un-assigning the old ID users will be decremented.
   *
   * NOTE: Currently unused by main remapping code, since user-count is handled by
   * `foreach_libblock_remap_callback_apply` there, depending on whether the remapped pointer does
   * use it or not. Need for rare cases in UI handling though (see e.g. `image_id_remap` in
   * `space_image.c`).
   */
  ID_REMAP_APPLY_UPDATE_REFCOUNT = (1 << 0),

  /**
   * Make sure that the new ID data-block will have a 'real' user.
   *
   * NOTE: See Note for #ID_REMAP_APPLY_UPDATE_REFCOUNT above.
   */
  ID_REMAP_APPLY_ENSURE_REAL = (1 << 1),

  /**
   * Unassign in stead of remap when the new ID data-block would become id_self.
   *
   * To use this option 'KKE_id_remapper_apply_ex' must be used with a not-null id_self parameter.
   */
  ID_REMAP_APPLY_UNMAP_WHEN_REMAPPING_TO_SELF = (1 << 2),

  ID_REMAP_APPLY_DEFAULT = 0,
} IDRemapperApplyOptions;

/* Also IDRemap->flag. */
enum
{
  /** Do not remap indirect usages of IDs (that is, when user is some linked data). */
  ID_REMAP_SKIP_INDIRECT_USAGE = 1 << 0,
  /**
   * This flag should always be set, *except for 'unlink' scenarios*
   * (only relevant when new_id == NULL).
   * Basically, when unset, NEVER_NULL ID usages will keep pointing to old_id, but (if needed)
   * old_id user count will still be decremented.
   * This is mandatory for 'delete ID' case,
   * but in all other situation this would lead to invalid user counts!
   */
  ID_REMAP_SKIP_NEVER_NULL_USAGE = 1 << 1,
  /**
   * This tells the callback func to flag with #LIB_DOIT all IDs
   * using target one with a 'never NULL' pointer (like e.g. #Object.data).
   */
  ID_REMAP_FLAG_NEVER_NULL_USAGE = 1 << 2,
  /**
   * This tells the callback func to force setting IDs
   * using target one with a 'never NULL' pointer to NULL.
   * \warning Use with extreme care, this will leave database in broken state
   * and can cause crashes very easily!
   */
  ID_REMAP_FORCE_NEVER_NULL_USAGE = 1 << 3,
  /** Do not remap library override pointers. */
  ID_REMAP_SKIP_OVERRIDE_LIBRARY = 1 << 5,
  /** Don't touch the special user counts (use when the 'old' remapped ID remains in use):
   * - Do not transfer 'fake user' status from old to new ID.
   * - Do not clear 'extra user' from old ID. */
  ID_REMAP_SKIP_USER_CLEAR = 1 << 6,
  /**
   * Force internal ID runtime pointers (like `ID.newid`, `ID.orig_id` etc.) to also be processed.
   * This should only be needed in some very specific cases, typically only BKE ID management code
   * should need it (e.g. required from `id_delete` to ensure no runtime pointer remains using
   * freed ones).
   */
  ID_REMAP_FORCE_INTERNAL_RUNTIME_POINTERS = 1 << 7,
  /** Force handling user count even for IDs that are outside of Main (used in some cases when
   * dealing with IDs temporarily out of Main, but which will be put in it ultimately).
   */
  ID_REMAP_FORCE_USER_REFCOUNT = 1 << 8,
  /**
   * Force obdata pointers to also be processed, even when object (`id_owner`) is in Edit mode.
   * This is required by some tools creating/deleting IDs while operating in Edit mode, like e.g.
   * the 'separate' mesh operator.
   */
  ID_REMAP_FORCE_OBDATA_IN_EDITMODE = 1 << 9,
};

typedef void (*IDRemapperIterFunction)(struct ID *old_id, struct ID *new_id, void *user_data);

/**
 * Create a new ID Remapper.
 *
 * An ID remapper stores multiple remapping rules.
 */
struct IDRemapper *KKE_id_remapper_create(void);
void KKE_id_remapper_free(struct IDRemapper *id_remapper);
void KKE_id_remapper_clear(struct IDRemapper *id_remapper);
bool KKE_id_remapper_is_empty(const struct IDRemapper *id_remapper);
void KKE_id_remapper_add(struct IDRemapper *id_remapper, struct ID *old_id, struct ID *new_id);
bool KKE_id_remapper_has_mapping_for(const struct IDRemapper *id_remapper, uint64_t type_filter);
IDRemapperApplyResult KKE_id_remapper_get_mapping_result(const struct IDRemapper *id_remapper,
                                                         struct ID *id,
                                                         IDRemapperApplyOptions options,
                                                         const struct ID *id_self);
IDRemapperApplyResult KKE_id_remapper_apply_ex(const struct IDRemapper *id_remapper,
                                               struct ID **r_id_ptr,
                                               const IDRemapperApplyOptions options,
                                               struct ID *id_self);
void KKE_id_remapper_iter(const struct IDRemapper *id_remapper,
                          IDRemapperIterFunction func,
                          void *user_data);
/* status */
enum
{
  IDWALK_STOP = 1 << 0,
};

#define KKE_LIB_FOREACHID_PROCESS_ID(_data, _id, _cb_flag)               \
  {                                                                      \
    CHECK_TYPE_ANY((_id), ID *, void *);                                 \
    KKE_lib_query_foreachid_process((_data), (ID **)&(_id), (_cb_flag)); \
    if (KKE_lib_query_foreachid_iter_stop((_data))) {                    \
      return;                                                            \
    }                                                                    \
  }                                                                      \
  ((void)0)

#define KKE_LIB_FOREACHID_PROCESS_IDSUPER(_data, _id_super, _cb_flag)          \
  {                                                                            \
    CHECK_TYPE(&((_id_super)->id), ID *);                                      \
    KKE_lib_query_foreachid_process((_data), (ID **)&(_id_super), (_cb_flag)); \
    if (KKE_lib_query_foreachid_iter_stop((_data))) {                          \
      return;                                                                  \
    }                                                                          \
  }                                                                            \
  ((void)0)

#define KKE_LIB_FOREACHID_PROCESS_FUNCTION_CALL(_data, _func_call) \
  {                                                                \
    _func_call;                                                    \
    if (KKE_lib_query_foreachid_iter_stop((_data))) {              \
      return;                                                      \
    }                                                              \
  }                                                                \
  ((void)0)

void KKE_lib_query_idpropertiesForeachIDLink_callback(struct IDProperty *id_prop, void *user_data);

void KKE_library_foreach_ID_link(struct Main *kmain,
                                 struct ID *id,
                                 LibraryIDLinkCallback callback,
                                 void *user_data,
                                 int flag);
uint64_t KKE_library_id_can_use_filter_id(const struct ID *id_owner);

void KKE_lib_libblock_session_uuid_ensure(struct ID *id);

/* --------- */

void id_us_plus(struct ID *id);
void id_us_min(struct ID *id);
void id_us_ensure_real(ID *id);
void id_us_plus_no_lib(struct ID *id);
void id_lib_extern(struct ID *id);
void id_us_clear_real(struct ID *id);
void id_fake_user_set(struct ID *id);
void id_fake_user_clear(struct ID *id);
void id_sort_by_name(struct ListBase *lb, struct ID *id, struct ID *id_sorting_hint);

/* --------- */

struct UniqueName_Map *KKE_main_namemap_create(void) ATTR_WARN_UNUSED_RESULT;
void KKE_main_namemap_destroy(struct UniqueName_Map **r_name_map) ATTR_NONNULL();

/**
 * Ensures the given name is unique within the given ID type.
 *
 * In case of name collisions, the name will be adjusted to be unique.
 *
 * @return true if the name had to be adjusted for uniqueness. */
bool KKE_main_namemap_get_name(struct Main *kmain, struct ID *id, char *name);

/**
 * Remove a given name from usage.
 *
 * Call this whenever deleting or renaming an object. */
void KKE_main_namemap_remove_name(struct Main *kmain, struct ID *id, const char *name)
  ATTR_NONNULL();

/**
 * Check that all ID names in given `kmain` are unique (per ID type and library), and that existing
 * name maps are consistent with existing relevant IDs.
 *
 * This is typically called within an assert, or in tests. */
bool KKE_main_namemap_validate(struct Main *kmain) ATTR_NONNULL();

/**
 * Same as #KKE_main_namemap_validate, but also fixes any issue by re-generating all name maps,
 * and ensuring again all ID names are unique.
 *
 * This is typically only used in `do_versions` code to fix broken files.
 */
bool KKE_main_namemap_validate_and_fix(struct Main *kmain) ATTR_NONNULL();

#ifdef __cplusplus
}
#endif