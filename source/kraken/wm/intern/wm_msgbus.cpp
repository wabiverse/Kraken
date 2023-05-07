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
 * Window Manager.
 * Making GUI Fly.
 */

#include <string.h>

#include "CLG_log.h"

#include "kraken/kraken.h"

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_ID.h"
#include "USD_object.h"
#include "USD_operator.h"

#include "WM_api.h"
#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_window.hh"

#include "KLI_rhash.h"
#include "KLI_listbase.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"

#include "LUXO_access.h"

#include <mutex>
#include <string>
#include <vector>

/* to replace string chars. */
#include <boost/algorithm/string/replace.hpp>

WABI_NAMESPACE_USING

/* -------------------------------------------------------------------------- */
/** @name Internal API
 * @{ */

typedef struct wmMsgBus
{
  RSet *messages_rset[WM_MSG_TYPE_NUM];
  /** Messages in order of being added. */
  ListBase messages;
  /** Avoid checking messages when no tags exist. */
  uint messages_tag_count;
} wmMsgBus;

/** @} */

/* -------------------------------------------------------------------------- */
/** @name Public API
 * @{ */

/* Initialization. */

static wmMsgTypeInfo wm_msg_types[WM_MSG_TYPE_NUM] = {{{NULL}}};

typedef void (*wmMsgTypeInitFn)(wmMsgTypeInfo *);

static wmMsgTypeInitFn wm_msg_init_fn[WM_MSG_TYPE_NUM] = {
  WM_msgtypeinfo_init_prim,
  WM_msgtypeinfo_init_static,
};

void WM_msgbus_types_init(void)
{
  for (uint i = 0; i < WM_MSG_TYPE_NUM; i++) {
    wm_msg_init_fn[i](&wm_msg_types[i]);
  }
}

struct wmMsgBus *WM_msgbus_create(void)
{
  struct wmMsgBus *mbus = (wmMsgBus *)MEM_callocN(sizeof(*mbus), __func__);
  const uint rset_reserve = 512;
  for (uint i = 0; i < WM_MSG_TYPE_NUM; i++) {
    wmMsgTypeInfo *info = &wm_msg_types[i];
    mbus->messages_rset[i] = KLI_rset_new_ex(info->rset.hash_fn,
                                             info->rset.cmp_fn,
                                             __func__,
                                             rset_reserve);
  }
  return mbus;
}

void WM_msgbus_destroy(struct wmMsgBus *mbus)
{
  for (uint i = 0; i < WM_MSG_TYPE_NUM; i++) {
    wmMsgTypeInfo *info = &wm_msg_types[i];
    KLI_rset_free(mbus->messages_rset[i], info->rset.key_free_fn);
  }
  MEM_freeN(mbus);
}

void WM_msgbus_clear_by_owner(struct wmMsgBus *mbus, void *owner)
{
  wmMsgSubscribeKey *msg_key, *msg_key_next;
  for (msg_key = (wmMsgSubscribeKey *)mbus->messages.first; msg_key; msg_key = msg_key_next) {
    msg_key_next = msg_key->next;

    wmMsgSubscribeValueLink *msg_lnk_next;
    for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)msg_key->values.first;
         msg_lnk;
         msg_lnk = msg_lnk_next) {
      msg_lnk_next = msg_lnk->next;
      if (msg_lnk->params.owner == owner) {
        if (msg_lnk->params.tag) {
          mbus->messages_tag_count -= 1;
        }
        if (msg_lnk->params.free_data) {
          msg_lnk->params.free_data(msg_key, &msg_lnk->params);
        }
        KLI_remlink(&msg_key->values, msg_lnk);
        MEM_freeN(msg_lnk);
      }
    }

    if (KLI_listbase_is_empty(&msg_key->values)) {
      const wmMsg *msg = wm_msg_subscribe_value_msg_cast(msg_key);
      wmMsgTypeInfo *info = &wm_msg_types[msg->type];
      KLI_remlink(&mbus->messages, msg_key);
      bool ok = KLI_rset_remove(mbus->messages_rset[msg->type], msg_key, info->rset.key_free_fn);
      KLI_assert(ok);
      UNUSED_VARS_NDEBUG(ok);
    }
  }
}

