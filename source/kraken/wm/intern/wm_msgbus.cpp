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

/**
 *  -----  The Kraken WindowManager. ----- */


/**
 *  -----  Initialization. ----- */

static wmMsgTypeInfo wm_msg_types[WM_MSG_TYPE_NUM] = {{{NULL}}};

typedef void (*wmMsgTypeInitFn)(wmMsgTypeInfo *);

KLI_INLINE uint void_hash_uint(const void *key)
{
  size_t y = (size_t)key >> (sizeof(void *));
  return (uint)y;
}

/* PRIM PATH MSG BUS INIT */

static uint wm_msg_prim_rset_hash(const void *key_p)
{
  const wmMsgSubscribeKey_PRIM *key = static_cast<const wmMsgSubscribeKey_PRIM *>(key_p);
  const wmMsgParams_PRIM *params = &key->msg.params;
  //  printf("%s\n", RNA_struct_identifier(params->ptr.type));
  uint k = void_hash_uint(params->ptr.type);
  k ^= void_hash_uint(params->ptr.data);
  k ^= void_hash_uint(params->ptr.owner_id);
  k ^= void_hash_uint(params->prop);
  return k;
}

static void wm_msg_subscribe_value_free(wmMsgSubscribeKey *msg_key,
                                        wmMsgSubscribeValueLink *msg_lnk)
{
  if (msg_lnk->params.free_data) {
    msg_lnk->params.free_data(msg_key, &msg_lnk->params);
  }

  std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_it = msg_key->values.begin();
  std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_end = msg_key->values.end();

  for (; msg_it != msg_end; ++msg_it) {
    if ((*msg_it) == msg_lnk) {
      msg_key->values.erase(msg_it);
    }
  }

  MEM_freeN(msg_lnk);
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
  wmMsgSubscribeKey_PRIM *key = static_cast<wmMsgSubscribeKey_PRIM *>(key_p);
  for (auto &msg_lnk : key->head.values) {
    wm_msg_subscribe_value_free(&key->head, msg_lnk);
  }
  if (!key->msg.params.data_path.IsEmpty()) {
    key->msg.params.data_path = TfToken();
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
          m->msg.params.ptr.type ? LUXO_struct_identifier(m->msg.params.ptr.type).data() : none,
          m->msg.params.prop ? LUXO_property_identifier((KrakenPROP *)m->msg.params.prop).data() :
                               none,
          (int)m->head.values.size());
}

