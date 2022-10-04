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

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#ifndef __cplusplus
#  error This is a C++ header
#endif

#include "KLI_fileops.h"
#include "KLI_string_ref.hh"
#include "KLI_path_utils.h"

#include <fstream>
#include <string>

/**
 * Check whether given path ends with a blend file compatible extension
 * (`.usd`, `.usda`, `.usdz`, or `.usdc`).
 *
 * @param str: The path to check.
 * @return true is this path ends with a blender file extension.
 */
bool KLI_has_kfile_extension(const char *str);

namespace kraken
{

  /**
   * std::fstream subclass that handles UTF-16 encoding on Windows.
   *
   * For documentation, see https://en.cppreference.com/w/cpp/io/basic_fstream
   */
  class fstream : public std::fstream
  {
   public:

    fstream() = default;
    explicit fstream(const char *filepath,
                     std::ios_base::openmode mode = ios_base::in | ios_base::out);
    explicit fstream(const std::string &filepath,
                     std::ios_base::openmode mode = ios_base::in | ios_base::out);

    void open(StringRefNull filepath, ios_base::openmode mode = ios_base::in | ios_base::out);
  };

}  // namespace kraken
