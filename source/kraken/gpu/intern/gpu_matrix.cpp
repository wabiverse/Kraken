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

#include "kraken/kraken.h"

#define SUPPRESS_GENERIC_MATRIX_API
#define USE_GPU_PY_MATRIX_API /* only so values are declared */
#include "GPU_matrix.h"
#undef USE_GPU_PY_MATRIX_API
#ifdef __APPLE__
/* temporary, til we bring the namespace out elsewhere. */
#  include "mtl_context.h"
#endif /* __APPLE__ */

#include "KLI_math_matrix.h"
#include "KLI_math_rotation.h"
#include "KLI_math_vector.h"

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

  GPUMatrixState *state = new GPUMatrixState();
  const MatrixStack identity_stack = {{MATRIX_4X4_IDENTITY}, 0};

  state->model_view_stack = state->projection_stack = identity_stack;
  state->dirty = true;

#undef MATRIX_4X4_IDENTITY

  return state;
}

const float (*GPU_matrix_projection_get(float m[4][4]))[4]
{
  if (m) {
    copy_m4_m4(m, Projection);
    return m;
  }

  return Projection;
}
