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
 * Window Manager.
 * Making GUI Fly.
 */

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "WM_event_system.h"
#include "WM_cursors_api.h"
#include "WM_debug_codes.h"
#include "WM_dragdrop.h"
#include "WM_operators.h"
#include "WM_window.h"
#include "WM_menu_type.h"

#include "USD_ID.h"
#include "USD_area.h"
#include "USD_factory.h"
#include "USD_operator.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "KKE_context.h"
#include "KKE_idtype.h"
#include "KKE_idprop.h"
#include "KKE_report.h"
#include "KKE_utils.h"

#include "KLI_assert.h"
#include "KLI_kraklib.h"
#include "KLI_time.h"

#include "ED_screen.h"

#include "LUXO_access.h"

KRAKEN_NAMESPACE_BEGIN

static RHash *menutypes_hash = NULL;

MenuType *WM_menutype_find(const TfToken &idname, bool quiet)
{
  if (idname.data()) {
    MenuType *mt = (MenuType *)KKE_rhash_lookup(menutypes_hash, idname);
    if (mt) {
      return mt;
    }
  }

  if (!quiet) {
    printf("search for unknown menutype %s\n", idname);
  }

  return NULL;
}

KRAKEN_NAMESPACE_END