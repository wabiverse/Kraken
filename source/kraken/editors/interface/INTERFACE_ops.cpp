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
 * Editors.
 * Tools for Artists.
 */

#include <cstring>

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_space_types.h"

#include "KLI_kraklib.h"
#include "KLI_math_color.h"

#include "KRF_api.h"

#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_idprop.h"
#include "KKE_lib_id.h"
#include "KKE_material.h"
#include "KKE_report.h"
#include "KKE_screen.h"

#include "IMB_colormanagement.h"

#include "LUXO_access.h"
#include "LUXO_define.h"
#include "LUXO_types.h"

#include "UI_interface.h"

#include "interface_intern.h"

/* only for UI_OT_editsource */
#include "KKE_main.h"
#include "KLI_rhash.h"
#include "ED_screen.h"


#ifdef WITH_PYTHON

/* ------------------------------------------------------------------------- */
/* EditSource Utility functions and operator,
 * NOTE: this includes utility functions and button matching checks. */

struct uiEditSourceStore
{
  uiBut but_orig;
  RHash *hash;
};

struct uiEditSourceButStore
{
  char py_dbg_fn[FILE_MAX];
  int py_dbg_line_number;
};

/* should only ever be set while the edit source operator is running */
static uiEditSourceStore *ui_editsource_info = nullptr;

/* sneaky sneaky. */
extern "C" {
void PyC_FileAndNum_Safe(const char **r_filename, int *r_lineno);
}

bool UI_editsource_enable_check(void)
{
  return (ui_editsource_info != nullptr);
}

void UI_editsource_active_but_test(uiBut *but)
{

  uiEditSourceButStore *but_store = MEM_cnew<uiEditSourceButStore>(__func__);

  const char *fn;
  int line_number = -1;

#  if 0
  printf("comparing buttons: '%s' == '%s'\n", but->drawstr, ui_editsource_info->but_orig.drawstr);
#  endif

  PyC_FileAndNum_Safe(&fn, &line_number);

  if (line_number != -1) {
    KLI_strncpy(but_store->py_dbg_fn, fn, sizeof(but_store->py_dbg_fn));
    but_store->py_dbg_line_number = line_number;
  } else {
    but_store->py_dbg_fn[0] = '\0';
    but_store->py_dbg_line_number = -1;
  }

  KLI_rhash_insert(ui_editsource_info->hash, but, but_store);
}

void UI_editsource_but_replace(const uiBut *old_but, uiBut *new_but)
{
  uiEditSourceButStore *but_store = static_cast<uiEditSourceButStore *>(
    KLI_rhash_lookup(ui_editsource_info->hash, old_but));
  if (but_store) {
    KLI_rhash_remove(ui_editsource_info->hash, old_but, nullptr, nullptr);
    KLI_rhash_insert(ui_editsource_info->hash, new_but, but_store);
  }
}

#endif /* WITH_PYTHON */