void WM_msg_dump(struct wmMsgBus *mbus, const char *info_str)
{
  printf(">>>> %s\n", info_str);
  LISTBASE_FOREACH(wmMsgSubscribeKey *, key, &mbus->messages)
  {
    const wmMsg *msg = wm_msg_subscribe_value_msg_cast(key);
    const wmMsgTypeInfo *info = &wm_msg_types[msg->type];
    info->repr(stdout, key);
  }
  printf("<<<< %s\n", info_str);
}

void WM_msgbus_handle(struct wmMsgBus *mbus, struct kContext *C)
{
  if (mbus->messages_tag_count == 0) {
    // printf("msgbus: skipping\n");
    return;
  }

  if (false) {
    WM_msg_dump(mbus, __func__);
  }

  // uint a = 0, b = 0;
  LISTBASE_FOREACH(wmMsgSubscribeKey *, key, &mbus->messages)
  {
    LISTBASE_FOREACH(wmMsgSubscribeValueLink *, msg_lnk, &key->values)
    {
      if (msg_lnk->params.tag) {
        msg_lnk->params.notify(C, key, &msg_lnk->params);
        msg_lnk->params.tag = false;
        mbus->messages_tag_count -= 1;
      }
      // b++;
    }
    // a++;
  }
  KLI_assert(mbus->messages_tag_count == 0);
  mbus->messages_tag_count = 0;
  // printf("msgbus: keys=%u values=%u\n", a, b);
}

wmMsgSubscribeKey *WM_msg_subscribe_with_key(struct wmMsgBus *mbus,
                                             const wmMsgSubscribeKey *msg_key_test,
                                             const wmMsgSubscribeValue *msg_val_params)
{
  const uint type = wm_msg_subscribe_value_msg_cast(msg_key_test)->type;
  const wmMsgTypeInfo *info = &wm_msg_types[type];
  wmMsgSubscribeKey *key;

  KLI_assert(wm_msg_subscribe_value_msg_cast(msg_key_test)->id != NULL);

  void **r_key;
  if (!KLI_rset_ensure_p_ex(mbus->messages_rset[type], msg_key_test, &r_key)) {
    key = static_cast<wmMsgSubscribeKey *>(*r_key = MEM_mallocN(info->msg_key_size, __func__));
    memcpy(key, msg_key_test, info->msg_key_size);
    KLI_addtail(&mbus->messages, key);
  } else {
    key = static_cast<wmMsgSubscribeKey *>(*r_key);
    LISTBASE_FOREACH(wmMsgSubscribeValueLink *, msg_lnk, &key->values)
    {
      if ((msg_lnk->params.notify == msg_val_params->notify) &&
          (msg_lnk->params.owner == msg_val_params->owner) &&
          (msg_lnk->params.user_data == msg_val_params->user_data)) {
        return key;
      }
    }
  }

  wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)MEM_mallocN(
    sizeof(wmMsgSubscribeValueLink),
    __func__);
  msg_lnk->params = *msg_val_params;
  KLI_addtail(&key->values, msg_lnk);
  return key;
}

void WM_msg_publish_with_key(struct wmMsgBus *mbus, wmMsgSubscribeKey *msg_key)
{
  CLOG_INFO(WM_LOG_MSGBUS_SUB,
            2,
            "tagging subscribers: (ptr=%p, len=%d)",
            msg_key,
            KLI_listbase_count(&msg_key->values));

  LISTBASE_FOREACH(wmMsgSubscribeValueLink *, msg_lnk, &msg_key->values)
  {
    if (false) { /* make an option? */
      msg_lnk->params.notify(NULL, msg_key, &msg_lnk->params);
    } else {
      if (msg_lnk->params.tag == false) {
        msg_lnk->params.tag = true;
        mbus->messages_tag_count += 1;
      }
    }
  }
}

