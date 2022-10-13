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

#include <stdlib.h>
#include <string.h>

#include "USD_userdef_types.h"

#include "KLI_fnmatch.h"
#include "KLI_path_utils.h"
#include "KLI_utildefines.h"

#ifdef WIN32
#  include "KLI_string.h"
#endif

bool KKE_autoexec_match(const char *path)
{
  kPathCompare *path_cmp;

#ifdef WIN32
  const int fnmatch_flags = FNM_CASEFOLD;
#else
  const int fnmatch_flags = 0;
#endif

  KLI_assert((U.flag & USER_SCRIPT_AUTOEXEC_DISABLE) == 0);

  for (path_cmp = U.autoexec_paths.first; path_cmp; path_cmp = path_cmp->next) {
    if (path_cmp->path[0] == '\0') {
      /* pass */
    } else if (path_cmp->flag & USER_PATHCMP_GLOB) {
      if (fnmatch(path_cmp->path, path, fnmatch_flags) == 0) {
        return true;
      }
    } else if (KLI_path_ncmp(path_cmp->path, path, strlen(path_cmp->path)) == 0) {
      return true;
    }
  }

  return false;
}