static void wm_msg_prim_update_by_id(struct wmMsgBus *mbus, ID *id_src, ID *id_dst)
{
  RSet *rs = mbus->messages_rset[WM_MSG_TYPE_RNA];
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
      std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_it = key->head.values.begin();
      std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_end = key->head.values.end();

      for (; msg_it != msg_end; ++msg_it) {
        if ((*msg_it)->params.is_persistent == false) {
          if ((*msg_it)->params.tag) {
            mbus->messages_tag_count -= 1;
          }
          wm_msg_subscribe_value_free(&key->head, (*msg_it));
        }
      }

      bool remove = true;

      if (key->head.values.empty()) {
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

            /* set the prim and the prop. */
            key->msg.params.ptr = prim;
            key->msg.params.prop = &prop;
            remove = false;
          }
        }
      }

      if (remove) {
        for (; msg_it != msg_end; ++msg_it) {
          if ((*msg_it)->params.is_persistent == false) {
            if ((*msg_it)->params.tag) {
              mbus->messages_tag_count -= 1;
            }
            wm_msg_subscribe_value_free(&key->head, (*msg_it));
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
  RSet *rs = mbus->messages_rset[WM_MSG_TYPE_RNA];
  RSetIterator rs_iter;
  KLI_rsetIterator_init(&rs_iter, rs);
  while (KLI_rsetIterator_done(&rs_iter) == false) {
    wmMsgSubscribeKey_PRIM *key = static_cast<wmMsgSubscribeKey_PRIM *>(
      KLI_rsetIterator_getKey(&rs_iter));
    KLI_rsetIterator_step(&rs_iter);
    if (key->msg.params.ptr.owner_id == id) {
      std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_it = key->head.values.begin();
      std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_end = key->head.values.end();

      /* Clear here so we can decrement 'messages_tag_count'. */
      for (; msg_it != msg_end; ++msg_it) {
        if ((*msg_it)->params.tag) {
          mbus->messages_tag_count -= 1;
        }
        wm_msg_subscribe_value_free(&key->head, (*msg_it));
      }

      KLI_remlink(&mbus->messages, key);
      KLI_rset_remove(rs, key, NULL);
      wm_msg_prim_rset_key_free(key);
    }
  }
}

static void WM_msgtypeinfo_init_prim(wmMsgTypeInfo *msgtype_info)
{
  msgtype_info->rset.hash_fn = wm_msg_prim_rset_hash;
  msgtype_info->rset.cmp_fn = wm_msg_prim_rset_cmp;
  msgtype_info->rset.key_free_fn = wm_msg_prim_rset_key_free;

  msgtype_info->repr = wm_msg_prim_repr;
  msgtype_info->update_by_id = wm_msg_prim_update_by_id;
  msgtype_info->remove_by_id = wm_msg_prim_remove_by_id;

  msgtype_info->msg_key_size = sizeof(wmMsgSubscribeKey_PRIM);
}

/* STATIC MSG BUS INIT */

typedef struct wmMsgParams_Static
{
  int event;
} wmMsgParams_Static;

typedef struct wmMsg_Static
{
  wmMsg head; /* keep first */
  wmMsgParams_Static params;
} wmMsg_Static;

typedef struct wmMsgSubscribeKey_Static
{
  wmMsgSubscribeKey head;
  wmMsg_Static msg;
} wmMsgSubscribeKey_Static;

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
  wmMsgSubscribeKey *key = static_cast<wmMsgSubscribeKey *>(key_p);
  std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_it = key->values.begin();
  std::vector<wmMsgSubscribeValueLink *>::const_iterator msg_end = key->values.end();

  for (; msg_it != msg_end; ++msg_it) {
    key->values.erase(msg_it);
    MEM_freeN((*msg_it));
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
          (int)m->head.values.size());
}

static void WM_msgtypeinfo_init_static(wmMsgTypeInfo *msgtype_info)
{
  msgtype_info->rset.hash_fn = wm_msg_static_rset_hash;
  msgtype_info->rset.cmp_fn = wm_msg_static_rset_cmp;
  msgtype_info->rset.key_free_fn = wm_msg_static_rset_key_free;
  msgtype_info->repr = wm_msg_static_repr;

  msgtype_info->msg_key_size = sizeof(wmMsgSubscribeKey_Static);
}

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

/**
 *  -----  The MsgBus Callback. ----- */


MsgBusCallback::MsgBusCallback(wmNotifier *notice) : ref(1), note(notice) {}

void MsgBusCallback::wmCOMM(const TfNotice &notice, MsgBus const &sender)
{
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("MsgBus\n");

  switch (note->category) {
    case (NC_WM):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: WindowManager\n");
      break;
    case (NC_WINDOW):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Window\n");
      break;
    case (NC_SCREEN):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Screen\n");
      break;
    case (NC_SCENE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Scene\n");
      break;
    case (NC_OBJECT):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Object\n");
      break;
    case (NC_MATERIAL):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Material\n");
      break;
    case (NC_TEXTURE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Texture\n");
      break;
    case (NC_LAMP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Lamp\n");
      break;
    case (NC_GROUP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Group\n");
      break;
    case (NC_IMAGE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Image\n");
      break;
    case (NC_BRUSH):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Brush\n");
      break;
    case (NC_TEXT):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Text\n");
      break;
    case (NC_WORLD):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: World\n");
      break;
    case (NC_ANIMATION):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Animation\n");
      break;
    case (NC_SPACE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Space\n");
      break;
    case (NC_GEOM):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Geom\n");
      break;
    case (NC_NODE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Node\n");
      break;
    case (NC_ID):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: ID\n");
      break;
    case (NC_PAINTCURVE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: PaintCurve\n");
      break;
    case (NC_MOVIECLIP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: MovieClip\n");
      break;
    case (NC_MASK):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Mask\n");
      break;
    case (NC_GPENCIL):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: GPencil\n");
      break;
    case (NC_LINESTYLE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: LineStyle\n");
      break;
    case (NC_CAMERA):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Camera\n");
      break;
    case (NC_LIGHTPROBE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: LightProbe\n");
      break;
    case (NC_ASSET):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Asset\n");
      break;
    default:
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Undefined\n");
      break;
  }
  ++ref;
}


void wmNotifier::Push()
{
  MsgBusCallback *cb = new MsgBusCallback(this);
  MsgBus invoker(cb);
  // TfNotice::Register(invoker, &MsgBusCallback::wmCOMM, invoker);
  // notice.Send(invoker);
}


MsgBusCallback::MsgBusCallback(wmOperatorType *ot) : ref(1), op({ot}) {}

void MsgBusCallback::OperatorCOMM(const TfNotice &notice, MsgBus const &sender)
{
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("MsgBus\n");
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Type: Operator\n");
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    ID: %s\n", CHARALL(op.type->idname));
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    Name: %s\n", op.type->name);
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    Description: %s\n", op.type->description);
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("\n");
  ++ref;
}

void WM_msg_subscribe_prim_params(struct wmMsgBus *mbus,
                                  const wmMsgParams_PRIM *msg_key_params,
                                  const wmMsgSubscribeValue *msg_val_params,
                                  const char *id_repr)
{
  wmMsgSubscribeKey_PRIM msg_key_test = {{NULL}};

  /* use when added */
  msg_key_test.msg.head.id = id_repr;
  msg_key_test.msg.head.type = WM_MSG_TYPE_RNA;
  /* for lookup */
  msg_key_test.msg.params = *msg_key_params;

  const char *none = "<none>";
  printf("prim(id='%s', %s.%s, info='%s')\n",
         msg_key_params->ptr.owner_id ?
           std::string(((ID *)msg_key_params->ptr.owner_id)->name).c_str() :
           none,
         msg_key_params->ptr.type ? msg_key_params->ptr.type->identifier.GetText() : none,
         msg_key_params->prop ? ((const IDProperty *)msg_key_params->prop)->name : none,
         id_repr);

  wmMsgSubscribeKey_PRIM *msg_key = (wmMsgSubscribeKey_PRIM *)
    WM_msg_subscribe_with_key(mbus, &msg_key_test.head, msg_val_params);

  if (msg_val_params->is_persistent) {
    if (msg_key->msg.params.data_path.IsEmpty()) {
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
    for (auto &msg_lnk : key->values) {
      if ((msg_lnk->params.notify == msg_val_params->notify) &&
          (msg_lnk->params.owner == msg_val_params->owner) &&
          (msg_lnk->params.user_data == msg_val_params->user_data)) {
        return key;
      }
    }
  }

  wmMsgSubscribeValueLink *msg_lnk = static_cast<wmMsgSubscribeValueLink *>(
    MEM_mallocN(sizeof(wmMsgSubscribeValueLink), __func__));
  msg_lnk->params = *msg_val_params;
  key->values.push_back(msg_lnk);
  return key;
}

void WM_msg_publish_with_key(struct wmMsgBus *mbus, wmMsgSubscribeKey *msg_key)
{
  printf("tagging subscribers: (ptr=%p, len=%d)\n", msg_key, (int)msg_key->values.size());

  for (auto &msg_lnk : msg_key->values) {
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
