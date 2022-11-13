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
 * @file Render.
 * Absurd attempts for impossible outcomes.
 */

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "KLI_rhash.h"
#include "KLI_listbase.h"
#include "KLI_math_bits.h"
#include "KLI_rect.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"

#include "USD_object_types.h"

#include "KKE_colortools.h"
#include "KKE_global.h"
#include "KKE_report.h"
#include "KKE_scene.h"

#include "LUXO_access.h"

#ifdef WITH_PYTHON
#  include "KPY_extern.h"
#endif

#include "RE_engine.h"

#include "DRW_engine.h"

#include "GPU_context.h"
#include "WM_window.h"
#include "WM_window.hh"

/* Render Engine Types */

ListBase R_engines = {nullptr, nullptr};

void RE_engines_init(void)
{
  DRW_engines_register();
}

void RE_engines_register(RenderEngineType *render_type)
{
  if (render_type->draw_engine) {
    DRW_engine_register(render_type->draw_engine);
  }
  KLI_addtail(&R_engines, render_type);
}
