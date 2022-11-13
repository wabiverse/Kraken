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
#include "KKE_report.h"
#include "KKE_scene.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

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

static void handle_subversion_warning(Main *main, KrakenFileReadReport *reports)
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

static void KKE_usdfile_read_setup_ex(kContext *C,
                                      KrakenFileData *kfd,
                                      const struct KrakenFileReadParams *params,
                                      KrakenFileReadReport *reports,
                                      /* Extra args. */
                                      const bool startup_update_defaults,
                                      const char *startup_app_template)
{
  if (startup_update_defaults) {
    if ((params->skip_flags & KLO_READ_SKIP_DATA) == 0) {
      KLO_update_defaults_startup_blend(bfd->main, startup_app_template);
    }
  }
  setup_app_blend_file_data(C, bfd, params, reports);
  MEM_delete(kfd);
}

void KKE_usdfile_read_setup(kContext *C,
                              KrakenFileData *bfd,
                              const struct KrakenFileReadParams *params,
                              KrakenFileReadReport *reports)
{
  KKE_usdfile_read_setup_ex(C, bfd, params, reports, false, NULL);
}

struct KrakenFileData *KKE_usdfile_read(const char *filepath,
                                        const struct KrakenFileReadParams *params,
                                        KrakenFileReadReport *reports)
{
  KrakenFileData *kfd = MEM_new<KrakenFileData>(__func__);

  /* Don't print startup file loading. */
  if (params->is_startup == false) {
    printf("Read usd: %s\n", filepath);
  }

  kfd->sdf_handle = wabi::UsdStage::Open(filepath)->GetRootLayer();
  if (kfd->sdf_handle) {
    handle_subversion_warning(kfd->main, reports);
  } else {
    KKE_reports_prependf(reports->reports, "Loading '%s' failed: ", filepath);
  }
  return kfd;
}
