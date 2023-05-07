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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLG_log.h"

#include "MEM_guardedalloc.h"

#include "USD_ID.h"
#include "USD_collection.h"
#include "USD_defs.h"
#include "USD_wm_types.h"
#include "USD_object_types.h"
#include "USD_lights.h"
#include "USD_linestyles.h"
#include "USD_materials.h"
#include "USD_scene_types.h"
#include "USD_simulation.h"
#include "USD_space_types.h"
#include "USD_texture_types.h"
#include "USD_world.h"

#include "KLI_utildefines.h"

#include "KLI_alloca.h"
#include "KLI_assert.h"
#include "KLI_bitmap.h"
#include "KLI_kraklib.h"
#include "KLI_linklist.h"
#include "KLI_listbase.h"
#include "KLI_mempool.h"
#include "KLI_map.hh"
#include "KLI_math_base.hh"
#include "KLI_rhash.h"
#include "KLI_set.hh"
#include "KLI_string_utf8.h"
#include "KLI_string_utils.h"

#include "KKE_utils.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_idprop.h"
#include "KKE_idtype.h"
#include "KKE_lib_id.h"

#include "LUXO_access.h"

#include "atomic_ops.h"

#include "lib_intern.h"

#ifdef WITH_PYTHON
#  include "KPY_extern.h"
#endif

static CLG_LogRef LOG = {.identifier = "kke.lib_id"};

//#define DEBUG_PRINT_MEMORY_USAGE

KKE_library_free_notifier_reference_cb free_notifier_reference_cb = nullptr;

void KKE_library_callback_free_notifier_reference_set(KKE_library_free_notifier_reference_cb func)
{
  free_notifier_reference_cb = func;
}

KKE_library_remap_editor_id_reference_cb remap_editor_id_reference_cb = NULL;

void KKE_library_callback_remap_editor_id_reference_set(
  KKE_library_remap_editor_id_reference_cb func)
{
  remap_editor_id_reference_cb = func;
}

typedef struct IDRemap
{
  eIDRemapType type;
  Main *kmain; /* Only used to trigger depsgraph updates in the right kmain. */

  struct IDRemapper *id_remapper;

  /** The ID in which we are replacing old_id by new_id usages. */
  ID *id_owner;
  short flag;
} IDRemap;

using namespace kraken;

/* Assumes and ensure that the suffix number can never go beyond 1 billion. */
#define MAX_NUMBER 1000000000
/* We do not want to get "name.000", so minimal number is 1. */
#define MIN_NUMBER 1

IDTypeInfo IDType_ID_LINK_PLACEHOLDER = {
  .id_code = ID_LINK_PLACEHOLDER,
  .id_filter = 0,
  .main_listbase_index = INDEX_ID_NULL,
  .struct_size = sizeof(ID),
  .name = "LinkPlaceholder",
  .name_plural = "link_placeholders",
  .translation_context = "ID",
  .flags = IDTYPE_FLAGS_NO_COPY | IDTYPE_FLAGS_NO_LIBLINKING,
  .asset_type_info = nullptr,

  .init_data = nullptr,
  .copy_data = nullptr,
  .free_data = nullptr,
  .make_local = nullptr,
  .foreach_id = nullptr,
  .foreach_cache = nullptr,
  .foreach_path = nullptr,
  .owner_pointer_get = nullptr,

  .usd_write = nullptr,
  .usd_read_data = nullptr,
  .usd_read_lib = nullptr,
  .usd_read_expand = nullptr,

  .usd_read_undo_preserve = nullptr,

  .lib_override_apply_post = nullptr,
};

void id_us_plus(ID *id)
{
  if (id) {
    id_us_plus_no_lib(id);
    id_lib_extern(id);
  }
}

void id_us_min(ID *id)
{
  if (id) {
    const int limit = ID_FAKE_USERS(id);

    if (id->us <= limit) {
      /* Do not assert on deprecated ID types, we cannot really ensure that their ID refcounting
       * is valid... */
      CLOG_ERROR(&LOG,
                 "ID user decrement error: %s (from '%s'): %d <= %d",
                 id->name,
                 id->lib ? id->lib->filepath_abs : "[Main]",
                 id->us,
                 limit);
      id->us = limit;
    } else {
      id->us--;
    }

    if ((id->us == limit) && (id->tag & LIB_TAG_EXTRAUSER)) {
      /* We need an extra user here, but never actually incremented user count for it so far,
       * do it now. */
      id_us_ensure_real(id);
    }
  }
}

void id_us_ensure_real(ID *id)
{
  if (id) {
    const int limit = ID_FAKE_USERS(id);
    id->tag |= LIB_TAG_EXTRAUSER;
    if (id->us <= limit) {
      if (id->us < limit || ((id->us == limit) && (id->tag & LIB_TAG_EXTRAUSER_SET))) {
        CLOG_ERROR(&LOG,
                   "ID user count error: %s (from '%s')",
                   id->name,
                   id->lib ? id->lib->filepath_abs : "[Main]");
      }
      id->us = limit + 1;
      id->tag |= LIB_TAG_EXTRAUSER_SET;
    }
  }
}

void id_us_plus_no_lib(ID *id)
{
  if (id) {
    if ((id->tag & LIB_TAG_EXTRAUSER) && (id->tag & LIB_TAG_EXTRAUSER_SET)) {
      KLI_assert(id->us >= 1);
      /* No need to increase count, just tag extra user as no more set.
       * Avoids annoying & inconsistent +1 in user count. */
      id->tag &= ~LIB_TAG_EXTRAUSER_SET;
    } else {
      KLI_assert(id->us >= 0);
      id->us++;
    }
  }
}

void id_lib_extern(ID *id)
{
  if (id && ID_IS_LINKED(id)) {
    KLI_assert(KKE_idtype_idcode_is_linkable(GS(id->name)));
    if (id->tag & LIB_TAG_INDIRECT) {
      id->tag &= ~LIB_TAG_INDIRECT;
      id->flag &= ~LIB_INDIRECT_WEAK_LINK;
      id->tag |= LIB_TAG_EXTERN;
      id->lib->parent = nullptr;
    }
  }
}

void id_us_clear_real(ID *id)
{
  if (id && (id->tag & LIB_TAG_EXTRAUSER)) {
    if (id->tag & LIB_TAG_EXTRAUSER_SET) {
      id->us--;
      KLI_assert(id->us >= ID_FAKE_USERS(id));
    }
    id->tag &= ~(LIB_TAG_EXTRAUSER | LIB_TAG_EXTRAUSER_SET);
  }
}

void id_fake_user_set(ID *id)
{
  if (id && !(id->flag & LIB_FAKEUSER)) {
    id->flag |= LIB_FAKEUSER;
    id_us_plus(id);
  }
}

void id_fake_user_clear(ID *id)
{
  if (id && (id->flag & LIB_FAKEUSER)) {
    id->flag &= ~LIB_FAKEUSER;
    id_us_min(id);
  }
}

void id_sort_by_name(ListBase *lb, ID *id, ID *id_sorting_hint)
{
#define ID_SORT_STEP_SIZE 512

  ID *idtest;

  /* insert alphabetically */
  if (lb->first == lb->last) {
    return;
  }

  KLI_remlink(lb, id);

  /* Check if we can actually insert id before or after id_sorting_hint, if given. */
  if (!ELEM(id_sorting_hint, nullptr, id) && id_sorting_hint->lib == id->lib) {
    KLI_assert(KLI_findindex(lb, id_sorting_hint) >= 0);

    ID *id_sorting_hint_next = (ID *)id_sorting_hint->next;
    if (KLI_strcasecmp(id_sorting_hint->name, id->name) < 0 &&
        (id_sorting_hint_next == nullptr || id_sorting_hint_next->lib != id->lib ||
         KLI_strcasecmp(id_sorting_hint_next->name, id->name) > 0)) {
      KLI_insertlinkafter(lb, id_sorting_hint, id);
      return;
    }

    ID *id_sorting_hint_prev = (ID *)id_sorting_hint->prev;
    if (KLI_strcasecmp(id_sorting_hint->name, id->name) > 0 &&
        (id_sorting_hint_prev == nullptr || id_sorting_hint_prev->lib != id->lib ||
         KLI_strcasecmp(id_sorting_hint_prev->name, id->name) < 0)) {
      KLI_insertlinkbefore(lb, id_sorting_hint, id);
      return;
    }
  }

  void *item_array[ID_SORT_STEP_SIZE];
  int item_array_index;

  /* Step one: We go backward over a whole chunk of items at once, until we find a limit item
   * that is lower than, or equal (should never happen!) to the one we want to insert. */
  /* NOTE: We start from the end, because in typical 'heavy' case (insertion of lots of IDs at
   * once using the same base name), newly inserted items will generally be towards the end
   * (higher extension numbers). */
  bool is_in_library = false;
  item_array_index = ID_SORT_STEP_SIZE - 1;
  for (idtest = (ID *)lb->last; idtest != nullptr; idtest = (ID *)idtest->prev) {
    if (is_in_library) {
      if (idtest->lib != id->lib) {
        /* We got out of expected library 'range' in the list, so we are done here and can move on
         * to the next step. */
        break;
      }
    } else if (idtest->lib == id->lib) {
      /* We are entering the expected library 'range' of IDs in the list. */
      is_in_library = true;
    }

    if (!is_in_library) {
      continue;
    }

    item_array[item_array_index] = idtest;
    if (item_array_index == 0) {
      if (KLI_strcasecmp(idtest->name, id->name) <= 0) {
        break;
      }
      item_array_index = ID_SORT_STEP_SIZE;
    }
    item_array_index--;
  }

  /* Step two: we go forward in the selected chunk of items and check all of them, as we know
   * that our target is in there. */

  /* If we reached start of the list, current item_array_index is off-by-one.
   * Otherwise, we already know that it points to an item lower-or-equal-than the one we want to
   * insert, no need to redo the check for that one.
   * So we can increment that index in any case. */
  for (item_array_index++; item_array_index < ID_SORT_STEP_SIZE; item_array_index++) {
    idtest = (ID *)item_array[item_array_index];
    if (KLI_strcasecmp(idtest->name, id->name) > 0) {
      KLI_insertlinkbefore(lb, idtest, id);
      break;
    }
  }
  if (item_array_index == ID_SORT_STEP_SIZE) {
    if (idtest == nullptr) {
      /* If idtest is nullptr here, it means that in the first loop, the last comparison was
       * performed exactly on the first item of the list, and that it also failed. And that the
       * second loop was not walked at all.
       *
       * In other words, if `id` is local, all the items in the list are greater than the inserted
       * one, so we can put it at the start of the list. Or, if `id` is linked, it is the first one
       * of its library, and we can put it at the very end of the list. */
      if (ID_IS_LINKED(id)) {
        KLI_addtail(lb, id);
      } else {
        KLI_addhead(lb, id);
      }
    } else {
      KLI_insertlinkafter(lb, idtest, id);
    }
  }

#undef ID_SORT_STEP_SIZE
}

/* *********** ALLOC AND FREE *****************
 *
 * KKE_libblock_free(ListBase *lb, ID *id )
 * provide a list-basis and data-block, but only ID is read
 *
 * void *KKE_libblock_alloc(ListBase *lb, type, name)
 * inserts in list and returns a new ID
 *
 * **************************** */

size_t KKE_libblock_get_alloc_info(short type, const char **name)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(type);

  if (id_type == NULL) {
    if (name != NULL) {
      *name = NULL;
    }
    return 0;
  }

  if (name != NULL) {
    *name = id_type->name;
  }
  return id_type->struct_size;
}

void *KKE_libblock_alloc_notest(short type)
{
  const char *name;
  size_t size = KKE_libblock_get_alloc_info(type, &name);
  if (size != 0) {
    return MEM_callocN(size, name);
  }
  KLI_assert_msg(0, "Request to allocate unknown data type");
  return NULL;
}

