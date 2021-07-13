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

#include "KLI_threads.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_version.h"
#include "KKE_workspace.h"

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

struct kContext : public UniverseObject
{
  kContext() = default;

  int thread;

  /* windowmanager context */
  struct
  {
    wmWindowManager *manager;
    wmWindow *window;
    WorkSpace *workspace;
    kScreen *screen;
    ScrArea *area;
    ARegion *region;
    ARegion *menu;

    const char *operator_poll_msg;
    kContextPollMsgParams operator_poll_msg_params;
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

static void *ctx_wm_python_context_get(const kContext *C,
                                       const char *member,
                                       const void *member_type,
                                       void *fall_through)
{
  /**
   * TODO: Setup Python. */
  TF_UNUSED(C);
  TF_UNUSED(member);
  TF_UNUSED(member_type);

  /* don't allow UI context access from non-main threads */
  if (!KLI_thread_is_main()) {
    return NULL;
  }

  return fall_through;
}

/**
 * Main CTX Creation. */
kContext *CTX_create(void)
{
  TfAutoMallocTag2 tag("kContext", "CTX_create");

  return new kContext();
}

/**
 * Main CTX Deletion. */
void CTX_free(kContext *C)
{
  TfAutoMallocTag2 tag("kContext", "CTX_free");

  /**
   * CTX out - */

  delete C;
}

/**
 * Getters. */

Main *CTX_data_main(kContext *C)
{
  return C->data.main;
}

wmWindowManager *CTX_wm_manager(kContext *C)
{
  return C->wm.manager;
}

wmWindow *CTX_wm_window(kContext *C)
{
  UniverseObject MAELSTROM_Window;
  return (wmWindow*)ctx_wm_python_context_get(C, "window", &MAELSTROM_Window, C->wm.window);
}

WorkSpace *CTX_wm_workspace(kContext *C)
{
  UniverseObject MAELSTROM_WorkSpace;
  return (WorkSpace*)ctx_wm_python_context_get(C, "workspace", &MAELSTROM_WorkSpace, C->wm.workspace);
}

kScreen *CTX_wm_screen(kContext *C)
{
  UniverseObject MAELSTROM_Screen;
  return (kScreen*)ctx_wm_python_context_get(C, "screen", &MAELSTROM_Screen, C->wm.screen);
}

ScrArea *CTX_wm_area(kContext *C)
{
  UniverseObject MAELSTROM_Area;
  return (ScrArea*)ctx_wm_python_context_get(C, "area", &MAELSTROM_Area, C->wm.area);
}

ARegion *CTX_wm_region(kContext *C)
{
  UniverseObject MAELSTROM_Region;
  return (ARegion*)ctx_wm_python_context_get(C, "region", &MAELSTROM_Region, C->wm.region);
}

Scene *CTX_data_scene(kContext *C)
{
  return C->data.scene;
}

Stage CTX_data_stage(kContext *C)
{
  return C->data.stage;
}

UserDef *CTX_data_prefs(kContext *C)
{
  return C->data.prefs;
}

/**
 * Setters. */

void CTX_data_main_set(kContext *C, Main *kmain)
{
  C->data.main = kmain;
}

void CTX_wm_manager_set(kContext *C, wmWindowManager *wm)
{
  C->wm.manager = wm;
  C->wm.window = POINTER_ZERO;
  C->wm.screen = POINTER_ZERO;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;
}

void CTX_wm_window_set(kContext *C, wmWindow *win)
{
  C->wm.window = win;
  C->wm.workspace = (win) ? KKE_workspace_active_get(win->workspace_hook) : POINTER_ZERO;
  C->wm.screen = (win) ? KKE_workspace_active_screen_get(win->workspace_hook) : POINTER_ZERO;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;
}

void CTX_wm_screen_set(kContext *C, kScreen *screen)
{
  C->wm.screen = screen;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;
}

void CTX_wm_area_set(kContext *C, ScrArea *area)
{
  C->wm.area = area;
  C->wm.region = POINTER_ZERO;
}

void CTX_wm_region_set(kContext *C, ARegion *region)
{
  C->wm.region = region;
}

void CTX_wm_menu_set(kContext *C, ARegion *menu)
{
  C->wm.menu = menu;
}

void CTX_data_scene_set(kContext *C, Scene *cscene)
{
  C->data.scene = cscene;
  C->data.stage = cscene->stage;
}

void CTX_data_prefs_set(kContext *C, UserDef *uprefs)
{
  C->data.prefs = uprefs;
}

/**
 * Operator Polls. */

void CTX_wm_operator_poll_msg_clear(kContext *C)
{
  kContextPollMsgParams *params = &C->wm.operator_poll_msg_params;
  if (params->free_fn != NULL)
  {
    params->free_fn(C, params->user_data);
  }
  params->get_fn = NULL;
  params->free_fn = NULL;
  params->user_data = NULL;

  C->wm.operator_poll_msg = NULL;
}

void CTX_wm_operator_poll_msg_set(kContext *C, const char *msg)
{
  CTX_wm_operator_poll_msg_clear(C);

  C->wm.operator_poll_msg = msg;
}

WABI_NAMESPACE_END