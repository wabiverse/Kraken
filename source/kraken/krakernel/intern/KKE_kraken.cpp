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

/* STDLIB */
#include <fstream>
#include <string>

#ifdef __GNUC__
#  include <cstdlib>
#endif /*__GNUC__ */

/* UNIVERSE */
#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_pixar_utils.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

/* KERNEL */
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"
#include "KKE_version.h"

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


WABI_NAMESPACE_BEGIN

Global G;

/** User Prefs modifies this value globally. */
float UI_DPI_FAC = float(1.0f);

static void kraken_version_init()
{
  static std::string kraken_version_string;

  kraken_version_string = TfStringPrintf("%d.%02d.%d %s",
                                         KRAKEN_VERSION / 100,
                                         KRAKEN_VERSION % 100,
                                         KRAKEN_VERSION_PATCH,
                                         STRINGIFY(KRAKEN_VERSION_CYCLE));
  printf("Kraken v%s\n\n", CHARSTR(kraken_version_string));
}

static std::string kraken_get_version_decimal()
{
  return TfStringPrintf("%d.%02d", KRAKEN_VERSION / 100, KRAKEN_VERSION % 100);
}

Main *KKE_main_new(void)
{
  Main *cmain = new Main();
  return cmain;
}

void KKE_kraken_globals_init()
{
  kraken_version_init();

  memset(&G, 0, sizeof(Global));

  G.main = KKE_main_new();

  G.main->exe_path = kraken_exe_path_init();
  G.main->temp_dir = kraken_system_tempdir_path();

  G.main->kraken_version_decimal = kraken_get_version_decimal();

  G.main->datafiles_path = kraken_datafiles_path_init();
  G.main->python_path = kraken_python_path_init();
  G.main->icons_path = kraken_icon_path_init();
  G.main->stage_id = kraken_startup_file_init();
}

void KKE_kraken_main_init(cContext *C, int argc, const char **argv)
{
  /* Determine stage to load (from user or factory default). */
  if (!std::filesystem::exists(G.main->stage_id) ||
      G.main->stage_id.string().find("startup.usda") != std::string::npos)
  {
    G.factory_startup = true;
  }

  CTX_data_main_set(C, G.main);

  /* Init & embed python. */
  KKE_kraken_python_init(C);

  /** @em Always */
  UNI_create_stage(C);

  if (G.factory_startup)
  {
    /**
     * Create default Pixar stage. */
    // UNI_set_defaults(C);
    // UNI_save_stage(C);
  }
  else
  {
    /**
     * Open user's stage. */
    UNI_open_stage(C);
  }
}

WABI_NAMESPACE_END