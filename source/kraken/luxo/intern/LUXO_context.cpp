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
 * Luxo.
 * The Universe Gets Animated.
 */

#include <stdbool.h>

#include "KLI_compiler_attrs.h"

#include "MEM_guardedalloc.h"

#include "LUXO_runtime.h"

#include "USD_wm_types.h"
#include "USD_api.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_default_tables.h"
#include "USD_factory.h"
#include "USD_file.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "KLI_utildefines.h"
#include "KLI_string.h"
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_screen.h"

#include "LUXO_runtime.h"
#include "LUXO_access.h"
#include "LUXO_define.h"

#include "LUXO_internal.h"

#include "WM_tokens.h"

#include <wabi/base/tf/token.h>

void PRIM_def_context(const KrakenSTAGE &kstage)
{
  KrakenPRIM *kprim;

  kprim = PRIM_def_struct_ptr(kstage, SdfPath("Context"));
  PRIM_def_struct_ui_text(kprim, "Context", "Current windowmanager and data context");

  PrimFactory::TOKEN::Def(kprim, "usd_data", TfToken(), "USD Data", "Currently loaded USD stage data");
}
