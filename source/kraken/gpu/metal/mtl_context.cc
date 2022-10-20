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

#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_framebuffer.hh"
#include "mtl_immediate.hh"
#include "mtl_memory.hh"
// #include "mtl_primitive.hh"
#include "mtl_shader.hh"
#include "mtl_shader_interface.hh"
#include "mtl_state.hh"
#include "mtl_texture.hh"
// #include "mtl_uniform_buffer.hh"

#include "ANCHOR_api.h"
#include "ANCHOR_internal.h"
#include "backend/ANCHOR_CONTEXT_metal.h"

#include "USD_userdef_types.h"

#include "GPU_capabilities.h"
#include "GPU_matrix.h"
#include "GPU_shader.h"
#include "GPU_texture.h"
#include "GPU_uniform_buffer.h"
#include "GPU_vertex_buffer.h"
#include "intern/gpu_matrix_private.h"

#include "KLI_time.h"

#include "KKE_global.h"

#include <fstream>
#include <string>

using namespace kraken;
using namespace kraken::gpu;

namespace kraken::gpu
{

  /* Global memory manager. */
  MTLBufferPool MTLContext::global_memory_manager;

  /* Swap-chain and latency management. */
  std::atomic<int> MTLContext::max_drawables_in_flight = 0;
  std::atomic<int64_t> MTLContext::avg_drawable_latency_us = 0;
  int64_t MTLContext::frame_latency[MTL_FRAME_AVERAGE_COUNT] = {0};

  /* -------------------------------------------------------------------- */
  /** \name Anchor Context interaction.
   * \{ */

  void MTLContext::set_anchor_context(AnchorContext *anchorCtxHandle)
  {
    AnchorContext *anchor_ctx = reinterpret_cast<AnchorContext *>(anchorCtxHandle);
    KLI_assert(anchor_ctx != nullptr);

    /* Release old MTLTexture handle */
    if (m_default_fbo_mtltexture) {
      m_default_fbo_mtltexture->release();
      m_default_fbo_mtltexture = nil;
    }

    /* Release Framebuffer attachments */
    MTLFrameBuffer *mtl_front_left = static_cast<MTLFrameBuffer *>(this->front_left);
    MTLFrameBuffer *mtl_back_left = static_cast<MTLFrameBuffer *>(this->back_left);
    mtl_front_left->remove_all_attachments();
    mtl_back_left->remove_all_attachments();

    AnchorContextMetal *anchor_metal_ctx = dynamic_cast<AnchorContextMetal *>(anchor_ctx);
    if (anchor_metal_ctx != NULL) {
      m_default_fbo_mtltexture = anchor_metal_ctx->GetMetalOverlayTexture();

      MTL_LOG_INFO(
        "Binding ANCHOR context METAL %p to GPU context %p. (Device: %p, queue: %p, texture: "
        "%p)\n",
        anchor_metal_ctx,
        this,
        this->device,
        this->queue,
        m_default_fbo_gputexture);

      /* Check if the ANCHOR Context provides a default framebuffer: */
      if (m_default_fbo_mtltexture) {

        /* Release old GPUTexture handle */
        if (m_default_fbo_gputexture) {
          GPU_texture_free(wrap(static_cast<Texture *>(m_default_fbo_gputexture)));
          m_default_fbo_gputexture = nullptr;
        }

        /* Retain handle */
        m_default_fbo_mtltexture->retain();

        /*** Create front and back-buffers ***/
        /* Create gpu::MTLTexture objects */
        m_default_fbo_gputexture = new gpu::MTLTexture("MTL_BACKBUFFER",
                                                       GPU_RGBA16F,
                                                       GPU_TEXTURE_2D,
                                                       m_default_fbo_mtltexture);

        /* Update frame-buffers with new texture attachments. */
        mtl_front_left->add_color_attachment(m_default_fbo_gputexture, 0, 0, 0);
        mtl_back_left->add_color_attachment(m_default_fbo_gputexture, 0, 0, 0);
#ifndef NDEBUG
        this->label = m_default_fbo_mtltexture->label();
#endif
      } else {

        /* Add default texture for cases where no other framebuffer is bound */
        if (!m_default_fbo_gputexture) {
          m_default_fbo_gputexture = static_cast<gpu::MTLTexture *>(
            unwrap(GPU_texture_create_2d(__func__, 16, 16, 1, GPU_RGBA16F, nullptr)));
        }
        mtl_back_left->add_color_attachment(m_default_fbo_gputexture, 0, 0, 0);

        MTL_LOG_INFO(
          "-- Bound context %p for GPU context: %p is offscreen and does not have a default "
          "framebuffer\n",
          anchor_metal_ctx,
          this);
#ifndef NDEBUG
        this->label = NS_STRING_("Offscreen Metal Context");
#endif
      }
    } else {
      MTL_LOG_INFO(
        "[ERROR] Failed to bind ANCHOR context to MTLContext -- AnchorContextMetal is null "
        "(AnchorContext: %p, AnchorContextMetal: %p)\n",
        anchor_ctx,
        anchor_metal_ctx);
      KLI_assert(false);
    }
  }

