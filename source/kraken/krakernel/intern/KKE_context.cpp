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

#include "KLI_threads.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_version.h"
#include "KKE_workspace.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"

#include "USD_area.h"
#include "USD_object.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_system.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "WM_operators.h"

#include "kpy/KPY_extern_python.h"

#include <wabi/base/tf/mallocTag.h>
#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

struct kContext
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

    bool py_init;
    void *py_context;
  } data;
};

struct kContextDataResult
{
  KrakenPRIM *ptr;
  std::vector<const KrakenPRIM *> list;
  const char **dir;
  short type; /* 0: normal, 1: seq */
};

void *CTX_py_dict_get(const kContext *C)
{
  return C->data.py_context;
}

void CTX_data_pointer_set_ptr(kContextDataResult *result, const KrakenPRIM *ptr)
{
  result->ptr->data = ptr->data;
}

void CTX_data_list_add_ptr(kContextDataResult *result, const KrakenPRIM *ptr)
{
  result->list.push_back(ptr);
}

void CTX_data_type_set(kContextDataResult *result, short type)
{
  result->type = type;
}

static void *ctx_wm_python_context_get(const kContext *C,
                                       const char *member,
                                       KrakenPRIM *member_type,
                                       void *fall_through)
{
#ifdef WITH_PYTHON
  if (ARCH_UNLIKELY(C && CTX_py_dict_get(C))) {
    kContextDataResult result;
    memset(&result, 0, sizeof(kContextDataResult));
    KPY_context_member_get((kContext *)C, member, &result);

    if (result.ptr->data) {
      if (LUXO_struct_is_a(result.ptr, member_type)) {
        return result.ptr->data;
      }
    }
  }
#else
  UNUSED_VARS(C, member, member_type);
#endif

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

bool CTX_py_init_get(kContext *C)
{
  return C->data.py_init;
}

void CTX_py_init_set(kContext *C, bool value)
{
  C->data.py_init = value;

  if (value == true) {
    TF_MSG_SUCCESS("The python runtime has been initialized.");
  }
}

/**
 * Getters. */

Main *CTX_data_main(const kContext *C)
{
  return C->data.main;
}

wmWindowManager *CTX_wm_manager(const kContext *C)
{
  return C->wm.manager;
}

wmWindow *CTX_wm_window(const kContext *C)
{
  return (
    wmWindow *)ctx_wm_python_context_get(C, "window", (KrakenPRIM *)&LUXO_Window, C->wm.window);
}

WorkSpace *CTX_wm_workspace(const kContext *C)
{
  return (WorkSpace *)
    ctx_wm_python_context_get(C, "workspace", (KrakenPRIM *)&LUXO_WorkSpace, C->wm.workspace);
}

kScreen *CTX_wm_screen(const kContext *C)
{
  return (
    kScreen *)ctx_wm_python_context_get(C, "screen", (KrakenPRIM *)&LUXO_Screen, C->wm.screen);
}

ScrArea *CTX_wm_area(const kContext *C)
{
  return (ScrArea *)ctx_wm_python_context_get(C, "area", (KrakenPRIM *)&LUXO_Area, C->wm.area);
}

ARegion *CTX_wm_region(const kContext *C)
{
  return (
    ARegion *)ctx_wm_python_context_get(C, "region", (KrakenPRIM *)&LUXO_Region, C->wm.region);
}

ARegion *CTX_wm_menu(const kContext *C)
{
  return C->wm.menu;
}

Scene *CTX_data_scene(const kContext *C)
{
  return C->data.scene;
}

Stage CTX_data_stage(const kContext *C)
{
  return C->data.stage;
}

UserDef *CTX_data_prefs(const kContext *C)
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
  if (params->free_fn != NULL) {
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