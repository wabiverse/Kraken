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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#include "KKE_api.h"
#include "UNI_object.h"

#include <wabi/usd/usd/common.h>

WABI_NAMESPACE_BEGIN

typedef UsdStageRefPtr Stage;

struct ScrArea;
struct Main;
struct ARegion;
struct Scene;
struct kScreen;
struct UserDef;
struct wmWindowManager;
struct wmWindow;
struct WorkSpace;
struct kContext;

/**
 * Kraken Context:
 *  - Creation.
 *  - Destruction. */

kContext *CTX_create(void);
void CTX_free(kContext *C);

/**
 * Kraken Context Getters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - Stage data. */

Main *CTX_data_main(kContext *C);
wmWindowManager *CTX_wm_manager(kContext *C);
wmWindow *CTX_wm_window(kContext *C);
WorkSpace *CTX_wm_workspace(kContext *C);
kScreen *CTX_wm_screen(kContext *C);
ScrArea *CTX_wm_area(kContext *C);
ARegion *CTX_wm_region(kContext *C);
Scene *CTX_data_scene(kContext *C);
Stage CTX_data_stage(kContext *C);
UserDef *CTX_data_prefs(kContext *C);

/**
 * Kraken Context Setters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - Stage data. */

void CTX_data_main_set(kContext *C, Main *kmain);
void CTX_wm_manager_set(kContext *C, wmWindowManager *wm);
void CTX_wm_screen_set(kContext *C, kScreen *screen);
void CTX_wm_area_set(kContext *C, ScrArea *area);
void CTX_wm_region_set(kContext *C, ARegion *region);
void CTX_wm_menu_set(kContext *C, ARegion *menu);
void CTX_wm_window_set(kContext *C, wmWindow *win);
void CTX_data_scene_set(kContext *C, Scene *cscene);
void CTX_data_prefs_set(kContext *C, UserDef *uprefs);

void CTX_wm_operator_poll_msg_clear(kContext *C);
void CTX_wm_operator_poll_msg_set(kContext *C, const char *msg);

struct kContextDataResult
{
  PointerUNI ptr;
  std::vector<UniverseObject *> list;
  const char **dir;
  short type; /* 0: normal, 1: seq */
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

WABI_NAMESPACE_END