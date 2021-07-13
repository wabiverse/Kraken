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
 * Universe.
 * Set the Stage.
 */

#include "UNI_pixar_utils.h"

#include <wabi/base/tf/diagnostic.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/usd/ar/resolver.h>

WABI_NAMESPACE_BEGIN


void UNI_pixutil_convert_usda(const fs::path &path, bool verbose)
{
  const fs::path usda_path = STRCAT(path.parent_path().string(), "/" + path.stem().string() + ".usda");

  UsdStageRefPtr stage = UsdStage::Open(path.string());

  const bool success = stage->Export(usda_path.string());

  if(verbose) {
    if(success && fs::exists(usda_path)) {
        TF_SUCCESS_MSG("Converted new file: %s", CHARALL(usda_path.string()));
      return;
    }

    TF_ERROR_MSG("Could not convert file %s", CHARALL(path.string()));
  }
}


std::string UNI_pixutil_resolve_asset(const fs::path &path, bool verbose)
{
  ArResolver &resolver = ArGetResolver();

  const std::string resolved_path = resolver.Resolve(path.string());

  if(verbose) {
    if(!resolved_path.empty()) {
      TF_SUCCESS_MSG("Asset Resolved Path: %s", CHARALL(resolved_path));
    } else {
      TF_ERROR_MSG("Asset %s does not exist.", CHARALL(path.string()));
    }
  }

  return resolved_path;
}


WABI_NAMESPACE_END