void WM_msg_id_update(struct wmMsgBus *mbus, struct ID *id_src, struct ID *id_dst)
{
  for (uint i = 0; i < WM_MSG_TYPE_NUM; i++) {
    wmMsgTypeInfo *info = &wm_msg_types[i];
    if (info->update_by_id != NULL) {
      info->update_by_id(mbus, id_src, id_dst);
    }
  }
}

void WM_msg_id_remove(struct wmMsgBus *mbus, const struct ID *id)
{
  for (uint i = 0; i < WM_MSG_TYPE_NUM; i++) {
    wmMsgTypeInfo *info = &wm_msg_types[i];
    if (info->remove_by_id != NULL) {
      info->remove_by_id(mbus, id);
    }
  }
}

/** @} */

/* -------------------------------------------------------------------------- */
/** @name MsgBus Hash Prims
 * @{ */

KLI_INLINE uint void_hash_uint(const void *key)
{
  size_t y = (size_t)key >> (sizeof(void *));
  return (uint)y;
}

static uint wm_msg_prim_rset_hash(const void *key_p)
{
  const wmMsgSubscribeKey_PRIM *key = static_cast<const wmMsgSubscribeKey_PRIM *>(key_p);
  const wmMsgParams_PRIM *params = &key->msg.params;
  //  printf("%s\n", LUXO_prim_identifier(params->ptr.type));
  uint k = void_hash_uint(params->ptr.type);
  k ^= void_hash_uint(params->ptr.data);
  k ^= void_hash_uint(params->ptr.owner_id);
  k ^= void_hash_uint(params->prop);
  return k;
}

static bool wm_msg_prim_rset_cmp(const void *key_a_p, const void *key_b_p)
{
  const wmMsgParams_PRIM *params_a = &((const wmMsgSubscribeKey_PRIM *)key_a_p)->msg.params;
  const wmMsgParams_PRIM *params_b = &((const wmMsgSubscribeKey_PRIM *)key_b_p)->msg.params;
  return !((params_a->ptr.type == params_b->ptr.type) &&
           (params_a->ptr.owner_id == params_b->ptr.owner_id) &&
           (params_a->ptr.data == params_b->ptr.data) && (params_a->prop == params_b->prop));
}

static void wm_msg_prim_rset_key_free(void *key_p)
{
  wmMsgSubscribeKey_PRIM *key = (wmMsgSubscribeKey_PRIM *)key_p;
  wmMsgSubscribeValueLink *msg_lnk_next;
  for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)key->head.values.first;
       msg_lnk;
       msg_lnk = msg_lnk_next) {
    msg_lnk_next = msg_lnk->next;
    WM_msg_subscribe_value_free(&key->head, msg_lnk);
  }
  if (key->msg.params.data_path != NULL) {
    // MEM_freeN(key->msg.params.data_path);
  }
  MEM_freeN(key);
}

static void wm_msg_prim_repr(FILE *stream, const wmMsgSubscribeKey *msg_key)
{
  const wmMsgSubscribeKey_PRIM *m = (wmMsgSubscribeKey_PRIM *)msg_key;
  const char *none = "<none>";
  fprintf(stream,
          "<wmMsg_PRIM %p, "
          "id='%s', "
          "%s.%s values_len=%d\n",
          m,
          m->msg.head.id,
          m->msg.params.ptr.type ? LUXO_prim_identifier(m->msg.params.ptr.type).data() : none,
          m->msg.params.prop ? LUXO_prop_identifier((KrakenPROP *)m->msg.params.prop).data() :
                               none,
          KLI_listbase_count(&m->head.values));
}

