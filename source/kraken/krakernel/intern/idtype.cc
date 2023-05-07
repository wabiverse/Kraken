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

#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_rhash.h"
#include "KLI_utildefines.h"

#include "USD_ID.h"
#include "USD_collection.h"
#include "USD_lights.h"
#include "USD_linestyles.h"
#include "USD_materials.h"
#include "USD_scene_types.h"
#include "USD_simulation.h"
#include "USD_space_types.h"
#include "USD_texture_types.h"
#include "USD_world.h"

#include "KKE_main.h"

#include "KKE_idtype.h"

uint KKE_idtype_cache_key_hash(const void *key_v)
{
  const IDCacheKey *key = (IDCacheKey *)key_v;
  size_t hash = KLI_rhashutil_uinthash(key->id_session_uuid);
  hash = KLI_rhashutil_combine_hash(hash, KLI_rhashutil_uinthash((uint)key->offset_in_ID));
  return (uint)hash;
}

bool KKE_idtype_cache_key_cmp(const void *key_a_v, const void *key_b_v)
{
  const IDCacheKey *key_a = (IDCacheKey *)key_a_v;
  const IDCacheKey *key_b = (IDCacheKey *)key_b_v;
  return (key_a->id_session_uuid != key_b->id_session_uuid) ||
         (key_a->offset_in_ID != key_b->offset_in_ID);
}

static IDTypeInfo *id_types[INDEX_ID_MAX] = {NULL};

