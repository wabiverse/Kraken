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

/**
 * Main CTX Creation. */
cContext CTX_create(void)
{
  TfAutoMallocTag2 tag("cContext", "CTX_create");

  return TfCreateRefPtr(new CovahContext());
}

/**
 * Main CTX Deletion. */
void CTX_free(const cContext &C)
{
  TfAutoMallocTag2 tag("cContext", "CTX_free");

  /**
   * CTX out - */

  C.~TfRefPtr();
}

/**
 * Getters. */

Main CTX_data_main(const cContext &C)
{
  return C->data.main;
}

wmWindowManager CTX_wm_manager(const cContext &C)
{
  return C->wm.manager;
}

wmWindow CTX_wm_window(const cContext &C)
{
  return C->wm.window;
}

WorkSpace CTX_wm_workspace(const cContext &C)
{
  return C->wm.workspace;
}

cScreen CTX_wm_screen(const cContext &C)
{
  return C->wm.screen;
}

ScrArea CTX_wm_area(const cContext &C)
{
  return C->wm.area;
}

ARegion CTX_wm_region(const cContext &C)
{
  return C->wm.region;
}

Scene CTX_data_scene(const cContext &C)
{
  return C->data.scene;
}

Stage CTX_data_stage(const cContext &C)
{
  return C->data.stage;
}

UserDef CTX_data_prefs(const cContext &C)
{
  return C->data.prefs;
}

/**
 * Setters. */

void CTX_data_main_set(const cContext &C, const Main &cmain)
{
  C->data.main = cmain;
}

void CTX_wm_manager_set(const cContext &C, const wmWindowManager &wm)
{
  C->wm.manager = wm;
  C->wm.window = NULL;
  C->wm.screen = NULL;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_window_set(const cContext &C, const wmWindow &win)
{
  C->wm.window = win;
  C->wm.workspace = win->prims.workspace;
  C->wm.screen = win->prims.screen;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_screen_set(const cContext &C, const cScreen &screen)
{
  C->wm.screen = screen;
  C->wm.area = NULL;
  C->wm.region = NULL;
}

void CTX_wm_area_set(const cContext &C, const ScrArea &area)
{
  C->wm.area = area;
  C->wm.region = NULL;
}

void CTX_wm_region_set(const cContext &C, const ARegion &region)
{
  C->wm.region = region;
}

void CTX_wm_menu_set(const cContext &C, const ARegion &menu)
{
  C->wm.menu = menu;
}

void CTX_data_scene_set(const cContext &C, const Scene &cscene)
{
  C->data.scene = cscene;
  C->data.stage = cscene->stage;
}

void CTX_data_prefs_set(const cContext &C, const UserDef &uprefs)
{
  C->data.prefs = uprefs;
}

/**
 * Operator Polls. */

void CTX_wm_operator_poll_msg_clear(const cContext &C)
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

void CTX_wm_operator_poll_msg_set(const cContext &C, const char *msg)
{
  CTX_wm_operator_poll_msg_clear(C);

  C->wm.operator_poll_msg = msg;
}

WABI_NAMESPACE_END