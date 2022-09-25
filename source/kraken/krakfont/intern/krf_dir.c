/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 *
 * Manage search paths for font files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "MEM_guardedalloc.h"

#include "USD_vec_types.h"

#include "KLI_fileops.h"
#include "KLI_listbase.h"
#include "KLI_path_utils.h"
#include "KLI_string.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "KRF_api.h"
#include "krf_internal.h"
#include "krf_internal_types.h"

static ListBase global_font_dir = {NULL, NULL};

static DirKRF *krf_dir_find(const char *path)
{
  DirKRF *p;

  p = global_font_dir.first;
  while (p) {
    if (KLI_path_cmp(p->path, path) == 0) {
      return p;
    }
    p = p->next;
  }
  return NULL;
}

void KRF_dir_add(const char *path)
{
  DirKRF *dir;

  dir = krf_dir_find(path);
  if (dir) { /* already in the list ? just return. */
    return;
  }

  dir = (DirKRF *)MEM_callocN(sizeof(DirKRF), "KRF_dir_add");
  dir->path = KLI_strdup(path);
  KLI_addhead(&global_font_dir, dir);
}

void KRF_dir_rem(const char *path)
{
  DirKRF *dir;

  dir = krf_dir_find(path);
  if (dir) {
    KLI_remlink(&global_font_dir, dir);
    MEM_freeN(dir->path);
    MEM_freeN(dir);
  }
}

char **KRF_dir_get(int *ndir)
{
  DirKRF *p;
  char **dirs;
  char *path;
  int i, count;

  count = KLI_listbase_count(&global_font_dir);
  if (!count) {
    return NULL;
  }

  dirs = (char **)MEM_callocN(sizeof(char *) * count, "KRF_dir_get");
  p = global_font_dir.first;
  i = 0;
  while (p) {
    path = KLI_strdup(p->path);
    dirs[i] = path;
    p = p->next;
  }
  *ndir = i;
  return dirs;
}

void KRF_dir_free(char **dirs, int count)
{
  for (int i = 0; i < count; i++) {
    char *path = dirs[i];
    MEM_freeN(path);
  }
  MEM_freeN(dirs);
}

char *krf_dir_search(const char *file)
{
  KLI_assert_msg(!KLI_path_is_rel(file), "Relative paths must always be expanded!");

  DirKRF *dir;
  char full_path[FILE_MAX];
  char *s = NULL;

  for (dir = global_font_dir.first; dir; dir = dir->next) {
    KLI_join_dirfile(full_path, sizeof(full_path), dir->path, file);
    if (KLI_exists(full_path)) {
      s = KLI_strdup(full_path);
      break;
    }
  }

  if (!s) {
    /* This may be an absolute path which exists. */
    if (KLI_exists(file)) {
      s = KLI_strdup(file);
    }
  }

  return s;
}

char *krf_dir_metrics_search(const char *filepath)
{
  char *mfile;
  char *s;

  mfile = KLI_strdup(filepath);
  s = strrchr(mfile, '.');
  if (s) {
    if (KLI_strnlen(s, 4) < 4) {
      MEM_freeN(mfile);
      return NULL;
    }
    s++;
    s[0] = 'a';
    s[1] = 'f';
    s[2] = 'm';

    /* First check `.afm`. */
    if (KLI_exists(mfile)) {
      return mfile;
    }

    /* And now check `.pfm`. */
    s[0] = 'p';

    if (KLI_exists(mfile)) {
      return mfile;
    }
  }
  MEM_freeN(mfile);
  return NULL;
}