  void MTLContext::set_anchor_window(AnchorSystemWindowHandle anchorWinHandle)
  {
    AnchorSystemWindow *anchorWin = reinterpret_cast<AnchorSystemWindow *>(anchorWinHandle);
    this->set_anchor_context(ANCHOR::GetCurrentContext());
  }

  /** \} */

  MTLContext::MTLContext(void *anchor_window, void *anchor_context)
    : memory_manager(*this),
      main_command_buffer(*this)
  {
    /* Init debug. */
    debug::mtl_debug_init();

    /* Initialize Render-pass and Frame-buffer State. */
    this->back_left = nullptr;

    /* Initialize command buffer state. */
    this->main_command_buffer.prepare();

    /* Initialize IMM and pipeline state */
    this->pipeline_state.initialised = false;

    /* Frame management. */
    m_is_inside_frame = false;
    m_current_frame_index = 0;

    /* Prepare null data buffer. */
    m_null_buffer = nil;
    m_null_attribute_buffer = nil;

    /* Zero-initialize MTL textures. */
    m_default_fbo_mtltexture = nil;
    m_default_fbo_gputexture = nullptr;

    /** Fetch AnchorContextMetal and fetch Metal device/queue. */
    m_anchor_window = anchor_window;
    if (m_anchor_window && anchor_context == NULL) {
      /* NOTE(Metal): Fetch anchor_context from anchor_window if it is not provided.
       * Regardless of whether windowed or not, we need access to the AnchorContextMetal
       * for presentation, and device/queue access. */
      AnchorSystemWindow *anchorWin = reinterpret_cast<AnchorSystemWindow *>(m_anchor_window);
      anchor_context = ANCHOR::GetCurrentContext();
    }
    KLI_assert(anchor_context);
    this->m_anchor_ctx = static_cast<AnchorContextMetal *>(anchor_context);
    this->queue = this->m_anchor_ctx->GetMetalCommandQueue();
    this->device = this->m_anchor_ctx->GetMetalDevice();
    KLI_assert(this->queue);
    KLI_assert(this->device);
    this->queue->retain();
    this->device->retain();

    /* Register present callback. */
    this->m_anchor_ctx->RegisterMetalPresentCallback(&present);

    /* Create FrameBuffer handles. */
    MTLFrameBuffer *mtl_front_left = new MTLFrameBuffer(this, "front_left");
    MTLFrameBuffer *mtl_back_left = new MTLFrameBuffer(this, "back_left");
    this->front_left = mtl_front_left;
    this->back_left = mtl_back_left;
    this->active_fb = this->back_left;

    /* Prepare platform and capabilities. (NOTE: With METAL, this needs to be done after CTX
     * initialization). */
    MTLBackend::platform_init(this);
    MTLBackend::capabilities_init(this);

    /* Initialize Metal modules. */
    this->memory_manager.init();
    this->state_manager = new MTLStateManager(this);
    this->imm = new MTLImmediate(this);

    /* Ensure global memory manager is initialized. */
    MTLContext::global_memory_manager.init(this->device);

    /* Initialize texture read/update structures. */
    this->get_texture_utils().init();

    /* Bound Samplers struct. */
    for (int i = 0; i < MTL_MAX_TEXTURE_SLOTS; i++) {
      m_samplers.mtl_sampler[i] = nil;
      m_samplers.mtl_sampler_flags[i] = DEFAULT_SAMPLER_STATE;
    }

    /* Initialize samplers. */
    for (uint i = 0; i < GPU_SAMPLER_MAX; i++) {
      MTLSamplerState state;
      state.state = static_cast<eGPUSamplerState>(i);
      m_sampler_state_cache[i] = this->generate_sampler_from_state(state);
    }
  }