/* ********** ID session-wise UUID management. ********** */
static uint global_session_uuid = 0;

void KKE_lib_libblock_session_uuid_ensure(ID *id)
{
  if (id->session_uuid == MAIN_ID_SESSION_UUID_UNSET) {
    KLI_assert((id->tag & LIB_TAG_TEMP_MAIN) == 0); /* Caller must ensure this. */
    id->session_uuid = atomic_add_and_fetch_uint32(&global_session_uuid, 1);
    /* In case overflow happens, still assign a valid ID. This way opening files many times works
     * correctly. */
    if (UNLIKELY(id->session_uuid == MAIN_ID_SESSION_UUID_UNSET)) {
      id->session_uuid = atomic_add_and_fetch_uint32(&global_session_uuid, 1);
    }
  }
}

void *KKE_libblock_alloc(Main *kmain, short type, const char *name, const int flag)
{
  KLI_assert((flag & LIB_ID_CREATE_NO_ALLOCATE) == 0);
  KLI_assert((flag & LIB_ID_CREATE_NO_MAIN) != 0 || kmain != NULL);
  KLI_assert((flag & LIB_ID_CREATE_NO_MAIN) != 0 || (flag & LIB_ID_CREATE_LOCAL) == 0);

  ID *id = (ID *)KKE_libblock_alloc_notest(type);

  if (id) {
    if ((flag & LIB_ID_CREATE_NO_MAIN) != 0) {
      id->tag |= LIB_TAG_NO_MAIN;
    }
    if ((flag & LIB_ID_CREATE_NO_USER_REFCOUNT) != 0) {
      id->tag |= LIB_TAG_NO_USER_REFCOUNT;
    }
    if (flag & LIB_ID_CREATE_LOCAL) {
      id->tag |= LIB_TAG_LOCALIZED;
    }

    id->icon_id = 0;
    *((short *)id->name) = type;
    if ((flag & LIB_ID_CREATE_NO_USER_REFCOUNT) == 0) {
      id->us = 1;
    }
    if ((flag & LIB_ID_CREATE_NO_MAIN) == 0) {
      /* Note that 2.8x versioning has tested not to cause conflicts. Node trees are
       * skipped in this check to allow adding a geometry node tree for versioning. */
      KLI_assert(kmain->is_locked_for_linking == false || ELEM(type, ID_WS, ID_GR, ID_NT));
      ListBase *lb = which_libbase(kmain, type);

      KKE_main_lock(kmain);
      KLI_addtail(lb, id);
      KKE_id_new_name_validate(kmain, lb, id, name, false);
      kmain->is_memfile_undo_written = false;
      /* alphabetic insertion: is in new_id */
      KKE_main_unlock(kmain);

      /* This assert avoids having to keep name_map consistency when changing the library of an ID,
       * if this check is not true anymore it will have to be done here too. */
      KLI_assert(kmain->curlib == NULL || kmain->curlib->runtime.name_map == NULL);
      /* This is important in 'readfile doversion after liblink' context mainly, but is a good
       * consistency change in general: ID created for a Main should get that main's current
       * library pointer. */
      id->lib = kmain->curlib;

      /* TODO: to be removed from here! */
      // if ((flag & LIB_ID_CREATE_NO_DEG_TAG) == 0) {
      //   DEG_id_type_tag(kmain, type);
      // }
    } else {
      KLI_strncpy(id->name + 2, name, sizeof(id->name) - 2);
    }

    /* We also need to ensure a valid `session_uuid` for some non-main data (like embedded IDs).
     * IDs not allocated however should not need those (this would e.g. avoid generating session
     * uuids for depsgraph CoW IDs, if it was using this function). */
    if ((flag & LIB_ID_CREATE_NO_ALLOCATE) == 0) {
      KKE_lib_libblock_session_uuid_ensure(id);
    }
  }

  return id;
}

ID *KKE_id_owner_get(ID *id)
{
  const IDTypeInfo *idtype = KKE_idtype_get_info_from_id(id);
  if (idtype->owner_pointer_get != nullptr) {
    ID **owner_id_pointer = idtype->owner_pointer_get(id);
    if (owner_id_pointer != nullptr) {
      return *owner_id_pointer;
    }
  }
  return nullptr;
}

void *KKE_id_new(Main *kmain, const short type, const char *name)
{
  KLI_assert(kmain != NULL);

  if (name == NULL) {
    name = DATA_(KKE_idtype_idcode_to_name(type));
  }

  ID *id = (ID *)KKE_libblock_alloc(kmain, type, name, 0);
  KKE_libblock_init_empty(id);

  return id;
}

void KKE_libblock_init_empty(ID *id)
{
  const IDTypeInfo *idtype_info = KKE_idtype_get_info_from_id(id);

  if (idtype_info != NULL) {
    if (idtype_info->init_data != NULL) {
      idtype_info->init_data(id);
    }
    return;
  }

  KLI_assert_msg(0, "IDType Missing IDTypeInfo");
}

void KKE_libblock_runtime_reset_remapping_status(ID *id)
{
  id->runtime.remap.status = 0;
  id->runtime.remap.skipped_refcounted = 0;
  id->runtime.remap.skipped_direct = 0;
  id->runtime.remap.skipped_indirect = 0;
}

void KKE_libblock_ensure_unique_name(Main *kmain, const char *name)
{
  ListBase *lb;
  ID *idtest;

  lb = which_libbase(kmain, GS(name));
  if (lb == nullptr) {
    return;
  }

  /* search for id */
  idtest = (ID *)KLI_findstring(lb, name + 2, offsetof(ID, name) + 2);
  if (idtest != nullptr && !ID_IS_LINKED(idtest)) {
    /* KKE_id_new_name_validate also takes care of sorting. */
    KKE_id_new_name_validate(kmain, lb, idtest, nullptr, false);
    kmain->is_memfile_undo_written = false;
  }
}

Key **KKE_key_from_id_p(ID *id)
{
  switch (GS(id->name)) {
    case ID_ME: {
      Mesh *me = (Mesh *)id;
      return &me->key;
    }
    case ID_CV: {
      Curve *cu = (Curve *)id;
      if (cu->vfont == NULL) {
        return &cu->key;
      }
      break;
    }
    case ID_LT: {
      Lattice *lt = (Lattice *)id;
      return &lt->key;
    }
    default:
      break;
  }

  return nullptr;
}

Key *KKE_key_from_id(ID *id)
{
  Key **key_p;
  key_p = KKE_key_from_id_p(id);
  if (key_p) {
    return *key_p;
  }

  return NULL;
}

void KKE_id_free_ex(Main *kmain, void *idv, int flag, const bool use_flag_from_idtag)
{
  ID *id = static_cast<ID *>(idv);

  if (use_flag_from_idtag) {
    if ((id->tag & LIB_TAG_NO_MAIN) != 0) {
      flag |= LIB_ID_FREE_NO_MAIN | LIB_ID_FREE_NO_UI_USER | LIB_ID_FREE_NO_DEG_TAG;
    } else {
      flag &= ~LIB_ID_FREE_NO_MAIN;
    }

    if ((id->tag & LIB_TAG_NO_USER_REFCOUNT) != 0) {
      flag |= LIB_ID_FREE_NO_USER_REFCOUNT;
    } else {
      flag &= ~LIB_ID_FREE_NO_USER_REFCOUNT;
    }

    if ((id->tag & LIB_TAG_NOT_ALLOCATED) != 0) {
      flag |= LIB_ID_FREE_NOT_ALLOCATED;
    } else {
      flag &= ~LIB_ID_FREE_NOT_ALLOCATED;
    }
  }

  KLI_assert((flag & LIB_ID_FREE_NO_MAIN) != 0 || kmain != NULL);
  KLI_assert((flag & LIB_ID_FREE_NO_MAIN) != 0 || (flag & LIB_ID_FREE_NOT_ALLOCATED) == 0);
  KLI_assert((flag & LIB_ID_FREE_NO_MAIN) != 0 || (flag & LIB_ID_FREE_NO_USER_REFCOUNT) == 0);

  const short type = GS(id->name);

  if (kmain && (flag & LIB_ID_FREE_NO_DEG_TAG) == 0) {
    KLI_assert(kmain->is_locked_for_linking == false);

    // DEG_id_type_tag(kmain, type);
  }

  KKE_libblock_free_data_py(id);

  Key *key = ((flag & LIB_ID_FREE_NO_MAIN) == 0) ? KKE_key_from_id(id) : NULL;

  if ((flag & LIB_ID_FREE_NO_USER_REFCOUNT) == 0) {
    KKE_libblock_relink_ex(kmain, id, NULL, NULL, ID_REMAP_SKIP_USER_CLEAR);
  }

  if ((flag & LIB_ID_FREE_NO_MAIN) == 0 && key != NULL) {
    KKE_id_free_ex(kmain, &key->id, flag, use_flag_from_idtag);
  }

  KKE_libblock_free_datablock(id, flag);

  /* avoid notifying on removed data */
  if ((flag & LIB_ID_FREE_NO_MAIN) == 0) {
    KKE_main_lock(kmain);
  }

  if ((flag & LIB_ID_FREE_NO_UI_USER) == 0) {
    if (free_notifier_reference_cb) {
      free_notifier_reference_cb(id);
    }

    if (remap_editor_id_reference_cb) {
      struct IDRemapper *remapper = KKE_id_remapper_create();
      KKE_id_remapper_add(remapper, id, NULL);
      remap_editor_id_reference_cb(remapper);
      KKE_id_remapper_free(remapper);
    }
  }

  if ((flag & LIB_ID_FREE_NO_MAIN) == 0) {
    ListBase *lb = which_libbase(kmain, type);
    KLI_remlink(lb, id);
    if ((flag & LIB_ID_FREE_NO_NAMEMAP_REMOVE) == 0) {
      KKE_main_namemap_remove_name(kmain, id, id->name + 2);
    }
  }

  KKE_libblock_free_data(id, (flag & LIB_ID_FREE_NO_USER_REFCOUNT) == 0);

  if ((flag & LIB_ID_FREE_NO_MAIN) == 0) {
    KKE_main_unlock(kmain);
  }

  if ((flag & LIB_ID_FREE_NOT_ALLOCATED) == 0) {
    MEM_freeN(id);
  }
}

using IDTypeFilter = uint64_t;

namespace kraken::kke::id::remapper
{
  struct IDRemapper
  {
   private:

    Map<ID *, ID *> mappings;
    IDTypeFilter source_types = 0;

   public:

    void clear()
    {
      mappings.clear();
      source_types = 0;
    }

    bool is_empty() const
    {
      return mappings.is_empty();
    }

    void add(ID *old_id, ID *new_id)
    {
      KLI_assert(old_id != nullptr);
      KLI_assert(new_id == nullptr || (GS(old_id->name) == GS(new_id->name)));
      mappings.add(old_id, new_id);
      KLI_assert(KKE_idtype_idcode_to_idfilter(GS(old_id->name)) != 0);
      source_types |= KKE_idtype_idcode_to_idfilter(GS(old_id->name));
    }

    bool contains_mappings_for_any(IDTypeFilter filter) const
    {
      return (source_types & filter) != 0;
    }

    IDRemapperApplyResult get_mapping_result(ID *id,
                                             IDRemapperApplyOptions options,
                                             const ID *id_self) const
    {
      if (!mappings.contains(id)) {
        return ID_REMAP_RESULT_SOURCE_UNAVAILABLE;
      }
      const ID *new_id = mappings.lookup(id);
      if ((options & ID_REMAP_APPLY_UNMAP_WHEN_REMAPPING_TO_SELF) != 0 && id_self == new_id) {
        new_id = nullptr;
      }
      if (new_id == nullptr) {
        return ID_REMAP_RESULT_SOURCE_UNASSIGNED;
      }

      return ID_REMAP_RESULT_SOURCE_REMAPPED;
    }

