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

#include "KKE_main.h"
#include "KKE_version.h"

#include "UNI_pixar_utils.h"

#include <wabi/usd/usd/stage.h>
#include <wabi/usd/ar/resolver.h>

WABI_NAMESPACE_BEGIN


void UNI_pixutil_convert_usd(const fs::path &path, const TfToken &format, bool verbose)
{
  const fs::path usda_path = STRCAT(path.parent_path().string(), "/" + path.stem().string() + ".usda");

  /**
   * Setup File Formatting Args. */
  SdfFileFormat::FileFormatArguments args;
  args[UsdUsdFileFormatTokens->FormatArg] = format;

  /**
   * Open the path as an SdfLayer & Convert. */
  SdfLayerRefPtr layer = SdfLayer::FindOrOpen(path.string());
  const bool success = layer->Export(usda_path.string(),
                                     TfStringPrintf("Kraken v%d.%d",
                                                    KRAKEN_VERSION_MAJOR,
                                                    KRAKEN_VERSION_MINOR),
                                     args);

  if(verbose) {
    if(success && fs::exists(usda_path)) {
        TF_SUCCESS_MSG("Converted new file: %s", CHARALL(usda_path.string()));
      return;
    }

    TF_ERROR_MSG("Could not convert file %s", CHARALL(path.string()));
  }
}


std::string UNI_pixutil_resolve_asset(const std::string &asset, bool verbose)
{
  ArResolver &resolver = ArGetResolver();

  const std::string resolved_asset = resolver.Resolve(asset);

  if(verbose) {
    if(!resolved_asset.empty()) {
      TF_SUCCESS_MSG("Asset Resolved Path: %s", CHARALL(resolved_asset));
    } else {
      TF_ERROR_MSG("Asset %s does not exist.", CHARALL(asset));
    }
  }

  return resolved_asset;
}


WABI_NAMESPACE_END