static void wm_msg_prim_update_by_id(struct wmMsgBus *mbus, ID *id_src, ID *id_dst)
{
  RSet *rs = mbus->messages_rset[WM_MSG_TYPE_PRIM];
  RSetIterator rs_iter;
  KLI_rsetIterator_init(&rs_iter, rs);
  while (KLI_rsetIterator_done(&rs_iter) == false) {
    wmMsgSubscribeKey_PRIM *key = static_cast<wmMsgSubscribeKey_PRIM *>(
      KLI_rsetIterator_getKey(&rs_iter));
    KLI_rsetIterator_step(&rs_iter);
    if (key->msg.params.ptr.owner_id == id_src) {

      /* RSet always needs updating since the key changes. */
      KLI_rset_remove(rs, key, NULL);

      /* Remove any non-persistent values, so a single persistent
       * value doesn't modify behavior for the rest. */
      for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)key->head.values.first,
                                   *msg_lnk_next;
           msg_lnk;
           msg_lnk = msg_lnk_next) {
        msg_lnk_next = msg_lnk->next;
        if (msg_lnk->params.is_persistent == false) {
          if (msg_lnk->params.tag) {
            mbus->messages_tag_count -= 1;
          }
          WM_msg_subscribe_value_free(&key->head, msg_lnk);
        }
      }

      bool remove = true;

      if (KLI_listbase_is_empty(&key->head.values)) {
        /* Remove, no reason to keep. */
      } else if (key->msg.params.ptr.data == key->msg.params.ptr.owner_id) {
        /* Simple, just update the ID. */
        key->msg.params.ptr.data = id_dst;
        key->msg.params.ptr.owner_id = id_dst;
        remove = false;
      } else {
        /* find or create the sdf type for this new property. */
        SdfValueTypeName type_name = SdfSchema::GetInstance().FindOrCreateType(
          TfToken(id_dst->name));
        /* Resolve this property path. */
        UsdPrimRange spaths = KRAKEN_STAGE->Traverse();
        for (const auto &prim : spaths) {
          if ((prim.GetName() == key->msg.params.data_path) &&
              (key->msg.params.prop == nullptr || !key->msg.params.prop->IsValid()) &&
              (!prim.HasAttribute(key->msg.params.prop->GetName()))) {
            /* create the new property since it does not exist. */
            KrakenPROP prop = prim.CreateAttribute(TfToken(id_dst->name), type_name);
            key->msg.params.ptr = prim;
            key->msg.params.prop = &prop;
            remove = false;
          }
        }
      }

      if (remove) {
        for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)key->head.values.first,
                                     *msg_lnk_next;
             msg_lnk;
             msg_lnk = msg_lnk_next) {
          msg_lnk_next = msg_lnk->next;
          if (msg_lnk->params.is_persistent == false) {
            if (msg_lnk->params.tag) {
              mbus->messages_tag_count -= 1;
            }
            WM_msg_subscribe_value_free(&key->head, msg_lnk);
          }
        }
        /* Failed to persist, remove the key. */
        KLI_remlink(&mbus->messages, key);
        wm_msg_prim_rset_key_free(key);
      } else {
        /* Note that it's not impossible this key exists, however it is very unlikely
         * since a subscriber would need to register in the middle of an undo for eg.
         * so assert for now. */
        KLI_assert(!KLI_rset_haskey(rs, key));
        KLI_rset_add(rs, key);
      }
    }
  }
}

static void wm_msg_prim_remove_by_id(struct wmMsgBus *mbus, const ID *id)
{
  RSet *rs = mbus->messages_rset[WM_MSG_TYPE_PRIM];
  RSetIterator rs_iter;
  KLI_rsetIterator_init(&rs_iter, rs);
  while (KLI_rsetIterator_done(&rs_iter) == false) {
    wmMsgSubscribeKey_PRIM *key = (wmMsgSubscribeKey_PRIM *)KLI_rsetIterator_getKey(&rs_iter);
    KLI_rsetIterator_step(&rs_iter);
    if (key->msg.params.ptr.owner_id == id) {
      /* Clear here so we can decrement 'messages_tag_count'. */
      for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)key->head.values.first,
                                   *msg_lnk_next;
           msg_lnk;
           msg_lnk = msg_lnk_next) {
        msg_lnk_next = msg_lnk->next;
        if (msg_lnk->params.tag) {
          mbus->messages_tag_count -= 1;
        }
        WM_msg_subscribe_value_free(&key->head, msg_lnk);
      }

      KLI_remlink(&mbus->messages, key);
      KLI_rset_remove(rs, key, NULL);
      wm_msg_prim_rset_key_free(key);
    }
  }
}

