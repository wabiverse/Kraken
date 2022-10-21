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

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "KLI_math_base.h"
#include "KLI_math_bits.h"

#include "GPU_framebuffer.h"

#include "mtl_context.hh"
#include "mtl_framebuffer.hh"
#include "mtl_shader_interface_type.hh"
#include "mtl_state.hh"

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** @name MTLStateManager
   * @{ */

  void MTLStateManager::mtl_state_init()
  {
    KLI_assert(m_context);
    m_context->pipeline_state_init();
  }

  MTLStateManager::MTLStateManager(MTLContext *ctx) : StateManager()
  {
    /* Initialize State. */
    m_context = ctx;
    mtl_state_init();

    /* Force update using default state. */
    m_current = ~state;
    m_current_mutable = ~mutable_state;
    set_state(state);
    set_mutable_state(mutable_state);
  }

  void MTLStateManager::apply_state()
  {
    this->set_state(this->state);
    this->set_mutable_state(this->mutable_state);

    /* Apply active FrameBuffer state. */
    static_cast<MTLFrameBuffer *>(m_context->active_fb)->apply_state();
  };

  void MTLStateManager::force_state()
  {
    /* Little exception for clip distances since they need to keep the old count correct. */
    uint32_t clip_distances = m_current.clip_distances;
    m_current = ~this->state;
    m_current.clip_distances = clip_distances;
    m_current_mutable = ~this->mutable_state;
    this->set_state(this->state);
    this->set_mutable_state(this->mutable_state);
  };

  void MTLStateManager::set_state(const GPUState &state)
  {
    GPUState changed = state ^ m_current;

    if (changed.blend != 0) {
      set_blend((eGPUBlend)state.blend);
    }
    if (changed.write_mask != 0) {
      set_write_mask((eGPUWriteMask)state.write_mask);
    }
    if (changed.depth_test != 0) {
      set_depth_test((eGPUDepthTest)state.depth_test);
    }
    if (changed.stencil_test != 0 || changed.stencil_op != 0) {
      set_stencil_test((eGPUStencilTest)state.stencil_test, (eGPUStencilOp)state.stencil_op);
      set_stencil_mask((eGPUStencilTest)state.stencil_test, mutable_state);
    }
    if (changed.clip_distances != 0) {
      set_clip_distances(state.clip_distances, m_current.clip_distances);
    }
    if (changed.culling_test != 0) {
      set_backface_culling((eGPUFaceCullTest)state.culling_test);
    }
    if (changed.logic_op_xor != 0) {
      set_logic_op(state.logic_op_xor);
    }
    if (changed.invert_facing != 0) {
      set_facing(state.invert_facing);
    }
    if (changed.provoking_vert != 0) {
      set_provoking_vert((eGPUProvokingVertex)state.provoking_vert);
    }
    if (changed.shadow_bias != 0) {
      set_shadow_bias(state.shadow_bias);
    }

    /* TODO remove (Following GLState). */
    if (changed.polygon_smooth) {
      /* NOTE: Unsupported in Metal. */
    }
    if (changed.line_smooth) {
      /* NOTE: Unsupported in Metal. */
    }

    m_current = state;
  }

  void MTLStateManager::mtl_depth_range(float near, float far)
  {
    KLI_assert(m_context);
    KLI_assert(near >= 0.0 && near < 1.0);
    KLI_assert(far > 0.0 && far <= 1.0);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    MTLContextDepthStencilState &ds_state = pipeline_state.depth_stencil_state;

    ds_state.depth_range_near = near;
    ds_state.depth_range_far = far;
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_VIEWPORT_FLAG;
  }

  void MTLStateManager::set_mutable_state(const GPUStateMutable &state)
  {
    GPUStateMutable changed = state ^ m_current_mutable;
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;

    if (float_as_uint(changed.point_size) != 0) {
      pipeline_state.point_size = state.point_size;
      pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_PSO_FLAG;
    }

    if (changed.line_width != 0) {
      pipeline_state.line_width = state.line_width;
      pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_PSO_FLAG;
    }

    if (changed.depth_range[0] != 0 || changed.depth_range[1] != 0) {
      /* TODO remove, should modify the projection matrix instead. */
      mtl_depth_range(state.depth_range[0], state.depth_range[1]);
    }

    if (changed.stencil_compare_mask != 0 || changed.stencil_reference != 0 ||
        changed.stencil_write_mask != 0) {
      set_stencil_mask((eGPUStencilTest)m_current.stencil_test, state);
    }

    m_current_mutable = state;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name State setting functions
   * @{ */

  void MTLStateManager::set_write_mask(const eGPUWriteMask value)
  {
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    pipeline_state.depth_stencil_state.depth_write_enable = ((value & GPU_WRITE_DEPTH) != 0);
    pipeline_state.color_write_mask =
      (((value & GPU_WRITE_RED) != 0) ? MTL::ColorWriteMaskRed : MTL::ColorWriteMaskNone) |
      (((value & GPU_WRITE_GREEN) != 0) ? MTL::ColorWriteMaskGreen : MTL::ColorWriteMaskNone) |
      (((value & GPU_WRITE_BLUE) != 0) ? MTL::ColorWriteMaskBlue : MTL::ColorWriteMaskNone) |
      (((value & GPU_WRITE_ALPHA) != 0) ? MTL::ColorWriteMaskAlpha : MTL::ColorWriteMaskNone);
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_PSO_FLAG;
  }

  static MTL::CompareFunction gpu_depth_function_to_metal(eGPUDepthTest depth_func)
  {
    switch (depth_func) {
      case GPU_DEPTH_NONE:
        return MTL::CompareFunctionNever;
      case GPU_DEPTH_LESS:
        return MTL::CompareFunctionLess;
      case GPU_DEPTH_EQUAL:
        return MTL::CompareFunctionEqual;
      case GPU_DEPTH_LESS_EQUAL:
        return MTL::CompareFunctionLessEqual;
      case GPU_DEPTH_GREATER:
        return MTL::CompareFunctionGreater;
      case GPU_DEPTH_GREATER_EQUAL:
        return MTL::CompareFunctionGreaterEqual;
      case GPU_DEPTH_ALWAYS:
        return MTL::CompareFunctionAlways;
      default:
        KLI_assert(false && "Invalid eGPUDepthTest");
        break;
    }
    return MTL::CompareFunctionAlways;
  }

  static MTL::CompareFunction gpu_stencil_func_to_metal(eGPUStencilTest stencil_func)
  {
    switch (stencil_func) {
      case GPU_STENCIL_NONE:
        return MTL::CompareFunctionAlways;
      case GPU_STENCIL_EQUAL:
        return MTL::CompareFunctionEqual;
      case GPU_STENCIL_NEQUAL:
        return MTL::CompareFunctionNotEqual;
      case GPU_STENCIL_ALWAYS:
        return MTL::CompareFunctionAlways;
      default:
        KLI_assert(false && "Unrecognized eGPUStencilTest function");
        break;
    }
    return MTL::CompareFunctionAlways;
  }

  void MTLStateManager::set_depth_test(const eGPUDepthTest value)
  {
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    MTLContextDepthStencilState &ds_state = pipeline_state.depth_stencil_state;

    ds_state.depth_test_enabled = (value != GPU_DEPTH_NONE);
    ds_state.depth_function = gpu_depth_function_to_metal(value);
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
  }

  void MTLStateManager::mtl_stencil_mask(uint mask)
  {
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    pipeline_state.depth_stencil_state.stencil_write_mask = mask;
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
  }

  void MTLStateManager::mtl_stencil_set_func(eGPUStencilTest stencil_func, int ref, uint mask)
  {
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    MTLContextDepthStencilState &ds_state = pipeline_state.depth_stencil_state;

    ds_state.stencil_func = gpu_stencil_func_to_metal(stencil_func);
    ds_state.stencil_ref = ref;
    ds_state.stencil_read_mask = mask;
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
  }

  static void mtl_stencil_set_op_separate(MTLContext *context,
                                          eGPUFaceCullTest face,
                                          MTL::StencilOperation stencil_fail,
                                          MTL::StencilOperation depth_test_fail,
                                          MTL::StencilOperation depthstencil_pass)
  {
    KLI_assert(context);
    MTLContextGlobalShaderPipelineState &pipeline_state = context->pipeline_state;
    MTLContextDepthStencilState &ds_state = pipeline_state.depth_stencil_state;

    if (face == GPU_CULL_FRONT) {
      ds_state.stencil_op_front_stencil_fail = stencil_fail;
      ds_state.stencil_op_front_depth_fail = depth_test_fail;
      ds_state.stencil_op_front_depthstencil_pass = depthstencil_pass;
    } else if (face == GPU_CULL_BACK) {
      ds_state.stencil_op_back_stencil_fail = stencil_fail;
      ds_state.stencil_op_back_depth_fail = depth_test_fail;
      ds_state.stencil_op_back_depthstencil_pass = depthstencil_pass;
    }

    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
  }

  static void mtl_stencil_set_op(MTLContext *context,
                                 MTL::StencilOperation stencil_fail,
                                 MTL::StencilOperation depth_test_fail,
                                 MTL::StencilOperation depthstencil_pass)
  {
    mtl_stencil_set_op_separate(context,
                                GPU_CULL_FRONT,
                                stencil_fail,
                                depth_test_fail,
                                depthstencil_pass);
    mtl_stencil_set_op_separate(context,
                                GPU_CULL_BACK,
                                stencil_fail,
                                depth_test_fail,
                                depthstencil_pass);
  }

  void MTLStateManager::set_stencil_test(const eGPUStencilTest test, const eGPUStencilOp operation)
  {
    switch (operation) {
      case GPU_STENCIL_OP_REPLACE:
        mtl_stencil_set_op(m_context,
                           MTL::StencilOperationKeep,
                           MTL::StencilOperationKeep,
                           MTL::StencilOperationReplace);
        break;
      case GPU_STENCIL_OP_COUNT_DEPTH_PASS:
        /* Winding inversed due to flipped Y coordinate system in Metal. */
        mtl_stencil_set_op_separate(m_context,
                                    GPU_CULL_FRONT,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationIncrementWrap);
        mtl_stencil_set_op_separate(m_context,
                                    GPU_CULL_BACK,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationDecrementWrap);
        break;
      case GPU_STENCIL_OP_COUNT_DEPTH_FAIL:
        /* Winding inversed due to flipped Y coordinate system in Metal. */
        mtl_stencil_set_op_separate(m_context,
                                    GPU_CULL_FRONT,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationDecrementWrap,
                                    MTL::StencilOperationKeep);
        mtl_stencil_set_op_separate(m_context,
                                    GPU_CULL_BACK,
                                    MTL::StencilOperationKeep,
                                    MTL::StencilOperationIncrementWrap,
                                    MTL::StencilOperationKeep);
        break;
      case GPU_STENCIL_OP_NONE:
      default:
        mtl_stencil_set_op(m_context,
                           MTL::StencilOperationKeep,
                           MTL::StencilOperationKeep,
                           MTL::StencilOperationKeep);
    }

    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    pipeline_state.depth_stencil_state.stencil_test_enabled = (test != GPU_STENCIL_NONE);
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
  }

  void MTLStateManager::set_stencil_mask(const eGPUStencilTest test, const GPUStateMutable state)
  {
    if (test == GPU_STENCIL_NONE) {
      mtl_stencil_mask(0x00);
      mtl_stencil_set_func(GPU_STENCIL_ALWAYS, 0x00, 0x00);
    } else {
      mtl_stencil_mask(state.stencil_write_mask);
      mtl_stencil_set_func(test, state.stencil_reference, state.stencil_compare_mask);
    }
  }

  void MTLStateManager::set_clip_distances(const int new_dist_len, const int old_dist_len)
  {
    /* TODO(Metal): Support Clip distances in METAL. Clip distance
     * assignment via shader is supported, but global clip-states require
     * support. */
  }

  void MTLStateManager::set_logic_op(const bool enable)
  {
    /* NOTE(Metal): Logic Operations not directly supported. */
  }

  void MTLStateManager::set_facing(const bool invert)
  {
    /* Check Current Context. */
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;

    /* Apply State -- opposite of GL, as METAL default is GPU_CLOCKWISE, GL default is
     * COUNTERCLOCKWISE. This needs to be the inverse of the default. */
    pipeline_state.front_face = (invert) ? GPU_COUNTERCLOCKWISE : GPU_CLOCKWISE;

    /* Mark Dirty - Ensure context updates state between draws. */
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_FRONT_FACING_FLAG;
    pipeline_state.dirty = true;
  }

  void MTLStateManager::set_backface_culling(const eGPUFaceCullTest test)
  {
    /* Check Current Context. */
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;

    /* Apply State. */
    pipeline_state.culling_enabled = (test != GPU_CULL_NONE);
    pipeline_state.cull_mode = test;

    /* Mark Dirty - Ensure context updates state between draws. */
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_CULLMODE_FLAG;
    pipeline_state.dirty = true;
  }

  void MTLStateManager::set_provoking_vert(const eGPUProvokingVertex vert)
  {
    /* NOTE(Metal): Provoking vertex is not a feature in the Metal API.
     * Shaders are handled on a case-by-case basis using a modified vertex shader.
     * For example, wireframe rendering and edit-mesh shaders utilize an SSBO-based
     * vertex fetching mechanism which considers the inverse convention for flat
     * shading, to ensure consistent results with OpenGL. */
  }

  void MTLStateManager::set_shadow_bias(const bool enable)
  {
    /* Check Current Context. */
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;
    MTLContextDepthStencilState &ds_state = pipeline_state.depth_stencil_state;

    /* Apply State. */
    if (enable) {
      ds_state.depth_bias_enabled_for_lines = true;
      ds_state.depth_bias_enabled_for_tris = true;
      ds_state.depth_bias = 2.0f;
      ds_state.depth_slope_scale = 1.0f;
    } else {
      ds_state.depth_bias_enabled_for_lines = false;
      ds_state.depth_bias_enabled_for_tris = false;
      ds_state.depth_bias = 0.0f;
      ds_state.depth_slope_scale = 0.0f;
    }

    /* Mark Dirty - Ensure context updates depth-stencil state between draws. */
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG;
    pipeline_state.dirty = true;
  }

  void MTLStateManager::set_blend(const eGPUBlend value)
  {
    /**
     * Factors to the equation.
     * SRC is fragment shader output.
     * DST is framebuffer color.
     * final.rgb = SRC.rgb * src_rgb + DST.rgb * dst_rgb;
     * final.a = SRC.a * src_alpha + DST.a * dst_alpha;
     **/
    MTL::BlendFactor src_rgb;
    MTL::BlendFactor dst_rgb;
    MTL::BlendFactor src_alpha;
    MTL::BlendFactor dst_alpha;
    switch (value) {
      default:
      case GPU_BLEND_ALPHA: {
        src_rgb = MTL::BlendFactorSourceAlpha;
        dst_rgb = MTL::BlendFactorOneMinusSourceAlpha;
        src_alpha = MTL::BlendFactorOne;
        dst_alpha = MTL::BlendFactorOneMinusSourceAlpha;
        break;
      }
      case GPU_BLEND_ALPHA_PREMULT: {
        src_rgb = MTL::BlendFactorOne;
        dst_rgb = MTL::BlendFactorOneMinusSourceAlpha;
        src_alpha = MTL::BlendFactorOne;
        dst_alpha = MTL::BlendFactorOneMinusSourceAlpha;
        break;
      }
      case GPU_BLEND_ADDITIVE: {
        /* Do not let alpha accumulate but pre-multiply the source RGB by it. */
        src_rgb = MTL::BlendFactorSourceAlpha;
        dst_rgb = MTL::BlendFactorOne;
        src_alpha = MTL::BlendFactorZero;
        dst_alpha = MTL::BlendFactorOne;
        break;
      }
      case GPU_BLEND_SUBTRACT:
      case GPU_BLEND_ADDITIVE_PREMULT: {
        /* Let alpha accumulate. */
        src_rgb = MTL::BlendFactorOne;
        dst_rgb = MTL::BlendFactorOne;
        src_alpha = MTL::BlendFactorOne;
        dst_alpha = MTL::BlendFactorOne;
        break;
      }
      case GPU_BLEND_MULTIPLY: {
        src_rgb = MTL::BlendFactorDestinationColor;
        dst_rgb = MTL::BlendFactorZero;
        src_alpha = MTL::BlendFactorDestinationAlpha;
        dst_alpha = MTL::BlendFactorZero;
        break;
      }
      case GPU_BLEND_INVERT: {
        src_rgb = MTL::BlendFactorOneMinusDestinationColor;
        dst_rgb = MTL::BlendFactorZero;
        src_alpha = MTL::BlendFactorZero;
        dst_alpha = MTL::BlendFactorOne;
        break;
      }
      case GPU_BLEND_OIT: {
        src_rgb = MTL::BlendFactorOne;
        dst_rgb = MTL::BlendFactorOne;
        src_alpha = MTL::BlendFactorZero;
        dst_alpha = MTL::BlendFactorOneMinusSourceAlpha;
        break;
      }
      case GPU_BLEND_BACKGROUND: {
        src_rgb = MTL::BlendFactorOneMinusDestinationAlpha;
        dst_rgb = MTL::BlendFactorSourceAlpha;
        src_alpha = MTL::BlendFactorZero;
        dst_alpha = MTL::BlendFactorSourceAlpha;
        break;
      }
      case GPU_BLEND_ALPHA_UNDER_PREMUL: {
        src_rgb = MTL::BlendFactorOneMinusDestinationAlpha;
        dst_rgb = MTL::BlendFactorOne;
        src_alpha = MTL::BlendFactorOneMinusDestinationAlpha;
        dst_alpha = MTL::BlendFactorOne;
        break;
      }
      case GPU_BLEND_CUSTOM: {
        src_rgb = MTL::BlendFactorOne;
        dst_rgb = MTL::BlendFactorSource1Color;
        src_alpha = MTL::BlendFactorOne;
        dst_alpha = MTL::BlendFactorSource1Alpha;
        break;
      }
    }

    /* Check Current Context. */
    KLI_assert(m_context);
    MTLContextGlobalShaderPipelineState &pipeline_state = m_context->pipeline_state;

    if (value == GPU_BLEND_SUBTRACT) {
      pipeline_state.rgb_blend_op = MTL::BlendOperationReverseSubtract;
      pipeline_state.alpha_blend_op = MTL::BlendOperationReverseSubtract;
    } else {
      pipeline_state.rgb_blend_op = MTL::BlendOperationAdd;
      pipeline_state.alpha_blend_op = MTL::BlendOperationAdd;
    }

    /* Apply State. */
    pipeline_state.blending_enabled = (value != GPU_BLEND_NONE);
    pipeline_state.src_rgb_blend_factor = src_rgb;
    pipeline_state.dest_rgb_blend_factor = dst_rgb;
    pipeline_state.src_alpha_blend_factor = src_alpha;
    pipeline_state.dest_alpha_blend_factor = dst_alpha;

    /* Mark Dirty - Ensure context updates PSOs between draws. */
    pipeline_state.dirty_flags |= MTL_PIPELINE_STATE_PSO_FLAG;
    pipeline_state.dirty = true;
  }

/** @} */

/* -------------------------------------------------------------------- */
/** @name Memory barrier
 * @{ */

/* NOTE(Metal): Granular option for specifying before/after stages for a barrier
 * Would be a useful feature. */
#if 0
void MTLStateManager::issue_barrier(eGPUBarrier barrier_bits,
                                    eGPUStageBarrierBits before_stages,
                                    eGPUStageBarrierBits after_stages)
#endif
  void MTLStateManager::issue_barrier(eGPUBarrier barrier_bits)
  {
    /* NOTE(Metal): The Metal API implicitly tracks dependencies between resources.
     * Memory barriers and execution barriers (Fences/Events) can be used to coordinate
     * this explicitly, however, in most cases, the driver will be able to
     * resolve these dependencies automatically.
     * For untracked resources, such as MTLHeap's, explicit barriers are necessary. */
    eGPUStageBarrierBits before_stages = GPU_BARRIER_STAGE_ANY;
    eGPUStageBarrierBits after_stages = GPU_BARRIER_STAGE_ANY;

    MTLContext *ctx = reinterpret_cast<MTLContext *>(GPU_context_active_get());
    KLI_assert(ctx);

    /* Apple Silicon does not support memory barriers.
     * We do not currently need these due to implicit API guarantees.
     * NOTE(Metal): MTLFence/MTLEvent may be required to synchronize work if
     * untracked resources are ever used. */
    if (ctx->device->hasUnifiedMemory()) {
      return;
    }

    ctx->main_command_buffer.insert_memory_barrier(barrier_bits, before_stages, after_stages);
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Texture State Management
   * @{ */

  void MTLStateManager::texture_unpack_row_length_set(uint len)
  {
    /* Set source image row data stride when uploading image data to the GPU. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    ctx->pipeline_state.unpack_row_length = len;
  }

  void MTLStateManager::texture_bind(Texture *tex_, eGPUSamplerState sampler_type, int unit)
  {
    KLI_assert(tex_);
    gpu::MTLTexture *mtl_tex = static_cast<gpu::MTLTexture *>(tex_);
    KLI_assert(mtl_tex);

    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    if (unit >= 0) {
      ctx->texture_bind(mtl_tex, unit);

      /* Fetching textures default sampler configuration and applying
       * eGPUSampler State on top. This path exists to support
       * Any of the sampler state which is associated with the
       * texture itself such as min/max mip levels. */
      MTLSamplerState sampler = mtl_tex->get_sampler_state();
      sampler.state = sampler_type;

      ctx->sampler_bind(sampler, unit);
    }
  }

  void MTLStateManager::texture_unbind(Texture *tex_)
  {
    KLI_assert(tex_);
    gpu::MTLTexture *mtl_tex = static_cast<gpu::MTLTexture *>(tex_);
    KLI_assert(mtl_tex);
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    ctx->texture_unbind(mtl_tex);
  }

  void MTLStateManager::texture_unbind_all()
  {
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);
    ctx->texture_unbind_all();
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Image Binding (from image load store)
   * @{ */

  void MTLStateManager::image_bind(Texture *tex_, int unit)
  {
    this->texture_bind(tex_, GPU_SAMPLER_DEFAULT, unit);
  }

  void MTLStateManager::image_unbind(Texture *tex_)
  {
    this->texture_unbind(tex_);
  }

  void MTLStateManager::image_unbind_all()
  {
    this->texture_unbind_all();
  }

  /** @} */

}  // namespace kraken::gpu
