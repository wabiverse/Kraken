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

#include <stdlib.h>

#include "MEM_guardedalloc.h"

#include "KLI_kraklib.h"
#include "KLI_utildefines.h"

#include "USD_scene_types.h"
#include "USD_userdef_types.h"
#include "USD_wm_types.h"

#include "KKE_context.h"

#include "GPU_state.h"

#include "UI_interface.h"
#include "UI_view2d.h"

#include "ED_screen.h"
#include "ED_space_api.h"
#include "ED_userpref.h"
#include "ED_util.h"

void ED_spacetypes_init(void)
{
  /* UI unit is a variable, may be used in some space type initialization. */
  U.widget_unit = 20;

  ED_operatortypes_userpref();
}