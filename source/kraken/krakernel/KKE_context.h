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

#pragma once

#include <kraken/kraken.h>
#include "KKE_api.h"

#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usd/common.h>

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

kContext *CTX_create(void);
void CTX_free(kContext *C);


/* store */

struct kContextStore *CTX_store_add(std::vector<struct kContextStore *> contexts,
                                    const char *name,
                                    const KrakenPRIM *ptr);
struct kContextStore *CTX_store_add_all(std::vector<struct kContextStore *> contexts,
                                        struct kContextStore *context);
struct kContextStore *CTX_store_get(kContext *C);
void CTX_store_set(kContext *C, struct kContextStore *store);
const KrakenPRIM *CTX_store_ptr_lookup(const struct kContextStore *store,
                                              const char *name,
                                              const KrakenPRIM *type);
struct kContextStore *CTX_store_copy(struct kContextStore *store);
void CTX_store_free(struct kContextStore *store);
void CTX_store_free_list(const std::vector<kContextStore*> &contexts);

/**
 * need to store if python
 * is initialized or not */
bool CTX_py_init_get(kContext *C);
void CTX_py_init_set(kContext *C, bool value);
void *CTX_py_dict_get(const kContext *C);
void CTX_data_pointer_set_ptr(kContextDataResult *result, const KrakenPRIM *ptr);
void CTX_data_type_set(kContextDataResult *result, short type);
void CTX_data_list_add_ptr(kContextDataResult *result, const KrakenPRIM *ptr);

/**
 * Kraken Context Getters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - KrakenSTAGE data. */

Main *CTX_data_main(const kContext *C);
wmWindowManager *CTX_wm_manager(const kContext *C);
wmWindow *CTX_wm_window(const kContext *C);
WorkSpace *CTX_wm_workspace(const kContext *C);
kScreen *CTX_wm_screen(const kContext *C);
ScrArea *CTX_wm_area(const kContext *C);
ARegion *CTX_wm_region(const kContext *C);
ARegion *CTX_wm_menu(const kContext *C);
kScene *CTX_data_scene(const kContext *C);
KrakenSTAGE CTX_data_stage(const kContext *C);
kUserDef *CTX_data_prefs(const kContext *C);
ReportList *CTX_wm_reports(const kContext *C);

/**
 * Kraken Context Setters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - KrakenSTAGE data. */

void CTX_data_main_set(kContext *C, Main *kmain);
void CTX_wm_manager_set(kContext *C, wmWindowManager *wm);
void CTX_wm_screen_set(kContext *C, kScreen *screen);
void CTX_wm_area_set(kContext *C, ScrArea *area);
void CTX_wm_region_set(kContext *C, ARegion *region);
void CTX_wm_menu_set(kContext *C, ARegion *menu);
void CTX_wm_window_set(kContext *C, wmWindow *win);
void CTX_data_scene_set(kContext *C, kScene *cscene);
void CTX_data_prefs_set(kContext *C, kUserDef *uprefs);

void CTX_wm_operator_poll_msg_clear(kContext *C);
void CTX_wm_operator_poll_msg_set(kContext *C, const char *msg);

struct kContextStoreEntry
{
  wabi::TfToken name;
  KrakenPRIM *ptr;
};

struct kContextStore
{
  std::vector<struct kContextStoreEntry *> entries;
  bool used;
};

typedef int (*kContextDataCallback)(const kContext *C,
                                    const char *member,
                                    kContextDataResult *result);

struct kContextPollMsgParams
{
  char *(*get_fn)(kContext *C, void *user_data);
  void (*free_fn)(kContext *C, void *user_data);
  void *user_data;
};