void WM_msgtypeinfo_init_prim(wmMsgTypeInfo *msgtype_info)
{
  msgtype_info->rset.hash_fn = wm_msg_prim_rset_hash;
  msgtype_info->rset.cmp_fn = wm_msg_prim_rset_cmp;
  msgtype_info->rset.key_free_fn = wm_msg_prim_rset_key_free;

  msgtype_info->repr = wm_msg_prim_repr;
  msgtype_info->update_by_id = wm_msg_prim_update_by_id;
  msgtype_info->remove_by_id = wm_msg_prim_remove_by_id;

  msgtype_info->msg_key_size = sizeof(wmMsgSubscribeKey_PRIM);
}

/** @} */

/* -------------------------------------------------------------------------- */
/** @name MsgBus PRIM API
 * @{ */

wmMsgSubscribeKey_PRIM *WM_msg_lookup_prim(struct wmMsgBus *mbus,
                                           const wmMsgParams_PRIM *msg_key_params)
{
  wmMsgSubscribeKey_PRIM key_test;
  key_test.msg.params = *msg_key_params;
  return (wmMsgSubscribeKey_PRIM *)KLI_rset_lookup(mbus->messages_rset[WM_MSG_TYPE_PRIM],
                                                   &key_test);
}

void WM_msg_publish_prim_params(struct wmMsgBus *mbus, const wmMsgParams_PRIM *msg_key_params)
{
  wmMsgSubscribeKey_PRIM *key;

  const char *none = "<none>";
  CLOG_INFO(
    WM_LOG_MSGBUS_PUB,
    2,
    "rna(id='%s', %s.%s)",
    msg_key_params->ptr.owner_id ? ((ID *)msg_key_params->ptr.owner_id)->name : none,
    msg_key_params->ptr.type ? LUXO_prim_identifier(msg_key_params->ptr.type).data() : none,
    msg_key_params->prop ? LUXO_prop_identifier((KrakenPROP *)msg_key_params->prop).data() : none);

  if ((key = WM_msg_lookup_prim(mbus, msg_key_params))) {
    WM_msg_publish_with_key(mbus, &key->head);
  }

  /* Support anonymous subscribers, this may be some extra overhead
   * but we want to be able to be more ambiguous. */
  if (msg_key_params->ptr.owner_id || msg_key_params->ptr.data) {
    wmMsgParams_PRIM msg_key_params_anon = *msg_key_params;

    /* We might want to enable this later? */
    if (msg_key_params_anon.prop != NULL) {
      /* All properties for this type. */
      msg_key_params_anon.prop = NULL;
      if ((key = WM_msg_lookup_prim(mbus, &msg_key_params_anon))) {
        WM_msg_publish_with_key(mbus, &key->head);
      }
      msg_key_params_anon.prop = msg_key_params->prop;
    }

    msg_key_params_anon.ptr.owner_id = NULL;
    msg_key_params_anon.ptr.data = NULL;
    if ((key = WM_msg_lookup_prim(mbus, &msg_key_params_anon))) {
      WM_msg_publish_with_key(mbus, &key->head);
    }

    /* Support subscribers to a type. */
    if (msg_key_params->prop) {
      msg_key_params_anon.prop = NULL;
      if ((key = WM_msg_lookup_prim(mbus, &msg_key_params_anon))) {
        WM_msg_publish_with_key(mbus, &key->head);
      }
    }
  }
}

void WM_msg_publish_prim(struct wmMsgBus *mbus, KrakenPRIM *ptr, KrakenPROP *prop)
{
  const wmMsgParams_PRIM params = {
    .ptr = *ptr, 
    .prop = prop
  };

  WM_msg_publish_prim_params(mbus, &params);
}

