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

#include "kraken/kraken.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "MEM_guardedalloc.h"

#include "KLI_listbase.h"
#include "KLI_string.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_scene.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"
#include "LUXO_types.h"

#include "USD_area.h"
#include "USD_object.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_system.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "WM_operators.h"

#include <wabi/base/tf/mallocTag.h>
#include <wabi/usd/usd/attribute.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usd/common.h>

#include "LUXO_access.h"

#include "CLG_log.h"

#ifdef WITH_PYTHON
#  include "KPY_extern.h"
#  include "KPY_extern_python.h"
#endif

static CLG_LogRef LOG = {"kke.context"};

struct kContext
{
  /** @note default ctor. */
  kContext() = default;

  /** @note copy ctor. */
  kContext(const kContext &other)
  {
    thread = other.thread;

    wm = other.wm;
    wm.manager = other.wm.manager;
    wm.window = other.wm.window;
    wm.workspace = other.wm.workspace;
    wm.screen = other.wm.screen;
    wm.area = other.wm.area;
    wm.region = other.wm.region;
    wm.menu = other.wm.menu;
    wm.store = other.wm.store;
    wm.operator_poll_msg = other.wm.operator_poll_msg;
    wm.operator_poll_msg_params = other.wm.operator_poll_msg_params;

    data = other.data;
    data.main = other.data.main;
    data.scene = other.data.scene;
    data.prefs = other.data.prefs;
    data.recursion = other.data.recursion;
    data.py_init = other.data.py_init;
    data.py_context = other.data.py_context;
    data.py_context_orig = other.data.py_context_orig;
  }

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
    kContextStore *store;

    const char *operator_poll_msg;
    kContextPollMsgParams operator_poll_msg_params;
  } wm;

  /* data context */
  struct
  {
    Main *main;
    kScene *scene;
    KrakenSTAGE stage;
    kUserDef *prefs;

    int recursion;

    /** True if python is initialized. */
    bool py_init;
    void *py_context;
    /**
     * If we need to remove members, do so in a copy
     * (keep this to check if the copy needs freeing).
     */
    void *py_context_orig;
  } data;
};

struct kContextDataResult
{
  KrakenPRIM ptr;
  ListBase list;
  const char **dir;
  short type; /* 0: normal, 1: seq */
};

static void *ctx_wm_python_context_get(const kContext *C,
                                       const char *member,
                                       KrakenPRIM *member_type,
                                       void *fall_through)
{
#ifdef WITH_PYTHON
  if (UNLIKELY(C && CTX_py_dict_get(C))) {
    kContextDataResult result;
    memset(&result, 0, sizeof(kContextDataResult));
    KPY_context_member_get((kContext *)C, member, &result);

    if (result.ptr.data) {
      if (LUXO_prim_is_a(result.ptr.type, member_type)) {
        return result.ptr.data;
      }

      CLOG_WARN(&LOG,
                "PyContext '%s' is a '%s', expected a '%s'",
                member,
                LUXO_prim_identifier(result.ptr.type).data(),
                LUXO_prim_identifier(member_type).data());
    }
  }
#else
  UNUSED_VARS(C, member, member_type);
#endif

  /* don't allow UI context access from non-main threads */
  if (!KLI_thread_is_main()) {
    return nullptr;
  }

  return fall_through;
}

static eContextResult ctx_data_get(kContext *C, const char *member, kContextDataResult *result)
{
  kScreen *screen;
  ScrArea *area;
  ARegion *region;
  int done = 0, recursion = C->data.recursion;
  int ret = 0;

  memset(result, 0, sizeof(kContextDataResult));
#ifdef WITH_PYTHON
  if (CTX_py_dict_get(C)) {
    if (KPY_context_member_get(C, member, result)) {
      return static_cast<eContextResult>(1);
    }
  }
#endif

  /* don't allow UI context access from non-main threads */
  if (!KLI_thread_is_main()) {
    return static_cast<eContextResult>(done);
  }

  /* we check recursion to ensure that we do not get infinite
   * loops requesting data from ourselves in a context callback */

  /* Ok, this looks evil...
   * if (ret) done = -(-ret | -done);
   *
   * Values in order of importance
   * (0, -1, 1) - Where 1 is highest priority
   */
  if (done != 1 && recursion < 1 && C->wm.store) {
    C->data.recursion = 1;

    const KrakenPRIM *ptr = CTX_store_ptr_lookup(C->wm.store, member, nullptr);

    if (ptr) {
      result->ptr = *ptr;
      done = 1;
    }
  }
  if (done != 1 && recursion < 2 && (region = CTX_wm_region(C))) {
    C->data.recursion = 2;
    if (region->type && region->type->context) {
      ret = region->type->context(C, member, result);
      if (ret) {
        done = -(-ret | -done);
      }
    }
  }
  if (done != 1 && recursion < 3 && (area = CTX_wm_area(C))) {
    C->data.recursion = 3;
    if (area->type && area->type->context) {
      ret = area->type->context(C, member, result);
      if (ret) {
        done = -(-ret | -done);
      }
    }
  }

  if (done != 1 && recursion < 4 && (screen = CTX_wm_screen(C))) {
    kContextDataCallback cb = screen->context;
    C->data.recursion = 4;
    if (cb) {
      ret = cb(C, member, result);
      if (ret) {
        done = -(-ret | -done);
      }
    }
  }

  C->data.recursion = recursion;

  return static_cast<eContextResult>(done);
}

