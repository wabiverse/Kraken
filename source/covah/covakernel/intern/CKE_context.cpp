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
 * COVAH Kernel.
 * Purple Underground.
 */

#include "CKE_context.h"
#include "CKE_main.h"
#include "CKE_version.h"

#include "UNI_area.h"
#include "UNI_object.h"
#include "UNI_region.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_system.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "WM_operators.h"

#include <wabi/base/tf/mallocTag.h>
#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

struct cContext : public CovahObject
{
  cContext() = default;

  int thread;

  /* windowmanager context */
  struct
  {
    wmWindowManager *manager;
    wmWindow *window;
    WorkSpace *workspace;
    cScreen *screen;
    ScrArea *area;
    ARegion *region;
    ARegion *menu;

    const char *operator_poll_msg;
    cContextPollMsgParams operator_poll_msg_params;
  } wm;

  /* data context */
  struct
  {
    Main *main;
    Scene *scene;
    Stage stage;
    UserDef *prefs;
  } data;
};

/**
 * Main CTX Creation. */
cContext *CTX_create(void)
{
  TfAutoMallocTag2 tag("cContext", "CTX_create");

  return new cContext();
}

/**
 * Main CTX Deletion. */
void CTX_free(cContext *C)
{
  TfAutoMallocTag2 tag("cContext", "CTX_free");

  /**
   * CTX out - */

  delete C;
}

/**
 * Getters. */

Main *CTX_data_main(cContext *C)
{
  return C->data.main;
}

wmWindowManager *CTX_wm_manager(cContext *C)
{
  return C->wm.manager;
}

wmWindow *CTX_wm_window(cContext *C)
{
  return C->wm.window;
}

WorkSpace *CTX_wm_workspace(cContext *C)
{
  return C->wm.workspace;
}

cScreen *CTX_wm_screen(cContext *C)
{
  return C->wm.screen;
}

ScrArea *CTX_wm_area(cContext *C)
{
  return C->wm.area;
}

ARegion *CTX_wm_region(cContext *C)
{
  return C->wm.region;
}

Scene *CTX_data_scene(cContext *C)
{
  return C->data.scene;
}

Stage CTX_data_stage(cContext *C)
{
  return C->data.stage;
}

UserDef *CTX_data_prefs(cContext *C)
{
  return C->data.prefs;
}

/**
 * Setters. */

void CTX_data_main_set(cContext *C, Main *cmain)
{
  C->data.main = cmain;
}

void CTX_wm_manager_set(cContext *C, wmWindowManager *wm)
{
  C->wm.manager = wm;
  C->wm.window = NULL;
  C->wm.screen = NULL;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_window_set(cContext *C, wmWindow *win)
{
  C->wm.window = win;
  C->wm.workspace = win->prims.workspace;
  C->wm.screen = win->prims.screen;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_screen_set(cContext *C, cScreen *screen)
{
  C->wm.screen = screen;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_area_set(cContext *C, ScrArea *area)
{
  C->wm.area = area;
  C->wm.region = NULL;
}

void CTX_wm_region_set(cContext *C, ARegion *region)
{
  C->wm.region = region;
}

void CTX_wm_menu_set(cContext *C, ARegion *menu)
{
  C->wm.menu = menu;
}

void CTX_data_scene_set(cContext *C, Scene *cscene)
{
  C->data.scene = cscene;
  C->data.stage = cscene->stage;
}

void CTX_data_prefs_set(cContext *C, UserDef *uprefs)
{
  C->data.prefs = uprefs;
}

/**
 * Operator Polls. */

void CTX_wm_operator_poll_msg_clear(cContext *C)
{
  cContextPollMsgParams *params = &C->wm.operator_poll_msg_params;
  if (params->free_fn != NULL)
  {
    params->free_fn(C, params->user_data);
  }
  params->get_fn = NULL;
  params->free_fn = NULL;
  params->user_data = NULL;

  C->wm.operator_poll_msg = NULL;
}

void CTX_wm_operator_poll_msg_set(cContext *C, const char *msg)
{
  CTX_wm_operator_poll_msg_clear(C);

  C->wm.operator_poll_msg = msg;
}

WABI_NAMESPACE_END