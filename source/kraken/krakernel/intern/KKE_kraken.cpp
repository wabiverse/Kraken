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
#include "USD_window.h"
#include "USD_workspace.h"

/* KRAKEN LIBRARY */
#include "KLI_listbase.h"
#include "KLI_string.h"

/* KRAKEN KERNEL */
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"
#include "KKE_global.h"

/* ANCHOR */
#include "ANCHOR_api.h"
#include "ANCHOR_debug_codes.h"

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
  if (STREQ(STRINGIFY(BLENDER_VERSION_CYCLE), "alpha")) {
    version_cycle = " Alpha";
  }
  else if (STREQ(STRINGIFY(BLENDER_VERSION_CYCLE), "beta")) {
    version_cycle = " Beta";
  }
  else if (STREQ(STRINGIFY(BLENDER_VERSION_CYCLE), "rc")) {
    version_cycle = " Release Candidate";
  }
  else if (STREQ(STRINGIFY(BLENDER_VERSION_CYCLE), "release")) {
    version_cycle = "";
  }
  else {
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
  Main *kmain = new Main();
  return kmain;
}

const char *KKE_main_usdfile_path(const Main *kmain)
{
  return kmain->stage_id;
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
  if (G_MAIN == NULL) {
    return;
  }
  KLI_assert(G_MAIN->is_global_main);
  KKE_main_free(G_MAIN);

  G_MAIN = NULL;
}

void KKE_kraken_globals_main_replace(Main *kmain)
{
  KLI_assert(!kmain->is_global_main);
  KKE_kraken_globals_clear();
  kmain->is_global_main = true;
  G_MAIN = kmain;
}

void KKE_main_free(Main *mainvar)
{
  LISTBASE_FOREACH(wmWindowManager *, windowmanager, &mainvar->wm)
  {
    delete windowmanager;
  }
  KLI_listbase_clear(&mainvar->wm);

  LISTBASE_FOREACH(WorkSpace *, workspace, &mainvar->workspaces)
  {
    delete workspace;
  }
  KLI_listbase_clear(&mainvar->workspaces);

  LISTBASE_FOREACH(kScreen *, screen, &mainvar->screens)
  {
    delete screen;
  }
  KLI_listbase_clear(&mainvar->screens);

  delete mainvar;
}

void KKE_kraken_free(void)
{
  // KKE_studiolight_free();

  KKE_main_free(G.main);
  G.main = nullptr;

  // if (G.log.file != NULL) {
  //   fclose(G.log.file);
  // }

  // KKE_spacetypes_free();

  // IMB_exit();
  // KKE_cachefiles_exit();
  // KKE_images_exit();
  // DEG_free_node_types();

  // KKE_brush_system_exit();
  // RE_texture_rng_exit();

  // KKE_callback_global_finalize();

  // IMB_moviecache_destruct();

  // KKE_node_system_exit();
}


static struct AtExitData
{
  struct AtExitData *next;

  void (*func)(void *user_data);
  void *user_data;
} *g_atexit = NULL;

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

void KKE_kraken_atexit(void)
{
  struct AtExitData *ae = g_atexit, *ae_next;
  while (ae) {
    ae_next = ae->next;

    ae->func(ae->user_data);

    free(ae);
    ae = ae_next;
  }
  g_atexit = NULL;
}