static int ctx_data_pointer_verify(const kContext *C, const char *member, void **pointer)
{
  /* if context is nullptr, pointer must be nullptr too and that is a valid return */
  if (C == nullptr) {
    *pointer = nullptr;
    return 1;
  }

  kContextDataResult result;
  if (ctx_data_get((kContext *)C, member, &result) == CTX_RESULT_OK) {
    KLI_assert(result.type == CTX_DATA_TYPE_POINTER);
    *pointer = result.ptr.data;
    return 1;
  }

  *pointer = nullptr;
  return 0;
}

/* Context */

kContext *CTX_create(void)
{
  kContext *C = MEM_new<kContext>("kContext");

  return C;
}

kContext *CTX_copy(const kContext *C)
{
  kContext *newC = MEM_new<kContext>("kContext", *C);

  memset(&newC->wm.operator_poll_msg_params, 0, sizeof(newC->wm.operator_poll_msg_params));

  return newC;
}

void CTX_free(kContext *C)
{
  /* This may contain a dynamically allocated message, free. */
  CTX_wm_operator_poll_msg_clear(C);

  /** CTX out. */
  MEM_delete(C);
}

/* Stored Context */

kContextStore *CTX_store_add(ListBase *contexts, const char *name, const KrakenPRIM *ptr)
{
  /* ensure we have a context to put the entry in, if it was already used
   * we have to copy the context to ensure */
  kContextStore *ctx = static_cast<kContextStore *>(contexts->last);

  if (!ctx || ctx->used) {
    if (ctx) {
      kContextStore *lastctx = ctx;
      ctx = (kContextStore *)MEM_dupallocN(lastctx);
      KLI_duplicatelist(&ctx->entries, &lastctx->entries);
    } else {
      ctx = (kContextStore *)MEM_callocN(sizeof(kContextStore), "kContextStore");
    }

    KLI_addtail(contexts, ctx);
  }

  kContextStoreEntry *entry = MEM_new<kContextStoreEntry>("kContextStoreEntry");
  KLI_strncpy(entry->name, name, sizeof(entry->name));
  entry->ptr = *ptr;

  KLI_addtail(&ctx->entries, entry);

  return ctx;
}

kContextStore *CTX_store_add_all(ListBase *contexts, kContextStore *context)
{
  /* ensure we have a context to put the entries in, if it was already used
   * we have to copy the context to ensure */
  kContextStore *ctx = (kContextStore *)contexts->last;

  if (!ctx || ctx->used) {
    if (ctx) {
      kContextStore *lastctx = ctx;
      ctx = (kContextStore *)MEM_dupallocN(lastctx);
      KLI_duplicatelist(&ctx->entries, &lastctx->entries);
    } else {
      ctx = (kContextStore *)MEM_callocN(sizeof(kContextStore), "kContextStore");
    }

    KLI_addtail(contexts, ctx);
  }

  LISTBASE_FOREACH(kContextStoreEntry *, tentry, &context->entries)
  {
    kContextStoreEntry *entry = MEM_new<kContextStoreEntry>(__func__, *tentry);
    KLI_addtail(&ctx->entries, entry);
  }

  return ctx;
}

kContextStore *CTX_store_get(kContext *C)
{
  return C->wm.store;
}

void CTX_store_set(kContext *C, kContextStore *store)
{
  C->wm.store = store;
}

const KrakenPRIM *CTX_store_ptr_lookup(const kContextStore *store,
                                       const char *name,
                                       const KrakenPRIM *type)
{
  kContextStoreEntry *entry = (kContextStoreEntry *)
    KLI_rfindstring(&store->entries, name, offsetof(kContextStoreEntry, name));
  if (!entry) {
    return nullptr;
  }

  if (type && !LUXO_prim_is_a(entry->ptr.type, type)) {
    return nullptr;
  }
  return &entry->ptr;
}