    IDRemapperApplyResult apply(ID **r_id_ptr, IDRemapperApplyOptions options, ID *id_self) const
    {
      KLI_assert(r_id_ptr != nullptr);
      if (*r_id_ptr == nullptr) {
        return ID_REMAP_RESULT_SOURCE_NOT_MAPPABLE;
      }

      if (!mappings.contains(*r_id_ptr)) {
        return ID_REMAP_RESULT_SOURCE_UNAVAILABLE;
      }

      if (options & ID_REMAP_APPLY_UPDATE_REFCOUNT) {
        id_us_min(*r_id_ptr);
      }

      *r_id_ptr = mappings.lookup(*r_id_ptr);
      if (options & ID_REMAP_APPLY_UNMAP_WHEN_REMAPPING_TO_SELF && *r_id_ptr == id_self) {
        *r_id_ptr = nullptr;
      }
      if (*r_id_ptr == nullptr) {
        return ID_REMAP_RESULT_SOURCE_UNASSIGNED;
      }

      if (options & ID_REMAP_APPLY_UPDATE_REFCOUNT) {
        /* Do not handle LIB_TAG_INDIRECT/LIB_TAG_EXTERN here. */
        id_us_plus_no_lib(*r_id_ptr);
      }

      if (options & ID_REMAP_APPLY_ENSURE_REAL) {
        id_us_ensure_real(*r_id_ptr);
      }
      return ID_REMAP_RESULT_SOURCE_REMAPPED;
    }

    void iter(IDRemapperIterFunction func, void *user_data) const
    {
      for (auto item : mappings.items()) {
        func(item.key, item.value, user_data);
      }
    }
  };

}  // namespace kraken::kke::id::remapper

/** @brief wrap CPP IDRemapper to a C handle. */
static IDRemapper *wrap(kraken::kke::id::remapper::IDRemapper *remapper)
{
  return static_cast<IDRemapper *>(static_cast<void *>(remapper));
}

/** @brief wrap C handle to a CPP IDRemapper. */
static kraken::kke::id::remapper::IDRemapper *unwrap(IDRemapper *remapper)
{
  return static_cast<kraken::kke::id::remapper::IDRemapper *>(static_cast<void *>(remapper));
}

/** @brief wrap C handle to a CPP IDRemapper. */
static const kraken::kke::id::remapper::IDRemapper *unwrap(const IDRemapper *remapper)
{
  return static_cast<const kraken::kke::id::remapper::IDRemapper *>(
    static_cast<const void *>(remapper));
}

extern "C" {
IDRemapper *KKE_id_remapper_create(void)
{
  kraken::kke::id::remapper::IDRemapper *remapper = MEM_new<kraken::kke::id::remapper::IDRemapper>(
    __func__);
  return wrap(remapper);
}

void KKE_id_remapper_free(IDRemapper *id_remapper)
{
  kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  MEM_delete<kraken::kke::id::remapper::IDRemapper>(remapper);
}

void KKE_id_remapper_clear(struct IDRemapper *id_remapper)
{
  kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  remapper->clear();
}

bool KKE_id_remapper_is_empty(const struct IDRemapper *id_remapper)
{
  const kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  return remapper->is_empty();
}

void KKE_id_remapper_add(IDRemapper *id_remapper, ID *old_id, ID *new_id)
{
  kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  remapper->add(old_id, new_id);
}

bool KKE_id_remapper_has_mapping_for(const struct IDRemapper *id_remapper, uint64_t type_filter)
{
  const kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  return remapper->contains_mappings_for_any(type_filter);
}

IDRemapperApplyResult KKE_id_remapper_get_mapping_result(const struct IDRemapper *id_remapper,
                                                         struct ID *id,
                                                         IDRemapperApplyOptions options,
                                                         const struct ID *id_self)
{
  const kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  return remapper->get_mapping_result(id, options, id_self);
}

IDRemapperApplyResult KKE_id_remapper_apply_ex(const IDRemapper *id_remapper,
                                               ID **r_id_ptr,
                                               const IDRemapperApplyOptions options,
                                               ID *id_self)
{
  KLI_assert_msg((options & ID_REMAP_APPLY_UNMAP_WHEN_REMAPPING_TO_SELF) == 0 ||
                   id_self != nullptr,
                 "ID_REMAP_APPLY_WHEN_REMAPPING_TO_SELF requires id_self parameter.");
  const kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  return remapper->apply(r_id_ptr, options, id_self);
}

void KKE_id_remapper_iter(const struct IDRemapper *id_remapper,
                          IDRemapperIterFunction func,
                          void *user_data)
{
  const kraken::kke::id::remapper::IDRemapper *remapper = unwrap(id_remapper);
  remapper->iter(func, user_data);
}
}

static void libblock_remap_reset_remapping_status_callback(ID *old_id,
                                                           ID *new_id,
                                                           void *UNUSED(user_data))
{
  KKE_libblock_runtime_reset_remapping_status(old_id);
  if (new_id != NULL) {
    KKE_libblock_runtime_reset_remapping_status(new_id);
  }
}

static void foreach_libblock_remap_callback_skip(const ID *UNUSED(id_owner),
                                                 ID **id_ptr,
                                                 const int cb_flag,
                                                 const bool is_indirect,
                                                 const bool is_reference,
                                                 const bool violates_never_null,
                                                 const bool UNUSED(is_obj),
                                                 const bool is_obj_editmode)
{
  ID *id = *id_ptr;
  KLI_assert(id != NULL);

  if (is_indirect) {
    id->runtime.remap.skipped_indirect++;
  } else if (violates_never_null || is_obj_editmode || is_reference) {
    id->runtime.remap.skipped_direct++;
  } else {
    KLI_assert_unreachable();
  }

  if (cb_flag & IDWALK_CB_USER) {
    id->runtime.remap.skipped_refcounted++;
  } else if (cb_flag & IDWALK_CB_USER_ONE) {
    /* No need to count number of times this happens, just a flag is enough. */
    id->runtime.remap.status |= ID_REMAP_IS_USER_ONE_SKIPPED;
  }
}

static void foreach_libblock_remap_callback_apply(ID *id_owner,
                                                  ID *id_self,
                                                  ID **id_ptr,
                                                  IDRemap *id_remap_data,
                                                  const struct IDRemapper *mappings,
                                                  const IDRemapperApplyOptions id_remapper_options,
                                                  const int cb_flag,
                                                  const bool is_indirect,
                                                  const bool violates_never_null,
                                                  const bool force_user_refcount)
{
  ID *old_id = *id_ptr;
  if (!violates_never_null) {
    KKE_id_remapper_apply_ex(mappings, id_ptr, id_remapper_options, id_self);
    // DEG_id_tag_update_ex(id_remap_data->kmain,
    //                      id_self,
    //                      ID_RECALC_COPY_ON_WRITE | ID_RECALC_TRANSFORM | ID_RECALC_GEOMETRY);
    if (id_self != id_owner) {
      // DEG_id_tag_update_ex(id_remap_data->kmain,
      //                      id_owner,
      //                      ID_RECALC_COPY_ON_WRITE | ID_RECALC_TRANSFORM | ID_RECALC_GEOMETRY);
    }
  }
  /* Get the new_id pointer. When the mapping is violating never null we should use a NULL
   * pointer otherwise the incorrect users are decreased and increased on the same instance. */
  ID *new_id = violates_never_null ? NULL : *id_ptr;

  if (cb_flag & IDWALK_CB_USER) {
    /* NOTE: by default we don't user-count IDs which are not in the main database.
     * This is because in certain conditions we can have data-blocks in
     * the main which are referencing data-blocks outside of it.
     * For example, KKE_mesh_new_from_object() called on an evaluated
     * object will cause such situation.
     */
    if (force_user_refcount || (old_id->tag & LIB_TAG_NO_MAIN) == 0) {
      id_us_min(old_id);
    }
    if (new_id != NULL && (force_user_refcount || (new_id->tag & LIB_TAG_NO_MAIN) == 0)) {
      /* Do not handle LIB_TAG_INDIRECT/LIB_TAG_EXTERN here. */
      id_us_plus_no_lib(new_id);
    }
  } else if (cb_flag & IDWALK_CB_USER_ONE) {
    id_us_ensure_real(new_id);
    /* We cannot affect old_id->us directly, LIB_TAG_EXTRAUSER(_SET)
     * are assumed to be set as needed, that extra user is processed in final handling. */
  }
  if (!is_indirect && new_id) {
    new_id->runtime.remap.status |= ID_REMAP_IS_LINKED_DIRECT;
  }
}

static bool KKE_object_is_in_editmode(const Object *ob)
{
  if (ob->data == nullptr) {
    return false;
  }

  switch (ob->type) {
    case OB_MESH:
      return ((Mesh *)ob->data)->edit_mesh != nullptr;
    // case OB_ARMATURE:
    //   return ((kArmature *)ob->data)->edbo != nullptr;
    case OB_FONT:
      return ((Curve *)ob->data)->editfont != nullptr;
    // case OB_MBALL:
    //   return ((MetaBall *)ob->data)->editelems != nullptr;
    case OB_LATTICE:
      return ((Lattice *)ob->data)->editlatt != nullptr;
    case OB_SURF:
    case OB_CURVES:
      return ((Curve *)ob->data)->editnurb != nullptr;
    // case OB_GPENCIL:
    //   /* Grease Pencil object has no edit mode data. */
    //   return GPENCIL_EDIT_MODE((kGPdata *)ob->data);
    default:
      return false;
  }
}

