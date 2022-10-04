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
#include "WM_window.h"

#include "KLI_rhash.h"
#include "KLI_listbase.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"

#include <mutex>
#include <string>
#include <vector>

/* to replace string chars. */
#include <boost/algorithm/string/replace.hpp>


/**
 *  -----  The Kraken WindowManager. ----- */





/* ------ */

static wmMsgTypeInfo wm_msg_types[WM_MSG_TYPE_NUM] = {{{NULL}}};

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
  printf("rna(id='%s', %s.%s, info='%s')\n",
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