kContextStore *CTX_store_copy(kContextStore *store)
{
  kContextStore *ctx = (kContextStore *)MEM_dupallocN(store);
  KLI_duplicatelist(&ctx->entries, &store->entries);

  return ctx;
}

void CTX_store_free(kContextStore *store)
{
  KLI_freelistN(&store->entries);
  MEM_freeN(store);
}

void CTX_store_free_list(ListBase *contexts)
{
  kContextStore *ctx;
  while ((ctx = (kContextStore *)KLI_pophead(contexts))) {
    CTX_store_free(ctx);
  }
}

/* Python */

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

void *CTX_py_dict_get(const kContext *C)
{
  return C->data.py_context;
}

void *CTX_py_dict_get_orig(const kContext *C)
{
  return C->data.py_context_orig;
}

void CTX_py_state_push(kContext *C, struct kContext_PyState *pystate, void *value)
{
  pystate->py_context = C->data.py_context;
  pystate->py_context_orig = C->data.py_context_orig;

  C->data.py_context = value;
  C->data.py_context_orig = value;
}

void CTX_py_state_pop(kContext *C, struct kContext_PyState *pystate)
{
  C->data.py_context = pystate->py_context;
  C->data.py_context_orig = pystate->py_context_orig;
}

/* Window Manager Context */

wmWindowManager *CTX_wm_manager(const kContext *C)
{
  return C->wm.manager;
}

wmWindow *CTX_wm_window(const kContext *C)
{
  return (
    wmWindow *)ctx_wm_python_context_get(C, "window", (KrakenPRIM *)&PRIM_Window, C->wm.window);
}

WorkSpace *CTX_wm_workspace(const kContext *C)
{
  return (WorkSpace *)
    ctx_wm_python_context_get(C, "workspace", (KrakenPRIM *)&PRIM_WorkSpace, C->wm.workspace);
}

kScreen *CTX_wm_screen(const kContext *C)
{
  return (
    kScreen *)ctx_wm_python_context_get(C, "screen", (KrakenPRIM *)&PRIM_Screen, C->wm.screen);
}

ScrArea *CTX_wm_area(const kContext *C)
{
  return (ScrArea *)ctx_wm_python_context_get(C, "area", (KrakenPRIM *)&PRIM_Area, C->wm.area);
}

ARegion *CTX_wm_region(const kContext *C)
{
  return (
    ARegion *)ctx_wm_python_context_get(C, "region", (KrakenPRIM *)&PRIM_Region, C->wm.region);
}

ARegion *CTX_wm_menu(const kContext *C)
{
  return C->wm.menu;
}

struct wmMsgBus *CTX_wm_message_bus(const kContext *C)
{
  return C->wm.manager ? C->wm.manager->message_bus : nullptr;
}

ReportList *CTX_wm_reports(const kContext *C)
{
  if (C->wm.manager) {
    return &(C->wm.manager->reports);
  }

  return nullptr;
}

void CTX_wm_manager_set(kContext *C, wmWindowManager *wm)
{
  C->wm.manager = wm;
  C->wm.window = POINTER_ZERO;
  C->wm.screen = POINTER_ZERO;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;
}

#ifdef WITH_PYTHON
#  define PYCTX_REGION_MEMBERS "region", "region_data"
#  define PYCTX_AREA_MEMBERS "area", "space_data", PYCTX_REGION_MEMBERS
#  define PYCTX_SCREEN_MEMBERS "screen", PYCTX_AREA_MEMBERS
#  define PYCTX_WINDOW_MEMBERS "window", "scene", "workspace", PYCTX_SCREEN_MEMBERS
#endif

void CTX_wm_window_set(kContext *C, wmWindow *win)
{
  C->wm.window = win;
  if (win) {
    C->data.scene = win->scene;
  }
  C->wm.workspace = (win) ? KKE_workspace_active_get(win->workspace_hook) : POINTER_ZERO;
  C->wm.screen = (win) ? KKE_workspace_active_screen_get(win->workspace_hook) : POINTER_ZERO;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;

#ifdef WITH_PYTHON
  if (C->data.py_context != nullptr) {
    KPY_context_dict_clear_members(C, PYCTX_WINDOW_MEMBERS);
  }
#endif
}

void CTX_wm_screen_set(kContext *C, kScreen *screen)
{
  C->wm.screen = screen;
  C->wm.area = POINTER_ZERO;
  C->wm.region = POINTER_ZERO;

#ifdef WITH_PYTHON
  if (C->data.py_context != nullptr) {
    KPY_context_dict_clear_members(C, PYCTX_SCREEN_MEMBERS);
  }
#endif
}

