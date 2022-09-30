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

#pragma once

#include <wabi/base/tf/notice.h>
#include <wabi/base/tf/refBase.h>
#include <wabi/usd/usd/common.h>

#include <atomic>

#include "USD_window.h"

#include "WM_api.h"
#include "WM_operators.h"

#include "KKE_context.h"
#include "KKE_robinhood.h"

/**
 *  -----  The Kraken WindowManager. ----- */


KRAKEN_NAMESPACE_BEGIN


/* ------ */

struct wmMsgParams_PRIM
{
  /** when #PointerRNA.data & owner_id are NULL. match against all. */
  struct KrakenPRIM ptr;
  /** when NULL, match against any property. */
  const KrakenPROP *prop;

  /**
   * Optional RNA data path for persistent RNA properties, ignore if NULL.
   * otherwise it's allocated.
   */
  TfToken data_path;
};

struct wmMsgTypeInfo {
  struct {
    unsigned int (*hash_fn)(const void *msg);
    bool (*cmp_fn)(const void *a, const void *b);
    void (*key_free_fn)(void *key);
  } gset;

  void (*update_by_id)(struct wmMsgBus *mbus, struct ID *id_src, struct ID *id_dst);
  void (*remove_by_id)(struct wmMsgBus *mbus, const struct ID *id);
  void (*repr)(FILE *stream, const struct wmMsgSubscribeKey *msg_key);

  /* sizeof(wmMsgSubscribeKey_*) */
  uint msg_key_size;
};

struct wmMsg {
  unsigned int type;
  // #ifdef DEBUG
  /* For debugging: '__func__:__LINE__'. */
  const char *id;
  // #endif
};

struct wmMsg_PRIM {
  wmMsg head; /* keep first */
  wmMsgParams_PRIM params;
};

typedef void (*wmMsgNotifyFn)(struct kContext *C,
                              struct wmMsgSubscribeKey *msg_key,
                              struct wmMsgSubscribeValue *msg_val);
typedef void (*wmMsgSubscribeValueFreeDataFn)(struct wmMsgSubscribeKey *msg_key,
                                              struct wmMsgSubscribeValue *msg_val);

/* Exactly what arguments here is not obvious. */
typedef void (*wmMsgSubscribeValueUpdateIdFn)(struct kContext *C,
                                              struct wmMsgBus *mbus,
                                              struct ID *id_src,
                                              struct ID *id_dst,
                                              struct wmMsgSubscribeValue *msg_val);

/** One of many in #wmMsgSubscribeKey.values */
struct wmMsgSubscribeValue {
  struct wmMsgSubscribe *next, *prev;

  /** Handle, used to iterate and clear. */
  void *owner;
  /** User data, can be whatever we like, free using the 'free_data' callback if it's owned. */
  void *user_data;

  /** Callbacks */
  wmMsgNotifyFn notify;
  wmMsgSubscribeValueUpdateIdFn update_id;
  wmMsgSubscribeValueFreeDataFn free_data;

  /** Keep this subscriber if possible. */
  uint is_persistent : 1;
  /* tag to run when handling events,
   * we may want option for immediate execution. */
  uint tag : 1;
};

/** One of many in #wmMsgSubscribeKey.values */
struct wmMsgSubscribeValueLink {
  struct wmMsgSubscribeValueLink *next, *prev;
  wmMsgSubscribeValue params;
};

struct wmMsgSubscribeKey {
  /** Linked list for predictable ordering, otherwise we would depend on #GHash bucketing. */
  struct wmMsgSubscribeKey *next, *prev;
  std::vector<wmMsgSubscribeValueLink *> values;
  /* over-alloc, eg: wmMsgSubscribeKey_RNA */
  /* Last member will be 'wmMsg_*' */
};

struct wmMsgSubscribeKey_PRIM {
  wmMsgSubscribeKey head;
  wmMsg_PRIM msg;
};

wmMsgSubscribeKey *WM_msg_subscribe_with_key(struct wmMsgBus *mbus,
                                             const wmMsgSubscribeKey *msg_key_test,
                                             const wmMsgSubscribeValue *msg_val_params);

void WM_msg_publish_with_key(struct wmMsgBus *mbus, wmMsgSubscribeKey *msg_key);

/**
 *  -----  Forward Declarations. ----- */

TF_DECLARE_WEAK_AND_REF_PTRS(MsgBusCallback);

typedef MsgBusCallbackPtr MsgBus;

/**
 *  -----  The MsgBus Callback. ----- */


struct MsgBusCallback : public TfWeakBase
{
  /** Kraken WM Notifications. */
  MsgBusCallback(wmNotifier *note);
  void wmCOMM(const TfNotice &notice, MsgBus const &sender);

  /** Kraken Operators. */
  MsgBusCallback(wmOperatorType *ot);
  void OperatorCOMM(const TfNotice &notice, MsgBus const &sender);

  /** Reference Count. */
  std::atomic<int> ref;

  /** Notify @ Subscribe MsgBus. */
  // TfNotice notice;

  wmNotifier *note;

  struct
  {
    wmOperatorType *type;
  } op;
};

struct wmMsgSubscribeKey_Generic {
  wmMsgSubscribeKey head;
  wmMsg msg;
};

void WM_msg_subscribe_prim(struct wmMsgBus *mbus,
                           KrakenPRIM *ptr,
                           const KrakenPROP *prop,
                           const wmMsgSubscribeValue *msg_val_params,
                           const char *id_repr);

KLI_INLINE const wmMsg *wm_msg_subscribe_value_msg_cast(const wmMsgSubscribeKey *key)
{
  return &((wmMsgSubscribeKey_Generic *)key)->msg;
}
KLI_INLINE wmMsg *wm_msg_subscribe_value_msg_cast_mut(wmMsgSubscribeKey *key)
{
  return &((wmMsgSubscribeKey_Generic *)key)->msg;
}

/* ------ */

KRAKEN_NAMESPACE_END