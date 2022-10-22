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

#include "USD_userdef_types.h"

#include "mtl_backend.hh"
#include "mtl_common.hh"
#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_framebuffer.hh"

#include <fstream>

using namespace kraken;
using namespace kraken::gpu;

namespace kraken::gpu
{

  /* Global sync event used across MTLContext's.
   * This resolves flickering artifacts from command buffer
   * dependencies not being honored for work submitted between
   * different GPUContext's. */
  MTL::Event *MTLCommandBufferManager::sync_event = nil;
  uint64_t MTLCommandBufferManager::event_signal_val = 0;

  /* Counter for active command buffers. */
  int MTLCommandBufferManager::num_active_cmd_bufs = 0;

  /* -------------------------------------------------------------------- */
  /** @name MTLCommandBuffer initialization and render coordination.
   * @{ */

  void MTLCommandBufferManager::prepare(bool supports_render)
  {
    m_render_pass_state.reset_state();
  }

  void MTLCommandBufferManager::register_encoder_counters()
  {
    m_encoder_count++;
    m_empty = false;
  }

  MTL::CommandBuffer *MTLCommandBufferManager::ensure_begin()
  {
    if (m_active_command_buffer == nil) {

      /* Verify number of active command buffers is below limit.
       * Exceeding this limit will mean we either have a leak/GPU hang
       * or we should increase the command buffer limit during MTLQueue creation */
      KLI_assert(MTLCommandBufferManager::num_active_cmd_bufs < MTL_MAX_COMMAND_BUFFERS);

      if (G.debug & G_DEBUG_GPU) {
        /* Debug: Enable Advanced Errors for GPU work execution. */
        MTL::CommandBufferDescriptor *desc = MTL::CommandBufferDescriptor::alloc()->init();
        desc->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
        desc->setRetainedReferences(YES);
        KLI_assert(m_context.queue != nil);
        m_active_command_buffer = m_context.queue->commandBuffer(desc);
      } else {
        m_active_command_buffer = m_context.queue->commandBuffer();
      }
      m_active_command_buffer->retain();
      MTLCommandBufferManager::num_active_cmd_bufs++;

      /* Ensure command buffers execute in submission order across multiple MTLContext's. */
      if (this->sync_event != nil) {
        m_active_command_buffer->encodeWait(this->sync_event, this->event_signal_val);
      }

      /* Ensure we begin new Scratch Buffer if we are on a new frame. */
      MTLScratchBufferManager &mem = m_context.memory_manager;
      mem.ensure_increment_scratch_buffer();

      /* Reset Command buffer heuristics. */
      this->reset_counters();
    }
    KLI_assert(m_active_command_buffer != nil);
    return m_active_command_buffer;
  }

