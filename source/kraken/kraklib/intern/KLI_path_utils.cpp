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
#include "KLI_fileops.h"
#include "KLI_string_utils.h"
#include "KLI_path_utils.h"

#include <cstring>
#include <stdarg.h>
#include <string>

#ifdef WIN32
#  include "utfconv.h"
#  include <io.h>
#  ifdef _WIN32_IE
#    undef _WIN32_IE
#  endif
#  define _WIN32_IE 0x0501
#  include <shlobj.h>
#  include <windows.h>
#else /* non windows */
#  ifdef WITH_BINRELOC
#    include "binreloc.h"
#  endif
/* #mkdtemp on OSX (and probably all *BSD?), not worth making specific check for this OS. */
#  include <unistd.h>
#endif /* WIN32 */

#include <wabi/base/arch/hints.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/pathUtils.h>

WABI_NAMESPACE_BEGIN

/**
 * Get an env var, result has to be used immediately.
 *
 * On windows #getenv gets its variables from a static copy of the environment variables taken at
 * process start-up, causing it to not pick up on environment variables created during runtime.
 * This function uses an alternative method to get environment variables that does pick up on
 * runtime environment variables. The result will be UTF-8 encoded. */
const char *KLI_getenv(const char *env)
{
#ifdef _MSC_VER
  const char *result = NULL;
  /* 32767 is the maximum size of the environment variable on windows,
   * reserve one more character for the zero terminator. */
  static wchar_t buffer[32768];
  wchar_t *env_16 = alloc_utf16_from_8(env, 0);
  if (env_16) {
    if (GetEnvironmentVariableW(env_16, buffer, ARRAY_SIZE(buffer))) {
      char *res_utf8 = alloc_utf_8_from_16(buffer, 0);
      /* Make sure the result is valid, and will fit into our temporary storage buffer. */
      if (res_utf8) {
        if (strlen(res_utf8) + 1 < sizeof(buffer)) {
          /* We are re-using the utf16 buffer here, since allocating a second static buffer to
           * contain the UTF-8 version to return would be wasteful. */
          memcpy(buffer, res_utf8, strlen(res_utf8) + 1);
          result = (const char *)buffer;
        }
        free(res_utf8);
      }
    }
  }
  return result;
#else
  return getenv(env);
#endif
}

/**
 * Simple appending of filename to dir, does not check for valid path!
 * Puts result into `dst`, which may be same area as `dir`. */
void KLI_join_dirfile(char *__restrict dst,
                      const size_t maxlen,
                      const char *__restrict dir,
                      const char *__restrict file)
{
#ifdef DEBUG_STRSIZE
  memset(dst, 0xff, sizeof(*dst) * maxlen);
#endif
  size_t dirlen = KLI_strnlen(dir, maxlen);

  /* args can't match */
  KLI_assert((dst != dir) && (dst != file));

  if (dirlen == maxlen) {
    memcpy(dst, dir, dirlen);
    dst[dirlen - 1] = '\0';
    return; /* dir fills the path */
  }

  memcpy(dst, dir, dirlen + 1);

  if (dirlen + 1 >= maxlen) {
    return; /* fills the path */
  }

  /* inline KLI_path_slash_ensure */
  if ((dirlen > 0) && !((dst[dirlen - 1] != SEP) && (dst[dirlen - 1] != ALTSEP))) {
    dst[dirlen++] = SEP;
    dst[dirlen] = '\0';
  }

  if (dirlen >= maxlen) {
    return; /* fills the path */
  }

  KLI_strncpy(dst + dirlen, file, maxlen - dirlen);
}
/**
 * Joins infinite strings into @param dst,
 * ensuring only a single path separator between each. */
size_t KLI_path_join(char *__restrict dst, const size_t dst_len, const char *path, ...)
{
#ifdef DEBUG_STRSIZE
  memset(dst, 0xff, sizeof(*dst) * dst_len);
#endif
  if (ARCH_UNLIKELY(dst_len == 0)) {
    return 0;
  }
  const size_t dst_last = dst_len - 1;
  size_t ofs = KLI_strncpy_rlen(dst, path, dst_len);

  if (ofs == dst_last) {
    return ofs;
  }

  /* remove trailing slashes, unless there are _only_ trailing slashes
   * (allow "//" as the first argument). */
  bool has_trailing_slash = false;
  if (ofs != 0) {
    size_t len = ofs;
    while ((len != 0) && ((path[len - 1] == SEP) || (path[len - 1] == ALTSEP))) {
      len -= 1;
    }
    if (len != 0) {
      ofs = len;
    }
    has_trailing_slash = (path[len] != '\0');
  }

  va_list args;
  va_start(args, path);
  while ((path = (const char *)va_arg(args, const char *))) {
    has_trailing_slash = false;
    const char *path_init = path;
    while ((path[0] == SEP) || (path[0] == ALTSEP)) {
      path++;
    }
    size_t len = strlen(path);
    if (len != 0) {
      while ((len != 0) && ((path[len - 1] == SEP) || path[len - 1] == ALTSEP)) {
        len -= 1;
      }

      if (len != 0) {
        /* the very first path may have a slash at the end */
        if (ofs && ((dst[ofs - 1] != SEP) || (dst[ofs - 1] != ALTSEP))) {
          dst[ofs++] = SEP;
          if (ofs == dst_last) {
            break;
          }
        }
        has_trailing_slash = (path[len] != '\0');
        if (ofs + len >= dst_last) {
          len = dst_last - ofs;
        }
        memcpy(&dst[ofs], path, len);
        ofs += len;
        if (ofs == dst_last) {
          break;
        }
      }
    }
    else {
      has_trailing_slash = (path_init != path);
    }
  }
  va_end(args);

  if (has_trailing_slash) {
    if ((ofs != dst_last) && (ofs != 0) && ((dst[ofs - 1] == SEP) || (dst[ofs - 1] == ALTSEP) == 0)) {
      dst[ofs++] = SEP;
    }
  }

  KLI_assert(ofs <= dst_last);
  dst[ofs] = '\0';

  return ofs;
}

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