static int foreach_libblock_remap_callback(LibraryIDLinkCallbackData *cb_data)
{
  const int cb_flag = cb_data->cb_flag;

  if (cb_flag & IDWALK_CB_EMBEDDED) {
    return IDWALK_RET_NOP;
  }

  ID *id_owner = cb_data->id_owner;
  ID *id_self = cb_data->id_self;
  ID **id_p = cb_data->id_pointer;
  IDRemap *id_remap_data = (IDRemap *)cb_data->user_data;

  /* Those asserts ensure the general sanity of ID tags regarding 'embedded' ID data (root
   * nodetrees and co). */
  KLI_assert(id_owner == id_remap_data->id_owner);
  KLI_assert(id_self == id_owner || (id_self->flag & LIB_EMBEDDED_DATA) != 0);

  /* Early exit when id pointer isn't set. */
  if (*id_p == NULL) {
    return IDWALK_RET_NOP;
  }

  struct IDRemapper *id_remapper = id_remap_data->id_remapper;
  IDRemapperApplyOptions id_remapper_options = ID_REMAP_APPLY_DEFAULT;

  /* Used to cleanup all IDs used by a specific one. */
  if (id_remap_data->type == ID_REMAP_TYPE_CLEANUP) {
    /* Clearing existing instance to reduce potential lookup times for IDs referencing many other
     * IDs. This makes sure that there will only be a single rule in the id_remapper. */
    KKE_id_remapper_clear(id_remapper);
    KKE_id_remapper_add(id_remapper, *id_p, NULL);
  }

  /* Better remap to NULL than not remapping at all,
   * then we can handle it as a regular remap-to-NULL case. */
  if (cb_flag & IDWALK_CB_NEVER_SELF) {
    id_remapper_options = static_cast<IDRemapperApplyOptions>(
      id_remapper_options | ID_REMAP_APPLY_UNMAP_WHEN_REMAPPING_TO_SELF);
  }

  const IDRemapperApplyResult expected_mapping_result =
    KKE_id_remapper_get_mapping_result(id_remapper, *id_p, id_remapper_options, id_self);
  /* Exit, when no modifications will be done; ensuring id->runtime counters won't changed. */
  if (ELEM(expected_mapping_result,
           ID_REMAP_RESULT_SOURCE_UNAVAILABLE,
           ID_REMAP_RESULT_SOURCE_NOT_MAPPABLE)) {
    KLI_assert_msg(id_remap_data->type == ID_REMAP_TYPE_REMAP,
                   "Cleanup should always do unassign.");
    return IDWALK_RET_NOP;
  }

  const bool is_reference = (cb_flag & IDWALK_CB_OVERRIDE_LIBRARY_REFERENCE) != 0;
  const bool is_indirect = (cb_flag & IDWALK_CB_INDIRECT_USAGE) != 0;
  const bool skip_indirect = (id_remap_data->flag & ID_REMAP_SKIP_INDIRECT_USAGE) != 0;
  const bool is_obj = (GS(id_owner->name) == ID_OB);
  /* NOTE: Edit Mode is a 'skip direct' case, unless specifically requested, obdata should not be
   * remapped in this situation. */
  const bool is_obj_editmode = (is_obj && KKE_object_is_in_editmode((Object *)id_owner) &&
                                (id_remap_data->flag & ID_REMAP_FORCE_OBDATA_IN_EDITMODE) == 0);
  const bool violates_never_null = ((cb_flag & IDWALK_CB_NEVER_NULL) &&
                                    (expected_mapping_result ==
                                     ID_REMAP_RESULT_SOURCE_UNASSIGNED) &&
                                    (id_remap_data->flag & ID_REMAP_FORCE_NEVER_NULL_USAGE) == 0);
  const bool skip_reference = (id_remap_data->flag & ID_REMAP_SKIP_OVERRIDE_LIBRARY) != 0;
  const bool skip_never_null = (id_remap_data->flag & ID_REMAP_SKIP_NEVER_NULL_USAGE) != 0;
  const bool force_user_refcount = (id_remap_data->flag & ID_REMAP_FORCE_USER_REFCOUNT) != 0;

#ifdef DEBUG_PRINT
  printf(
    "In %s (lib %p): Remapping %s (%p) remap operation: %s "
    "(is_indirect: %d, skip_indirect: %d, is_reference: %d, skip_reference: %d)\n",
    id_owner->name,
    id_owner->lib,
    (*id_p)->name,
    *id_p,
    KKE_id_remapper_result_string(expected_mapping_result),
    is_indirect,
    skip_indirect,
    is_reference,
    skip_reference);
#endif

  if ((id_remap_data->flag & ID_REMAP_FLAG_NEVER_NULL_USAGE) && (cb_flag & IDWALK_CB_NEVER_NULL)) {
    id_owner->tag |= LIB_TAG_DOIT;
  }

  /* Special hack in case it's Object->data and we are in edit mode, and new_id is not NULL
   * (otherwise, we follow common NEVER_NULL flags).
   * (skipped_indirect too). */
  if ((violates_never_null && skip_never_null) ||
      (is_obj_editmode && (((Object *)id_owner)->data == *id_p) &&
       (expected_mapping_result == ID_REMAP_RESULT_SOURCE_REMAPPED)) ||
      (skip_indirect && is_indirect) || (is_reference && skip_reference)) {
    foreach_libblock_remap_callback_skip(id_owner,
                                         id_p,
                                         cb_flag,
                                         is_indirect,
                                         is_reference,
                                         violates_never_null,
                                         is_obj,
                                         is_obj_editmode);
  } else {
    foreach_libblock_remap_callback_apply(id_owner,
                                          id_self,
                                          id_p,
                                          id_remap_data,
                                          id_remapper,
                                          id_remapper_options,
                                          cb_flag,
                                          is_indirect,
                                          violates_never_null,
                                          force_user_refcount);
  }

  return IDWALK_RET_NOP;
}

static void libblock_remap_data_preprocess_ob(Object *ob,
                                              eIDRemapType remap_type,
                                              const struct IDRemapper *id_remapper)
{
  if (ob->type != OB_ARMATURE) {
    return;
  }
  if (/*ob->pose == NULL*/ true) {
    return;
  }
#if 0
  const bool is_cleanup_type = remap_type == ID_REMAP_TYPE_CLEANUP;
  /* Early exit when mapping, but no armature mappings present. */
  if (!is_cleanup_type && !KKE_id_remapper_has_mapping_for(id_remapper, FILTER_ID_AR)) {
    return;
  }

  /* Object's pose holds reference to armature bones. sic */
  /* Note that in theory, we should have to bother about linked/non-linked/never-null/etc.
   * flags/states.
   * Fortunately, this is just a tag, so we can accept to 'over-tag' a bit for pose recalc,
   * and avoid another complex and risky condition nightmare like the one we have in
   * foreach_libblock_remap_callback(). */
  const IDRemapperApplyResult expected_mapping_result =
    KKE_id_remapper_get_mapping_result(id_remapper, ob->data, ID_REMAP_APPLY_DEFAULT, NULL);
  if (is_cleanup_type || expected_mapping_result == ID_REMAP_RESULT_SOURCE_REMAPPED) {
    ob->pose->flag |= POSE_RECALC;
    /* We need to clear pose bone pointers immediately, some code may access those before
     * pose is actually recomputed, which can lead to segfault. */
    KKE_pose_clear_pointers(ob->pose);
  }
#endif
}

static void libblock_remap_data_preprocess(ID *id_owner,
                                           eIDRemapType remap_type,
                                           const struct IDRemapper *id_remapper)
{
  switch (GS(id_owner->name)) {
    case ID_OB: {
      Object *ob = (Object *)id_owner;
      libblock_remap_data_preprocess_ob(ob, remap_type, id_remapper);
      break;
    }
    default:
      break;
  }
}

static void libblock_remap_data_update_tags(ID *old_id, ID *new_id, void *user_data)
{
  IDRemap *id_remap_data = (IDRemap *)user_data;
  const int remap_flags = id_remap_data->flag;
  if ((remap_flags & ID_REMAP_SKIP_USER_CLEAR) == 0) {
    /* XXX We may not want to always 'transfer' fake-user from old to new id...
     *     Think for now it's desired behavior though,
     *     we can always add an option (flag) to control this later if needed. */
    if (old_id != NULL && (old_id->flag & LIB_FAKEUSER) && new_id != NULL) {
      id_fake_user_clear(old_id);
      id_fake_user_set(new_id);
    }

    id_us_clear_real(old_id);
  }

  if (new_id != NULL && (new_id->tag & LIB_TAG_INDIRECT) &&
      (new_id->runtime.remap.status & ID_REMAP_IS_LINKED_DIRECT)) {
    new_id->tag &= ~LIB_TAG_INDIRECT;
    new_id->flag &= ~LIB_INDIRECT_WEAK_LINK;
    new_id->tag |= LIB_TAG_EXTERN;
  }
}

ATTR_NONNULL(1)
static void libblock_remap_data(Main *kmain,
                                ID *id,
                                eIDRemapType remap_type,
                                struct IDRemapper *id_remapper,
                                const short remap_flags)
{
  IDRemap id_remap_data = {static_cast<eIDRemapType>(0)};
  const int foreach_id_flags = ((remap_flags & ID_REMAP_FORCE_INTERNAL_RUNTIME_POINTERS) != 0 ?
                                  IDWALK_DO_INTERNAL_RUNTIME_POINTERS :
                                  IDWALK_NOP);

  id_remap_data.id_remapper = id_remapper;
  id_remap_data.type = remap_type;
  id_remap_data.kmain = kmain;
  id_remap_data.id_owner = NULL;
  id_remap_data.flag = remap_flags;

  KKE_id_remapper_iter(id_remapper, libblock_remap_reset_remapping_status_callback, NULL);

  if (id) {
#ifdef DEBUG_PRINT
    printf("\tchecking id %s (%p, %p)\n", id->name, id, id->lib);
#endif
    id_remap_data.id_owner = id;
    libblock_remap_data_preprocess(id_remap_data.id_owner, remap_type, id_remapper);
    KKE_library_foreach_ID_link(NULL,
                                id,
                                foreach_libblock_remap_callback,
                                &id_remap_data,
                                foreach_id_flags);
  } else {
    /* Note that this is a very 'brute force' approach,
     * maybe we could use some depsgraph to only process objects actually using given old_id...
     * sounds rather unlikely currently, though, so this will do for now. */
    ID *id_curr;

    FOREACH_MAIN_ID_BEGIN(kmain, id_curr)
    {
      const uint64_t can_use_filter_id = KKE_library_id_can_use_filter_id(id_curr);
      const bool has_mapping = KKE_id_remapper_has_mapping_for(id_remapper, can_use_filter_id);

      /* Continue when id_remapper doesn't have any mappings that can be used by id_curr. */
      if (!has_mapping) {
        continue;
      }

      /* Note that we cannot skip indirect usages of old_id
       * here (if requested), we still need to check it for the
       * user count handling...
       * XXX No more true (except for debug usage of those
       * skipping counters). */
      id_remap_data.id_owner = id_curr;
      libblock_remap_data_preprocess(id_remap_data.id_owner, remap_type, id_remapper);
      KKE_library_foreach_ID_link(NULL,
                                  id_curr,
                                  foreach_libblock_remap_callback,
                                  &id_remap_data,
                                  foreach_id_flags);
    }
    FOREACH_MAIN_ID_END;
  }

  KKE_id_remapper_iter(id_remapper, libblock_remap_data_update_tags, &id_remap_data);
}

/* Can be called with both old_collection and new_collection being NULL,
 * this means we have to check whole Main database then. */
static void libblock_remap_data_postprocess_collection_update(Main *kmain,
                                                              Collection *owner_collection,
                                                              Collection *UNUSED(old_collection),
                                                              Collection *new_collection)
{
  if (new_collection == NULL) {
    /* XXX Complex cases can lead to NULL pointers in other collections than old_collection,
     * and KKE_main_collection_sync_remap() does not tolerate any of those, so for now always check
     * whole existing collections for NULL pointers.
     * I'd consider optimizing that whole collection remapping process a TODO: for later. */
    // KKE_collections_child_remove_nulls(kmain, owner_collection, NULL /*old_collection*/);
  } else {
    /* Temp safe fix, but a "tad" brute force... We should probably be able to use parents from
     * old_collection instead? */
    /* NOTE: Also takes care of duplicated child collections that remapping may have created. */
    // KKE_main_collections_parent_relations_rebuild(kmain);
  }

  // KKE_main_collection_sync_remap(kmain);
}

/**
 * Can be called with both old_ob and new_ob being NULL,
 * this means we have to check whole Main database then.
 */
static void libblock_remap_data_postprocess_object_update(Main *kmain,
                                                          Object *old_ob,
                                                          Object *new_ob,
                                                          const bool do_sync_collection)
{
  if (new_ob == NULL) {
    /* In case we unlinked old_ob (new_ob is NULL), the object has already
     * been removed from the scenes and their collections. We still have
     * to remove the NULL children from collections not used in any scene. */
    // KKE_collections_object_remove_nulls(kmain);
  } else {
    /* Remapping may have created duplicates of CollectionObject pointing to the same object within
     * the same collection. */
    // KKE_collections_object_remove_duplicates(kmain);
  }

  if (do_sync_collection) {
    // KKE_main_collection_sync_remap(kmain);
  }

  if (old_ob == NULL) {
    // for (Object *ob = kmain->objects.first; ob != NULL; ob = ob->id.next) {
    //   if (ob->type == OB_MBALL && KKE_mball_is_basis(ob)) {
    //     DEG_id_tag_update(&ob->id, ID_RECALC_GEOMETRY);
    //   }
    // }
  } else {
    // for (Object *ob = kmain->objects.first; ob != NULL; ob = ob->id.next) {
    //   if (ob->type == OB_MBALL && KKE_mball_is_basis_for(ob, old_ob)) {
    //     // DEG_id_tag_update(&ob->id, ID_RECALC_GEOMETRY);
    //     break; /* There is only one basis... */
    //   }
    // }
  }
}

