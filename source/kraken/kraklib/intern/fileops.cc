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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_fileops.hh"

#ifdef WIN32
#  include "utfconv.h"
#endif

namespace kraken
{
  fstream::fstream(const char *filepath, std::ios_base::openmode mode)
  {
    this->open(filepath, mode);
  }

  fstream::fstream(const std::string &filepath, std::ios_base::openmode mode)
  {
    this->open(filepath, mode);
  }

  void fstream::open(StringRefNull filepath, ios_base::openmode mode)
  {
#ifdef WIN32
    const char *filepath_cstr = filepath.c_str();
    UTF16_ENCODE(filepath_cstr);
    std::wstring filepath_wstr(filepath_cstr_16);
    std::fstream::open(filepath_wstr.c_str(), mode);
    UTF16_UN_ENCODE(filepath_cstr);
#else
    std::fstream::open(filepath, mode);
#endif
  }

}  // namespace kraken

bool KLI_has_kfile_extension(const char *str)
{
  const char *ext_test[5] = {".usd", ".usda", ".usdz", ".usdc", nullptr};
  return KLI_path_extension_check_array(str, ext_test);
}
