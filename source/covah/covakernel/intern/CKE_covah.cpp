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

/* STDLIB */
#include <fstream>
#include <string>

#ifdef __GNUC__
#  include <cstdlib>
#endif /*__GNUC__ */

/* KERNEL */
#include "CKE_context.h"
#include "CKE_main.h"
#include "CKE_utils.h"
#include "CKE_version.h"

/* ANCHOR */
#include "ANCHOR_api.h"
#include "ANCHOR_debug_codes.h"

/* WINDOW MANAGER */
#include "WM_init_exit.h"

/* UNIVERSE */
#include "UNI_context.h"
#include "UNI_pixar_utils.h"
#include "UNI_window.h"

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

static void covah_version_init()
{
  static std::string covah_version_string;

  covah_version_string = TfStringPrintf("%d.%02d.%d %s",
                                        COVAH_VERSION / 100,
                                        COVAH_VERSION % 100,
                                        COVAH_VERSION_PATCH,
                                        STRINGIFY(COVAH_VERSION_CYCLE));
  printf("Covah v%s\n\n", CHARSTR(covah_version_string));
}

static std::string covah_get_version_decimal()
{
  return TfStringPrintf("%d.%02d", COVAH_VERSION / 100, COVAH_VERSION % 100);
}

Main CKE_main_new(void)
{
  Main cmain = TfCreateRefPtr(new CovahMain());
  return cmain;
}

void CKE_covah_globals_init()
{
  covah_version_init();

  memset(&G, 0, sizeof(Global));

  G.main = CKE_main_new();

  G.main->exe_path = covah_exe_path_init();
  G.main->temp_dir = covah_system_tempdir_path();

  G.main->datafiles_path = covah_datafiles_path_init(G);
  G.main->python_path = covah_python_path_init(G);
  G.main->icons_path = covah_icon_path_init(G);
  G.main->styles_path = covah_styles_path_init(G);
  G.main->stage_id = covah_startup_file_init(G);
  G.main->covah_version_decimal = covah_get_version_decimal();
}

void CKE_covah_main_init(const cContext &C, int argc, const char **argv)
{
  /* Determine stage to load (from user or factory default). */
  if (!std::filesystem::exists(G.main->stage_id) ||
      G.main->stage_id.string().find("startup.usda") != std::string::npos)
  {
    G.factory_startup = true;
  }

  CTX_data_main_set(C, G.main);

  /* Init & embed python. */
  CKE_covah_python_init(C);

  /** @em Always */
  UNI_create_stage(C);

  if (G.factory_startup)
  {
    /**
     * Create default Pixar stage. */
    UNI_set_defaults(C);
    UNI_author_default_scene(C);
    UNI_save_stage(C);
  }
  else
  {
    /**
     * Open user's stage. */
    UNI_open_stage(C);
  }
}

WABI_NAMESPACE_END