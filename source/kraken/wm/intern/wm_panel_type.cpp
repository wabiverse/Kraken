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

#include "USD_area.h"
#include "USD_context.h"
#include "USD_factory.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "WM_window.h"

#include "KLI_rhash.h"
#include "KLI_time.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"
#include "KKE_workspace.h"

#include "UI_interface.h"



static RHash *g_paneltypes_hash = NULL;

PanelType *WM_paneltype_find(const char *idname, bool quiet)
{
  if (idname[0]) {
    PanelType *pt = (PanelType *)KLI_rhash_lookup(g_paneltypes_hash, idname);
    if (pt) {
      return pt;
    }
  }

  if (!quiet) {
    printf("search for unknown paneltype %s\n", idname);
  }

  return NULL;
}

