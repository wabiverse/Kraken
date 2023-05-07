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
#include "KLI_system.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_scene.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLO_readfile.h"

#include "LUXO_access.h"
#include "LUXO_runtime.h"
#include "LUXO_types.h"

#include "USD_area.h"
#include "USD_file.h"
#include "USD_object.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_system.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "WM_files.h"
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

using namespace wabi;

static void handle_subversion_warning(Main *main, USDFileReadReport *reports)
{
  if (main->minversionfile > KRAKEN_FILE_VERSION ||
      (main->minversionfile == KRAKEN_FILE_VERSION &&
       main->minsubversionfile > KRAKEN_FILE_SUBVERSION)) {
    KKE_reportf(reports->reports,
                RPT_WARNING,
                "File written by newer Kraken binary (%d.%d), expect loss of data!",
                main->minversionfile,
                main->minsubversionfile);
  }
}

static void setup_app_userdef(USDFileData *usd)
{
  if (usd->user) {
    /* only here free userdef themes... */
    KKE_kraken_userdef_data_set_and_free(usd->user);
    usd->user = NULL;

    /* Security issue: any blend file could include a USER block.
     *
     * Currently we load prefs from BLENDER_STARTUP_FILE and later on load BLENDER_USERPREF_FILE,
     * to load the preferences defined in the users home dir.
     *
     * This means we will never accidentally (or maliciously)
     * enable scripts auto-execution by loading a '.blend' file.
     */
    U.flag |= USER_SCRIPT_AUTOEXEC_DISABLE;
  }
}

static void setup_app_usd_file_data(kContext *C,
                                      USDFileData *usd,
                                      const struct USDFileReadParams *params,
                                      USDFileReadReport *reports)
{
  if ((params->skip_flags & KLO_READ_SKIP_USERDEF) == 0) {
    setup_app_userdef(usd);
  }
  if ((params->skip_flags & KLO_READ_SKIP_DATA) == 0) {
    //setup_app_data(C, usd, params, reports);
  }
}

void KKE_usdfile_read_setup_ex(kContext *C,
                               USDFileData *usd,
                               const struct USDFileReadParams *params,
                               USDFileReadReport *reports,
                               /* Extra args. */
                               const bool startup_update_defaults,
                               const char *startup_app_template)
{
  if (startup_update_defaults) {
    if ((params->skip_flags & KLO_READ_SKIP_DATA) == 0) {
      KLO_update_defaults_startup_usd(usd->main, startup_app_template);
    }
  }
  setup_app_usd_file_data(C, usd, params, reports);
  MEM_delete(usd);
}

void KKE_usdfile_read_setup(kContext *C,
                              USDFileData *usd,
                              const struct USDFileReadParams *params,
                              USDFileReadReport *reports)
{
  KKE_usdfile_read_setup_ex(C, usd, params, reports, false, NULL);
}

struct USDFileData *KKE_usdfile_read(const char *filepath,
                                        const struct USDFileReadParams *params,
                                        USDFileReadReport *reports)
{
  USDFileData *usd = MEM_new<USDFileData>(__func__);

  /* Don't print startup file loading. */
  if (params->is_startup == false) {
    printf("Read usd: %s\n", filepath);
  }

  usd->sdf_handle = wabi::UsdStage::Open(filepath)->GetRootLayer();
  if (usd->sdf_handle) {
    handle_subversion_warning(usd->main, reports);
  } else {
    KKE_reports_prependf(reports->reports, "Loading '%s' failed: ", filepath);
  }
  return usd;
}


UserDef *KKE_usdfile_userdef_from_defaults(void)
{
  UserDef *userdef = (UserDef *)MEM_mallocN(sizeof(*userdef), __func__);
  memcpy(userdef, &U_default, sizeof(*userdef));

  /* Add-ons. */
#if 0
  {
    const char *addons[] = {
        "io_anim_bvh",
        "io_curve_svg",
        "io_mesh_ply",
        "io_mesh_stl",
        "io_mesh_uv_layout",
        "io_scene_fbx",
        "io_scene_gltf2",
        "io_scene_obj",
        "io_scene_x3d",
        "cycles",
        "pose_library",
    };
    for (int i = 0; i < ARRAY_SIZE(addons); i++) {
      kAddon *addon = KKE_addon_new();
      STRNCPY(addon->module, addons[i]);
      KLI_addtail(&userdef->addons, addon);
    }
  }
#endif /* Addons. */

  /* Theme. */
  {
    kTheme *ktheme = (kTheme *)MEM_mallocN(sizeof(*ktheme), __func__);
    memcpy(ktheme, &U_theme_default, sizeof(*ktheme));

    KLI_addtail(&userdef->themes, ktheme);
  }

#ifdef WITH_PYTHON_SECURITY
  /* use alternative setting for security nuts
   * otherwise we'd need to patch the binary blob - startup.blend.c */
  userdef->flag |= USER_SCRIPT_AUTOEXEC_DISABLE;
#else
  userdef->flag &= ~USER_SCRIPT_AUTOEXEC_DISABLE;
#endif

  /* System-specific fonts directory. */
  KKE_appdir_font_folder_default(userdef->fontdir);

  userdef->memcachelimit = min_ii(KLI_system_memory_max_in_megabytes_int() / 2,
                                  userdef->memcachelimit);

  /* Init weight paint range. */
  //KKE_colorband_init(&userdef->coba_weight, true);

  /* Default studio light. */
  //KKE_studiolight_default(userdef->light_param, userdef->light_ambient);

  //KKE_preferences_asset_library_default_add(userdef);

  return userdef;
}

struct USDFileData *KKE_usdfile_read_from_memory(const void *filebuf,
                                                 int filelength,
                                                 const struct USDFileReadParams *params,
                                                 ReportList *reports)
{
  USDFileData *usd = KLO_read_from_memory(filebuf, filelength, static_cast<eKLOReadSkip>(params->skip_flags), reports);
  if (usd) {
    /* Pass. */
  }
  else {
    KKE_reports_prepend(reports, "Loading failed: ");
  }
  return usd;
}
