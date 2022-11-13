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

/* STDLIB */
#include <fstream>
#include <string>

#ifdef __GNUC__
#  include <cstdlib>
#endif /*__GNUC__ */

#include "MEM_guardedalloc.h"

/* KRAKEN */
#include "kraken/kraken.h"

/* UNIVERSE */
#include "USD_ID.h"
#include "USD_wm_types.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_userdef_types.h"
#include "USD_window.h"
#include "USD_workspace.h"

/* KRAKEN LIBRARY */
#include "KLI_listbase.h"
#include "KLI_string.h"
#include "KLI_threads.h"

/* KRAKEN KERNEL */
#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_lib_id.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"
#include "KKE_global.h"

/* ANCHOR */
#include "ANCHOR_api.h"
#include "ANCHOR_debug_codes.h"

#include "IMB_imbuf.h"

/* WINDOW MANAGER */
#include "WM_init_exit.h"

/* PIXAR */
#include <wabi/base/arch/hints.h>
#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/debug.h>
#include <wabi/base/tf/error.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/imaging/hd/debugCodes.h>
#include <wabi/usd/sdf/debugCodes.h>
#include <wabi/usd/usd/debugCodes.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/wabi.h>

Global G;
UserDef U;

static char kraken_version_string[48] = "";

static void kraken_version_init()
{
  const char *version_cycle = "";
  if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "alpha")) {
    version_cycle = " Alpha";
  } else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "beta")) {
    version_cycle = " Beta";
  } else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "rc")) {
    version_cycle = " Release Candidate";
  } else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "release")) {
    version_cycle = "";
  } else {
    KLI_assert_msg(0, "Invalid Kraken version cycle");
  }

  KLI_snprintf(kraken_version_string,
               ARRAY_SIZE(kraken_version_string),
               "%d.%01d.%d%s",
               KRAKEN_VERSION / 100,
               KRAKEN_VERSION % 100,
               KRAKEN_VERSION_PATCH,
               version_cycle);
}

const char *KKE_kraken_version_string(void)
{
  return kraken_version_string;
}

Main *KKE_main_new(void)
{
  Main *kmain = static_cast<Main *>(MEM_callocN(sizeof(Main), "new main"));
  kmain->lock = static_cast<MainLock *>(MEM_mallocN(sizeof(SpinLock), "main lock"));
  KLI_spin_init((SpinLock *)kmain->lock);
  kmain->is_global_main = false;
  return kmain;
}

void KKE_main_free(Main *mainvar)
{
  /* also call when reading a file, erase all, etc */
  ListBase *lbarray[INDEX_ID_MAX];
  int a;

  /* Since we are removing whole main, no need to bother 'properly'
   * (and slowly) removing each ID from it. */
  const int free_flag = (LIB_ID_FREE_NO_MAIN | LIB_ID_FREE_NO_UI_USER |
                         LIB_ID_FREE_NO_USER_REFCOUNT | LIB_ID_FREE_NO_DEG_TAG);

  MEM_SAFE_FREE(mainvar->kraken_thumb);

  a = set_listbasepointers(mainvar, lbarray);
  while (a--) {
    ListBase *lb = lbarray[a];
    ID *id, *id_next;

    for (id = lb->first; id != NULL; id = id_next) {
      id_next = id->next;

      KKE_id_free_ex(mainvar, id, free_flag, false);

    }
    KLI_listbase_clear(lb);
  }

  if (mainvar->relations) {
    KKE_main_relations_free(mainvar);
  }

  if (mainvar->id_map) {
    KKE_main_idmap_destroy(mainvar->id_map);
  }

  if (mainvar->name_map) {
    KKE_main_namemap_destroy(&mainvar->name_map);
  }

  KLI_spin_end((SpinLock *)mainvar->lock);
  MEM_freeN(mainvar->lock);
  MEM_freeN(mainvar);  
}

void KKE_main_lock(struct Main *kmain)
{
  KLI_spin_lock((SpinLock *)kmain->lock);
}

void KKE_main_unlock(struct Main *kmain)
{
  KLI_spin_unlock((SpinLock *)kmain->lock);
}

const char *KKE_main_usdfile_path(const Main *kmain)
{
  return kmain->filepath;
}

