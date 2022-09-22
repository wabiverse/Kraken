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

/* KRAKEN */
#include "kraken/kraken.h"

/* UNIVERSE */
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
#include "USD_wm_types.h"
#include "USD_workspace.h"

/* KRAKEN LIBRARY */
#include "KLI_string_utils.h"

/* KRAKEN KERNEL */
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"

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

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_BEGIN

Global G;

/** User Prefs modifies this value globally. */
int UI_FACTOR_DISPLAY_TYPE = USER_FACTOR_AS_FACTOR;
float UI_DPI_FAC = float(1.0f);
float UI_PRESSURE_SOFTNESS = float(1.0f);
float UI_PRESSURE_THRESHOLD_MAX = float(0.0f);
int UI_FLAG = int(0);
bool UI_MOUSE_EMULATE_3BUTTON_MODIFIER = bool(false);
int UI_DRAG_THRESHOLD_MOUSE = int(0);
int UI_DRAG_THRESHOLD_TABLET = int(0);
int UI_DRAG_THRESHOLD = int(0);
short UI_DOUBLE_CLICK_TIME = short(0);

static char kraken_version_string[48] = "";

static void kraken_version_init()
{
  printf("\n");
  KLI_snprintf(kraken_version_string,
               ARRAY_SIZE(kraken_version_string),
               "%d.%01d.%d%s",
               KRAKEN_VERSION / 100,
               KRAKEN_VERSION % 100,
               KRAKEN_VERSION_PATCH,
               STRINGIFY(KRAKEN_VERSION_CYCLE));
  TF_WARN("Kraken v%s", CHARALL(kraken_version_string));
  printf("\n");
}

static std::string kraken_get_version_decimal()
{
  return TfStringPrintf("%d.%02d", KRAKEN_VERSION / 100, KRAKEN_VERSION % 100);
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

Global &KKE_kraken_globals_init()
{
  kraken_version_init();

  memset(&G, 0, sizeof(Global));

  G.main = KKE_main_new();

  G.main->exe_path = kraken_exe_path_init();
  G.main->temp_dir = kraken_system_tempdir_path();

  G.main->kraken_version_decimal = kraken_get_version_decimal();

  G.main->datafiles_path = kraken_datafiles_path_init();
  G.main->fonts_path = kraken_fonts_path_init();
  G.main->python_path = kraken_python_path_init();
  G.main->icons_path = kraken_icon_path_init();
  G.main->stage_id = kraken_startup_file_init();
  G.main->ocio_cfg = kraken_ocio_file_init();

  return G;
}

void KKE_kraken_main_init(kContext *C, int argc, const char **argv)
{
  /* Determine stage to load (from user or factory default). */
  if (!std::filesystem::exists(G.main->stage_id) ||
      G.main->stage_id.string().find("userpref.usda") != std::string::npos) {
    G.factory_startup = true;
  }

  CTX_data_main_set(C, G.main);

  /** @em Always */
  USD_create_stage(C);

  if (G.factory_startup) {
    /**
     * Create default Pixar stage. */
    // USD_set_defaults(C);
    // USD_save_stage(C);
  } else {
    /**
     * Open user's stage. */
    // USD_open_stage(C);
  }
}

void KKE_main_free(Main *mainvar)
{
  UNIVERSE_FOR_ALL (windowmanager, mainvar->wm) {
    delete windowmanager;
  }
  mainvar->wm.clear();

  UNIVERSE_FOR_ALL (workspace, mainvar->workspaces) {
    delete workspace;
  }
  mainvar->workspaces.clear();

  UNIVERSE_FOR_ALL (screen, mainvar->screens) {
    delete screen;
  }
  mainvar->screens.clear();

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

KRAKEN_NAMESPACE_END