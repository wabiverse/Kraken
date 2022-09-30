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

#include <kraken/kraken.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/stat.h>

#if defined(__NetBSD__) || defined(__DragonFly__) || defined(__HAIKU__)
/* Other modern unix OS's should probably use this also. */
#  include <sys/statvfs.h>
#  define USE_STATFS_STATVFS
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || \
    defined(__DragonFly__)
/* For statfs */
#  include <sys/mount.h>
#  include <sys/param.h>
#endif

#if defined(__linux__) || defined(__hpux) || defined(__GNU__) || defined(__GLIBC__)
#  include <sys/vfs.h>
#endif

#include <fcntl.h>
#include <string.h> /* `strcpy` etc. */

#ifdef WIN32
#  include "KLI_string_utf8.h"
#  include "KLI_winstuff.h"
#  include "utfconv.h"
#  include <ShObjIdl.h>
#  include <direct.h>
#  include <io.h>
#  include <stdbool.h>
#else
#  include <pwd.h>
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

#include "KLI_fileops.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"
#include "KLI_path_utils.h"

#ifdef WIN32
#  include "utfconv.h"
#  include <io.h>
#else /* WIN32 */
#  include <sys/stat.h>
#  include <stdlib.h>
#  include <stdio.h>
#endif

#include <filesystem>

KRAKEN_NAMESPACE_BEGIN

int KLI_exists(const char *path)
{
#if defined(WIN32)
  KLI_stat_t st;
  wchar_t *tmp_16 = alloc_utf16_from_8(path, 1);
  int len, res;

  len = wcslen(tmp_16);
  /* in Windows #stat doesn't recognize dir ending on a slash
   * so we remove it here */
  if ((len > 3) && ELEM(tmp_16[len - 1], L'\\', L'/')) {
    tmp_16[len - 1] = '\0';
  }
  /* two special cases where the trailing slash is needed:
   * 1. after the share part of a UNC path
   * 2. after the C:\ when the path is the volume only
   */
  if ((len >= 3) && (tmp_16[0] == L'\\') && (tmp_16[1] == L'\\')) {
    KLI_path_normalize_unc_16(tmp_16);
  }

  if ((tmp_16[1] == L':') && (tmp_16[2] == L'\0')) {
    tmp_16[2] = L'\\';
    tmp_16[3] = L'\0';
  }

  res = KLI_wstat(tmp_16, &st);

  free(tmp_16);
  if (res == -1) {
    return 0;
  }
#else
  struct stat st;
  KLI_assert(!KLI_path_is_rel(path));
  if (stat(path, &st)) {
    return 0;
  }
#endif
  return (st.st_mode);
}

#ifdef WIN32
int KLI_fstat(int fd, KLI_stat_t *buffer)
{
#  if defined(_MSC_VER)
  return _fstat64(fd, buffer);
#  else
  return _fstat(fd, buffer);
#  endif
}

int KLI_stat(const char *path, KLI_stat_t *buffer)
{
  int r;
  UTF16_ENCODE(path);

  r = KLI_wstat(path_16, buffer);

  UTF16_UN_ENCODE(path);
  return r;
}

int KLI_wstat(const wchar_t *path, KLI_stat_t *buffer)
{
#  if defined(_MSC_VER)
  return _wstat64(path, buffer);
#  else
  return _wstat(path, buffer);
#  endif
}
#else
int KLI_fstat(int fd, struct stat *buffer)
{
  return fstat(fd, buffer);
}

int KLI_stat(const char *path, struct stat *buffer)
{
  return stat(path, buffer);
}
#endif

int64_t KLI_ftell(FILE *stream)
{
#ifdef WIN32
  return _ftelli64(stream);
#else
  return ftell(stream);
#endif
}

int KLI_fseek(FILE *stream, int64_t offset, int whence)
{
#ifdef WIN32
  return _fseeki64(stream, offset, whence);
#else
  return fseek(stream, offset, whence);
#endif
}

int64_t KLI_lseek(int fd, int64_t offset, int whence)
{
#ifdef WIN32
  return _lseeki64(fd, offset, whence);
#else
  return lseek(fd, offset, whence);
#endif
}

std::filesystem::file_status KLI_type(const std::filesystem::path &path)
{
  return std::filesystem::status(path);
}

/**
 * Does the specified path point to a directory? */
bool KLI_is_dir(const char *file)
{
  return S_ISDIR(KLI_exists(file));
}

/**
 * Does the specified path point to a non-directory? */
bool KLI_is_file(const char *path)
{
  const int mode = KLI_exists(path);
  return (mode && !S_ISDIR(mode));
}

KRAKEN_NAMESPACE_END