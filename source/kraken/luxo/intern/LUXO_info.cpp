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
 * Luxo.
 * The Universe Gets Animated.
 */

#include "kraken/kraken.h"

#include "KLI_path_utils.h"

#include "KKE_global.h"
#include "KKE_appdir.h"

#include "IMB_imbuf.h"
#include "IMB_colormanagement.h"

#include "LUXO_main.h"
#include "LUXO_internal.h"
#include "LUXO_access.h"

#include <wabi/usd/usdGeom/tokens.h>
#include <wabi/imaging/hdx/tokens.h>


/* not technically a prim, but meh. */
static void prim_def_config(const KrakenSTAGE &kstage)
{
  const char *configdir;
  char configfile[FILE_MAX];

#if 0
  const char *ocio_env;

  /**
   * @TODO: Support ocio from env var, to set on the pixar stage. */

  ocio_env = KLI_getenv("OCIO");

  if (ocio_env && ocio_env[0] != '\0') {
    config = OCIO_configCreateFromEnv();
    if (config != NULL) {
      printf("Color management: Using %s as a configuration file\n", ocio_env);
    }
  }
#endif

  configdir = KKE_appdir_folder_id(KRAKEN_DATAFILES, "colormanagement");

  if (configdir) {
    KLI_join_dirfile(configfile, sizeof(configfile), configdir, KCM_CONFIG_FILE);
  }

  kstage->GetRootLayer()->SetDocumentation(KRAKEN_FILE_VERSION_HEADER);
  kstage->SetColorConfiguration(wabi::SdfAssetPath(configfile));
  kstage->SetColorManagementSystem(wabi::HdxColorCorrectionTokens->openColorIO);
  kstage->SetMetadata(wabi::UsdGeomTokens->upAxis, wabi::UsdGeomTokens->z);
}

void PRIM_def_info(const KrakenSTAGE &kstage)
{
  prim_def_config(kstage);
}