ListBase *which_libbase(Main *kmain, short type)
{
  switch ((ID_Type)type) {
    case ID_SCE:
      return &(kmain->scenes);
    case ID_LI:
      return &(kmain->libraries);
    case ID_OB:
      return &(kmain->objects);
    case ID_ME:
      return &(kmain->meshes);
    case ID_CV:
      return &(kmain->curves);
    case ID_MB:
      return &(kmain->metaballs);
    case ID_MA:
      return &(kmain->materials);
    case ID_TE:
      return &(kmain->textures);
    case ID_IM:
      return &(kmain->images);
    case ID_LT:
      return &(kmain->lattices);
    case ID_LA:
      return &(kmain->lights);
    case ID_CA:
      return &(kmain->cameras);
    case ID_KE:
      return &(kmain->shapekeys);
    case ID_WO:
      return &(kmain->worlds);
    case ID_SCR:
      return &(kmain->screens);
    case ID_VF:
      return &(kmain->fonts);
    case ID_TXT:
      return &(kmain->texts);
    case ID_SPK:
      return &(kmain->speakers);
    case ID_LP:
      return &(kmain->lightprobes);
    case ID_SO:
      return &(kmain->sounds);
    case ID_GR:
      return &(kmain->collections);
    case ID_AR:
      return &(kmain->armatures);
    case ID_AC:
      return &(kmain->actions);
    case ID_NT:
      return &(kmain->nodetrees);
    case ID_BR:
      return &(kmain->brushes);
    case ID_PA:
      return &(kmain->particles);
    case ID_WM:
      return &(kmain->wm);
    case ID_GD:
      return &(kmain->gpencils);
    case ID_MC:
      return &(kmain->movieclips);
    case ID_MSK:
      return &(kmain->masks);
    case ID_LS:
      return &(kmain->linestyles);
    case ID_PAL:
      return &(kmain->palettes);
    case ID_PC:
      return &(kmain->paintcurves);
    case ID_CF:
      return &(kmain->cachefiles);
    case ID_WS:
      return &(kmain->workspaces);
    case ID_PT:
      return &(kmain->pointclouds);
    case ID_VO:
      return &(kmain->volumes);
    case ID_SIM:
      return &(kmain->simulations);
  }
  return nullptr;
}

int set_listbasepointers(Main *kmain, ListBase *lb[/*INDEX_ID_MAX*/])
{
  /* Libraries may be accessed from pretty much any other ID. */
  lb[INDEX_ID_LI] = &(kmain->libraries);

  /* Moved here to avoid problems when freeing with animato (aligorith). */
  lb[INDEX_ID_AC] = &(kmain->actions);

  lb[INDEX_ID_KE] = &(kmain->shapekeys);

  /* Referenced by gpencil, so needs to be before that to avoid crashes. */
  lb[INDEX_ID_PAL] = &(kmain->palettes);

  /* Referenced by nodes, objects, view, scene etc, before to free after. */
  lb[INDEX_ID_GD] = &(kmain->gpencils);

  lb[INDEX_ID_NT] = &(kmain->nodetrees);
  lb[INDEX_ID_IM] = &(kmain->images);
  lb[INDEX_ID_TE] = &(kmain->textures);
  lb[INDEX_ID_MA] = &(kmain->materials);
  lb[INDEX_ID_VF] = &(kmain->fonts);

  /* Important!: When adding a new object type,
   * the specific data should be inserted here. */

  lb[INDEX_ID_AR] = &(kmain->armatures);

  lb[INDEX_ID_CF] = &(kmain->cachefiles);
  lb[INDEX_ID_ME] = &(kmain->meshes);
  lb[INDEX_ID_CV] = &(kmain->curves);
  lb[INDEX_ID_MB] = &(kmain->metaballs);
  lb[INDEX_ID_PT] = &(kmain->pointclouds);
  lb[INDEX_ID_VO] = &(kmain->volumes);

  lb[INDEX_ID_LT] = &(kmain->lattices);
  lb[INDEX_ID_LA] = &(kmain->lights);
  lb[INDEX_ID_CA] = &(kmain->cameras);

  lb[INDEX_ID_TXT] = &(kmain->texts);
  lb[INDEX_ID_SO] = &(kmain->sounds);
  lb[INDEX_ID_GR] = &(kmain->collections);
  lb[INDEX_ID_PAL] = &(kmain->palettes);
  lb[INDEX_ID_PC] = &(kmain->paintcurves);
  lb[INDEX_ID_BR] = &(kmain->brushes);
  lb[INDEX_ID_PA] = &(kmain->particles);
  lb[INDEX_ID_SPK] = &(kmain->speakers);
  lb[INDEX_ID_LP] = &(kmain->lightprobes);

  lb[INDEX_ID_WO] = &(kmain->worlds);
  lb[INDEX_ID_MC] = &(kmain->movieclips);
  lb[INDEX_ID_SCR] = &(kmain->screens);
  lb[INDEX_ID_OB] = &(kmain->objects);
  lb[INDEX_ID_LS] = &(kmain->linestyles); /* referenced by scenes */
  lb[INDEX_ID_SCE] = &(kmain->scenes);
  lb[INDEX_ID_WS] = &(kmain->workspaces); /* before wm, so it's freed after it! */
  lb[INDEX_ID_WM] = &(kmain->wm);
  lb[INDEX_ID_MSK] = &(kmain->masks);
  lb[INDEX_ID_SIM] = &(kmain->simulations);

  lb[INDEX_ID_NULL] = NULL;

  return (INDEX_ID_MAX - 1);
}


void KKE_kraken_globals_init(void)
{
  kraken_version_init();

  memset(&G, 0, sizeof(Global));

  U.savetime = 1;

  KKE_kraken_globals_main_replace(KKE_main_new());

  strcpy(G.ima, "//");

#ifndef WITH_PYTHON_SECURITY /* default */
  G.f |= G_FLAG_SCRIPT_AUTOEXEC;
#else
  G.f &= ~G_FLAG_SCRIPT_AUTOEXEC;
#endif

  G.log.level = 1;
}

