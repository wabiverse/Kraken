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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender GPU library. (source/blender/gpu).
 * 
 * With any additions or modifications specific to Kraken.
 * 
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * GPU.
 * Pixel Magic.
 *
 * GPU element list (AKA index buffer).
 */

#include "GPU_material.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FUNCTION_NAME 64
#define MAX_PARAMETER 36

struct RSet;

typedef enum
{
  FUNCTION_QUAL_IN,
  FUNCTION_QUAL_OUT,
  FUNCTION_QUAL_INOUT,
} GPUFunctionQual;

typedef struct GPUFunction
{
  char name[MAX_FUNCTION_NAME];
  eGPUType paramtype[MAX_PARAMETER];
  GPUFunctionQual paramqual[MAX_PARAMETER];
  int totparam;
  /* TODO(@fclem): Clean that void pointer. */
  void *source; /* GPUSource */
} GPUFunction;

GPUFunction *gpu_material_library_use_function(struct RSet *used_libraries, const char *name);

#ifdef __cplusplus
}
#endif
