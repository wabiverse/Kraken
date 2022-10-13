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

#include "KKE_api.h"
#include "KKE_main.h"
#include "KKE_global.h"
#include "KKE_appdir.h"

#include "KLI_time.h"
#include "KLI_string.h"

#include "CLG_log.h"

#include <wabi/wabi.h>

#include <wabi/base/tf/stackTrace.h>
#include <wabi/base/arch/attributes.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/plug/info.h>
#include <wabi/base/plug/registry.h>

static CLG_LogRef LOG = {"kke.plugins"};

WABI_NAMESPACE_USING

static char maelstrom_version_string[48] = "";

static void maelstrom_version_init()
{
  const char *version_cycle = "";
  if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "alpha")) {
    version_cycle = " Alpha";
  }
  else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "beta")) {
    version_cycle = " Beta";
  }
  else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "rc")) {
    version_cycle = " Release Candidate";
  }
  else if (STREQ(STRINGIFY(KRAKEN_VERSION_CYCLE), "release")) {
    version_cycle = "";
  }
  else {
    KLI_assert_msg(0, "Invalid Maelstrom version cycle");
  }

  KLI_snprintf(maelstrom_version_string,
               ARRAY_SIZE(maelstrom_version_string),
               "%d.%01d.%d%s",
               WABI_VERSION / 1000,
               WABI_VERSION % 1000,
               WABI_VERSION_PATCH,
               version_cycle);
}


void KKE_kraken_plugins_init()
{
  static bool plugin_path_registered = false;
  if (plugin_path_registered) {
    return;
  }
  plugin_path_registered = true;

  /**
   * Tell USD which directory to search for its JSON files. If 'datafiles/maelstrom'
   * does not exist, USD will not be able to read or write any files. 
   * @NOTE: we need datafiles/maelstrom and datafiles/plugin/maelstrom respectively. */
  const std::string kraken_maelstrom_datafiles = KKE_appdir_folder_id(KRAKEN_DATAFILES, "maelstrom");
  const std::string kraken_plugin_datafiles = KKE_appdir_folder_id(KRAKEN_DATAFILES, "plugin");

  /* The trailing slash indicates to USD that the path is a directory. */
  PlugRegistry::GetInstance().RegisterPlugins(kraken_maelstrom_datafiles + "/");
  PlugRegistry::GetInstance().RegisterPlugins(kraken_plugin_datafiles + "/maelstrom/");

  KLI_pretty_time(TfGetAppLaunchTime(), G.main->launch_time);
  maelstrom_version_init();

  CLOG_INFO(&LOG, 0, "Kraken Awakens: %s", G.main->launch_time);
  CLOG_INFO(&LOG, 0, "Pixar Universe: Maelstrom v%s", maelstrom_version_string);
}

