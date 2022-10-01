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
 * GPU.
 * Pixel Magic.
 */

#include "gpu_context_private.hh"
#include "gpu_matrix_private.h"

#define SUPPRESS_GENERIC_MATRIX_API
#define USE_GPU_PY_MATRIX_API /* only so values are declared */
#include "GPU_matrix.h"
#undef USE_GPU_PY_MATRIX_API

#include "KLI_math_matrix.h"
#include "KLI_math_rotation.h"
#include "KLI_math_vector.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

using namespace kraken::gpu;

#define MATRIX_STACK_DEPTH 32

using Mat4 = float[4][4];
using Mat3 = float[3][3];

struct MatrixStack
{
  Mat4 stack[MATRIX_STACK_DEPTH];
  uint top;
};

struct GPUMatrixState
{
  MatrixStack model_view_stack;
  MatrixStack projection_stack;

  bool dirty;

  /* TODO: cache of derived matrices (Normal, MVP, inverse MVP, etc)
   * generate as needed for shaders, invalidate when original matrices change
   *
   * TODO: separate Model from View transform? Batches/objects have model,
   * camera/eye has view & projection
   */
};

#define ModelViewStack Context::get()->matrix_state->model_view_stack
#define ModelView ModelViewStack.stack[ModelViewStack.top]

#define ProjectionStack Context::get()->matrix_state->projection_stack
#define Projection ProjectionStack.stack[ProjectionStack.top]

GPUMatrixState *GPU_matrix_state_create()
{
#define MATRIX_4X4_IDENTITY                                                       \
  {                                                                               \
    {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, \
    {                                                                             \
      0.0f, 0.0f, 0.0f, 1.0f                                                      \
    }                                                                             \
  }

  GPUMatrixState *state = (GPUMatrixState *)MEM_mallocN(sizeof(*state), __func__);
  const MatrixStack identity_stack = {{MATRIX_4X4_IDENTITY}, 0};

  state->model_view_stack = state->projection_stack = identity_stack;
  state->dirty = true;

#undef MATRIX_4X4_IDENTITY

  return state;
}

void GPU_matrix_state_discard(GPUMatrixState *state)
{
  MEM_freeN(state);
}

static void gpu_matrix_state_active_set_dirty(bool value)
{
  GPUMatrixState *state = Context::get()->matrix_state;
  state->dirty = value;
}

void GPU_matrix_reset()
{
  GPUMatrixState *state = Context::get()->matrix_state;
  state->model_view_stack.top = 0;
  state->projection_stack.top = 0;
  unit_m4(ModelView);
  unit_m4(Projection);
  gpu_matrix_state_active_set_dirty(true);
}

#ifdef WITH_GPU_SAFETY

/* Check if matrix is numerically good */
static void checkmat(cosnt float *m)
{
  const int n = 16;
  for (int i = 0; i < n; i++) {
#  if _MSC_VER
    KLI_assert(_finite(m[i]));
#  else
    KLI_assert(!isinf(m[i]));
#  endif
  }
}

#  define CHECKMAT(m) checkmat((const float *)m)

#else

#  define CHECKMAT(m)

#endif

void GPU_matrix_push()
{
  KLI_assert(ModelViewStack.top + 1 < MATRIX_STACK_DEPTH);
  ModelViewStack.top++;
  copy_m4_m4(ModelView, ModelViewStack.stack[ModelViewStack.top - 1]);
}

void GPU_matrix_pop()
{
  KLI_assert(ModelViewStack.top > 0);
  ModelViewStack.top--;
  gpu_matrix_state_active_set_dirty(true);
}

void GPU_matrix_pop_projection()
{
  KLI_assert(ProjectionStack.top > 0);
  ProjectionStack.top--;
  gpu_matrix_state_active_set_dirty(true);
}
