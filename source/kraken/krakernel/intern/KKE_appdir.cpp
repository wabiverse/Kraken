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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <iostream>

#include "kraken/kraken.h"

#include "KLI_assert.h"
#include "KLI_path_utils.h"
#include "KLI_string.h"
#include "KLI_string_utils.h"
#include "KLI_fileops.h"

#include "KKE_api.h"
#include "KKE_appdir.h" /* extern C. */
#include "KKE_appdir.hh" /* own include */
#include "KKE_main.h"

#include "USD_api.h"

#include "ANCHOR_system_paths.h"

#include <wabi/base/arch/systemInfo.h>

namespace fs = std::filesystem;

/**
 * Recursively copies all files and folders from src_id to target_id and overwrites existing files
 * in target. */
std::string KKE_appdir_copy_recursive(const int src_id, const int target_id)
{
  fs::path src = KKE_appdir_folder_id(src_id, NULL);
  fs::path target = KKE_appdir_folder_id_create(target_id, NULL);

  if (!std::filesystem::exists(src) || !std::filesystem::exists(target)) {
    return std::string();
  }

  try {
    fs::copy(src, target, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
    return target;
  }

  catch (std::exception &e) {
    std::cout << e.what();
  }

  return std::string();
}
