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

#include "KLI_fileops.h"
#include "KLI_path_utils.h"

#ifdef WIN32
# include "utfconv.h"
#endif /* WIN32 */

#include <filesystem>

WABI_NAMESPACE_BEGIN

bool KLI_exists(const fs::path &path)
{
  return fs::exists(path);
}

fs::file_status KLI_type(const fs::path &path)
{
  return fs::status(path);
}

/**
 * Does the specified path point to a directory? */
bool KLI_is_dir(const char *file)
{
  return KLI_ISDIR(KLI_type(file));
}

/**
 * Does the specified path point to a non-directory? */
bool KLI_is_file(const char *path)
{
  return (KLI_exists(path) && !KLI_ISDIR(KLI_type(path)));
}

WABI_NAMESPACE_END