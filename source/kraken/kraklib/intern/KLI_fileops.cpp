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

#include <stdlib.h> /* malloc */
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#include <zlib.h>

#ifdef WIN32
#  include <windows.h>

#  include "utfconv.h"
#  include <io.h>
#  include <shellapi.h>
#  include <shobjidl.h>
#else
#  if defined(__APPLE__)
#    include <CoreFoundation/CoreFoundation.h>
#    include <objc/message.h>
#    include <objc/runtime.h>
#  endif
#  include <dirent.h>
#  include <sys/param.h>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

#include "KLI_fileops.h"
#include "KLI_path_utils.h"
#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

WABI_NAMESPACE_BEGIN

#ifdef WIN32

/** @return true on success (i.e. given path now exists on FS), false otherwise. */
bool KLI_dir_create_recursive(const char *dirname)
{
  char *lslash;
  char tmp[MAXPATHLEN];
  bool ret = true;

  /* First remove possible slash at the end of the dirname.
   * This routine otherwise tries to create
   * blah1/blah2/ (with slash) after creating
   * blah1/blah2 (without slash) */

  KLI_strncpy(tmp, dirname, sizeof(tmp));
  KLI_path_slash_rstrip(tmp);

  /* check special case "c:\foo", don't try create "c:", harmless but prints an error below */
  if (isalpha(tmp[0]) && (tmp[1] == ':') && tmp[2] == '\0') {
    return true;
  }

  if (KLI_is_dir(tmp)) {
    return true;
  }
  else if (KLI_exists(tmp)) {
    return false;
  }

  lslash = (char *)KLI_path_slash_rfind(tmp);

  if (lslash) {
    /* Split about the last slash and recurse */
    *lslash = 0;
    if (!KLI_dir_create_recursive(tmp)) {
      ret = false;
    }
  }

  if (ret && dirname[0]) { /* patch, this recursive loop tries to create a nameless directory */
    if (umkdir(dirname) == -1) {
      printf("Unable to create directory %s\n", dirname);
      ret = false;
    }
  }
  return ret;
}

#else /* WIN32 */

/** \return true on success (i.e. given path now exists on FS), false otherwise. */
bool KLI_dir_create_recursive(const char *dirname)
{
  char *lslash;
  size_t size;
#  ifdef MAXPATHLEN
  char static_buf[MAXPATHLEN];
#  endif
  char *tmp;
  bool ret = true;

  if (KLI_is_dir(dirname)) {
    return true;
  }
  if (KLI_exists(dirname)) {
    return false;
  }

#  ifdef MAXPATHLEN
  size = MAXPATHLEN;
  tmp = static_buf;
#  else
  size = strlen(dirname) + 1;
  tmp = calloc(size, __func__);
#  endif

  KLI_strncpy(tmp, dirname, size);

  /* Avoids one useless recursion in case of '/foo/bar/' path... */
  KLI_path_slash_rstrip(tmp);

  lslash = (char *)KLI_path_slash_rfind(tmp);
  if (lslash) {
    /* Split about the last slash and recurse */
    *lslash = 0;
    if (!KLI_dir_create_recursive(tmp)) {
      ret = false;
    }
  }

#  ifndef MAXPATHLEN
  free(tmp);
#  endif

  if (ret) {
    ret = (mkdir(dirname, 0777) == 0);
  }
  return ret;
}

#endif /* __linux__ || _APPLE */

WABI_NAMESPACE_END