void CTX_wm_area_set(kContext *C, ScrArea *area)
{
  C->wm.area = area;
  C->wm.region = POINTER_ZERO;

#ifdef WITH_PYTHON
  if (C->data.py_context != nullptr) {
    KPY_context_dict_clear_members(C, PYCTX_AREA_MEMBERS);
  }
#endif
}

void CTX_wm_region_set(kContext *C, ARegion *region)
{
  C->wm.region = region;

#ifdef WITH_PYTHON
  if (C->data.py_context != nullptr) {
    KPY_context_dict_clear_members(C, PYCTX_REGION_MEMBERS);
  }
#endif
}

void CTX_wm_menu_set(kContext *C, ARegion *menu)
{
  C->wm.menu = menu;
}

/* Data Context */

Main *CTX_data_main(const kContext *C)
{
  Main *kmain;
  if (ctx_data_pointer_verify(C, "usd_data", (void **)&kmain)) {
    return kmain;
  }

  return C->data.main;
}

kScene *CTX_data_scene(const kContext *C)
{
  kScene *scene;
  if (ctx_data_pointer_verify(C, "scene", (void **)&scene)) {
    return scene;
  }

  return C->data.scene;
}

KrakenSTAGE CTX_data_stage(const kContext *C)
{
  return C->data.stage;
}

kUserDef *CTX_data_prefs(const kContext *C)
{
  return C->data.prefs;
}

void CTX_data_pointer_set_ptr(kContextDataResult *result, const KrakenPRIM *ptr)
{
  result->ptr = *ptr;
}

void CTX_data_list_add_ptr(kContextDataResult *result, const KrakenPRIM *ptr)
{
  CollectionPrimLINK *link = MEM_new<CollectionPrimLINK>("CTX_data_list_add");
  link->ptr = *ptr;

  KLI_addtail(&result->list, link);
}

void CTX_data_type_set(kContextDataResult *result, short type)
{
  result->type = type;
}

int ctx_data_list_count(const kContext *C, int (*func)(const kContext *, ListBase *))
{
  ListBase list;

  if (func(C, &list)) {
    int tot = KLI_listbase_count(&list);
    KLI_freelistN(&list);
    return tot;
  }

  return 0;
}

void CTX_data_main_set(kContext *C, Main *kmain)
{
  C->data.main = kmain;
}

static void *ctx_data_pointer_get(const kContext *C, const char *member)
{
  kContextDataResult result;
  if (C && ctx_data_get((kContext *)C, member, &result) == CTX_RESULT_OK) {
    KLI_assert(result.type == CTX_DATA_TYPE_POINTER);
    return result.ptr.data;
  }

  return nullptr;
}

struct Object *CTX_data_active_object(const kContext *C)
{
  return static_cast<Object *>(ctx_data_pointer_get(C, "active_object"));
}

struct Base *CTX_data_active_base(const kContext *C)
{
  Object *ob = static_cast<Object *>(ctx_data_pointer_get(C, "active_object"));

  if (ob == nullptr) {
    return nullptr;
  }
  const Scene *scene = CTX_data_scene(C);
  // ViewLayer *view_layer = CTX_data_view_layer(C);
  // KKE_view_layer_synced_ensure(scene, view_layer);
  // return KKE_view_layer_base_find(view_layer, ob);
  return nullptr;
}

struct Object *CTX_data_edit_object(const kContext *C)
{
  return static_cast<Object *>(ctx_data_pointer_get(C, "edit_object"));
}