  /* -------------------------------------------------------------------- */
  /** @name Swap-chain management and Metal presentation.
   * @{ */

  void present(MTL::RenderPassDescriptor *blit_descriptor,
               MTL::RenderPipelineState *blit_pso,
               MTL::Texture *swapchain_texture,
               CA::MetalDrawable *drawable)
  {

    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /* Flush any outstanding work. */
    ctx->flush();

    /* Always pace CPU to maximum of 3 drawables in flight.
     * nextDrawable may have more in flight if backing swapchain
     * textures are re-allocate, such as during resize events.
     *
     * Determine frames in flight based on current latency. If
     * we are in a high-latency situation, limit frames in flight
     * to increase app responsiveness and keep GPU execution under control.
     * If latency improves, increase frames in flight to improve overall
     * performance. */
    int perf_max_drawables = MTL_MAX_DRAWABLES;
    if (MTLContext::avg_drawable_latency_us > 185000) {
      perf_max_drawables = 1;
    } else if (MTLContext::avg_drawable_latency_us > 85000) {
      perf_max_drawables = 2;
    }

    while (MTLContext::max_drawables_in_flight > min_ii(perf_max_drawables, MTL_MAX_DRAWABLES)) {
      PIL_sleep_ms(2);
    }

    /* Present is submitted in its own CMD Buffer to ensure drawable reference released as early
     * as possible. This command buffer is separate as it does not utilize the global state for
     * rendering as the main context does. */
    MTL::CommandBuffer *cmdbuf = ctx->queue->commandBuffer();
    MTLCommandBufferManager::num_active_cmd_bufs++;

    if (MTLCommandBufferManager::sync_event != nil) {
      /* Ensure command buffer ordering. */
      cmdbuf->encodeWait(MTLCommandBufferManager::sync_event,
                         MTLCommandBufferManager::event_signal_val);
    }

    /* Do Present Call and final Blit to MTLDrawable. */
    MTL::RenderCommandEncoder *enc = cmdbuf->renderCommandEncoder(blit_descriptor);
    enc->setRenderPipelineState(blit_pso);
    enc->setFragmentTexture(swapchain_texture, 0);
    enc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
    enc->endEncoding();

    /* Present drawable. */
    KLI_assert(drawable);
    cmdbuf->presentDrawable(drawable);

    /* Ensure freed buffers have usage tracked against active CommandBuffer submissions. */
    MTLSafeFreeList *cmd_free_buffer_list =
      MTLContext::get_global_memory_manager().get_current_safe_list();
    KLI_assert(cmd_free_buffer_list);

    MTL::CommandBuffer *cmd_buffer_ref = cmdbuf;
    cmd_buffer_ref->retain();

    /* Increment drawables in flight limiter. */
    MTLContext::max_drawables_in_flight++;
    std::chrono::time_point submission_time = std::chrono::high_resolution_clock::now();

    /* Increment free pool reference and decrement upon command buffer completion. */
    cmd_free_buffer_list->increment_reference();
    cmdbuf->addCompletedHandler([&](MTL::CommandBuffer *buffer) -> void {
      /* Flag freed buffers associated with this CMD buffer as ready to be freed. */
      cmd_free_buffer_list->decrement_reference();
      cmd_buffer_ref->release();

      /* Decrement count */
      MTLCommandBufferManager::num_active_cmd_bufs--;
      MTL_LOG_INFO("[Metal] Active command buffers: %d\n",
                   MTLCommandBufferManager::num_active_cmd_bufs);

      /* Drawable count and latency management. */
      MTLContext::max_drawables_in_flight--;
      std::chrono::time_point completion_time = std::chrono::high_resolution_clock::now();
      int64_t microseconds_per_frame = std::chrono::duration_cast<std::chrono::microseconds>(
                                         completion_time - submission_time)
                                         .count();
      MTLContext::latency_resolve_average(microseconds_per_frame);

      MTL_LOG_INFO("Frame Latency: %f ms  (Rolling avg: %f ms  Drawables: %d)\n",
                   ((float)microseconds_per_frame) / 1000.0f,
                   ((float)MTLContext::avg_drawable_latency_us) / 1000.0f,
                   perf_max_drawables);
    });

    if (MTLCommandBufferManager::sync_event == nil) {
      MTLCommandBufferManager::sync_event = ctx->device->newEvent();
      KLI_assert(MTLCommandBufferManager::sync_event);
      MTLCommandBufferManager::sync_event->retain();
    }
    KLI_assert(MTLCommandBufferManager::sync_event != nil);

    MTLCommandBufferManager::event_signal_val++;
    cmdbuf->encodeSignalEvent(MTLCommandBufferManager::sync_event,
                              MTLCommandBufferManager::event_signal_val);

    cmdbuf->commit();

    /* When debugging, fetch advanced command buffer errors. */
    if (G.debug & G_DEBUG_GPU) {
      cmdbuf->waitUntilCompleted();
      NS::Error *error = cmdbuf->error();
      if (error != nil) {
        printf("%s", error->description()->utf8String());
        KLI_assert(false);

        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        const char *stringAsChar = error->description()->utf8String();

        std::ofstream outfile;
        outfile.open("command_buffer_error.txt", std::fstream::out | std::fstream::app);
        outfile << stringAsChar;
        outfile.close();

        pool->drain();
        pool = nil;

      } else {
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        NS::String *str = NS_STRING_("Command buffer completed successfully!\n");
        const char *stringAsChar = str->utf8String();

        std::ofstream outfile;
        outfile.open("command_buffer_error.txt", std::fstream::out | std::fstream::app);
        outfile << stringAsChar;
        outfile.close();

        pool->drain();
        pool = nil;
      }
    }
  }

