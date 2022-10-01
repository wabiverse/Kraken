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
 * @ingroup KRAKEN Library.
 * Gadget Vault.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_string.h"
#include "KLI_string_utf8.h"
#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

#include "USD_listBase.h"

/* Unique name utils. */

size_t KLI_split_name_num(char *left, int *nr, const char *name, const char delim)
{
  const size_t name_len = strlen(name);

  *nr = 0;
  memcpy(left, name, (name_len + 1) * sizeof(char));

  /* name doesn't end with a delimiter "foo." */
  if ((name_len > 1 && name[name_len - 1] == delim) == 0) {
    size_t a = name_len;
    while (a--) {
      if (name[a] == delim) {
        left[a] = '\0'; /* truncate left part here */
        *nr = atol(name + a + 1);
        /* casting down to an int, can overflow for large numbers */
        if (*nr < 0) {
          *nr = 0;
        }
        return a;
      }
      if (isdigit(name[a]) == 0) {
        /* non-numeric suffix - give up */
        break;
      }
    }
  }

  return name_len;
}

bool KLI_uniquename_cb(UniquenameCheckCallback unique_check,
                       void *arg,
                       const char *defname,
                       char delim,
                       char *name,
                       size_t name_len)
{
  if (name[0] == '\0') {
    KLI_strncpy(name, defname, name_len);
  }

  if (unique_check(arg, name)) {
    char numstr[16];
    char *tempname = alloca(name_len);
    char *left = alloca(name_len);
    int number;
    size_t len = KLI_split_name_num(left, &number, name, delim);
    do {
      /* add 1 to account for \0 */
      const size_t numlen = KLI_snprintf(numstr, sizeof(numstr), "%c%03d", delim, ++number) + 1;

      /* highly unlikely the string only has enough room for the number
       * but support anyway */
      if ((len == 0) || (numlen >= name_len)) {
        /* number is know not to be utf-8 */
        KLI_strncpy(tempname, numstr, name_len);
      } else {
        char *tempname_buf;
        tempname_buf = tempname + KLI_strncpy_utf8_rlen(tempname, left, name_len - numlen);
        memcpy(tempname_buf, numstr, numlen);
      }
    } while (unique_check(arg, tempname));

    KLI_strncpy(name, tempname, name_len);

    return true;
  }

  return false;
}

/**
 * Generic function to set a unique name. It is only designed to be used in situations
 * where the name is part of the struct.
 *
 * For places where this is used, see constraint.c for example...
 *
 * \param name_offset: should be calculated using `offsetof(structname, membername)`
 * macro from `stddef.h`
 */
static bool uniquename_find_dupe(ListBase *list, void *vlink, const char *name, int name_offset)
{
  Link *link;

  for (link = list->first; link; link = link->next) {
    if (link != vlink) {
      if (STREQ(POINTER_OFFSET((const char *)link, name_offset), name)) {
        return true;
      }
    }
  }

  return false;
}

static bool uniquename_unique_check(void *arg, const char *name)
{
  struct
  {
    ListBase *lb;
    void *vlink;
    int name_offset;
  } *data = arg;
  return uniquename_find_dupe(data->lb, data->vlink, name, data->name_offset);
}

bool KLI_uniquename(ListBase *list,
                    void *vlink,
                    const char *defname,
                    char delim,
                    int name_offset,
                    size_t name_len)
{
  struct
  {
    ListBase *lb;
    void *vlink;
    int name_offset;
  } data;

  data.lb = list;
  data.vlink = vlink;
  data.name_offset = name_offset;

  KLI_assert(name_len > 1);

  /* See if we are given an empty string */
  if (ELEM(NULL, vlink, defname)) {
    return false;
  }

  return KLI_uniquename_cb(uniquename_unique_check,
                           &data,
                           defname,
                           delim,
                           POINTER_OFFSET(vlink, name_offset),
                           name_len);
}