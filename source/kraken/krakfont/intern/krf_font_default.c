/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2011 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 *
 * API for loading default font files.
 */

#include <stdio.h>

#include "KRF_api.h"

#include "KLI_fileops.h"
#include "KLI_path_utils.h"

#include "KKE_appdir.h"

#ifdef WIN32
#  include "KLI_winstuff.h"
#endif

static int krf_load_font_default(const char *filename, const bool unique)
{
  const char *dir = KKE_appdir_folder_id(KRAKEN_DATAFILES, KRF_DATAFILES_FONTS_DIR);
  if (dir == NULL) {
    fprintf(stderr,
            "%s: 'fonts' data path not found for '%s', will not be able to display text\n",
            __func__,
            filename);
    return -1;
  }

  char filepath[FILE_MAX];
  KLI_join_dirfile(filepath, sizeof(filepath), dir, filename);

  return (unique) ? KRF_load_unique(filepath) : KRF_load(filepath);
}

int KRF_load_default(const bool unique)
{
  int font_id = krf_load_font_default(KRF_DEFAULT_PROPORTIONAL_FONT, unique);
  KRF_enable(font_id, KRF_DEFAULT);
  return font_id;
}

int KRF_load_mono_default(const bool unique)
{
  int font_id = krf_load_font_default(KRF_DEFAULT_MONOSPACED_FONT, unique);
  KRF_enable(font_id, KRF_MONOSPACED | KRF_DEFAULT);
  return font_id;
}

static void krf_load_datafiles_dir(void)
{
  const char *datafiles_fonts_dir = KRF_DATAFILES_FONTS_DIR SEP_STR;
  const char *path = KKE_appdir_folder_id(KRAKEN_DATAFILES, datafiles_fonts_dir);
  if (UNLIKELY(!path)) {
    fprintf(stderr, "Font data directory \"%s\" could not be detected!\n", datafiles_fonts_dir);
    return;
  }
  if (UNLIKELY(!KLI_exists(path))) {
    fprintf(stderr, "Font data directory \"%s\" does not exist!\n", path);
    return;
  }

  struct direntry *file_list;
  uint file_list_num = KLI_filelist_dir_contents(path, &file_list);
  for (int i = 0; i < file_list_num; i++) {
    if (S_ISDIR(file_list[i].s.st_mode)) {
      continue;
    }

    const char *filepath = file_list[i].path;
    if (!KLI_path_extension_check_n(
            filepath, ".ttf", ".ttc", ".otf", ".otc", ".woff", ".woff2", NULL)) {
      continue;
    }
    if (KRF_is_loaded(filepath)) {
      continue;
    }

    /* Attempt to load the font. */
    int font_id = KRF_load(filepath);
    if (font_id == -1) {
      fprintf(stderr, "Unable to load font: %s\n", filepath);
      continue;
    }

    KRF_enable(font_id, KRF_DEFAULT);
  }
  KLI_filelist_free(file_list, file_list_num);
}

void KRF_load_font_stack()
{
  /* Load these if not already, might have been replaced by user custom. */
  KRF_load_default(false);
  KRF_load_mono_default(false);
  krf_load_datafiles_dir();
}
