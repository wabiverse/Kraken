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

#include "KLI_utildefines.h"

#include "USD_defs.h"
#include "USD_listBase.h"

/* kraken fwd. */
struct uiStyle;
struct Scene;
struct kScene;
struct ARegion;
struct kContext;
struct kContextDataResult;
struct KrakenPRIM;
struct KrakenSTAGE;
struct kScreen;
struct ScrArea;
struct Main;
struct ReportList;
struct uiBlock;
struct kUserDef;
struct wmMsgBus;
struct wmNotifier;
struct wmSpaceTypeListenerParams;
struct wmWindowManager;
struct wmWindow;
struct WorkSpace;
struct WorkSpaceInstanceHook;

enum
{
  CTX_DATA_TYPE_POINTER = 0,
  CTX_DATA_TYPE_COLLECTION,
};

/**
 * Kraken Context:
 *  - Creation.
 *  - Destruction. */

struct kContext *CTX_create(void);
void CTX_free(struct kContext *C);


/* store */

struct kContextStore *CTX_store_add(ListBase *contexts,
                                    const char *name,
                                    const struct KrakenPRIM *ptr);
struct kContextStore *CTX_store_add_all(ListBase * contexts,
                                        struct kContextStore *context);
struct kContextStore *CTX_store_get(struct kContext *C);
void CTX_store_set(struct kContext *C, struct kContextStore *store);
const struct KrakenPRIM *CTX_store_ptr_lookup(const struct kContextStore *store,
                                       const char *name,
                                       const struct KrakenPRIM *type);
struct kContextStore *CTX_store_copy(struct kContextStore *store);
void CTX_store_free(struct kContextStore *store);
void CTX_store_free_list(ListBase *contexts);

/**
 * need to store if python
 * is initialized or not */
bool CTX_py_init_get(struct kContext *C);
void CTX_py_init_set(struct kContext *C, bool value);
void *CTX_py_dict_get(const struct kContext *C);
void CTX_data_pointer_set_ptr(struct kContextDataResult *result, const struct KrakenPRIM *ptr);
void CTX_data_type_set(struct kContextDataResult *result, short type);
void CTX_data_list_add_ptr(struct kContextDataResult *result, const struct KrakenPRIM *ptr);

/**
 * Kraken Context Getters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - KrakenSTAGE data. */

struct Main *CTX_data_main(const struct kContext *C);
struct wmWindowManager *CTX_wm_manager(const struct kContext *C);
struct wmWindow *CTX_wm_window(const struct kContext *C);
struct WorkSpace *CTX_wm_workspace(const struct kContext *C);
struct kScreen *CTX_wm_screen(const struct kContext *C);
struct ScrArea *CTX_wm_area(const struct kContext *C);
struct ARegion *CTX_wm_region(const struct kContext *C);
struct ARegion *CTX_wm_menu(const struct kContext *C);
struct kScene *CTX_data_scene(const struct kContext *C);
struct KrakenSTAGE CTX_data_stage(const struct kContext *C);
struct kUserDef *CTX_data_prefs(const struct kContext *C);
struct ReportList *CTX_wm_reports(const struct kContext *C);

/**
 * Kraken Context Setters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - KrakenSTAGE data. */

void CTX_data_main_set(struct kContext *C, struct Main *kmain);
void CTX_wm_manager_set(struct kContext *C, struct wmWindowManager *wm);
void CTX_wm_screen_set(struct kContext *C, struct kScreen *screen);
void CTX_wm_area_set(struct kContext *C, struct ScrArea *area);
void CTX_wm_region_set(struct kContext *C, struct ARegion *region);
void CTX_wm_menu_set(struct kContext *C, struct ARegion *menu);
void CTX_wm_window_set(struct kContext *C, struct wmWindow *win);
void CTX_data_scene_set(struct kContext *C, struct kScene *cscene);
void CTX_data_prefs_set(struct kContext *C, struct kUserDef *uprefs);

void CTX_wm_operator_poll_msg_clear(struct kContext *C);
void CTX_wm_operator_poll_msg_set(struct kContext *C, const char *msg);

struct kContextStoreEntry
{
  char name[MAX_NAME];
  struct KrakenPRIM *ptr;
};

struct kContextStore
{
  ListBase entries;
  bool used;
};

typedef int (*kContextDataCallback)(const struct kContext *C,
                                    const char *member,
                                    struct kContextDataResult *result);

struct kContextPollMsgParams
{
  char *(*get_fn)(struct kContext *C, void *user_data);
  void (*free_fn)(struct kContext *C, void *user_data);
  void *user_data;
};
