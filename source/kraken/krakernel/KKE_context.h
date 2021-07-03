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
struct cScreen;
struct UserDef;
struct wmWindowManager;
struct wmWindow;
struct WorkSpace;
struct cContext;

/**
 * Kraken Context:
 *  - Creation.
 *  - Destruction. */

cContext *CTX_create(void);
void CTX_free(cContext *C);

/**
 * Kraken Context Getters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - Stage data. */

Main *CTX_data_main(cContext *C);
wmWindowManager *CTX_wm_manager(cContext *C);
wmWindow *CTX_wm_window(cContext *C);
WorkSpace *CTX_wm_workspace(cContext *C);
cScreen *CTX_wm_screen(cContext *C);
ScrArea *CTX_wm_area(cContext *C);
ARegion *CTX_wm_region(cContext *C);
Scene *CTX_data_scene(cContext *C);
Stage CTX_data_stage(cContext *C);
UserDef *CTX_data_prefs(cContext *C);

/**
 * Kraken Context Setters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data.
 *  - Stage data. */

void CTX_data_main_set(cContext *C, Main *cmain);
void CTX_wm_manager_set(cContext *C, wmWindowManager *wm);
void CTX_wm_screen_set(cContext *C, cScreen *screen);
void CTX_wm_area_set(cContext *C, ScrArea *area);
void CTX_wm_region_set(cContext *C, ARegion *region);
void CTX_wm_menu_set(cContext *C, ARegion *menu);
void CTX_wm_window_set(cContext *C, wmWindow *win);
void CTX_data_scene_set(cContext *C, Scene *cscene);
void CTX_data_prefs_set(cContext *C, UserDef *uprefs);

void CTX_wm_operator_poll_msg_clear(cContext *C);
void CTX_wm_operator_poll_msg_set(cContext *C, const char *msg);

struct cContextDataResult
{
  PointerUNI ptr;
  std::vector<UniverseObject *> list;
  const char **dir;
  short type; /* 0: normal, 1: seq */
};

typedef int (*cContextDataCallback)(const cContext *C,
                                    const char *member,
                                    cContextDataResult *result);

struct cContextPollMsgParams
{
  char *(*get_fn)(cContext *C, void *user_data);
  void (*free_fn)(cContext *C, void *user_data);
  void *user_data;
};

WABI_NAMESPACE_END