void WM_msg_subscribe_prim_params(struct wmMsgBus *mbus,
                                  const wmMsgParams_PRIM *msg_key_params,
                                  const wmMsgSubscribeValue *msg_val_params,
                                  const char *id_repr)
{
  wmMsgSubscribeKey_PRIM msg_key_test = {{NULL}};

  /* use when added */
  msg_key_test.msg.head.id = id_repr;
  msg_key_test.msg.head.type = WM_MSG_TYPE_PRIM;
  /* for lookup */
  msg_key_test.msg.params = *msg_key_params;

  const char *none = "<none>";
  CLOG_INFO(
    WM_LOG_MSGBUS_SUB,
    3,
    "rna(id='%s', %s.%s, info='%s')",
    msg_key_params->ptr.owner_id ? ((ID *)msg_key_params->ptr.owner_id)->name : none,
    msg_key_params->ptr.type ? LUXO_prim_identifier(msg_key_params->ptr.type).data() : none,
    msg_key_params->prop ? LUXO_prop_identifier((KrakenPROP *)msg_key_params->prop).data() : none,
    id_repr);

  wmMsgSubscribeKey_PRIM *msg_key = (wmMsgSubscribeKey_PRIM *)
    WM_msg_subscribe_with_key(mbus, &msg_key_test.head, msg_val_params);

  if (msg_val_params->is_persistent) {
    if (msg_key->msg.params.data_path == TfToken()) {
      if ((ID *)msg_key->msg.params.ptr.data != msg_key->msg.params.ptr.owner_id) {
        /* We assume prop type can't change. */
        std::string screenob = msg_key->msg.params.ptr.GetPath().GetAsString();
        boost::replace_all(screenob, "/", ".");
        msg_key->msg.params.data_path = TfToken(screenob);
      }
    }
  }
}

void WM_msg_subscribe_prim(struct wmMsgBus *mbus,
                           KrakenPRIM *ptr,
                           const KrakenPROP *prop,
                           const wmMsgSubscribeValue *msg_val_params,
                           const char *id_repr)
{
  const wmMsgParams_PRIM params = {
    .ptr = *ptr,
    .prop = prop,
  };
  WM_msg_subscribe_prim_params(mbus, &params, msg_val_params, id_repr);
}


/** @} */

/* -------------------------------------------------------------------------- */
/** @name ID variants of RNA API
 * @{ */

void WM_msg_subscribe_ID(struct wmMsgBus *mbus,
                         ID *id,
                         const wmMsgSubscribeValue *msg_val_params,
                         const char *id_repr)
{
  wmMsgParams_PRIM msg_key_params = {{NULL}};
  LUXO_id_pointer_create(id, &msg_key_params.ptr);
  WM_msg_subscribe_prim_params(mbus, &msg_key_params, msg_val_params, id_repr);
}

void WM_msg_publish_ID(struct wmMsgBus *mbus, ID *id)
{
  wmMsgParams_PRIM msg_key_params = {{NULL}};
  LUXO_id_pointer_create(id, &msg_key_params.ptr);
  WM_msg_publish_prim_params(mbus, &msg_key_params);
}

/* -------------------------------------------------------------------------- */
/** @name MsgBus Hash Static
 * @{ */

static uint wm_msg_static_rset_hash(const void *key_p)
{
  const wmMsgSubscribeKey_Static *key = static_cast<const wmMsgSubscribeKey_Static *>(key_p);
  const wmMsgParams_Static *params = &key->msg.params;
  uint k = params->event;
  return k;
}

static bool wm_msg_static_rset_cmp(const void *key_a_p, const void *key_b_p)
{
  const wmMsgParams_Static *params_a = &((const wmMsgSubscribeKey_Static *)key_a_p)->msg.params;
  const wmMsgParams_Static *params_b = &((const wmMsgSubscribeKey_Static *)key_b_p)->msg.params;
  return !(params_a->event == params_b->event);
}