void KKE_kraken_globals_clear(void)
{
  if (G_MAIN == nullptr) {
    return;
  }
  KLI_assert(G_MAIN->is_global_main);
  KKE_main_free(G_MAIN);

  G_MAIN = nullptr;
}

void KKE_kraken_globals_main_replace(Main *kmain)
{
  KLI_assert(!kmain->is_global_main);
  KKE_kraken_globals_clear();
  kmain->is_global_main = true;
  G_MAIN = kmain;
}

void KKE_kraken_free(void)
{
  // KKE_studiolight_free();

  KKE_kraken_globals_clear();

  if (G.log.file != nullptr) {
    fclose(static_cast<FILE *>(G.log.file));
  }

  // KKE_spacetypes_free();

  IMB_exit();
  // KKE_cachefiles_exit();
  // KKE_images_exit();
  // DEG_free_node_types();

  // KKE_brush_system_exit();
  // RE_texture_rng_exit();

  KKE_callback_global_finalize();

  // IMB_moviecache_destruct();

  // KKE_node_system_exit();
}


static struct AtExitData
{
  struct AtExitData *next;

  void (*func)(void *user_data);
  void *user_data;
} *g_atexit = nullptr;

void KKE_kraken_atexit_register(void (*func)(void *user_data), void *user_data)
{
  struct AtExitData *ae = (AtExitData *)malloc(sizeof(*ae));
  ae->next = g_atexit;
  ae->func = func;
  ae->user_data = user_data;
  g_atexit = ae;
}

void KKE_kraken_atexit_unregister(void (*func)(void *user_data), const void *user_data)
{
  struct AtExitData *ae = g_atexit;
  struct AtExitData **ae_p = &g_atexit;

  while (ae) {
    if ((ae->func == func) && (ae->user_data == user_data)) {
      *ae_p = ae->next;
      free(ae);
      return;
    }
    ae_p = &ae->next;
    ae = ae->next;
  }
}

static ListBase callback_slots[KKE_CB_EVT_TOT] = {{nullptr}};

static bool callbacks_initialized = false;

#define ASSERT_CALLBACKS_INITIALIZED()                                                           \
  KLI_assert_msg(callbacks_initialized,                                                          \
                 "Callbacks should be initialized with KKE_callback_global_init() before using " \
                 "the callback system.")

void KKE_callback_exec(struct Main *kmain,
                       struct KrakenPRIM **pointers,
                       const int num_pointers,
                       eCbEvent evt)
{
  ASSERT_CALLBACKS_INITIALIZED();

  /* Use mutable iteration so handlers are able to remove themselves. */
  ListBase *lb = &callback_slots[evt];
  LISTBASE_FOREACH_MUTABLE(kCallbackFuncStore *, funcstore, lb)
  {
    funcstore->func(kmain, pointers, num_pointers, funcstore->arg);
  }
}

void KKE_callback_exec_null(struct Main *kmain, eCbEvent evt)
{
  KKE_callback_exec(kmain, nullptr, 0, evt);
}

void KKE_callback_remove(kCallbackFuncStore *funcstore, eCbEvent evt)
{
  /* The callback may have already been removed by KKE_callback_global_finalize(), for
   * example when removing callbacks in response to a KKE_blender_atexit_register callback
   * function. `KKE_blender_atexit()` runs after `KKE_callback_global_finalize()`. */
  if (!callbacks_initialized) {
    return;
  }

  ListBase *lb = &callback_slots[evt];

  /* Be noisy about potential programming errors. */
  KLI_assert_msg(KLI_findindex(lb, funcstore) != -1, "To-be-removed callback not found");

  KLI_remlink(lb, funcstore);

  if (funcstore->alloc) {
    MEM_freeN(funcstore);
  }
}

void KKE_callback_global_init(void)
{
  callbacks_initialized = true;
}

void KKE_callback_global_finalize(void)
{
  eCbEvent evt;
  for (evt = static_cast<eCbEvent>(0); evt < KKE_CB_EVT_TOT;
       evt = static_cast<eCbEvent>(static_cast<int>(evt) + 1)) {
    ListBase *lb = &callback_slots[evt];
    kCallbackFuncStore *funcstore;
    kCallbackFuncStore *funcstore_next;
    for (funcstore = static_cast<kCallbackFuncStore *>(lb->first); funcstore;
         funcstore = funcstore_next) {
      funcstore_next = funcstore->next;
      KKE_callback_remove(funcstore, evt);
    }
  }

  callbacks_initialized = false;
}

void KKE_kraken_atexit(void)
{
  struct AtExitData *ae = g_atexit, *ae_next;
  while (ae) {
    ae_next = ae->next;

    ae->func(ae->user_data);

    free(ae);
    ae = ae_next;
  }
  g_atexit = nullptr;
}
