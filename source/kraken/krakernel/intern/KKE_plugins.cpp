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

#include "KKE_api.h"
#include "KKE_main.h"

#include "KLI_time.h"
#include "KLI_string_utils.h"

#include <wabi/wabi.h>

#include <wabi/base/tf/stackTrace.h>
#include <wabi/base/arch/attributes.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/plug/info.h>
#include <wabi/base/plug/registry.h>

WABI_NAMESPACE_BEGIN

static char maelstrom_version_string[48] = "";

void KKE_kraken_plugins_init()
{
  PlugRegistry::GetInstance().RegisterPlugins(STRCAT(G.main->datafiles_path, "maelstrom/"));
  PlugRegistry::GetInstance().RegisterPlugins(STRCAT(G.main->datafiles_path, "plugin/maelstrom/"));

  KLI_pretty_time(TfGetAppLaunchTime(), G.main->launch_time);
  KLI_snprintf(maelstrom_version_string,
               ARRAY_SIZE(maelstrom_version_string),
               "%d.%d",
               WABI_VERSION_DECIMAL / 10000,
               WABI_VERSION_PATCH);

  TF_MSG_SUCCESS("Kraken Awakens | %s", G.main->launch_time);
  TF_MSG("Pixar Universe | Maelstrom v%s", maelstrom_version_string);
  printf("\n");
}

WABI_NAMESPACE_END