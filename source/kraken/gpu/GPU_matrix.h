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

#pragma once

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "kraken/kraken.h"
#include "KLI_sys_types.h"

KRAKEN_NAMESPACE_BEGIN

struct GPUShader;

const float (*GPU_matrix_projection_get(float m[4][4]))[4];

/* Python API needs to be able to inspect the stack so errors raise exceptions
 * instead of crashing. */
#ifdef USE_GPU_PY_MATRIX_API
int GPU_matrix_stack_level_get_model_view(void);
int GPU_matrix_stack_level_get_projection(void);
/* static assert ensures this doesn't change! */
#  define GPU_PY_MATRIX_STACK_LEN 31
#endif /* USE_GPU_PY_MATRIX_API */

#ifndef SUPPRESS_GENERIC_MATRIX_API

#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#    define _GPU_MAT3_CONST_CAST(x) \
      (_Generic((x), \
  void *:       (const float (*)[3])(x), \
  float *:      (const float (*)[3])(x), \
  float [9]:    (const float (*)[3])(x), \
  float (*)[4]: (const float (*)[3])(x), \
  float [4][4]: (const float (*)[3])(x), \
  const void *:       (const float (*)[3])(x), \
  const float *:      (const float (*)[3])(x), \
  const float [9]:    (const float (*)[3])(x), \
  const float (*)[3]: (const float (*)[3])(x), \
  const float [3][3]: (const float (*)[3])(x)) \
)
#    define _GPU_MAT3_CAST(x) \
      (_Generic((x), \
  void *:       (float (*)[3])(x), \
  float *:      (float (*)[3])(x), \
  float [9]:    (float (*)[3])(x), \
  float (*)[3]: (float (*)[3])(x), \
  float [3][3]: (float (*)[3])(x)) \
)
#    define _GPU_MAT4_CONST_CAST(x) \
      (_Generic((x), \
  void *:       (const float (*)[4])(x), \
  float *:      (const float (*)[4])(x), \
  float [16]:   (const float (*)[4])(x), \
  float (*)[4]: (const float (*)[4])(x), \
  float [4][4]: (const float (*)[4])(x), \
  const void *:       (const float (*)[4])(x), \
  const float *:      (const float (*)[4])(x), \
  const float [16]:   (const float (*)[4])(x), \
  const float (*)[4]: (const float (*)[4])(x), \
  const float [4][4]: (const float (*)[4])(x)) \
)
#    define _GPU_MAT4_CAST(x) \
      (_Generic((x), \
  void *:       (float (*)[4])(x), \
  float *:      (float (*)[4])(x), \
  float [16]:   (float (*)[4])(x), \
  float (*)[4]: (float (*)[4])(x), \
  float [4][4]: (float (*)[4])(x)) \
)
#  else
#    define _GPU_MAT3_CONST_CAST(x) (const float(*)[3])(x)
#    define _GPU_MAT3_CAST(x) (float(*)[3])(x)
#    define _GPU_MAT4_CONST_CAST(x) (const float(*)[4])(x)
#    define _GPU_MAT4_CAST(x) (float(*)[4])(x)
#  endif /* C11 */

/* make matrix inputs generic, to avoid warnings */
#  define GPU_matrix_mul(x) GPU_matrix_mul(_GPU_MAT4_CONST_CAST(x))
#  define GPU_matrix_set(x) GPU_matrix_set(_GPU_MAT4_CONST_CAST(x))
#  define GPU_matrix_projection_set(x) GPU_matrix_projection_set(_GPU_MAT4_CONST_CAST(x))
#  define GPU_matrix_model_view_get(x) GPU_matrix_model_view_get(_GPU_MAT4_CAST(x))
#  define GPU_matrix_projection_get(x) GPU_matrix_projection_get(_GPU_MAT4_CAST(x))
#  define GPU_matrix_model_view_projection_get(x) \
    GPU_matrix_model_view_projection_get(_GPU_MAT4_CAST(x))
#  define GPU_matrix_normal_get(x) GPU_matrix_normal_get(_GPU_MAT3_CAST(x))
#  define GPU_matrix_normal_inverse_get(x) GPU_matrix_normal_inverse_get(_GPU_MAT3_CAST(x))
#endif /* SUPPRESS_GENERIC_MATRIX_API */

/* Not part of the GPU_matrix API,
 * however we need to check these limits in code that calls into these API's. */
#define GPU_MATRIX_ORTHO_CLIP_NEAR_DEFAULT (-100)
#define GPU_MATRIX_ORTHO_CLIP_FAR_DEFAULT (100)

KRAKEN_NAMESPACE_END