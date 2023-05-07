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
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_listBase.h"
#include <stdio.h>

#ifdef __cplusplus
/* For TfToken. */
# include <wabi/base/tf/token.h>
#endif /* __cplusplus */

/* ------ */

struct ID;
struct kContext;
struct wmMsg;

struct wmMsgBus;
struct wmMsgSubscribeKey;
struct wmMsgSubscribeValue;
struct wmMsgSubscribeValueLink;

/* ------ */

#ifdef __cplusplus
extern "C" {
#endif

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

enum
{
  WM_MSG_TYPE_PRIM = 0,
  WM_MSG_TYPE_STATIC = 1,
};
#define WM_MSG_TYPE_NUM 2

typedef struct wmMsgTypeInfo
{
  struct
  {
    unsigned int (*hash_fn)(const void *msg);
    bool (*cmp_fn)(const void *a, const void *b);
    void (*key_free_fn)(void *key);
  } rset;

  void (*update_by_id)(struct wmMsgBus *mbus, struct ID *id_src, struct ID *id_dst);
  void (*remove_by_id)(struct wmMsgBus *mbus, const struct ID *id);
  void (*repr)(FILE *stream, const struct wmMsgSubscribeKey *msg_key);

  /* sizeof(wmMsgSubscribeKey_*) */
  uint msg_key_size;
} wmMsgTypeInfo;

typedef struct wmMsg
{
  unsigned int type;
  // #ifdef DEBUG
  /* For debugging: '__func__:__LINE__'. */
  const char *id;
  // #endif
} wmMsg;

typedef struct wmMsgSubscribeKey
{
  /** Linked list for predictable ordering, otherwise we would depend on #GHash bucketing. */
  struct wmMsgSubscribeKey *next, *prev;
  ListBase values;
  /* over-alloc, eg: wmMsgSubscribeKey_RNA */
  /* Last member will be 'wmMsg_*' */
} wmMsgSubscribeKey;

/** One of many in #wmMsgSubscribeKey.values */
typedef struct wmMsgSubscribeValue
{
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
} wmMsgSubscribeValue;

/** One of many in #wmMsgSubscribeKey.values */
typedef struct wmMsgSubscribeValueLink
{
  struct wmMsgSubscribeValueLink *next, *prev;
  wmMsgSubscribeValue params;
} wmMsgSubscribeValueLink;

void WM_msgbus_types_init(void);

struct wmMsgBus *WM_msgbus_create(void);
void WM_msgbus_destroy(struct wmMsgBus *mbus);

void WM_msgbus_clear_by_owner(struct wmMsgBus *mbus, void *owner);

void WM_msg_dump(struct wmMsgBus *mbus, const char *info_str);
void WM_msgbus_handle(struct wmMsgBus *mbus, struct kContext *C);

void WM_msg_publish_with_key(struct wmMsgBus *mbus, wmMsgSubscribeKey *msg_key);
/**
 * @param msg_key_test: Needs following #wmMsgSubscribeKey fields filled in:
 * - `msg.params`
 * - `msg.head.type`
 * - `msg.head.id`
 * .. other values should be zeroed.
 *
 * @return The key for this subscription.
 * note that this is only needed in rare cases when the key needs further manipulation.
 */
wmMsgSubscribeKey *WM_msg_subscribe_with_key(struct wmMsgBus *mbus,
                                             const wmMsgSubscribeKey *msg_key_test,
                                             const wmMsgSubscribeValue *msg_val_params);

void WM_msg_id_update(struct wmMsgBus *mbus, struct ID *id_src, struct ID *id_dst);
void WM_msg_id_remove(struct wmMsgBus *mbus, const struct ID *id);

enum
{
  /* generic window redraw */
  WM_MSG_STATICTYPE_WINDOW_DRAW = 0,
  WM_MSG_STATICTYPE_SCREEN_EDIT = 1,
  WM_MSG_STATICTYPE_FILE_READ = 2,
};

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

void WM_msgtypeinfo_init_prim(wmMsgTypeInfo *msgtype_info);
void WM_msgtypeinfo_init_static(wmMsgTypeInfo *msgtype_info);

wmMsgSubscribeKey_Static *WM_msg_lookup_static(struct wmMsgBus *mbus,
                                               const wmMsgParams_Static *msg_key_params);
void WM_msg_publish_static_params(struct wmMsgBus *mbus, const wmMsgParams_Static *msg_key_params);
void WM_msg_publish_static(struct wmMsgBus *mbus,
                           /* wmMsgParams_Static (expanded) */
                           int event);
void WM_msg_subscribe_static_params(struct wmMsgBus *mbus,
                                    const wmMsgParams_Static *msg_key_params,
                                    const wmMsgSubscribeValue *msg_val_params,
                                    const char *id_repr);
void WM_msg_subscribe_static(struct wmMsgBus *mbus,
                             int event,
                             const wmMsgSubscribeValue *msg_val_params,
                             const char *id_repr);

typedef struct wmMsgParams_PRIM
{
  /** when #KrakenPRIM.data & owner_id are NULL. match against all. */
  struct KrakenPRIM ptr;
  /** when NULL, match against any property. */
  const KrakenPROP *prop;

  /**
   * Optional Prim data path for persistent Prim properties, ignore if NULL.
   * otherwise it's allocated.
   */
  wabi::TfToken data_path;
} wmMsgParams_PRIM;

typedef struct wmMsg_PRIM
{
  wmMsg head; /* keep first */
  wmMsgParams_PRIM params;
} wmMsg_PRIM;

typedef struct wmMsgSubscribeKey_PRIM
{
  wmMsgSubscribeKey head;
  wmMsg_PRIM msg;
} wmMsgSubscribeKey_PRIM;

void WM_msg_publish_prim_params(struct wmMsgBus *mbus, const wmMsgParams_PRIM *msg_key_params);
void WM_msg_publish_prim(struct wmMsgBus *mbus,
                        /* wmMsgParams_RNA (expanded) */
                        KrakenPRIM *ptr,
                        KrakenPROP *prop);

wmMsgSubscribeKey *WM_msg_subscribe_with_key(struct wmMsgBus *mbus,
                                             const wmMsgSubscribeKey *msg_key_test,
                                             const wmMsgSubscribeValue *msg_val_params);


void WM_msg_subscribe_prim(struct wmMsgBus *mbus,
                           KrakenPRIM *ptr,
                           const KrakenPROP *prop,
                           const wmMsgSubscribeValue *msg_val_params,
                           const char *id_repr);

/* ------ */

void WM_msg_subscribe_value_free(wmMsgSubscribeKey *msg_key, wmMsgSubscribeValueLink *msg_lnk);

/* ------ */

typedef struct wmMsgSubscribeKey_Generic
{
  wmMsgSubscribeKey head;
  wmMsg msg;
} wmMsgSubscribeKey_Generic;

KLI_INLINE const wmMsg *wm_msg_subscribe_value_msg_cast(const wmMsgSubscribeKey *key)
{
  return &((wmMsgSubscribeKey_Generic *)key)->msg;
}
KLI_INLINE wmMsg *wm_msg_subscribe_value_msg_cast_mut(wmMsgSubscribeKey *key)
{
  return &((wmMsgSubscribeKey_Generic *)key)->msg;
}

#ifdef __cplusplus
}
#endif