static void libblock_remap_data_postprocess_obdata_relink(Main *kmain, Object *ob, ID *new_id)
{
  // if (ob->data == new_id) {
  //   switch (GS(new_id->name)) {
  //     case ID_ME:
  //       multires_force_sculpt_rebuild(ob);
  //       break;
  //     case ID_CU_LEGACY:
  //       KKE_curve_type_test(ob);
  //       break;
  //     default:
  //       break;
  //   }
  //   KKE_modifiers_test_object(ob);
  //   KKE_object_materials_test(kmain, ob, new_id);
  // }
}

typedef struct LibblockRelinkMultipleUserData
{
  Main *kmain;
  LinkNode *ids;
} LibBlockRelinkMultipleUserData;

static void libblock_relink_foreach_idpair_cb(ID *old_id, ID *new_id, void *user_data)
{
  LibBlockRelinkMultipleUserData *data = (LibBlockRelinkMultipleUserData *)user_data;
  Main *kmain = data->kmain;
  LinkNode *ids = data->ids;

  KLI_assert(old_id != NULL);
  KLI_assert((new_id == NULL) || GS(old_id->name) == GS(new_id->name));
  KLI_assert(old_id != new_id);

  bool is_object_update_processed = false;
  for (LinkNode *ln_iter = ids; ln_iter != NULL; ln_iter = ln_iter->next) {
    ID *id_iter = (ID *)ln_iter->link;

    /* Some after-process updates.
     * This is a bit ugly, but cannot see a way to avoid it.
     * Maybe we should do a per-ID callback for this instead?
     */
    switch (GS(id_iter->name)) {
      case ID_SCE:
      case ID_GR: {
        /* NOTE: here we know which collection we have affected, so at lest for NULL children
         * detection we can only process that one.
         * This is also a required fix in case `id` would not be in Main anymore, which can happen
         * e.g. when called from `id_delete`. */
        Collection *owner_collection = (GS(id_iter->name) == ID_GR) ?
                                         (Collection *)id_iter :
                                         ((Scene *)id_iter)->master_collection;
        switch (GS(old_id->name)) {
          case ID_OB:
            if (!is_object_update_processed) {
              libblock_remap_data_postprocess_object_update(kmain,
                                                            (Object *)old_id,
                                                            (Object *)new_id,
                                                            true);
              is_object_update_processed = true;
            }
            break;
          case ID_GR:
            libblock_remap_data_postprocess_collection_update(kmain,
                                                              owner_collection,
                                                              (Collection *)old_id,
                                                              (Collection *)new_id);
            break;
          default:
            break;
        }
        break;
      }
      case ID_OB:
        if (new_id != NULL) { /* Only affects us in case obdata was relinked (changed). */
          libblock_remap_data_postprocess_obdata_relink(kmain, (Object *)id_iter, new_id);
        }
        break;
      default:
        break;
    }
  }
}

void KKE_libblock_relink_multiple(Main *kmain,
                                  LinkNode *ids,
                                  const eIDRemapType remap_type,
                                  struct IDRemapper *id_remapper,
                                  const short remap_flags)
{
  KLI_assert(remap_type == ID_REMAP_TYPE_REMAP || KKE_id_remapper_is_empty(id_remapper));

  for (LinkNode *ln_iter = ids; ln_iter != NULL; ln_iter = ln_iter->next) {
    ID *id_iter = (ID *)ln_iter->link;
    libblock_remap_data(kmain, id_iter, remap_type, id_remapper, remap_flags);
  }

  switch (remap_type) {
    case ID_REMAP_TYPE_REMAP: {
      LibBlockRelinkMultipleUserData user_data = {0};
      user_data.kmain = kmain;
      user_data.ids = ids;

      KKE_id_remapper_iter(id_remapper, libblock_relink_foreach_idpair_cb, &user_data);
      break;
    }
    case ID_REMAP_TYPE_CLEANUP: {
      bool is_object_update_processed = false;
      for (LinkNode *ln_iter = ids; ln_iter != NULL; ln_iter = ln_iter->next) {
        ID *id_iter = (ID *)ln_iter->link;

        switch (GS(id_iter->name)) {
          case ID_SCE:
          case ID_GR: {
            /* NOTE: here we know which collection we have affected, so at lest for NULL children
             * detection we can only process that one.
             * This is also a required fix in case `id` would not be in Main anymore, which can
             * happen e.g. when called from `id_delete`. */
            Collection *owner_collection = (GS(id_iter->name) == ID_GR) ?
                                             (Collection *)id_iter :
                                             ((Scene *)id_iter)->master_collection;
            /* No choice but to check whole objects once, and all children collections. */
            if (!is_object_update_processed) {
              /* We only want to affect Object pointers here, not Collection ones, LayerCollections
               * will be resynced as part of the call to
               * `libblock_remap_data_postprocess_collection_update` below. */
              libblock_remap_data_postprocess_object_update(kmain, NULL, NULL, false);
              is_object_update_processed = true;
            }
            libblock_remap_data_postprocess_collection_update(kmain, owner_collection, NULL, NULL);
            break;
          }
          default:
            break;
        }
      }

      break;
    }
    default:
      KLI_assert_unreachable();
  }

  // DEG_relations_tag_update(kmain);
}

/**
 * @NOTE: Does not belong here...
 *  krakernel/intern/node.cpp
 */
static kNodeTree **KKE_ntree_ptr_from_id(ID *id)
{
  switch (GS(id->name)) {
    case ID_MA:
      return &((Material *)id)->nodetree;
    case ID_LA:
      return &((Light *)id)->nodetree;
    case ID_WO:
      return &((World *)id)->nodetree;
    case ID_TE:
      return &((Tex *)id)->nodetree;
    case ID_SCE:
      return &((Scene *)id)->nodetree;
    case ID_LS:
      return &((FreestyleLineStyle *)id)->nodetree;
    case ID_SIM:
      return &((Simulation *)id)->nodetree;
    default:
      return nullptr;
  }
}

/**
 * @NOTE: Does not belong here...
 *  krakernel/intern/node.cpp
 */
static kNodeTree *ntreeFromID(ID *id)
{
  kNodeTree **nodetree = KKE_ntree_ptr_from_id(id);
  return (nodetree != nullptr) ? *nodetree : nullptr;
}

uint64_t KKE_library_id_can_use_filter_id(const ID *id_owner)
{
  /* any type of ID can be used in custom props. */
  if (id_owner->properties) {
    return FILTER_ID_ALL;
  }
  const short id_type_owner = GS(id_owner->name);

  /* IDProps of armature bones and nodes, and bNode->id can use virtually any type of ID. */
  if (ELEM(id_type_owner, ID_NT, ID_AR)) {
    return FILTER_ID_ALL;
  }

  /* Casting to non const.
   * TODO(jbakker): We should introduce a ntree_id_has_tree function as we are actually not
   * interested in the result. */
  if (ntreeFromID((ID *)id_owner)) {
    return FILTER_ID_ALL;
  }

  // if (KKE_animdata_from_id(id_owner)) {
  //   /* AnimationData can use virtually any kind of data-blocks, through drivers especially. */
  //   return FILTER_ID_ALL;
  // }

  switch ((ID_Type)id_type_owner) {
    case ID_LI:
      return FILTER_ID_LI;
    case ID_SCE:
      return FILTER_ID_OB | FILTER_ID_WO | FILTER_ID_SCE | FILTER_ID_MC | FILTER_ID_MA |
             FILTER_ID_GR | FILTER_ID_TXT | FILTER_ID_LS | FILTER_ID_MSK | FILTER_ID_SO |
             FILTER_ID_GD | FILTER_ID_BR | FILTER_ID_PAL | FILTER_ID_IM | FILTER_ID_NT;
    case ID_OB:
      /* Could be more specific, but simpler to just always say 'yes' here. */
      return FILTER_ID_ALL;
    case ID_ME:
      return FILTER_ID_ME | FILTER_ID_MA | FILTER_ID_IM;
    case ID_CV:
      return FILTER_ID_OB | FILTER_ID_MA | FILTER_ID_VF;
    case ID_MB:
      return FILTER_ID_MA;
    case ID_MA:
      return FILTER_ID_TE | FILTER_ID_GR;
    case ID_TE:
      return FILTER_ID_IM | FILTER_ID_OB;
    case ID_LT:
      return 0;
    case ID_LA:
      return FILTER_ID_TE;
    case ID_CA:
      return FILTER_ID_OB | FILTER_ID_IM;
    case ID_KE:
      /* Warning! key->from, could be more types in future? */
      return FILTER_ID_ME | FILTER_ID_CV | FILTER_ID_LT;
    case ID_SCR:
      return FILTER_ID_SCE;
    case ID_WO:
      return FILTER_ID_TE;
    case ID_SPK:
      return FILTER_ID_SO;
    case ID_GR:
      return FILTER_ID_OB | FILTER_ID_GR;
    case ID_NT:
      /* Could be more specific, but node.id has no type restriction... */
      return FILTER_ID_ALL;
    case ID_BR:
      return FILTER_ID_BR | FILTER_ID_IM | FILTER_ID_PC | FILTER_ID_TE | FILTER_ID_MA;
    case ID_PA:
      return FILTER_ID_OB | FILTER_ID_GR | FILTER_ID_TE;
    case ID_MC:
      return FILTER_ID_GD | FILTER_ID_IM;
    case ID_MSK:
      /* WARNING! mask->parent.id, not typed. */
      return FILTER_ID_MC;
    case ID_LS:
      return FILTER_ID_TE | FILTER_ID_OB;
    case ID_LP:
      return FILTER_ID_IM;
    case ID_GD:
      return FILTER_ID_MA;
    case ID_WS:
      return FILTER_ID_SCE;
    case ID_PT:
      return FILTER_ID_MA;
    case ID_VO:
      return FILTER_ID_MA;
    case ID_SIM:
      return FILTER_ID_OB | FILTER_ID_IM;
    case ID_WM:
      return FILTER_ID_SCE | FILTER_ID_WS;
    case ID_IM:
    case ID_VF:
    case ID_TXT:
    case ID_SO:
    case ID_AR:
    case ID_AC:
    case ID_PAL:
    case ID_PC:
    case ID_CF:
      /* Those types never use/reference other IDs... */
      return 0;
  }

  KLI_assert_unreachable();
  return 0;
}

#define KLI_LINKSTACK_DECLARE(var, type) \
  LinkNode *var;                         \
  KLI_mempool *var##_pool_;              \
  type var##_type_

#define KLI_LINKSTACK_INIT(var)                                                 \
  {                                                                             \
    var = NULL;                                                                 \
    var##_pool_ = KLI_mempool_create(sizeof(LinkNode), 0, 64, KLI_MEMPOOL_NOP); \
  }                                                                             \
  (void)0

#define KLI_LINKSTACK_SIZE(var) KLI_mempool_len(var##_pool_)