enum eContextObjectMode CTX_data_mode_enum_ex(const Object *obedit,
                                              const Object *ob,
                                              const eObjectMode object_mode)
{
  // Object *obedit = CTX_data_edit_object(C);
  if (obedit) {
    switch (obedit->type) {
      case OB_MESH:
        return CTX_MODE_EDIT_MESH;
      case OB_CURVES:
        return CTX_MODE_EDIT_CURVE;
      case OB_SURF:
        return CTX_MODE_EDIT_SURFACE;
      case OB_FONT:
        return CTX_MODE_EDIT_TEXT;
      case OB_ARMATURE:
        return CTX_MODE_EDIT_ARMATURE;
      case OB_MBALL:
        return CTX_MODE_EDIT_METABALL;
      case OB_LATTICE:
        return CTX_MODE_EDIT_LATTICE;
    }
  } else {
    // Object *ob = CTX_data_active_object(C);
    if (ob) {
      if (object_mode & OB_MODE_POSE) {
        return CTX_MODE_POSE;
      }
      if (object_mode & OB_MODE_SCULPT) {
        return CTX_MODE_SCULPT;
      }
      if (object_mode & OB_MODE_WEIGHT_PAINT) {
        return CTX_MODE_PAINT_WEIGHT;
      }
      if (object_mode & OB_MODE_VERTEX_PAINT) {
        return CTX_MODE_PAINT_VERTEX;
      }
      if (object_mode & OB_MODE_TEXTURE_PAINT) {
        return CTX_MODE_PAINT_TEXTURE;
      }
      if (object_mode & OB_MODE_PARTICLE_EDIT) {
        return CTX_MODE_PARTICLE;
      }
      if (object_mode & OB_MODE_OBJECT) {
        return CTX_MODE_OBJECT;
      }
      if (object_mode & OB_MODE_PAINT_GPENCIL) {
        return CTX_MODE_PAINT_GPENCIL;
      }
      if (object_mode & OB_MODE_EDIT_GPENCIL) {
        return CTX_MODE_EDIT_GPENCIL;
      }
      if (object_mode & OB_MODE_SCULPT_GPENCIL) {
        return CTX_MODE_SCULPT_GPENCIL;
      }
      if (object_mode & OB_MODE_WEIGHT_GPENCIL) {
        return CTX_MODE_WEIGHT_GPENCIL;
      }
      if (object_mode & OB_MODE_VERTEX_GPENCIL) {
        return CTX_MODE_VERTEX_GPENCIL;
      }
      if (object_mode & OB_MODE_SCULPT_CURVES) {
        return CTX_MODE_SCULPT_CURVES;
      }
    }
  }

  return CTX_MODE_OBJECT;
}

enum eContextObjectMode CTX_data_mode_enum(const kContext *C)
{
  Object *obedit = CTX_data_edit_object(C);
  Object *obact = obedit ? nullptr : CTX_data_active_object(C);
  return CTX_data_mode_enum_ex(obedit,
                               obact,
                               obact ? static_cast<eObjectMode>(obact->mode) : OB_MODE_OBJECT);
}

/**
 * Would prefer if we can use the enum version below over this one - Campbell.
 *
 * @note Must be aligned with above enum.
 */
static const char *data_mode_strings[] = {
  "mesh_edit",
  "curve_edit",
  "surface_edit",
  "text_edit",
  "armature_edit",
  "mball_edit",
  "lattice_edit",
  "posemode",
  "sculpt_mode",
  "weightpaint",
  "vertexpaint",
  "imagepaint",
  "particlemode",
  "objectmode",
  "greasepencil_paint",
  "greasepencil_edit",
  "greasepencil_sculpt",
  "greasepencil_weight",
  "greasepencil_vertex",
  "curves_sculpt",
  NULL,
};
KLI_STATIC_ASSERT(ARRAY_SIZE(data_mode_strings) == CTX_MODE_NUM + 1,
                  "Must have a string for each context mode")
const char *CTX_data_mode_string(const kContext *C)
{
  return data_mode_strings[CTX_data_mode_enum(C)];
}

void CTX_data_scene_set(kContext *C, kScene *scene)
{
  C->data.scene = scene;

#ifdef WITH_PYTHON
  if (C->data.py_context != nullptr) {
    KPY_context_dict_clear_members(C, "scene");
  }
#endif
}

void CTX_data_prefs_set(kContext *C, kUserDef *uprefs)
{
  C->data.prefs = uprefs;
}

/* Operator Polls */

void CTX_wm_operator_poll_msg_clear(kContext *C)
{
  struct kContextPollMsgParams *params = &C->wm.operator_poll_msg_params;
  if (params->free_fn != nullptr) {
    params->free_fn(C, params->user_data);
  }
  params->get_fn = nullptr;
  params->free_fn = nullptr;
  params->user_data = nullptr;

  C->wm.operator_poll_msg = nullptr;
}

void CTX_wm_operator_poll_msg_set(kContext *C, const char *msg)
{
  CTX_wm_operator_poll_msg_clear(C);

  C->wm.operator_poll_msg = msg;
}

void CTX_wm_operator_poll_msg_set_dynamic(kContext *C, const struct kContextPollMsgParams *params)
{
  CTX_wm_operator_poll_msg_clear(C);

  C->wm.operator_poll_msg_params = *params;
}

const char *CTX_wm_operator_poll_msg_get(kContext *C, bool *r_free)
{
  struct kContextPollMsgParams *params = &C->wm.operator_poll_msg_params;
  if (params->get_fn != nullptr) {
    char *msg = params->get_fn(C, params->user_data);
    if (msg != nullptr) {
      *r_free = true;
    }
    return msg;
  }

  *r_free = false;
  return IFACE_(C->wm.operator_poll_msg);
}
