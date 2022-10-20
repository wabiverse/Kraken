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
 */

#include "MEM_guardedalloc.h"

#include "KLI_utildefines.h"

#include "GPU_state.h"
#include "gpu_state_private.hh"

#include "mtl_pso_descriptor_state.hh"

namespace kraken::gpu
{

  /* Forward Declarations. */
  class MTLContext;

  /**
   * State manager keeping track of the draw state and applying it before drawing.
   * Metal Implementation.
   **/
  class MTLStateManager : public StateManager
  {

   private:

    /* Current state of the associated MTLContext.
     * Avoids resetting the whole state for every change. */
    GPUState m_current;
    GPUStateMutable m_current_mutable;
    MTLContext *m_context;

    /* Global pipeline descriptors. */
    MTLRenderPipelineStateDescriptor m_pipeline_descriptor;

   public:

    MTLStateManager(MTLContext *ctx);

    void apply_state() override;
    void force_state() override;

    void issue_barrier(eGPUBarrier barrier_bits) override;

    void texture_bind(Texture *tex, eGPUSamplerState sampler, int unit) override;
    void texture_unbind(Texture *tex) override;
    void texture_unbind_all() override;

    void image_bind(Texture *tex, int unit) override;
    void image_unbind(Texture *tex) override;
    void image_unbind_all() override;

    void texture_unpack_row_length_set(uint len) override;

    /* Global pipeline descriptors. */
    MTLRenderPipelineStateDescriptor &get_pipeline_descriptor()
    {
      return m_pipeline_descriptor;
    }

   private:

    void set_write_mask(const eGPUWriteMask value);
    void set_depth_test(const eGPUDepthTest value);
    void set_stencil_test(const eGPUStencilTest test, const eGPUStencilOp operation);
    void set_stencil_mask(const eGPUStencilTest test, const GPUStateMutable state);
    void set_clip_distances(const int new_dist_len, const int old_dist_len);
    void set_logic_op(const bool enable);
    void set_facing(const bool invert);
    void set_backface_culling(const eGPUFaceCullTest test);
    void set_provoking_vert(const eGPUProvokingVertex vert);
    void set_shadow_bias(const bool enable);
    void set_blend(const eGPUBlend value);

    void set_state(const GPUState &state);
    void set_mutable_state(const GPUStateMutable &state);

    /* METAL State utility functions. */
    void mtl_state_init();
    void mtl_depth_range(float near, float far);
    void mtl_stencil_mask(uint mask);
    void mtl_stencil_set_func(eGPUStencilTest stencil_func, int ref, uint mask);

    MEM_CXX_CLASS_ALLOC_FUNCS("MTLStateManager")
  };

}  // namespace kraken::gpu