static void id_type_init(void)
{
#define INIT_TYPE(_id_code)                                                \
  {                                                                        \
    KLI_assert(IDType_##_id_code.main_listbase_index == INDEX_##_id_code); \
    id_types[INDEX_##_id_code] = &IDType_##_id_code;                       \
  }                                                                        \
  (void)0

  // INIT_TYPE(ID_SCE);
  // INIT_TYPE(ID_LI);
  // INIT_TYPE(ID_OB);
  // INIT_TYPE(ID_ME);
  // INIT_TYPE(ID_CV);
  // INIT_TYPE(ID_MB);
  // INIT_TYPE(ID_MA);
  // INIT_TYPE(ID_TE);
  // INIT_TYPE(ID_IM);
  // INIT_TYPE(ID_LT);
  // INIT_TYPE(ID_LA);
  // INIT_TYPE(ID_CA);
  // INIT_TYPE(ID_KE);
  // INIT_TYPE(ID_WO);
  // INIT_TYPE(ID_SCR);
  // INIT_TYPE(ID_VF);
  // INIT_TYPE(ID_TXT);
  // INIT_TYPE(ID_SPK);
  // INIT_TYPE(ID_SO);
  // INIT_TYPE(ID_GR);
  // INIT_TYPE(ID_AR);
  // INIT_TYPE(ID_AC);
  // INIT_TYPE(ID_NT);
  // INIT_TYPE(ID_BR);
  // INIT_TYPE(ID_PA);
  // INIT_TYPE(ID_GD);
  // INIT_TYPE(ID_WM);
  // INIT_TYPE(ID_MC);
  // INIT_TYPE(ID_MSK);
  // INIT_TYPE(ID_LS);
  // INIT_TYPE(ID_PAL);
  // INIT_TYPE(ID_PC);
  // INIT_TYPE(ID_CF);
  // INIT_TYPE(ID_WS);
  // INIT_TYPE(ID_LP);
  // INIT_TYPE(ID_PT);
  // INIT_TYPE(ID_VO);
  // INIT_TYPE(ID_SIM);

  /* Special naughty boy... */
  KLI_assert(IDType_ID_LINK_PLACEHOLDER.main_listbase_index == INDEX_ID_NULL);
  id_types[INDEX_ID_NULL] = &IDType_ID_LINK_PLACEHOLDER;

#undef INIT_TYPE
}

void KKE_idtype_init(void)
{
  /* Initialize data-block types. */
  id_type_init();
}

const IDTypeInfo *KKE_idtype_get_info_from_idcode(const short id_code)
{
  int id_index = KKE_idtype_idcode_to_index(id_code);

  if (id_index >= 0 && id_index < ARRAY_SIZE(id_types) && id_types[id_index] != NULL &&
      id_types[id_index]->name[0] != '\0') {
    return id_types[id_index];
  }

  return NULL;
}

const IDTypeInfo *KKE_idtype_get_info_from_id(const ID *id)
{
  return KKE_idtype_get_info_from_idcode(GS(id->name));
}

static const IDTypeInfo *idtype_get_info_from_name(const char *idtype_name)
{
  for (int i = ARRAY_SIZE(id_types); i--;) {
    if (id_types[i] != NULL && STREQ(idtype_name, id_types[i]->name)) {
      return id_types[i];
    }
  }

  return NULL;
}

/* Various helpers/wrappers around #IDTypeInfo structure. */

const char *KKE_idtype_idcode_to_name(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  return id_type != NULL ? id_type->name : NULL;
}

const char *KKE_idtype_idcode_to_name_plural(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  return id_type != NULL ? id_type->name_plural : NULL;
}

const char *KKE_idtype_idcode_to_translation_context(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  return id_type != NULL ? id_type->translation_context : NULL;
}

short KKE_idtype_idcode_from_name(const char *idtype_name)
{
  const IDTypeInfo *id_type = idtype_get_info_from_name(idtype_name);
  KLI_assert(id_type);
  return id_type != NULL ? id_type->id_code : 0;
}

bool KKE_idtype_idcode_is_valid(const short idcode)
{
  return KKE_idtype_get_info_from_idcode(idcode) != NULL ? true : false;
}

bool KKE_idtype_idcode_is_linkable(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  return id_type != NULL ? (id_type->flags & IDTYPE_FLAGS_NO_LIBLINKING) == 0 : false;
}

bool KKE_idtype_idcode_is_only_appendable(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  if (id_type != NULL && (id_type->flags & IDTYPE_FLAGS_ONLY_APPEND) != 0) {
    /* Only appendable ID types should also always be linkable. */
    KLI_assert((id_type->flags & IDTYPE_FLAGS_NO_LIBLINKING) == 0);
    return true;
  }
  return false;
}

bool KKE_idtype_idcode_append_is_reusable(const short idcode)
{
  const IDTypeInfo *id_type = KKE_idtype_get_info_from_idcode(idcode);
  KLI_assert(id_type != NULL);
  if (id_type != NULL && (id_type->flags & IDTYPE_FLAGS_APPEND_IS_REUSABLE) != 0) {
    /* All appendable ID types should also always be linkable. */
    KLI_assert((id_type->flags & IDTYPE_FLAGS_NO_LIBLINKING) == 0);
    return true;
  }
  return false;
}

uint64_t KKE_idtype_idcode_to_idfilter(const short idcode)
{
#define CASE_IDFILTER(_id) \
  case ID_##_id:           \
    return FILTER_ID_##_id

#define CASE_IDFILTER_NONE(_id) \
  case ID_##_id:                \
    return 0

  switch ((ID_Type)idcode) {
    CASE_IDFILTER(AC);
    CASE_IDFILTER(AR);
    CASE_IDFILTER(BR);
    CASE_IDFILTER(CA);
    CASE_IDFILTER(CF);
    CASE_IDFILTER(CV);
    CASE_IDFILTER(GD);
    CASE_IDFILTER(GR);
    CASE_IDFILTER(IM);
    CASE_IDFILTER(KE);
    CASE_IDFILTER(LA);
    CASE_IDFILTER(LI);
    CASE_IDFILTER(LP);
    CASE_IDFILTER(LS);
    CASE_IDFILTER(LT);
    CASE_IDFILTER(MA);
    CASE_IDFILTER(MB);
    CASE_IDFILTER(MC);
    CASE_IDFILTER(ME);
    CASE_IDFILTER(MSK);
    CASE_IDFILTER(NT);
    CASE_IDFILTER(OB);
    CASE_IDFILTER(PA);
    CASE_IDFILTER(PAL);
    CASE_IDFILTER(PC);
    CASE_IDFILTER(PT);
    CASE_IDFILTER(SCE);
    CASE_IDFILTER(SCR);
    CASE_IDFILTER(SIM);
    CASE_IDFILTER(SO);
    CASE_IDFILTER(SPK);
    CASE_IDFILTER(TE);
    CASE_IDFILTER(TXT);
    CASE_IDFILTER(VF);
    CASE_IDFILTER(VO);
    CASE_IDFILTER(WM);
    CASE_IDFILTER(WO);
    CASE_IDFILTER(WS);
  }

  KLI_assert_unreachable();
  return 0;

#undef CASE_IDFILTER
#undef CASE_IDFILTER_NONE
}

short KKE_idtype_idcode_from_idfilter(const uint64_t idfilter)
{
#define CASE_IDFILTER(_id) \
  case FILTER_ID_##_id:    \
    return ID_##_id

#define CASE_IDFILTER_NONE(_id) (void)0

  switch (idfilter) {
    CASE_IDFILTER(AC);
    CASE_IDFILTER(AR);
    CASE_IDFILTER(BR);
    CASE_IDFILTER(CA);
    CASE_IDFILTER(CF);
    CASE_IDFILTER(CV);
    CASE_IDFILTER(GD);
    CASE_IDFILTER(GR);
    CASE_IDFILTER(IM);
    CASE_IDFILTER(KE);
    CASE_IDFILTER(LA);
    CASE_IDFILTER(LI);
    CASE_IDFILTER(LP);
    CASE_IDFILTER(LS);
    CASE_IDFILTER(LT);
    CASE_IDFILTER(MA);
    CASE_IDFILTER(MB);
    CASE_IDFILTER(MC);
    CASE_IDFILTER(ME);
    CASE_IDFILTER(MSK);
    CASE_IDFILTER(NT);
    CASE_IDFILTER(OB);
    CASE_IDFILTER(PA);
    CASE_IDFILTER(PAL);
    CASE_IDFILTER(PC);
    CASE_IDFILTER(PT);
    CASE_IDFILTER(SCE);
    CASE_IDFILTER(SCR);
    CASE_IDFILTER(SIM);
    CASE_IDFILTER(SO);
    CASE_IDFILTER(SPK);
    CASE_IDFILTER(TE);
    CASE_IDFILTER(TXT);
    CASE_IDFILTER(VF);
    CASE_IDFILTER(VO);
    CASE_IDFILTER(WM);
    CASE_IDFILTER(WO);
    CASE_IDFILTER(WS);
  }

  KLI_assert_unreachable();
  return 0;

#undef CASE_IDFILTER
#undef CASE_IDFILTER_NONE
}

int KKE_idtype_idcode_to_index(const short idcode)
{
#define CASE_IDINDEX(_id) \
  case ID_##_id:          \
    return INDEX_ID_##_id

  switch ((ID_Type)idcode) {
    CASE_IDINDEX(AC);
    CASE_IDINDEX(AR);
    CASE_IDINDEX(BR);
    CASE_IDINDEX(CA);
    CASE_IDINDEX(CF);
    CASE_IDINDEX(CV);
    CASE_IDINDEX(GD);
    CASE_IDINDEX(GR);
    CASE_IDINDEX(IM);
    CASE_IDINDEX(KE);
    CASE_IDINDEX(LA);
    CASE_IDINDEX(LI);
    CASE_IDINDEX(LS);
    CASE_IDINDEX(LT);
    CASE_IDINDEX(MA);
    CASE_IDINDEX(MB);
    CASE_IDINDEX(MC);
    CASE_IDINDEX(ME);
    CASE_IDINDEX(MSK);
    CASE_IDINDEX(NT);
    CASE_IDINDEX(OB);
    CASE_IDINDEX(PA);
    CASE_IDINDEX(PAL);
    CASE_IDINDEX(PC);
    CASE_IDINDEX(PT);
    CASE_IDINDEX(LP);
    CASE_IDINDEX(SCE);
    CASE_IDINDEX(SCR);
    CASE_IDINDEX(SIM);
    CASE_IDINDEX(SPK);
    CASE_IDINDEX(SO);
    CASE_IDINDEX(TE);
    CASE_IDINDEX(TXT);
    CASE_IDINDEX(VF);
    CASE_IDINDEX(VO);
    CASE_IDINDEX(WM);
    CASE_IDINDEX(WO);
    CASE_IDINDEX(WS);
  }

  /* Special naughty boy... */
  if (idcode == ID_LINK_PLACEHOLDER) {
    return INDEX_ID_NULL;
  }

  return -1;

#undef CASE_IDINDEX
}

short KKE_idtype_idcode_from_index(const int index)
{
#define CASE_IDCODE(_id) \
  case INDEX_ID_##_id:   \
    return ID_##_id

  switch (index) {
    CASE_IDCODE(AC);
    CASE_IDCODE(AR);
    CASE_IDCODE(BR);
    CASE_IDCODE(CA);
    CASE_IDCODE(CF);
    CASE_IDCODE(CV);
    CASE_IDCODE(GD);
    CASE_IDCODE(GR);
    CASE_IDCODE(IM);
    CASE_IDCODE(KE);
    CASE_IDCODE(LA);
    CASE_IDCODE(LI);
    CASE_IDCODE(LS);
    CASE_IDCODE(LT);
    CASE_IDCODE(MA);
    CASE_IDCODE(MB);
    CASE_IDCODE(MC);
    CASE_IDCODE(ME);
    CASE_IDCODE(MSK);
    CASE_IDCODE(NT);
    CASE_IDCODE(OB);
    CASE_IDCODE(PA);
    CASE_IDCODE(PAL);
    CASE_IDCODE(PC);
    CASE_IDCODE(PT);
    CASE_IDCODE(LP);
    CASE_IDCODE(SCE);
    CASE_IDCODE(SCR);
    CASE_IDCODE(SIM);
    CASE_IDCODE(SPK);
    CASE_IDCODE(SO);
    CASE_IDCODE(TE);
    CASE_IDCODE(TXT);
    CASE_IDCODE(VF);
    CASE_IDCODE(VO);
    CASE_IDCODE(WM);
    CASE_IDCODE(WO);
    CASE_IDCODE(WS);
  }

  /* Special naughty boy... */
  if (index == INDEX_ID_NULL) {
    return ID_LINK_PLACEHOLDER;
  }

  return -1;

#undef CASE_IDCODE
}

short KKE_idtype_idcode_iter_step(int *index)
{
  return (*index < ARRAY_SIZE(id_types)) ? KKE_idtype_idcode_from_index((*index)++) : 0;
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

void KKE_idtype_id_foreach_cache(struct ID *id,
                                 IDTypeForeachCacheFunctionCallback function_callback,
                                 void *user_data)
{
  const IDTypeInfo *type_info = KKE_idtype_get_info_from_id(id);
  if (type_info->foreach_cache != NULL) {
    type_info->foreach_cache(id, function_callback, user_data);
  }

  /* Handle 'private IDs'. */
  kNodeTree *nodetree = ntreeFromID(id);
  if (nodetree != NULL) {
    type_info = KKE_idtype_get_info_from_id(&nodetree->id);
    if (type_info == NULL) {
      type_info = KKE_idtype_get_info_from_idcode(ID_NT);
    }
    if (type_info->foreach_cache != NULL) {
      type_info->foreach_cache(&nodetree->id, function_callback, user_data);
    }
  }

  if (GS(id->name) == ID_SCE) {
    Scene *scene = (Scene *)id;
    if (scene->master_collection != NULL) {
      type_info = KKE_idtype_get_info_from_id(&scene->master_collection->id);
      if (type_info->foreach_cache != NULL) {
        type_info->foreach_cache(&scene->master_collection->id, function_callback, user_data);
      }
    }
  }
}
