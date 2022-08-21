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

#include "KLI_fileops.h"
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

WABI_NAMESPACE_BEGIN

bool KLI_exists(const fs::path &path)
{
  return fs::exists(path);
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
int KLI_fstat(int fd, stat *buffer)
{
  // return fstat(fd, buffer);
  return 0;
}

int KLI_stat(const char *path, stat *buffer)
{
  // return stat(path, buffer);
  return 0;
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