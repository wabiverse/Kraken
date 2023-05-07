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

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "CLG_log.h"

#include "MEM_guardedalloc.h"

#include "USD_ID.h"
#include "USD_customdata_types.h"
#include "USD_materials.h"
#include "USD_space_types.h"
#include "USD_color_types.h"
#include "USD_scene_types.h"

#include "KLI_listbase.h"
#include "KLI_math.h"
#include "KLI_utildefines.h"

#include "KKE_cryptomatte.h"
#include "KKE_icons.h"
#include "KKE_idtype.h"
#include "KKE_lib_id.h"
#include "KKE_main.h"
#include "KKE_material.h"

#include "GPU_material.h"

static CLG_LogRef LOG = {"kke.material"};

static Material default_material_empty;
static Material default_material_holdout;
static Material default_material_surface;
static Material default_material_volume;
static Material default_material_gpencil;

static Material *default_materials[] = {&default_material_empty,
                                        &default_material_holdout,
                                        &default_material_surface,
                                        &default_material_volume,
                                        &default_material_gpencil,
                                        NULL};

void KKE_material_defaults_free_gpu(void)
{
  for (int i = 0; default_materials[i]; i++) {
    Material *ma = default_materials[i];
    if (ma->gpumaterial.first) {
      GPU_material_free(&ma->gpumaterial);
    }
  }
}

Material *KKE_material_add(Main *kmain, const char *name)
{
  Material *ma;

  ma = KKE_id_new(kmain, ID_MA, name);

  return ma;
}

Material *KKE_gpencil_material_add(Main *kmain, const char *name)
{
  Material *ma;

  ma = KKE_material_add(kmain, name);

  /* grease pencil settings */
  if (ma != NULL) {
    //KKE_gpencil_material_attr_init(ma);
  }
  return ma;
}