  MTL::SamplerState *MTLContext::generate_sampler_from_state(MTLSamplerState sampler_state)
  {
    /* Check if sampler already exists for given state. */
    MTL::SamplerDescriptor *descriptor = MTL::SamplerDescriptor::alloc()->init();
    descriptor->setNormalizedCoordinates(true);

    MTL::SamplerAddressMode clamp_type = (sampler_state.state & GPU_SAMPLER_CLAMP_BORDER) ?
                                           MTL::SamplerAddressModeClampToBorderColor :
                                           MTL::SamplerAddressModeClampToEdge;
    descriptor->setRAddressMode(
      (sampler_state.state & GPU_SAMPLER_REPEAT_R) ? MTL::SamplerAddressModeRepeat : clamp_type);
    descriptor->setSAddressMode(
      (sampler_state.state & GPU_SAMPLER_REPEAT_S) ? MTL::SamplerAddressModeRepeat : clamp_type);
    descriptor->setTAddressMode(
      (sampler_state.state & GPU_SAMPLER_REPEAT_T) ? MTL::SamplerAddressModeRepeat : clamp_type);
    descriptor->setBorderColor(MTL::SamplerBorderColorTransparentBlack);
    descriptor->setMinFilter((sampler_state.state & GPU_SAMPLER_FILTER) ?
                               MTL::SamplerMinMagFilterLinear :
                               MTL::SamplerMinMagFilterNearest);
    descriptor->setMagFilter((sampler_state.state & GPU_SAMPLER_FILTER) ?
                               MTL::SamplerMinMagFilterLinear :
                               MTL::SamplerMinMagFilterNearest);
    descriptor->setMipFilter((sampler_state.state & GPU_SAMPLER_MIPMAP) ?
                               MTL::SamplerMipFilterLinear :
                               MTL::SamplerMipFilterNotMipmapped);
    descriptor->setLodMinClamp(-1000);
    descriptor->setLodMaxClamp(1000);
    float aniso_filter = max_ff(16, U.anisotropic_filter);
    descriptor->setMaxAnisotropy((sampler_state.state & GPU_SAMPLER_MIPMAP) ? aniso_filter : 1);
    descriptor->setCompareFunction((sampler_state.state & GPU_SAMPLER_COMPARE) ?
                                     MTL::CompareFunctionLessEqual :
                                     MTL::CompareFunctionAlways);
    descriptor->setSupportArgumentBuffers(true);

    MTL::SamplerState *state = this->device->newSamplerState(descriptor);
    m_sampler_state_cache[(uint)sampler_state] = state;

    KLI_assert(state != nil);
    descriptor->autorelease();
    return state;
  }

}  // namespace kraken::gpu