static void wm_msg_static_rset_key_free(void *key_p)
{
  wmMsgSubscribeKey *key = (wmMsgSubscribeKey *)key_p;
  wmMsgSubscribeValueLink *msg_lnk_next;
  for (wmMsgSubscribeValueLink *msg_lnk = (wmMsgSubscribeValueLink *)key->values.first; msg_lnk;
       msg_lnk = msg_lnk_next) {
    msg_lnk_next = msg_lnk->next;
    KLI_remlink(&key->values, msg_lnk);
    MEM_freeN(msg_lnk);
  }
  MEM_freeN(key);
}

static void wm_msg_static_repr(FILE *stream, const wmMsgSubscribeKey *msg_key)
{
  const wmMsgSubscribeKey_Static *m = (wmMsgSubscribeKey_Static *)msg_key;
  fprintf(stream,
          "<wmMsg_Static %p, "
          "id='%s', "
          "values_len=%d\n",
          m,
          m->msg.head.id,
          KLI_listbase_count(&m->head.values));
}

void WM_msgtypeinfo_init_static(wmMsgTypeInfo *msgtype_info)
{
  msgtype_info->rset.hash_fn = wm_msg_static_rset_hash;
  msgtype_info->rset.cmp_fn = wm_msg_static_rset_cmp;
  msgtype_info->rset.key_free_fn = wm_msg_static_rset_key_free;
  msgtype_info->repr = wm_msg_static_repr;

  msgtype_info->msg_key_size = sizeof(wmMsgSubscribeKey_Static);
}

/* -------------------------------------------------------------------------- */

wmMsgSubscribeKey_Static *WM_msg_lookup_static(struct wmMsgBus *mbus,
                                               const wmMsgParams_Static *msg_key_params)
{
  wmMsgSubscribeKey_Static key_test;
  key_test.msg.params = *msg_key_params;
  return static_cast<wmMsgSubscribeKey_Static *>(KLI_rset_lookup(mbus->messages_rset[WM_MSG_TYPE_STATIC], &key_test));
}

void WM_msg_publish_static_params(struct wmMsgBus *mbus, const wmMsgParams_Static *msg_key_params)
{
  CLOG_INFO(WM_LOG_MSGBUS_PUB, 2, "static(event=%d)", msg_key_params->event);

  wmMsgSubscribeKey_Static *key = WM_msg_lookup_static(mbus, msg_key_params);
  if (key) {
    WM_msg_publish_with_key(mbus, &key->head);
  }
}

void WM_msg_publish_static(struct wmMsgBus *mbus, int event)
{
  const wmMsgParams_Static params = {
    .event = event,
  };

  WM_msg_publish_static_params(mbus, &params);
}

void WM_msg_subscribe_static_params(struct wmMsgBus *mbus,
                                    const wmMsgParams_Static *msg_key_params,
                                    const wmMsgSubscribeValue *msg_val_params,
                                    const char *id_repr)
{
  wmMsgSubscribeKey_Static msg_key_test = {{NULL}};

  /* use when added */
  msg_key_test.msg.head.id = id_repr;
  msg_key_test.msg.head.type = WM_MSG_TYPE_STATIC;
  /* for lookup */
  msg_key_test.msg.params = *msg_key_params;

  WM_msg_subscribe_with_key(mbus, &msg_key_test.head, msg_val_params);
}

void WM_msg_subscribe_static(struct wmMsgBus *mbus,
                             int event,
                             const wmMsgSubscribeValue *msg_val_params,
                             const char *id_repr)
{
  const wmMsgParams_Static params = {
    .event = event,
  };

  WM_msg_subscribe_static_params(mbus,
                                 &params,
                                 msg_val_params,
                                 id_repr);
}

/** @} */

/* -------------------------------------------------------------------------- */
/** @name MsgBus Internal API
 * @{ */

void WM_msg_subscribe_value_free(wmMsgSubscribeKey *msg_key, wmMsgSubscribeValueLink *msg_lnk)
{
  if (msg_lnk->params.free_data) {
    msg_lnk->params.free_data(msg_key, &msg_lnk->params);
  }
  KLI_remlink(&msg_key->values, msg_lnk);
  MEM_freeN(msg_lnk);
}

/** @} */
