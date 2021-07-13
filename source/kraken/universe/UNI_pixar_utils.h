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

#pragma once

#include "UNI_api.h"

#include <wabi/base/tf/hash.h>
#include <wabi/base/tf/hashmap.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>

namespace fs = std::filesystem;

WABI_NAMESPACE_BEGIN

/**
 * Converts a given binary usd file (usd|usdc|usdz) to a human readable
 * (.usda) file to the same directory as the binary usd file passed in.
 * @param path: filepath to (usd|usdc|usdz) file.
 * @param verbose: whether to log status to console. */
void UNI_pixutil_convert_usda(const fs::path &path, bool verbose = false);

/**
 * Uses Pixar's Asset Resolver to resolve a path to a given asset path.
 * @param path: path to asset to preform Asset Resolution.
 * @param verbose: whether to log status to console.
 * @returns the resolved path or empty string if the asset does not exist. */
std::string UNI_pixutil_resolve_asset(const std::string &path, bool verbose = false);

WABI_NAMESPACE_END