/* check for typeof() */
#ifdef __GNUC__
#  define KLI_LINKSTACK_PUSH(var, ptr)            \
    (CHECK_TYPE_INLINE(ptr, typeof(var##_type_)), \
     KLI_linklist_prepend_pool(&(var), ptr, var##_pool_))
#  define KLI_LINKSTACK_POP(var) \
    (var ? (typeof(var##_type_))KLI_linklist_pop_pool(&(var), var##_pool_) : NULL)
#  define KLI_LINKSTACK_POP_DEFAULT(var, r) \
    (var ? (typeof(var##_type_))KLI_linklist_pop_pool(&(var), var##_pool_) : r)
#else /* non gcc */
#  define KLI_LINKSTACK_PUSH(var, ptr) (KLI_linklist_prepend_pool(&(var), ptr, var##_pool_))
#  define KLI_LINKSTACK_POP(var) (var ? KLI_linklist_pop_pool(&(var), var##_pool_) : NULL)
#  define KLI_LINKSTACK_POP_DEFAULT(var, r) (var ? KLI_linklist_pop_pool(&(var), var##_pool_) : r)
#endif /* gcc check */

#define KLI_LINKSTACK_SWAP(var_a, var_b)               \
  {                                                    \
    CHECK_TYPE_PAIR(var_a##_type_, var_b##_type_);     \
    SWAP(LinkNode *, var_a, var_b);                    \
    SWAP(KLI_mempool *, var_a##_pool_, var_b##_pool_); \
  }                                                    \
  (void)0

#define KLI_LINKSTACK_FREE(var)       \
  {                                   \
    KLI_mempool_destroy(var##_pool_); \
    var##_pool_ = NULL;               \
    (void)var##_pool_;                \
    var = NULL;                       \
    (void)var;                        \
    (void)&(var##_type_);             \
  }                                   \
  (void)0

typedef struct LibraryForeachIDData
{
  Main *kmain;
  /**
   * 'Real' ID, the one that might be in `kmain`, only differs from self_id when the later is a
   * private one.
   */
  ID *owner_id;
  /**
   * ID from which the current ID pointer is being processed. It may be an embedded ID like master
   * collection or root node tree.
   */
  ID *self_id;

  /** Flags controlling the behavior of the 'foreach id' looping code. */
  int flag;
  /** Generic flags to be passed to all callback calls for current processed data. */
  int cb_flag;
  /** Callback flags that are forbidden for all callback calls for current processed data. */
  int cb_flag_clear;

  /* Function to call for every ID pointers of current processed data, and its opaque user data
   * pointer. */
  LibraryIDLinkCallback callback;
  void *user_data;
  /** Store the returned value from the callback, to decide how to continue the processing of ID
   * pointers for current data. */
  int status;

  /* To handle recursion. */
  RSet *ids_handled; /* All IDs that are either already done, or still in ids_todo stack. */
  KLI_LINKSTACK_DECLARE(ids_todo, ID *);
} LibraryForeachIDData;

static void library_foreach_ID_data_cleanup(LibraryForeachIDData *data)
{
  if (data->ids_handled != NULL) {
    KLI_rset_free(data->ids_handled, NULL);
    KLI_LINKSTACK_FREE(data->ids_todo);
  }
}

/** \return false in case iteration over ID pointers must be stopped, true otherwise. */
static bool library_foreach_ID_link(Main *kmain,
                                    ID *id_owner,
                                    ID *id,
                                    LibraryIDLinkCallback callback,
                                    void *user_data,
                                    int flag,
                                    LibraryForeachIDData *inherit_data)
{
  LibraryForeachIDData data = {.kmain = kmain};

  KLI_assert(inherit_data == NULL || data.kmain == inherit_data->kmain);

  if (flag & IDWALK_RECURSE) {
    /* For now, recursion implies read-only, and no internal pointers. */
    flag |= IDWALK_READONLY;
    flag &= ~IDWALK_DO_INTERNAL_RUNTIME_POINTERS;

    /* NOTE: This function itself should never be called recursively when IDWALK_RECURSE is set,
     * see also comments in #KKE_library_foreach_ID_embedded.
     * This is why we can always create this data here, and do not need to try and re-use it from
     * `inherit_data`. */
    data.ids_handled = KLI_rset_new(KLI_rhashutil_ptrhash, KLI_rhashutil_ptrcmp, __func__);
    KLI_LINKSTACK_INIT(data.ids_todo);

    KLI_rset_add(data.ids_handled, id);
  } else {
    data.ids_handled = NULL;
  }
  data.flag = flag;
  data.status = 0;
  data.callback = callback;
  data.user_data = user_data;

#define CALLBACK_INVOKE_ID(check_id, cb_flag)                              \
  {                                                                        \
    CHECK_TYPE_ANY((check_id), ID *, void *);                              \
    KKE_lib_query_foreachid_process(&data, (ID **)&(check_id), (cb_flag)); \
    if (KKE_lib_query_foreachid_iter_stop(&data)) {                        \
      library_foreach_ID_data_cleanup(&data);                              \
      return false;                                                        \
    }                                                                      \
  }                                                                        \
  ((void)0)

#define CALLBACK_INVOKE(check_id_super, cb_flag)                                 \
  {                                                                              \
    CHECK_TYPE(&((check_id_super)->id), ID *);                                   \
    KKE_lib_query_foreachid_process(&data, (ID **)&(check_id_super), (cb_flag)); \
    if (KKE_lib_query_foreachid_iter_stop(&data)) {                              \
      library_foreach_ID_data_cleanup(&data);                                    \
      return false;                                                              \
    }                                                                            \
  }                                                                              \
  ((void)0)

  for (; id != NULL; id = (flag & IDWALK_RECURSE) ? KLI_LINKSTACK_POP(data.ids_todo) : NULL) {
    data.self_id = id;
    /* Note that we may call this functions sometime directly on an embedded ID, without any
     * knowledge of the owner ID then.
     * While not great, and that should be probably sanitized at some point, we cal live with it
     * for now. */
    data.owner_id = ((id->flag & LIB_EMBEDDED_DATA) != 0 && id_owner != NULL) ? id_owner :
                                                                                data.self_id;

    /* inherit_data is non-NULL when this function is called for some sub-data ID
     * (like root node-tree of a material).
     * In that case, we do not want to generate those 'generic flags' from our current sub-data ID
     * (the node tree), but re-use those generated for the 'owner' ID (the material). */
    if (inherit_data == NULL) {
      data.cb_flag = ID_IS_LINKED(id) ? IDWALK_CB_INDIRECT_USAGE : 0;
      /* When an ID is defined as not refcounting its ID usages, it should never do it. */
      data.cb_flag_clear = (id->tag & LIB_TAG_NO_USER_REFCOUNT) ?
                             IDWALK_CB_USER | IDWALK_CB_USER_ONE :
                             0;
    } else {
      data.cb_flag = inherit_data->cb_flag;
      data.cb_flag_clear = inherit_data->cb_flag_clear;
    }

    if (kmain != NULL && kmain->relations != NULL && (flag & IDWALK_READONLY) &&
        (flag & IDWALK_DO_INTERNAL_RUNTIME_POINTERS) == 0 &&
        (((kmain->relations->flag & MAINIDRELATIONS_INCLUDE_UI) == 0) ==
         ((data.flag & IDWALK_INCLUDE_UI) == 0))) {
      /* Note that this is minor optimization, even in worst cases (like id being an object with
       * lots of drivers and constraints and modifiers, or material etc. with huge node tree),
       * but we might as well use it (Main->relations is always assumed valid,
       * it's responsibility of code creating it to free it,
       * especially if/when it starts modifying Main database). */
      MainIDRelationsEntry *entry = (MainIDRelationsEntry *)KLI_rhash_lookup(
        kmain->relations->relations_from_pointers,
        id);
      for (MainIDRelationsEntryItem *to_id_entry = entry->to_ids; to_id_entry != NULL;
           to_id_entry = to_id_entry->next) {
        KKE_lib_query_foreachid_process(&data,
                                        to_id_entry->id_pointer.to,
                                        to_id_entry->usage_flag);
        if (KKE_lib_query_foreachid_iter_stop(&data)) {
          library_foreach_ID_data_cleanup(&data);
          return false;
        }
      }
      continue;
    }

    /* NOTE: ID.lib pointer is purposefully fully ignored here...
     * We may want to add it at some point? */

    if (flag & IDWALK_DO_INTERNAL_RUNTIME_POINTERS) {
      CALLBACK_INVOKE_ID(id->newid, IDWALK_CB_INTERNAL);
      CALLBACK_INVOKE_ID(id->orig_id, IDWALK_CB_INTERNAL);
    }

    if (id->override_library != NULL) {
      CALLBACK_INVOKE_ID(id->override_library->reference,
                         IDWALK_CB_USER | IDWALK_CB_OVERRIDE_LIBRARY_REFERENCE);
      CALLBACK_INVOKE_ID(id->override_library->storage,
                         IDWALK_CB_USER | IDWALK_CB_OVERRIDE_LIBRARY_REFERENCE);

      CALLBACK_INVOKE_ID(id->override_library->hierarchy_root, IDWALK_CB_LOOPBACK);
    }

    IDP_foreach_property(id->properties,
                         IDP_TYPE_FILTER_ID,
                         KKE_lib_query_idpropertiesForeachIDLink_callback,
                         &data);
    if (KKE_lib_query_foreachid_iter_stop(&data)) {
      library_foreach_ID_data_cleanup(&data);
      return false;
    }

    // AnimData *adt = KKE_animdata_from_id(id);
    // if (adt) {
    //   KKE_animdata_foreach_id(adt, &data);
    //   if (KKE_lib_query_foreachid_iter_stop(&data)) {
    //     library_foreach_ID_data_cleanup(&data);
    //     return false;
    //   }
    // }

    const IDTypeInfo *id_type = KKE_idtype_get_info_from_id(id);
    if (id_type->foreach_id != NULL) {
      id_type->foreach_id(id, &data);

      if (KKE_lib_query_foreachid_iter_stop(&data)) {
        library_foreach_ID_data_cleanup(&data);
        return false;
      }
    }
  }

  library_foreach_ID_data_cleanup(&data);
  return true;

#undef CALLBACK_INVOKE_ID
#undef CALLBACK_INVOKE
}

void KKE_lib_query_idpropertiesForeachIDLink_callback(IDProperty *id_prop, void *user_data)
{
  KLI_assert(id_prop->type == IDP_ID);

  LibraryForeachIDData *data = (LibraryForeachIDData *)user_data;
  const int cb_flag = IDWALK_CB_USER | ((id_prop->flag & IDP_FLAG_OVERRIDABLE_LIBRARY) ?
                                          0 :
                                          IDWALK_CB_OVERRIDE_LIBRARY_NOT_OVERRIDABLE);
  KKE_LIB_FOREACHID_PROCESS_ID(data, id_prop->data.pointer, cb_flag);
}

bool KKE_lib_query_foreachid_iter_stop(LibraryForeachIDData *data)
{
  return (data->status & IDWALK_STOP) != 0;
}

void KKE_lib_query_foreachid_process(LibraryForeachIDData *data, ID **id_pp, int cb_flag)
{
  if (KKE_lib_query_foreachid_iter_stop(data)) {
    return;
  }

  const int flag = data->flag;
  ID *old_id = *id_pp;

  /* Update the callback flags with the ones defined (or forbidden) in `data` by the generic
   * caller code. */
  cb_flag = ((cb_flag | data->cb_flag) & ~data->cb_flag_clear);

  /* Update the callback flags with some extra information regarding overrides: all 'loopback',
   * 'internal', 'embedded' etc. ID pointers are never overridable. */
  if (cb_flag & (IDWALK_CB_INTERNAL | IDWALK_CB_LOOPBACK | IDWALK_CB_OVERRIDE_LIBRARY_REFERENCE)) {
    cb_flag |= IDWALK_CB_OVERRIDE_LIBRARY_NOT_OVERRIDABLE;
  }

  LibraryIDLinkCallbackData cbdata = {
    .user_data = data->user_data,
    .kmain = data->kmain,
    .id_owner = data->owner_id,
    .id_self = data->self_id,
    .id_pointer = id_pp,
    .cb_flag = cb_flag
  };

  const int callback_return = data->callback(&cbdata);
  if (flag & IDWALK_READONLY) {
    KLI_assert(*(id_pp) == old_id);
  }
  if (old_id && (flag & IDWALK_RECURSE)) {
    if (KLI_rset_add((data)->ids_handled, old_id)) {
      if (!(callback_return & IDWALK_RET_STOP_RECURSION)) {
        KLI_LINKSTACK_PUSH(data->ids_todo, old_id);
      }
    }
  }
  if (callback_return & IDWALK_RET_STOP_ITER) {
    data->status |= IDWALK_STOP;
  }
}

void KKE_library_foreach_ID_link(Main *kmain,
                                 ID *id,
                                 LibraryIDLinkCallback callback,
                                 void *user_data,
                                 int flag)
{
  library_foreach_ID_link(kmain, NULL, id, callback, user_data, flag, NULL);
}

void KKE_libblock_relink_ex(Main *kmain,
                            void *idv,
                            void *old_idv,
                            void *new_idv,
                            const short remap_flags)
{

  /* Should be able to replace all _relink() functions (constraints, rigidbody, etc.) ? */

  ID *id = (ID *)idv;
  ID *old_id = (ID *)old_idv;
  ID *new_id = (ID *)new_idv;
  LinkNode ids = {.next = NULL, .link = idv};

  /* No need to lock here, we are only affecting given ID, not kmain database. */
  struct IDRemapper *id_remapper = KKE_id_remapper_create();
  eIDRemapType remap_type = ID_REMAP_TYPE_REMAP;

  KLI_assert(id != NULL);
  UNUSED_VARS_NDEBUG(id);
  if (old_id != NULL) {
    KLI_assert((new_id == NULL) || GS(old_id->name) == GS(new_id->name));
    KLI_assert(old_id != new_id);
    KKE_id_remapper_add(id_remapper, old_id, new_id);
  } else {
    KLI_assert(new_id == NULL);
    remap_type = ID_REMAP_TYPE_CLEANUP;
  }

  KKE_libblock_relink_multiple(kmain, &ids, remap_type, id_remapper, remap_flags);

  KKE_id_remapper_free(id_remapper);
}

void KKE_id_free(Main *kmain, void *idv)
{
  KKE_id_free_ex(kmain, idv, 0, true);
}

bool KKE_id_new_name_validate(struct Main *kmain,
                              ListBase *lb,
                              ID *id,
                              const char *tname,
                              const bool do_linked_data)
{
  bool result = false;
  char name[MAX_ID_NAME - 2];

  /* If library, don't rename (unless explicitly required), but do ensure proper sorting. */
  if (!do_linked_data && ID_IS_LINKED(id)) {
    id_sort_by_name(lb, id, nullptr);

    return result;
  }

  /* if no name given, use name of current ID
   * else make a copy (tname args can be const) */
  if (tname == nullptr) {
    tname = id->name + 2;
  }

  KLI_strncpy(name, tname, sizeof(name));

  if (name[0] == '\0') {
    /* Disallow empty names. */
    KLI_strncpy(name, DATA_(KKE_idtype_idcode_to_name(GS(id->name))), sizeof(name));
  } else {
    /* disallow non utf8 chars,
     * the interface checks for this but new ID's based on file names don't */
    KLI_str_utf8_invalid_strip(name, strlen(name));
  }

  result = KKE_main_namemap_get_name(kmain, id, name);

  strcpy(id->name + 2, name);
  id_sort_by_name(lb, id, nullptr);
  return result;
}

/* --------------------------------------------------------------------------------------------- */

void KKE_libblock_free_data(ID *id, const bool do_id_user)
{
  if (id->properties) {
    IDP_FreePropertyContent_ex(id->properties, do_id_user);
    MEM_freeN(id->properties);
    id->properties = NULL;
  }

  if (id->override_library) {
    KKE_lib_override_library_free(&id->override_library, do_id_user);
    id->override_library = NULL;
  }

  if (id->asset_data) {
    KKE_asset_metadata_free(&id->asset_data);
  }

  if (id->library_weak_reference != NULL) {
    MEM_freeN(id->library_weak_reference);
  }

  // KKE_animdata_free(id, do_id_user);
}

void KKE_libblock_free_datablock(ID *id, const int UNUSED(flag))
{
  const IDTypeInfo *idtype_info = KKE_idtype_get_info_from_id(id);

  if (idtype_info != NULL) {
    if (idtype_info->free_data != NULL) {
      idtype_info->free_data(id);
    }
    return;
  }

  KLI_assert_msg(0, "IDType Missing IDTypeInfo");
}

void KKE_libblock_free_data_py(ID *id)
{
#ifdef WITH_PYTHON
#  ifdef WITH_PYTHON_SAFETY
  KPY_id_release(id);
#  endif
  if (id->py_instance) {
    KPY_DECREF_PRIM_INVALIDATE(id->py_instance);
  }
#else
  UNUSED_VARS(id);
#endif
}

/* --------------------------------------------------------------------------------------------- */

void lib_override_library_property_operation_clear(IDOverrideLibraryPropertyOperation *opop)
{
  if (opop->subitem_reference_name) {
    MEM_freeN(opop->subitem_reference_name);
  }
  if (opop->subitem_local_name) {
    MEM_freeN(opop->subitem_local_name);
  }
}

void lib_override_library_property_clear(IDOverrideLibraryProperty *op)
{
  KLI_assert(op->rna_path != nullptr);

  MEM_freeN(op->rna_path);

  LISTBASE_FOREACH(IDOverrideLibraryPropertyOperation *, opop, &op->operations)
  {
    lib_override_library_property_operation_clear(opop);
  }
  KLI_freelistN(&op->operations);
}

void KKE_lib_override_library_clear(IDOverrideLibrary *override, const bool do_id_user)
{
  KLI_assert(override != nullptr);

  if (!ELEM(nullptr, override->runtime, override->runtime->rna_path_to_override_properties)) {
    KLI_rhash_clear(override->runtime->rna_path_to_override_properties, nullptr, nullptr);
  }

  LISTBASE_FOREACH(IDOverrideLibraryProperty *, op, &override->properties)
  {
    lib_override_library_property_clear(op);
  }
  KLI_freelistN(&override->properties);

  if (do_id_user) {
    id_us_min(override->reference);
    /* override->storage should never be refcounted... */
  }
}

void KKE_lib_override_library_free(IDOverrideLibrary **override, const bool do_id_user)
{
  KLI_assert(*override != nullptr);

  if ((*override)->runtime != nullptr) {
    if ((*override)->runtime->rna_path_to_override_properties != nullptr) {
      KLI_rhash_free((*override)->runtime->rna_path_to_override_properties, nullptr, nullptr);
    }
    MEM_SAFE_FREE((*override)->runtime);
  }

  KKE_lib_override_library_clear(*override, do_id_user);
  MEM_freeN(*override);
  *override = nullptr;
}

/* --------------------------------------------------------------------------------------------- */

/** @TODO: Does not belong here. */
void KKE_asset_metadata_free(AssetMetaData **asset_data)
{
  if ((*asset_data)->properties) {
    IDP_FreeProperty((*asset_data)->properties);
  }
  MEM_SAFE_FREE((*asset_data)->author);
  MEM_SAFE_FREE((*asset_data)->description);
  KLI_freelistN(&(*asset_data)->tags);

  MEM_SAFE_FREE(*asset_data);
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Helper building final ID name from given base_name and number.
 *
 * If everything goes well and we do generate a valid final ID name in given name, we return
 * true. In case the final name would overflow the allowed ID name length, or given number is
 * bigger than maximum allowed value, we truncate further the base_name (and given name, which is
 * assumed to have the same 'base_name' part), and return false. */
static bool id_name_final_build(char *name, char *base_name, size_t base_name_len, int number)
{
  char number_str[11]; /* Dot + nine digits + NULL terminator. */
  size_t number_str_len = KLI_snprintf_rlen(number_str, ARRAY_SIZE(number_str), ".%.3d", number);

  /* If the number would lead to an overflow of the maximum ID name length, we need to truncate
   * the base name part and do all the number checks again. */
  if (base_name_len + number_str_len >= MAX_NAME || number >= MAX_NUMBER) {
    if (base_name_len + number_str_len >= MAX_NAME) {
      base_name_len = MAX_NAME - number_str_len - 1;
    } else {
      base_name_len--;
    }
    base_name[base_name_len] = '\0';

    /* Code above may have generated invalid utf-8 string, due to raw truncation.
     * Ensure we get a valid one now. */
    base_name_len -= size_t(KLI_str_utf8_invalid_strip(base_name, base_name_len));

    /* Also truncate orig name, and start the whole check again. */
    name[base_name_len] = '\0';
    return false;
  }

  /* We have our final number, we can put it in name and exit the function. */
  KLI_strncpy(name + base_name_len, number_str, number_str_len + 1);
  return true;
}

/* Key used in set/map lookups: just a string name. */
struct UniqueName_Key
{
  char name[MAX_NAME];
  uint64_t hash() const
  {
    return KLI_rhashutil_strhash_n(name, MAX_NAME);
  }
  bool operator==(const UniqueName_Key &o) const
  {
    return !KLI_rhashutil_strcmp(name, o.name);
  }
};

/* Tracking of used numeric suffixes. For each base name:
 *
 * - Exactly track which of the lowest 1024 suffixes are in use,
 *   whenever there is a name collision we pick the lowest "unused"
 *   one. This is done with a bit map.
 * - Above 1024, do not track them exactly, just track the maximum
 *   suffix value seen so far. Upon collision, assign number that is
 *   one larger.
 */
struct UniqueName_Value
{
  static constexpr uint max_exact_tracking = 1024;
  KLI_BITMAP_DECLARE(mask, max_exact_tracking);
  int max_value = 0;

  void mark_used(int number)
  {
    if (number >= 0 && number < max_exact_tracking) {
      KLI_BITMAP_ENABLE(mask, number);
    }
    if (number < MAX_NUMBER) {
      math::max_inplace(max_value, number);
    }
  }

  void mark_unused(int number)
  {
    if (number >= 0 && number < max_exact_tracking) {
      KLI_BITMAP_DISABLE(mask, number);
    }
    if (number > 0 && number == max_value) {
      --max_value;
    }
  }

  bool use_if_unused(int number)
  {
    if (number >= 0 && number < max_exact_tracking) {
      if (!KLI_BITMAP_TEST_BOOL(mask, number)) {
        KLI_BITMAP_ENABLE(mask, number);
        math::max_inplace(max_value, number);
        return true;
      }
    }
    return false;
  }

  int use_smallest_unused()
  {
    /* Find the smallest available one <1k.
     * However we never want to pick zero ("none") suffix, even if it is
     * available, e.g. if Foo.001 was used and we want to create another
     * Foo.001, we should return Foo.002 and not Foo.
     * So while searching, mark #0 as "used" to make sure we don't find it,
     * and restore the value afterwards. */

    KLI_bitmap prev_first = mask[0];
    mask[0] |= 1;
    int result = KLI_bitmap_find_first_unset(mask, max_exact_tracking);
    if (result >= 0) {
      KLI_BITMAP_ENABLE(mask, result);
      math::max_inplace(max_value, result);
    }
    mask[0] |= prev_first & 1; /* Restore previous value of #0 bit. */
    return result;
  }
};

/* Tracking of names for a single ID type. */
struct UniqueName_TypeMap
{
  /* Set of full names that are in use. */
  Set<UniqueName_Key> full_names;
  /* For each base name (i.e. without numeric suffix), track the
   * numeric suffixes that are in use. */
  Map<UniqueName_Key, UniqueName_Value> base_name_to_num_suffix;
};

struct UniqueName_Map
{
  UniqueName_TypeMap type_maps[INDEX_ID_MAX];

  UniqueName_TypeMap *find_by_type(short id_type)
  {
    int index = KKE_idtype_idcode_to_index(id_type);
    return index >= 0 ? &type_maps[index] : nullptr;
  }
};

struct UniqueName_Map *KKE_main_namemap_create()
{
  struct UniqueName_Map *map = MEM_new<UniqueName_Map>(__func__);
  return map;
}

void KKE_main_namemap_destroy(struct UniqueName_Map **r_name_map)
{
#ifdef DEBUG_PRINT_MEMORY_USAGE
  int64_t size_sets = 0;
  int64_t size_maps = 0;
  for (const UniqueName_TypeMap &type_map : (*r_name_map)->type_maps) {
    size_sets += type_map.full_names.size_in_bytes();
    size_maps += type_map.base_name_to_num_suffix.size_in_bytes();
  }
  printf("NameMap memory usage: sets %.1fKB, maps %.1fKB\n",
         size_sets / 1024.0,
         size_maps / 1024.0);
#endif
  MEM_delete<UniqueName_Map>(*r_name_map);
  *r_name_map = nullptr;
}

static void main_namemap_populate(UniqueName_Map *name_map, struct Main *kmain, ID *ignore_id)
{
  KLI_assert_msg(name_map != nullptr, "name_map should not be null");
  for (UniqueName_TypeMap &type_map : name_map->type_maps) {
    type_map.base_name_to_num_suffix.clear();
  }
  Library *library = ignore_id->lib;
  ID *id;
  FOREACH_MAIN_ID_BEGIN(kmain, id)
  {
    if ((id == ignore_id) || (id->lib != library)) {
      continue;
    }
    UniqueName_TypeMap *type_map = name_map->find_by_type(GS(id->name));
    KLI_assert(type_map != nullptr);

    /* Insert the full name into the set. */
    UniqueName_Key key;
    KLI_strncpy(key.name, id->name + 2, MAX_NAME);
    type_map->full_names.add(key);

    /* Get the name and number parts ("name.number"). */
    int number = MIN_NUMBER;
    KLI_split_name_num(key.name, &number, id->name + 2, '.');

    /* Get and update the entry for this base name. */
    UniqueName_Value &val = type_map->base_name_to_num_suffix.lookup_or_add_default(key);
    val.mark_used(number);
  }
  FOREACH_MAIN_ID_END;
}

/* Get the name map object used for the given Main/ID.
 * Lazily creates and populates the contents of the name map, if ensure_created is true.
 * NOTE: if the contents are populated, the name of the given ID itself is not added. */
static UniqueName_Map *get_namemap_for(Main *kmain, ID *id, bool ensure_created)
{
  if (id->lib != nullptr) {
    if (ensure_created && id->lib->runtime.name_map == nullptr) {
      id->lib->runtime.name_map = KKE_main_namemap_create();
      main_namemap_populate(id->lib->runtime.name_map, kmain, id);
    }
    return id->lib->runtime.name_map;
  }
  if (ensure_created && kmain->name_map == nullptr) {
    kmain->name_map = KKE_main_namemap_create();
    main_namemap_populate(kmain->name_map, kmain, id);
  }
  return kmain->name_map;
}

bool KKE_main_namemap_get_name(struct Main *kmain, struct ID *id, char *name)
{
#ifndef __GNUC__ /* GCC warns with `nonull-compare`. */
  KLI_assert(kmain != nullptr);
  KLI_assert(id != nullptr);
#endif
  UniqueName_Map *name_map = get_namemap_for(kmain, id, true);
  KLI_assert(name_map != nullptr);
  KLI_assert(strlen(name) < MAX_NAME);
  UniqueName_TypeMap *type_map = name_map->find_by_type(GS(id->name));
  KLI_assert(type_map != nullptr);

  bool is_name_changed = false;

  UniqueName_Key key;
  while (true) {
    /* Check if the full original name has a duplicate. */
    KLI_strncpy(key.name, name, MAX_NAME);
    const bool has_dup = type_map->full_names.contains(key);

    /* Get the name and number parts ("name.number"). */
    int number = MIN_NUMBER;
    size_t base_name_len = KLI_split_name_num(key.name, &number, name, '.');

    bool added_new = false;
    UniqueName_Value &val = type_map->base_name_to_num_suffix.lookup_or_add_cb(key, [&]() {
      added_new = true;
      return UniqueName_Value();
    });
    if (added_new || !has_dup) {
      /* This base name is not used at all yet, or the full original
       * name has no duplicates. The latter could happen if splitting
       * by number would produce the same values, for different name
       * strings (e.g. Foo.001 and Foo.1). */
      val.mark_used(number);

      if (!has_dup) {
        KLI_strncpy(key.name, name, MAX_NAME);
        type_map->full_names.add(key);
      }
      return is_name_changed;
    }

    /* The base name is already used. But our number suffix might not be used yet. */
    int number_to_use = -1;
    if (val.use_if_unused(number)) {
      /* Our particular number suffix is not used yet: use it. */
      number_to_use = number;
    } else {
      /* Find lowest free under 1k and use it. */
      number_to_use = val.use_smallest_unused();

      /* Did not find one under 1k. */
      if (number_to_use == -1) {
        if (number >= MIN_NUMBER && number > val.max_value) {
          val.max_value = number;
          number_to_use = number;
        } else {
          val.max_value++;
          number_to_use = val.max_value;
        }
      }
    }

    /* Try to build final name from the current base name and the number.
     * Note that this can fail due to too long base name, or a too large number,
     * in which case it will shorten the base name, and we'll start again. */
    KLI_assert(number_to_use >= MIN_NUMBER);
    if (id_name_final_build(name, key.name, base_name_len, number_to_use)) {
      /* All good, add final name to the set. */
      KLI_strncpy(key.name, name, MAX_NAME);
      type_map->full_names.add(key);
      break;
    }

    /* Name had to be truncated, or number too large: mark
     * the output name as definitely changed, and proceed with the
     * truncated name again. */
    is_name_changed = true;
  }
  return is_name_changed;
}

void KKE_main_namemap_remove_name(struct Main *kmain, struct ID *id, const char *name)
{
#ifndef __GNUC__ /* GCC warns with `nonull-compare`. */
  KLI_assert(kmain != nullptr);
  KLI_assert(id != nullptr);
  KLI_assert(name != nullptr);
#endif
  /* Name is empty or not initialized yet, nothing to remove. */
  if (name[0] == '\0') {
    return;
  }

  struct UniqueName_Map *name_map = get_namemap_for(kmain, id, false);
  if (name_map == nullptr) {
    return;
  }
  KLI_assert(strlen(name) < MAX_NAME);
  UniqueName_TypeMap *type_map = name_map->find_by_type(GS(id->name));
  KLI_assert(type_map != nullptr);

  UniqueName_Key key;
  /* Remove full name from the set. */
  KLI_strncpy(key.name, name, MAX_NAME);
  type_map->full_names.remove(key);

  int number = MIN_NUMBER;
  KLI_split_name_num(key.name, &number, name, '.');
  UniqueName_Value *val = type_map->base_name_to_num_suffix.lookup_ptr(key);
  if (val == nullptr) {
    return;
  }
  if (number == 0 && val->max_value == 0) {
    /* This was the only base name usage, remove whole key. */
    type_map->base_name_to_num_suffix.remove(key);
    return;
  }
  val->mark_unused(number);
}

struct Uniqueness_Key
{
  char name[MAX_ID_NAME];
  Library *lib;
  uint64_t hash() const
  {
    return KLI_rhashutil_combine_hash(KLI_rhashutil_strhash_n(name, MAX_ID_NAME),
                                      KLI_rhashutil_ptrhash(lib));
  }
  bool operator==(const Uniqueness_Key &o) const
  {
    return lib == o.lib && !KLI_rhashutil_strcmp(name, o.name);
  }
};

static bool main_namemap_validate_and_fix(Main *kmain, const bool do_fix)
{
  Set<Uniqueness_Key> id_names_libs;
  bool is_valid = true;
  ListBase *lb_iter;
  FOREACH_MAIN_LISTBASE_BEGIN(kmain, lb_iter)
  {
    LISTBASE_FOREACH_MUTABLE(ID *, id_iter, lb_iter)
    {
      Uniqueness_Key key;
      KLI_strncpy(key.name, id_iter->name, MAX_ID_NAME);
      key.lib = id_iter->lib;
      if (!id_names_libs.add(key)) {
        is_valid = false;
        CLOG_ERROR(&LOG,
                   "ID name '%s' (from library '%s') is found more than once",
                   id_iter->name,
                   id_iter->lib != nullptr ? id_iter->lib->filepath : "<None>");
        if (do_fix) {
          /* NOTE: this may imply moving this ID in its listbase, however re-checking it later is
           * not really an issue. */
          KKE_id_new_name_validate(kmain,
                                   which_libbase(kmain, GS(id_iter->name)),
                                   id_iter,
                                   nullptr,
                                   true);
          KLI_strncpy(key.name, id_iter->name, MAX_ID_NAME);
          if (!id_names_libs.add(key)) {
            CLOG_ERROR(&LOG,
                       "\tID has been renamed to '%s', but it still seems to be already in use",
                       id_iter->name);
          } else {
            CLOG_WARN(&LOG, "\tID has been renamed to '%s'", id_iter->name);
          }
        }
      }

      UniqueName_Map *name_map = get_namemap_for(kmain, id_iter, false);
      if (name_map == nullptr) {
        continue;
      }
      UniqueName_TypeMap *type_map = name_map->find_by_type(GS(id_iter->name));
      KLI_assert(type_map != nullptr);

      UniqueName_Key key_namemap;
      /* Remove full name from the set. */
      KLI_strncpy(key_namemap.name, id_iter->name + 2, MAX_NAME);
      if (!type_map->full_names.contains(key_namemap)) {
        is_valid = false;
        CLOG_ERROR(&LOG,
                   "ID name '%s' (from library '%s') exists in current Main, but is not listed in "
                   "the namemap",
                   id_iter->name,
                   id_iter->lib != nullptr ? id_iter->lib->filepath : "<None>");
      }
    }
  }
  FOREACH_MAIN_LISTBASE_END;

  Library *lib = nullptr;
  UniqueName_Map *name_map = kmain->name_map;
  do {
    if (name_map != nullptr) {
      int i = 0;
      for (short idcode = KKE_idtype_idcode_iter_step(&i); idcode != 0;
           idcode = KKE_idtype_idcode_iter_step(&i)) {
        UniqueName_TypeMap *type_map = name_map->find_by_type(idcode);
        if (type_map != nullptr) {
          for (const UniqueName_Key &id_name : type_map->full_names) {
            Uniqueness_Key key;
            *(reinterpret_cast<short *>(key.name)) = idcode;
            KLI_strncpy(key.name + 2, id_name.name, MAX_NAME);
            key.lib = lib;
            if (!id_names_libs.contains(key)) {
              is_valid = false;
              CLOG_ERROR(&LOG,
                         "ID name '%s' (from library '%s') is listed in the namemap, but does not "
                         "exists in current Main",
                         key.name,
                         lib != nullptr ? lib->filepath : "<None>");
            }
          }
        }
      }
    }
    lib = static_cast<Library *>((lib == nullptr) ? kmain->libraries.first : lib->id.next);
    name_map = (lib != nullptr) ? lib->runtime.name_map : nullptr;
  } while (lib != nullptr);

  if (is_valid || !do_fix) {
    return is_valid;
  }

  /* Clear all existing namemaps. */
  lib = nullptr;
  UniqueName_Map **name_map_p = &kmain->name_map;
  do {
    KLI_assert(name_map_p != nullptr);
    if (*name_map_p != nullptr) {
      KKE_main_namemap_destroy(name_map_p);
    }
    lib = static_cast<Library *>((lib == nullptr) ? kmain->libraries.first : lib->id.next);
    name_map_p = (lib != nullptr) ? &lib->runtime.name_map : nullptr;
  } while (lib != nullptr);

  return is_valid;
}

bool KKE_main_namemap_validate_and_fix(Main *kmain)
{
  const bool is_valid = main_namemap_validate_and_fix(kmain, true);
  KLI_assert(main_namemap_validate_and_fix(kmain, false));
  return is_valid;
}

bool KKE_main_namemap_validate(Main *kmain)
{
  return main_namemap_validate_and_fix(kmain, false);
}
