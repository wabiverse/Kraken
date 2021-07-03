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
 * KRAKEN Library.
 * Gadget Vault.
 */

#pragma once

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_string_utils.h"

#include <cstring>
#include <stdarg.h>
#include <string>

WABI_NAMESPACE_BEGIN

static bool path_extension_check_ex(const char *str,
                                    const size_t str_len,
                                    const char *ext,
                                    const size_t ext_len)
{
  KLI_assert(strlen(str) == str_len);
  KLI_assert(strlen(ext) == ext_len);

  return (((str_len == 0 || ext_len == 0 || ext_len >= str_len) == 0) &&
          (KLI_strcasecmp(ext, str + str_len - ext_len) == 0));
}

bool KLI_path_extension_check(const char *str, const char *ext)
{
  return path_extension_check_ex(str, strlen(str), ext, strlen(ext));
}

bool KLI_path_extension_check_n(const char *str, ...)
{
  const size_t str_len = strlen(str);

  va_list args;
  const char *ext;
  bool ret = false;

  va_start(args, str);

  while ((ext = (const char *)va_arg(args, void *)))
  {
    if (path_extension_check_ex(str, str_len, ext, strlen(ext)))
    {
      ret = true;
      break;
    }
  }

  va_end(args);

  return ret;
}

/* does str end with any of the suffixes in *ext_array. */
bool KLI_path_extension_check_array(const std::string &str, const char **ext_array)
{
  const size_t str_len = str.length();
  int i = 0;

  while (ext_array[i])
  {
    if (path_extension_check_ex(str.c_str(), str_len, ext_array[i], strlen(ext_array[i])))
    {
      return true;
    }

    i++;
  }
  return false;
}

bool KLI_has_pixar_extension(const std::string &str)
{
  const char *ext_test[5] = {".usd", ".usda", ".usdc", ".usdz", NULL};
  return KLI_path_extension_check_array(str, ext_test);
}

WABI_NAMESPACE_END