/**
 * Append a filename to a dir, ensuring slash separates.
 */
void KLI_path_append(char *__restrict dst, const size_t maxlen, const char *__restrict file)
{
  size_t dirlen = KLI_strnlen(dst, maxlen);

  /* inline BLI_path_slash_ensure */
  if ((dirlen > 0) && (dst[dirlen - 1] != SEP)) {
    dst[dirlen++] = SEP;
    dst[dirlen] = '\0';
  }

  if (dirlen >= maxlen) {
    return; /* fills the path */
  }

  KLI_strncpy(dst + dirlen, file, maxlen - dirlen);
}

/**
 * Search for a binary (executable) */
bool KLI_path_program_search(char *fullname, const size_t maxlen, const char *name)
{
#ifdef DEBUG_STRSIZE
  memset(fullname, 0xff, sizeof(*fullname) * maxlen);
#endif
  const char *path;
  bool retval = false;

#ifdef _WIN32
  const char separator = ';';
#else
  const char separator = ':';
#endif

  path = KLI_getenv("PATH");
  if (path) {
    char filename[FILE_MAX];
    const char *temp;

    do {
      temp = strchr(path, separator);
      if (temp) {
        memcpy(filename, path, temp - path);
        filename[temp - path] = 0;
        path = temp + 1;
      }
      else {
        KLI_strncpy(filename, path, sizeof(filename));
      }

      KLI_path_append(filename, maxlen, name);
      if (
#ifdef _WIN32
          KLI_path_program_extensions_add_win32(filename, maxlen)
#else
          KLI_exists(filename)
#endif
      ) {
        KLI_strncpy(fullname, filename, maxlen);
        retval = true;
        break;
      }
    } while (temp);
  }

  if (retval == false) {
    *fullname = '\0';
  }

  return retval;
}

/**
 * Returns pointer to the rightmost path separator in string. */
const char *KLI_path_slash_rfind(const char *string)
{
  const char *const lfslash = strrchr(string, '/');
  const char *const lbslash = strrchr(string, '\\');

  if (!lfslash) {
    return lbslash;
  }
  if (!lbslash) {
    return lfslash;
  }

  return (lfslash > lbslash) ? lfslash : lbslash;
}

/**
 * Removes the last slash and everything after it to the end of string, if there is one. */
void KLI_path_slash_rstrip(char *string)
{
  int len = strlen(string);
  while (len) {
    if (string[len - 1] == SEP) {
      string[len - 1] = '\0';
      len--;
    }
    else {
      break;
    }
  }
}

#ifdef _WIN32
/**
 * Tries appending each of the semicolon-separated extensions in the PATHEXT
 * environment variable (Windows-only) onto `name` in turn until such a file is found.
 * Returns success/failure.
 */
bool KLI_path_program_extensions_add_win32(char *name, const size_t maxlen)
{
  bool retval = false;
  fs::file_status type;

  type = KLI_type(name);
  if (KLI_exists(name) || KLI_ISDIR(type)) {
    /* typically 3-5, ".EXE", ".BAT"... etc */
    const int ext_max = 12;
    const char *ext = KLI_getenv("PATHEXT");
    if (ext) {
      const int name_len = strlen(name);
      char *filename = (char *)alloca(name_len + ext_max);
      char *filename_ext;
      const char *ext_next;

      /* null terminated in the loop */
      memcpy(filename, name, name_len);
      filename_ext = filename + name_len;

      do {
        int ext_len;
        ext_next = strchr(ext, ';');
        ext_len = ext_next ? ((ext_next++) - ext) : strlen(ext);

        if (ARCH_LIKELY(ext_len < ext_max)) {
          memcpy(filename_ext, ext, ext_len);
          filename_ext[ext_len] = '\0';

          type = KLI_type(filename);
          if (KLI_exists(name) && (!KLI_ISDIR(type))) {
            retval = true;
            KLI_strncpy(name, filename, maxlen);
            break;
          }
        }
      } while ((ext = ext_next));
    }
  }
  else {
    retval = true;
  }

  return retval;
}
#endif /* WIN32 */

WABI_NAMESPACE_END