  /* If wait is true, CPU will stall until GPU work has completed. */
  bool MTLCommandBufferManager::submit(bool wait)
  {
    /* Skip submission if command buffer is empty. */
    if (m_empty || m_active_command_buffer == nil) {
      return false;
    }

    /* Ensure current encoders are finished. */
    this->end_active_command_encoder();
    KLI_assert(m_active_command_encoder_type == MTL_NO_COMMAND_ENCODER);

    /* Flush active ScratchBuffer associated with parent MTLContext. */
    m_context.memory_manager.flush_active_scratch_buffer();

    /*** Submit Command Buffer. ***/
    /* Strict ordering ensures command buffers are guaranteed to execute after a previous
     * one has completed. Resolves flickering when command buffers are submitted from
     * different MTLContext's. */
    if (MTLCommandBufferManager::sync_event == nil) {
      MTLCommandBufferManager::sync_event = m_context.device->newEvent();
      KLI_assert(MTLCommandBufferManager::sync_event);
      MTLCommandBufferManager::sync_event->retain();
    }
    KLI_assert(MTLCommandBufferManager::sync_event != nil);
    MTLCommandBufferManager::event_signal_val++;

    m_active_command_buffer->encodeSignalEvent(MTLCommandBufferManager::sync_event,
                                               MTLCommandBufferManager::event_signal_val);

    /* Command buffer lifetime tracking. */
    /* Increment current MTLSafeFreeList reference counter to flag MTLBuffers freed within
     * the current command buffer lifetime as used.
     * This ensures that in-use resources are not prematurely de-referenced and returned to the
     * available buffer pool while they are in-use by the GPU. */
    MTLSafeFreeList *cmd_free_buffer_list =
      MTLContext::get_global_memory_manager().get_current_safe_list();
    KLI_assert(cmd_free_buffer_list);
    cmd_free_buffer_list->increment_reference();

    MTL::CommandBuffer *cmd_buffer_ref = m_active_command_buffer;
    cmd_buffer_ref->retain();

    cmd_buffer_ref->addCompletedHandler([&](MTL::CommandBuffer *buffer) -> void {
      /* Upon command buffer completion, decrement MTLSafeFreeList reference count
       * to allow buffers no longer in use by this CommandBuffer to be freed. */
      cmd_free_buffer_list->decrement_reference();

      /* Release command buffer after completion callback handled. */
      cmd_buffer_ref->release();

      /* Decrement count. */
      MTLCommandBufferManager::num_active_cmd_bufs--;
    });

    /* Submit command buffer to GPU. */
    m_active_command_buffer->commit();

    if (wait || (G.debug & G_DEBUG_GPU)) {
      /* Wait until current GPU work has finished executing. */
      m_active_command_buffer->waitUntilCompleted();

      /* Command buffer execution debugging can return an error message if
       * execution has failed or encountered GPU-side errors. */
      if (G.debug & G_DEBUG_GPU) {

        NS::Error *error = m_active_command_buffer->error();
        if (error != nil) {
          MTL_LOG_ERROR("%s\n", error->localizedFailureReason()->utf8String());
          KLI_assert(false);

          NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
          const char *stringAsChar = error->localizedFailureReason()->utf8String();

          std::ofstream outfile;
          outfile.open("command_buffer_error.txt", std::fstream::out | std::fstream::app);
          outfile << stringAsChar;
          outfile.close();

          pool->drain();
          pool = nil;
        }
      }
    }

    /* Release previous frames command buffer and reset active cmd buffer. */
    if (m_last_submitted_command_buffer != nil) {

      KLI_assert(MTLBackend::get()->is_inside_render_boundary());
      m_last_submitted_command_buffer->autorelease();
      m_last_submitted_command_buffer = nil;
    }
    m_last_submitted_command_buffer = m_active_command_buffer;
    m_active_command_buffer = nil;

    return true;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Render Command Encoder Utility and management functions.
   * @{ */

  /* Fetch/query current encoder. */
  bool MTLCommandBufferManager::is_inside_render_pass()
  {
    return (m_active_command_encoder_type == MTL_RENDER_COMMAND_ENCODER);
  }

  bool MTLCommandBufferManager::is_inside_blit()
  {
    return (m_active_command_encoder_type == MTL_BLIT_COMMAND_ENCODER);
  }

  bool MTLCommandBufferManager::is_inside_compute()
  {
    return (m_active_command_encoder_type == MTL_COMPUTE_COMMAND_ENCODER);
  }

  MTL::RenderCommandEncoder *MTLCommandBufferManager::get_active_render_command_encoder()
  {
    /* Calling code should check if inside render pass. Otherwise nil. */
    return m_active_render_command_encoder;
  }

  MTL::BlitCommandEncoder *MTLCommandBufferManager::get_active_blit_command_encoder()
  {
    /* Calling code should check if inside render pass. Otherwise nil. */
    return m_active_blit_command_encoder;
  }

  MTL::ComputeCommandEncoder *MTLCommandBufferManager::get_active_compute_command_encoder()
  {
    /* Calling code should check if inside render pass. Otherwise nil. */
    return m_active_compute_command_encoder;
  }

  MTLFrameBuffer *MTLCommandBufferManager::get_active_framebuffer()
  {
    /* If outside of RenderPass, nullptr will be returned. */
    if (this->is_inside_render_pass()) {
      return m_active_frame_buffer;
    }
    return nullptr;
  }

  /* Encoder and Pass management. */
  /* End currently active MTLCommandEncoder. */
  bool MTLCommandBufferManager::end_active_command_encoder()
  {

    /* End active encoder if one is active. */
    if (m_active_command_encoder_type != MTL_NO_COMMAND_ENCODER) {

      switch (m_active_command_encoder_type) {
        case MTL_RENDER_COMMAND_ENCODER: {
          /* Verify a RenderCommandEncoder is active and end. */
          KLI_assert(m_active_render_command_encoder != nil);

          /* Complete Encoding. */
          m_active_render_command_encoder->endEncoding();
          m_active_render_command_encoder->release();
          m_active_render_command_encoder = nil;
          m_active_command_encoder_type = MTL_NO_COMMAND_ENCODER;

          /* Reset associated frame-buffer flag. */
          m_active_frame_buffer = nullptr;
          m_active_pass_descriptor = nullptr;
          return true;
        }

        case MTL_BLIT_COMMAND_ENCODER: {
          /* Verify a RenderCommandEncoder is active and end. */
          KLI_assert(m_active_blit_command_encoder != nil);
          m_active_blit_command_encoder->endEncoding();
          m_active_blit_command_encoder->release();
          m_active_blit_command_encoder = nil;
          m_active_command_encoder_type = MTL_NO_COMMAND_ENCODER;
          return true;
        }

        case MTL_COMPUTE_COMMAND_ENCODER: {
          /* Verify a RenderCommandEncoder is active and end. */
          KLI_assert(m_active_compute_command_encoder != nil);
          m_active_compute_command_encoder->endEncoding();
          m_active_compute_command_encoder->release();
          m_active_compute_command_encoder = nil;
          m_active_command_encoder_type = MTL_NO_COMMAND_ENCODER;
          return true;
        }

        default: {
          KLI_assert(false && "Invalid command encoder type");
          return false;
        }
      };
    } else {
      /* MTL_NO_COMMAND_ENCODER. */
      KLI_assert(m_active_render_command_encoder == nil);
      KLI_assert(m_active_blit_command_encoder == nil);
      KLI_assert(m_active_compute_command_encoder == nil);
      return false;
    }
  }

  MTL::RenderCommandEncoder *MTLCommandBufferManager::ensure_begin_render_command_encoder(
    MTLFrameBuffer *ctx_framebuffer,
    bool force_begin,
    bool *new_pass)
  {
    /* Ensure valid frame-buffer. */
    KLI_assert(ctx_framebuffer != nullptr);

    /* Ensure active command buffer. */
    MTL::CommandBuffer *cmd_buf = this->ensure_begin();
    KLI_assert(cmd_buf);

    /* Begin new command encoder if the currently active one is
     * incompatible or requires updating. */
    if (m_active_command_encoder_type != MTL_RENDER_COMMAND_ENCODER ||
        m_active_frame_buffer != ctx_framebuffer || force_begin) {
      this->end_active_command_encoder();

      /* Determine if this is a re-bind of the same frame-buffer. */
      bool is_rebind = (m_active_frame_buffer == ctx_framebuffer);

      /* Generate RenderPassDescriptor from bound frame-buffer. */
      KLI_assert(ctx_framebuffer);
      m_active_frame_buffer = ctx_framebuffer;
      m_active_pass_descriptor = m_active_frame_buffer->bake_render_pass_descriptor(
        is_rebind && (!m_active_frame_buffer->get_pending_clear()));

      /* Determine if there is a visibility buffer assigned to the context. */
      gpu::MTLBuffer *visibility_buffer = m_context.get_visibility_buffer();
      this->m_active_pass_descriptor->setVisibilityResultBuffer(
        (visibility_buffer) ? visibility_buffer->get_metal_buffer() : nil);
      m_context.clear_visibility_dirty();

      /* Ensure we have already cleaned up our previous render command encoder. */
      KLI_assert(m_active_render_command_encoder == nil);

      /* Create new RenderCommandEncoder based on descriptor (and begin encoding). */
      m_active_render_command_encoder = cmd_buf->renderCommandEncoder(m_active_pass_descriptor);
      m_active_render_command_encoder->retain();
      m_active_command_encoder_type = MTL_RENDER_COMMAND_ENCODER;

      /* Update command buffer encoder heuristics. */
      this->register_encoder_counters();

      /* Apply initial state. */
      /* Update Viewport and Scissor State */
      m_active_frame_buffer->apply_state();

      /* FLAG FRAMEBUFFER AS CLEARED -- A clear only lasts as long as one has been specified.
       * After this, resets to Load attachments to parallel GL behavior. */
      m_active_frame_buffer->mark_cleared();

      /* Reset RenderPassState to ensure resource bindings are re-applied. */
      m_render_pass_state.reset_state();

      /* Return true as new pass started. */
      *new_pass = true;
    } else {
      /* No new pass. */
      *new_pass = false;
    }

    KLI_assert(m_active_render_command_encoder != nil);
    return m_active_render_command_encoder;
  }

  MTL::BlitCommandEncoder *MTLCommandBufferManager::ensure_begin_blit_encoder()
  {
    /* Ensure active command buffer. */
    MTL::CommandBuffer *cmd_buf = this->ensure_begin();
    KLI_assert(cmd_buf);

    /* Ensure no existing command encoder of a different type is active. */
    if (m_active_command_encoder_type != MTL_BLIT_COMMAND_ENCODER) {
      this->end_active_command_encoder();
    }

    /* Begin new Blit Encoder. */
    if (m_active_blit_command_encoder == nil) {
      m_active_blit_command_encoder = cmd_buf->blitCommandEncoder();
      KLI_assert(m_active_blit_command_encoder != nil);
      m_active_blit_command_encoder->retain();
      m_active_command_encoder_type = MTL_BLIT_COMMAND_ENCODER;

      /* Update command buffer encoder heuristics. */
      this->register_encoder_counters();
    }
    KLI_assert(m_active_blit_command_encoder != nil);
    return m_active_blit_command_encoder;
  }

  MTL::ComputeCommandEncoder *MTLCommandBufferManager::ensure_begin_compute_encoder()
  {
    /* Ensure active command buffer. */
    MTL::CommandBuffer *cmd_buf = this->ensure_begin();
    KLI_assert(cmd_buf);

    /* Ensure no existing command encoder of a different type is active. */
    if (m_active_command_encoder_type != MTL_COMPUTE_COMMAND_ENCODER) {
      this->end_active_command_encoder();
    }

    /* Begin new Compute Encoder. */
    if (m_active_compute_command_encoder == nil) {
      m_active_compute_command_encoder = cmd_buf->computeCommandEncoder();
      KLI_assert(m_active_compute_command_encoder != nil);
      m_active_compute_command_encoder->retain();
      m_active_command_encoder_type = MTL_COMPUTE_COMMAND_ENCODER;

      /* Update command buffer encoder heuristics. */
      this->register_encoder_counters();
    }
    KLI_assert(m_active_compute_command_encoder != nil);
    return m_active_compute_command_encoder;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Command buffer heuristics.
   * @{ */

  /* Rendering Heuristics. */
  void MTLCommandBufferManager::register_draw_counters(int vertex_submission)
  {
    m_current_draw_call_count++;
    m_vertex_submitted_count += vertex_submission;
    m_empty = false;
  }

  /* Reset workload counters. */
  void MTLCommandBufferManager::reset_counters()
  {
    m_empty = true;
    m_current_draw_call_count = 0;
    m_encoder_count = 0;
    m_vertex_submitted_count = 0;
  }

  /* Workload evaluation. */
  bool MTLCommandBufferManager::do_break_submission()
  {
    /* Skip if no active command buffer. */
    if (m_active_command_buffer == nil) {
      return false;
    }

    /* Use optimized heuristic to split heavy command buffer submissions to better saturate the
     * hardware and also reduce stalling from individual large submissions. */
    if (GPU_type_matches(GPU_DEVICE_INTEL, GPU_OS_ANY, GPU_DRIVER_ANY) ||
        GPU_type_matches(GPU_DEVICE_ATI, GPU_OS_ANY, GPU_DRIVER_ANY)) {
      return ((m_current_draw_call_count > 30000) || (m_vertex_submitted_count > 100000000) ||
              (m_encoder_count > 25));
    } else {
      /* Apple Silicon is less efficient if splitting submissions. */
      return false;
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Command buffer debugging.
   * @{ */

  /* Debug. */
  void MTLCommandBufferManager::push_debug_group(const char *name, int index)
  {
    MTL::CommandBuffer *cmd = this->ensure_begin();
    if (cmd != nil) {
      char debug_msg[128];
      snprintf(debug_msg, sizeof(debug_msg), "%s_%d", name, index);
      cmd->pushDebugGroup(NS_STRING_(debug_msg));
    }
  }

  void MTLCommandBufferManager::pop_debug_group()
  {
    MTL::CommandBuffer *cmd = this->ensure_begin();
    if (cmd != nil) {
      cmd->popDebugGroup();
    }
  }

  /* Workload Synchronization. */
  bool MTLCommandBufferManager::insert_memory_barrier(eGPUBarrier barrier_bits,
                                                      eGPUStageBarrierBits before_stages,
                                                      eGPUStageBarrierBits after_stages)
  {
/* Only supporting Metal on 10.14 onward anyway - Check required for warnings. */
#if defined(MAC_OS_X_VERSION_10_14) && __MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_14
    /* Resolve scope. */
    MTL::BarrierScope scope = 0;
    if (barrier_bits & GPU_BARRIER_SHADER_IMAGE_ACCESS ||
        barrier_bits & GPU_BARRIER_TEXTURE_FETCH) {
      scope = scope | MTL::BarrierScopeTextures | MTL::BarrierScopeRenderTargets;
    }
    if (barrier_bits & GPU_BARRIER_SHADER_STORAGE ||
        barrier_bits & GPU_BARRIER_VERTEX_ATTRIB_ARRAY ||
        barrier_bits & GPU_BARRIER_ELEMENT_ARRAY) {
      scope = scope | MTL::BarrierScopeBuffers;
    }

    if (scope != 0) {
      /* Issue barrier based on encoder. */
      switch (m_active_command_encoder_type) {
        case MTL_NO_COMMAND_ENCODER:
        case MTL_BLIT_COMMAND_ENCODER: {
          /* No barrier to be inserted. */
          return false;
        }

        /* Rendering. */
        case MTL_RENDER_COMMAND_ENCODER: {
          /* Currently flagging both stages -- can use bits above to filter on stage type --
           * though full barrier is safe for now. */
          MTL::RenderStages before_stage_flags = 0;
          MTL::RenderStages after_stage_flags = 0;
          if (before_stages & GPU_BARRIER_STAGE_VERTEX &&
              !(before_stages & GPU_BARRIER_STAGE_FRAGMENT)) {
            before_stage_flags = before_stage_flags | MTL::RenderStageVertex;
          }
          if (before_stages & GPU_BARRIER_STAGE_FRAGMENT) {
            before_stage_flags = before_stage_flags | MTL::RenderStageFragment;
          }
          if (after_stages & GPU_BARRIER_STAGE_VERTEX) {
            after_stage_flags = after_stage_flags | MTL::RenderStageVertex;
          }
          if (after_stages & GPU_BARRIER_STAGE_FRAGMENT) {
            after_stage_flags = MTL::RenderStageFragment;
          }

          MTL::RenderCommandEncoder *rec = this->get_active_render_command_encoder();
          KLI_assert(rec != nil);
          rec->memoryBarrier(scope, after_stage_flags, before_stage_flags);
          return true;
        }

        /* Compute. */
        case MTL_COMPUTE_COMMAND_ENCODER: {
          MTL::ComputeCommandEncoder *rec = this->get_active_compute_command_encoder();
          KLI_assert(rec != nil);
          rec->memoryBarrier(scope);
          return true;
        }
      }
    }
#else
    /* No barrier support. */
    return false;
#endif
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Render Pass State for active RenderCommandEncoder
   * @{ */
  /* Reset binding state when a new RenderCommandEncoder is bound, to ensure
   * pipeline resources are re-applied to the new Encoder.
   * NOTE: In Metal, state is only persistent within an MTLCommandEncoder,
   * not globally. */
  void MTLRenderPassState::reset_state()
  {
    /* Reset Cached pipeline state. */
    this->bound_pso = nil;
    this->bound_ds_state = nil;

    /* Clear shader binding. */
    this->last_bound_shader_state.set(nullptr, 0);

    /* Other states. */
    MTLFrameBuffer *fb = this->cmd.get_active_framebuffer();
    this->last_used_stencil_ref_value = 0;
    this->last_scissor_rect = {0,
                               0,
                               (uint)((fb != nullptr) ? fb->get_width() : 0),
                               (uint)((fb != nullptr) ? fb->get_height() : 0)};

    /* Reset cached resource binding state */
    for (int ubo = 0; ubo < MTL_MAX_UNIFORM_BUFFER_BINDINGS; ubo++) {
      this->cached_vertex_buffer_bindings[ubo].is_bytes = false;
      this->cached_vertex_buffer_bindings[ubo].metal_buffer = nil;
      this->cached_vertex_buffer_bindings[ubo].offset = -1;

      this->cached_fragment_buffer_bindings[ubo].is_bytes = false;
      this->cached_fragment_buffer_bindings[ubo].metal_buffer = nil;
      this->cached_fragment_buffer_bindings[ubo].offset = -1;
    }

    /* Reset cached texture and sampler state binding state. */
    for (int tex = 0; tex < MTL_MAX_TEXTURE_SLOTS; tex++) {
      this->cached_vertex_texture_bindings[tex].metal_texture = nil;
      this->cached_vertex_sampler_state_bindings[tex].sampler_state = nil;
      this->cached_vertex_sampler_state_bindings[tex].is_arg_buffer_binding = false;

      this->cached_fragment_texture_bindings[tex].metal_texture = nil;
      this->cached_fragment_sampler_state_bindings[tex].sampler_state = nil;
      this->cached_fragment_sampler_state_bindings[tex].is_arg_buffer_binding = false;
    }
  }

  /* Bind Texture to current RenderCommandEncoder. */
  void MTLRenderPassState::bind_vertex_texture(MTL::Texture *tex, uint slot)
  {
    if (this->cached_vertex_texture_bindings[slot].metal_texture != tex) {
      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      KLI_assert(rec != nil);
      rec->setVertexTexture(tex, slot);
      this->cached_vertex_texture_bindings[slot].metal_texture = tex;
    }
  }

  void MTLRenderPassState::bind_fragment_texture(MTL::Texture *tex, uint slot)
  {
    if (this->cached_fragment_texture_bindings[slot].metal_texture != tex) {
      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      KLI_assert(rec != nil);
      rec->setFragmentTexture(tex, slot);
      this->cached_fragment_texture_bindings[slot].metal_texture = tex;
    }
  }

  void MTLRenderPassState::bind_vertex_sampler(MTLSamplerBinding &sampler_binding,
                                               bool use_argument_buffer_for_samplers,
                                               uint slot)
  {
    /* Range check. */
    const MTLShaderInterface *shader_interface = ctx.pipeline_state.active_shader->get_interface();
    KLI_assert(slot >= 0);
    KLI_assert(slot <= shader_interface->get_max_texture_index());
    KLI_assert(slot < MTL_MAX_TEXTURE_SLOTS);
    UNUSED_VARS_NDEBUG(shader_interface);

    /* If sampler state has not changed for the given slot, we do not need to fetch. */
    if (this->cached_vertex_sampler_state_bindings[slot].sampler_state == nil ||
        !(this->cached_vertex_sampler_state_bindings[slot].binding_state ==
          sampler_binding.state) ||
        use_argument_buffer_for_samplers) {

      MTL::SamplerState *sampler_state = (sampler_binding.state == DEFAULT_SAMPLER_STATE) ?
                                           ctx.get_default_sampler_state() :
                                           ctx.get_sampler_from_state(sampler_binding.state);
      if (!use_argument_buffer_for_samplers) {
        /* Update binding and cached state. */
        MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
        KLI_assert(rec != nil);
        rec->setVertexSamplerState(sampler_state, slot);
        this->cached_vertex_sampler_state_bindings[slot].binding_state = sampler_binding.state;
        this->cached_vertex_sampler_state_bindings[slot].sampler_state = sampler_state;
      }

      /* Flag last binding type. */
      this->cached_vertex_sampler_state_bindings[slot].is_arg_buffer_binding =
        use_argument_buffer_for_samplers;

      /* Always assign to argument buffer samplers binding array - Efficiently ensures the value in
       * the samplers array is always up to date. */
      ctx.m_samplers.mtl_sampler[slot] = sampler_state;
      ctx.m_samplers.mtl_sampler_flags[slot] = sampler_binding.state;
    }
  }

  void MTLRenderPassState::bind_fragment_sampler(MTLSamplerBinding &sampler_binding,
                                                 bool use_argument_buffer_for_samplers,
                                                 uint slot)
  {
    /* Range check. */
    const MTLShaderInterface *shader_interface = ctx.pipeline_state.active_shader->get_interface();
    KLI_assert(slot >= 0);
    KLI_assert(slot <= shader_interface->get_max_texture_index());
    KLI_assert(slot < MTL_MAX_TEXTURE_SLOTS);
    UNUSED_VARS_NDEBUG(shader_interface);

    /* If sampler state has not changed for the given slot, we do not need to fetch*/
    if (this->cached_fragment_sampler_state_bindings[slot].sampler_state == nil ||
        !(this->cached_fragment_sampler_state_bindings[slot].binding_state ==
          sampler_binding.state) ||
        use_argument_buffer_for_samplers) {

      MTL::SamplerState *sampler_state = (sampler_binding.state == DEFAULT_SAMPLER_STATE) ?
                                           ctx.get_default_sampler_state() :
                                           ctx.get_sampler_from_state(sampler_binding.state);
      if (!use_argument_buffer_for_samplers) {
        /* Update binding and cached state. */
        MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
        KLI_assert(rec != nil);
        rec->setFragmentSamplerState(sampler_state, slot);
        this->cached_fragment_sampler_state_bindings[slot].binding_state = sampler_binding.state;
        this->cached_fragment_sampler_state_bindings[slot].sampler_state = sampler_state;
      }

      /* Flag last binding type */
      this->cached_fragment_sampler_state_bindings[slot].is_arg_buffer_binding =
        use_argument_buffer_for_samplers;

      /* Always assign to argument buffer samplers binding array - Efficiently ensures the value in
       * the samplers array is always up to date. */
      ctx.m_samplers.mtl_sampler[slot] = sampler_state;
      ctx.m_samplers.mtl_sampler_flags[slot] = sampler_binding.state;
    }
  }

  void MTLRenderPassState::bind_vertex_buffer(MTL::Buffer *buffer, uint buffer_offset, uint index)
  {
    KLI_assert(index >= 0);
    KLI_assert(buffer_offset >= 0);
    KLI_assert(buffer != nil);

    BufferBindingCached &current_vert_ubo_binding = this->cached_vertex_buffer_bindings[index];
    if (current_vert_ubo_binding.offset != buffer_offset ||
        current_vert_ubo_binding.metal_buffer != buffer || current_vert_ubo_binding.is_bytes) {

      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      KLI_assert(rec != nil);

      if (current_vert_ubo_binding.metal_buffer == buffer) {
        /* If buffer is the same, but offset has changed. */
        rec->setVertexBufferOffset(buffer_offset, index);
      } else {
        /* Bind Vertex Buffer. */
        rec->setVertexBuffer(buffer, buffer_offset, index);
      }

      /* Update Bind-state cache. */
      this->cached_vertex_buffer_bindings[index].is_bytes = false;
      this->cached_vertex_buffer_bindings[index].metal_buffer = buffer;
      this->cached_vertex_buffer_bindings[index].offset = buffer_offset;
    }
  }

  void MTLRenderPassState::bind_fragment_buffer(MTL::Buffer *buffer,
                                                uint buffer_offset,
                                                uint index)
  {
    KLI_assert(index >= 0);
    KLI_assert(buffer_offset >= 0);
    KLI_assert(buffer != nil);

    BufferBindingCached &current_frag_ubo_binding = this->cached_fragment_buffer_bindings[index];
    if (current_frag_ubo_binding.offset != buffer_offset ||
        current_frag_ubo_binding.metal_buffer != buffer || current_frag_ubo_binding.is_bytes) {

      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      KLI_assert(rec != nil);

      if (current_frag_ubo_binding.metal_buffer == buffer) {
        /* If buffer is the same, but offset has changed. */
        rec->setFragmentBufferOffset(buffer_offset, index);
      } else {
        /* Bind Fragment Buffer */
        rec->setFragmentBuffer(buffer, buffer_offset, index);
      }

      /* Update Bind-state cache */
      this->cached_fragment_buffer_bindings[index].is_bytes = false;
      this->cached_fragment_buffer_bindings[index].metal_buffer = buffer;
      this->cached_fragment_buffer_bindings[index].offset = buffer_offset;
    }
  }

  void MTLRenderPassState::bind_vertex_bytes(void *bytes, uint length, uint index)
  {
    /* Bytes always updated as source data may have changed. */
    KLI_assert(index >= 0 && index < MTL_MAX_UNIFORM_BUFFER_BINDINGS);
    KLI_assert(length > 0);
    KLI_assert(bytes != nullptr);

    if (length < MTL_MAX_SET_BYTES_SIZE) {
      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      rec->setVertexBytes(bytes, length, index);
    } else {
      /* We have run over the setBytes limit, bind buffer instead. */
      MTLTemporaryBuffer range =
        ctx.get_scratchbuffer_manager().scratch_buffer_allocate_range_aligned(length, 256);
      memcpy(range.data, bytes, length);
      this->bind_vertex_buffer(range.metal_buffer, range.buffer_offset, index);
    }

    /* Update Bind-state cache */
    this->cached_vertex_buffer_bindings[index].is_bytes = true;
    this->cached_vertex_buffer_bindings[index].metal_buffer = nil;
    this->cached_vertex_buffer_bindings[index].offset = -1;
  }

  void MTLRenderPassState::bind_fragment_bytes(void *bytes, uint length, uint index)
  {
    /* Bytes always updated as source data may have changed. */
    KLI_assert(index >= 0 && index < MTL_MAX_UNIFORM_BUFFER_BINDINGS);
    KLI_assert(length > 0);
    KLI_assert(bytes != nullptr);

    if (length < MTL_MAX_SET_BYTES_SIZE) {
      MTL::RenderCommandEncoder *rec = this->cmd.get_active_render_command_encoder();
      rec->setFragmentBytes(bytes, length, index);
    } else {
      /* We have run over the setBytes limit, bind buffer instead. */
      MTLTemporaryBuffer range =
        ctx.get_scratchbuffer_manager().scratch_buffer_allocate_range_aligned(length, 256);
      memcpy(range.data, bytes, length);
      this->bind_fragment_buffer(range.metal_buffer, range.buffer_offset, index);
    }

    /* Update Bind-state cache. */
    this->cached_fragment_buffer_bindings[index].is_bytes = true;
    this->cached_fragment_buffer_bindings[index].metal_buffer = nil;
    this->cached_fragment_buffer_bindings[index].offset = -1;
  }

  /** @} */

}  // namespace kraken::gpu
