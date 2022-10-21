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

#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_framebuffer.hh"
#include "mtl_immediate.hh"
#include "mtl_memory.hh"
#include "mtl_primitive.hh"
#include "mtl_shader.hh"
#include "mtl_shader_interface.hh"
#include "mtl_state.hh"
#include "mtl_texture.hh"
#include "mtl_vertex_buffer.hh"
#include "mtl_uniform_buffer.hh"

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

  void MTLContext::set_anchor_context(AnchorContextHandle anchorCtxHandle)
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
    this->set_anchor_context((AnchorContextHandle)(ANCHOR::GetCurrentContext()));
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name MTLContext
   * @{ */

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

  void MTLContext::begin_frame()
  {
    KLI_assert(MTLBackend::get()->is_inside_render_boundary());
    if (this->get_inside_frame()) {
      return;
    }

    /* Begin Command buffer for next frame. */
    m_is_inside_frame = true;
  }

  void MTLContext::end_frame()
  {
    KLI_assert(this->get_inside_frame());

    /* Ensure pre-present work is committed. */
    this->flush();

    /* Increment frame counter. */
    m_is_inside_frame = false;
  }

  void MTLContext::check_error(const char *info)
  {
    /* TODO(Metal): Implement. */
  }

  void MTLContext::activate()
  {
    /* Make sure no other context is already bound to this thread. */
    KLI_assert(m_is_active == false);
    m_is_active = true;
    m_thread = pthread_self();

    /* Re-apply anchor window/context for resizing */
    if (m_anchor_window) {
      this->set_anchor_window((AnchorSystemWindowHandle)m_anchor_window);
    } else if (m_anchor_ctx) {
      this->set_anchor_context((AnchorContextHandle)m_anchor_ctx);
    }

    /* Reset UBO bind state. */
    for (int i = 0; i < MTL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
      if (this->pipeline_state.ubo_bindings[i].bound &&
          this->pipeline_state.ubo_bindings[i].ubo != nullptr) {
        this->pipeline_state.ubo_bindings[i].bound = false;
        this->pipeline_state.ubo_bindings[i].ubo = nullptr;
      }
    }

    /* Ensure imm active. */
    immActivate();
  }

  void MTLContext::deactivate()
  {
    KLI_assert(this->is_active_on_thread());
    /* Flush context on deactivate. */
    this->flush();
    m_is_active = false;
    immDeactivate();
  }

  void MTLContext::flush()
  {
    this->main_command_buffer.submit(false);
  }

  void MTLContext::finish()
  {
    this->main_command_buffer.submit(true);
  }

  void MTLContext::memory_statistics_get(int *total_mem, int *free_mem)
  {
    /* TODO(Metal): Implement. */
    *total_mem = 0;
    *free_mem = 0;
  }

  void MTLContext::framebuffer_bind(MTLFrameBuffer *framebuffer)
  {
    /* We do not yet begin the pass -- We defer beginning the pass until a draw is requested. */
    KLI_assert(framebuffer);
    this->active_fb = framebuffer;
  }

  void MTLContext::framebuffer_restore()
  {
    /* Bind default framebuffer from context --
     * We defer beginning the pass until a draw is requested. */
    this->active_fb = this->back_left;
  }

  MTL::RenderCommandEncoder *MTLContext::ensure_begin_render_pass()
  {
    KLI_assert(this);

    /* Ensure the rendering frame has started. */
    if (!this->get_inside_frame()) {
      this->begin_frame();
    }

    /* Check whether a framebuffer is bound. */
    if (!this->active_fb) {
      KLI_assert(false && "No framebuffer is bound!");
      return this->main_command_buffer.get_active_render_command_encoder();
    }

    /* Ensure command buffer workload submissions are optimal --
     * Though do not split a batch mid-IMM recording. */
    if (this->main_command_buffer.do_break_submission() &&
        !((MTLImmediate *)(this->imm))->imm_is_recording()) {
      this->flush();
    }

    /* Begin pass or perform a pass switch if the active framebuffer has been changed, or if the
     * framebuffer state has been modified (is_dirty). */
    if (!this->main_command_buffer.is_inside_render_pass() ||
        this->active_fb != this->main_command_buffer.get_active_framebuffer() ||
        this->main_command_buffer.get_active_framebuffer()->get_dirty() ||
        this->is_visibility_dirty()) {

      /* Validate bound framebuffer before beginning render pass. */
      if (!static_cast<MTLFrameBuffer *>(this->active_fb)->validate_render_pass()) {
        MTL_LOG_WARNING("Framebuffer validation failed, falling back to default framebuffer\n");
        this->framebuffer_restore();

        if (!static_cast<MTLFrameBuffer *>(this->active_fb)->validate_render_pass()) {
          MTL_LOG_ERROR("CRITICAL: DEFAULT FRAMEBUFFER FAIL VALIDATION!!\n");
        }
      }

      /* Begin RenderCommandEncoder on main CommandBuffer. */
      bool new_render_pass = false;
      MTL::RenderCommandEncoder *new_enc =
        this->main_command_buffer.ensure_begin_render_command_encoder(
          static_cast<MTLFrameBuffer *>(this->active_fb),
          true,
          &new_render_pass);
      if (new_render_pass) {
        /* Flag context pipeline state as dirty - dynamic pipeline state need re-applying. */
        this->pipeline_state.dirty_flags = MTL_PIPELINE_STATE_ALL_FLAG;
      }
      return new_enc;
    }
    KLI_assert(!this->main_command_buffer.get_active_framebuffer()->get_dirty());
    return this->main_command_buffer.get_active_render_command_encoder();
  }

  MTLFrameBuffer *MTLContext::get_current_framebuffer()
  {
    MTLFrameBuffer *last_bound = static_cast<MTLFrameBuffer *>(this->active_fb);
    return last_bound ? last_bound : this->get_default_framebuffer();
  }

  MTLFrameBuffer *MTLContext::get_default_framebuffer()
  {
    return static_cast<MTLFrameBuffer *>(this->back_left);
  }

  MTLShader *MTLContext::get_active_shader()
  {
    return this->pipeline_state.active_shader;
  }

  MTL::Buffer *MTLContext::get_null_buffer()
  {
    if (m_null_buffer != nil) {
      return m_null_buffer;
    }

    static const int null_buffer_size = 4096;
    m_null_buffer = this->device->newBuffer(null_buffer_size, MTL::ResourceStorageModeManaged);
    m_null_buffer->retain();
    uint32_t *null_data = (uint32_t *)calloc(0, null_buffer_size);
    memcpy(m_null_buffer->contents(), null_data, null_buffer_size);
    m_null_buffer->didModifyRange(NS::Range::Make(0, null_buffer_size));
    free(null_data);

    KLI_assert(m_null_buffer != nil);
    return m_null_buffer;
  }

  MTL::Buffer *MTLContext::get_null_attribute_buffer()
  {
    if (m_null_attribute_buffer != nil) {
      return m_null_attribute_buffer;
    }

    /* Allocate Null buffer if it has not yet been created.
     * Min buffer size is 256 bytes -- though we only need 64 bytes of data. */
    static const int null_buffer_size = 256;
    m_null_attribute_buffer = this->device->newBuffer(null_buffer_size,
                                                      MTL::ResourceStorageModeManaged);
    KLI_assert(m_null_attribute_buffer != nil);
    m_null_attribute_buffer->retain();
    float data[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    memcpy(m_null_attribute_buffer->contents(), data, sizeof(float) * 4);
    m_null_attribute_buffer->didModifyRange(NS::Range::Make(0, null_buffer_size));

    return m_null_attribute_buffer;
  }

  gpu::MTLTexture *MTLContext::get_dummy_texture(eGPUTextureType type)
  {
    /* Decrement 1 from texture type as they start from 1 and go to 32 (inclusive). Remap to 0..31
     */
    gpu::MTLTexture *dummy_tex = m_dummy_textures[type - 1];
    if (dummy_tex != nullptr) {
      return dummy_tex;
    } else {
      GPUTexture *tex = nullptr;
      switch (type) {
        case GPU_TEXTURE_1D:
          tex = GPU_texture_create_1d("Dummy 1D", 128, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_1D_ARRAY:
          tex = GPU_texture_create_1d_array("Dummy 1DArray", 128, 1, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_2D:
          tex = GPU_texture_create_2d("Dummy 2D", 128, 128, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_2D_ARRAY:
          tex = GPU_texture_create_2d_array("Dummy 2DArray", 128, 128, 1, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_3D:
          tex =
            GPU_texture_create_3d("Dummy 3D", 128, 128, 1, 1, GPU_RGBA8, GPU_DATA_UBYTE, nullptr);
          break;
        case GPU_TEXTURE_CUBE:
          tex = GPU_texture_create_cube("Dummy Cube", 128, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_CUBE_ARRAY:
          tex = GPU_texture_create_cube_array("Dummy CubeArray", 128, 1, 1, GPU_RGBA8, nullptr);
          break;
        case GPU_TEXTURE_BUFFER:
          if (!m_dummy_verts) {
            GPU_vertformat_clear(&m_dummy_vertformat);
            GPU_vertformat_attr_add(&m_dummy_vertformat,
                                    "dummy",
                                    GPU_COMP_F32,
                                    4,
                                    GPU_FETCH_FLOAT);
            m_dummy_verts = GPU_vertbuf_create_with_format_ex(&m_dummy_vertformat,
                                                              GPU_USAGE_STATIC);
            GPU_vertbuf_data_alloc(m_dummy_verts, 64);
          }
          tex = GPU_texture_create_from_vertbuf("Dummy TextureBuffer", m_dummy_verts);
          break;
        default:
          KLI_assert_msg(false, "Unrecognised texture type");
          return nullptr;
      }
      gpu::MTLTexture *metal_tex = static_cast<gpu::MTLTexture *>(
        reinterpret_cast<Texture *>(tex));
      m_dummy_textures[type - 1] = metal_tex;
      return metal_tex;
    }
    return nullptr;
  }

  void MTLContext::free_dummy_resources()
  {
    for (int tex = 0; tex < GPU_TEXTURE_BUFFER; tex++) {
      if (m_dummy_textures[tex]) {
        GPU_texture_free(
          reinterpret_cast<GPUTexture *>(static_cast<Texture *>(m_dummy_textures[tex])));
        m_dummy_textures[tex] = nullptr;
      }
    }
    if (m_dummy_verts) {
      GPU_vertbuf_discard(m_dummy_verts);
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Command Encoder and pipeline state
   * These utilities ensure that all of the globally bound resources and state have been
   * correctly encoded within the current RenderCommandEncoder. This involves managing
   * buffer bindings, texture bindings, depth stencil state and dynamic pipeline state.
   *
   * We will also trigger compilation of new PSOs where the input state has changed
   * and is required.
   * All of this setup is required in order to perform a valid draw call.
   * @{ */

  bool MTLContext::ensure_render_pipeline_state(MTL::PrimitiveType mtl_prim_type)
  {
    KLI_assert(this->pipeline_state.initialised);

    /* Check if an active shader is bound. */
    if (!this->pipeline_state.active_shader) {
      MTL_LOG_WARNING("No Metal shader for bound active shader\n");
      return false;
    }

    /* Also ensure active shader is valid. */
    if (!this->pipeline_state.active_shader->is_valid()) {
      MTL_LOG_WARNING(
        "Bound active shader is not valid (Missing/invalid implementation for Metal).\n", );
      return false;
    }

    /* Apply global state. */
    this->state_manager->apply_state();

    /* Main command buffer tracks the current state of the render pass, based on bound
     * MTLFrameBuffer. */
    MTLRenderPassState &rps = this->main_command_buffer.get_render_pass_state();

    /* Debug Check: Ensure Framebuffer instance is not dirty. */
    KLI_assert(!this->main_command_buffer.get_active_framebuffer()->get_dirty());

    /* Fetch shader interface. */
    MTLShaderInterface *shader_interface = this->pipeline_state.active_shader->get_interface();
    if (shader_interface == nullptr) {
      MTL_LOG_WARNING("Bound active shader does not have a valid shader interface!\n", );
      return false;
    }

    /* Fetch shader and bake valid PipelineStateObject (PSO) based on current
     * shader and state combination. This PSO represents the final GPU-executable
     * permutation of the shader. */
    MTLRenderPipelineStateInstance *pipeline_state_instance =
      this->pipeline_state.active_shader->bake_current_pipeline_state(
        this,
        mtl_prim_type_to_topology_class(mtl_prim_type));
    if (!pipeline_state_instance) {
      MTL_LOG_ERROR("Failed to bake Metal pipeline state for shader: %s\n",
                    shader_interface->get_name());
      return false;
    }

    bool result = false;
    if (pipeline_state_instance->pso) {

      /* Fetch render command encoder. A render pass should already be active.
       * This will be NULL if invalid. */
      MTL::RenderCommandEncoder *rec =
        this->main_command_buffer.get_active_render_command_encoder();
      KLI_assert(rec);
      if (rec == nil) {
        MTL_LOG_ERROR("ensure_render_pipeline_state called while render pass is not active.\n");
        return false;
      }

      /* Bind Render Pipeline State. */
      KLI_assert(pipeline_state_instance->pso);
      if (rps.bound_pso != pipeline_state_instance->pso) {
        rec->setRenderPipelineState(pipeline_state_instance->pso);
        rps.bound_pso = pipeline_state_instance->pso;
      }

      /** Ensure resource bindings. */
      /* Texture Bindings. */
      /* We will iterate through all texture bindings on the context and determine if any of the
       * active slots match those in our shader interface. If so, textures will be bound. */
      if (shader_interface->get_total_textures() > 0) {
        this->ensure_texture_bindings(rec, shader_interface, pipeline_state_instance);
      }

      /* Transform feedback buffer binding. */
#if 0
      /**
       * @NOTE: I think this is ready to go now... Need to test. - furby.
       *  
       * TODO(Metal): Include this code once MTLVertBuf is merged. We bind the vertex buffer to
       * which transform feedback data will be written. */
      GPUVertBuf *tf_vbo = this->pipeline_state.active_shader->get_transform_feedback_active_buffer();
      if (tf_vbo != nullptr && pipeline_state_instance->transform_feedback_buffer_index >= 0) {

        /* Ensure primitive type is either GPU_LINES, GPU_TRIANGLES or GPU_POINT */
        KLI_assert(mtl_prim_type == MTL::PrimitiveTypeLine ||
                   mtl_prim_type == MTL::PrimitiveTypeTriangle ||
                   mtl_prim_type == MTL::PrimitiveTypePoint);

        /* Fetch active transform feedback buffer from vertbuf */
        MTLVertBuf *tf_vbo_mtl = static_cast<MTLVertBuf *>(reinterpret_cast<VertBuf *>(tf_vbo));
        int tf_buffer_offset = 0;
        MTL::Buffer *tf_buffer_mtl = tf_vbo_mtl->get_metal_buffer(&tf_buffer_offset);

        if (tf_buffer_mtl != nil && tf_buffer_offset >= 0) {
          rec->setVertexBuffer(tf_buffer_mtl,
                               tf_buffer_offset,
                               pipeline_state_instance->transform_feedback_buffer_index);
          printf("Successfully bound VBO: %p for transform feedback (MTL Buffer: %p)\n",
                 tf_vbo_mtl,
                 tf_buffer_mtl);
        }
      }
#endif /* TODO(Metal) */

      /* Matrix Bindings. */
      /* This is now called upon shader bind. We may need to re-evaluate this though,
       * as was done here to ensure uniform changes between draws were tracked.
       * NOTE(Metal): We may be able to remove this. */
      GPU_matrix_bind(reinterpret_cast<struct GPUShader *>(
        static_cast<Shader *>(this->pipeline_state.active_shader)));

      /* Bind Uniforms */
      this->ensure_uniform_buffer_bindings(rec, shader_interface, pipeline_state_instance);

      /* Bind Null attribute buffer, if needed. */
      if (pipeline_state_instance->null_attribute_buffer_index >= 0) {
        if (G.debug & G_DEBUG_GPU) {
          MTL_LOG_INFO("Binding null attribute buffer at index: %d\n",
                       pipeline_state_instance->null_attribute_buffer_index);
        }
        rps.bind_vertex_buffer(this->get_null_attribute_buffer(),
                               0,
                               pipeline_state_instance->null_attribute_buffer_index);
      }

      /** Dynamic Per-draw Render State on RenderCommandEncoder. */
      /* State: Viewport. */
      if (this->pipeline_state.dirty_flags & MTL_PIPELINE_STATE_VIEWPORT_FLAG) {
        MTL::Viewport viewport;
        viewport.originX = (double)this->pipeline_state.viewport_offset_x;
        viewport.originY = (double)this->pipeline_state.viewport_offset_y;
        viewport.width = (double)this->pipeline_state.viewport_width;
        viewport.height = (double)this->pipeline_state.viewport_height;
        viewport.znear = this->pipeline_state.depth_stencil_state.depth_range_near;
        viewport.zfar = this->pipeline_state.depth_stencil_state.depth_range_far;
        rec->setViewport(viewport);

        this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags &
                                            ~MTL_PIPELINE_STATE_VIEWPORT_FLAG);
      }

      /* State: Scissor. */
      if (this->pipeline_state.dirty_flags & MTL_PIPELINE_STATE_SCISSOR_FLAG) {

        /* Get FrameBuffer associated with active RenderCommandEncoder. */
        MTLFrameBuffer *render_fb = this->main_command_buffer.get_active_framebuffer();

        MTL::ScissorRect scissor;
        if (this->pipeline_state.scissor_enabled) {
          scissor.x = this->pipeline_state.scissor_x;
          scissor.y = this->pipeline_state.scissor_y;
          scissor.width = this->pipeline_state.scissor_width;
          scissor.height = this->pipeline_state.scissor_height;

          /* Some scissor assignments exceed the bounds of the viewport due to implicitly added
           * padding to the width/height - Clamp width/height. */
          KLI_assert(scissor.x >= 0 && scissor.x < render_fb->get_width());
          KLI_assert(scissor.y >= 0 && scissor.y < render_fb->get_height());
          scissor.width = min_ii(scissor.width, render_fb->get_width() - scissor.x);
          scissor.height = min_ii(scissor.height, render_fb->get_height() - scissor.y);
          KLI_assert(scissor.width > 0 && (scissor.x + scissor.width <= render_fb->get_width()));
          KLI_assert(scissor.height > 0 && (scissor.height <= render_fb->get_height()));
        } else {
          /* Scissor is disabled, reset to default size as scissor state may have been previously
           * assigned on this encoder. */
          scissor.x = 0;
          scissor.y = 0;
          scissor.width = render_fb->get_width();
          scissor.height = render_fb->get_height();
        }

        /* Scissor state can still be flagged as changed if it is toggled on and off, without
         * parameters changing between draws. */
        if (memcmp(&scissor, &rps.last_scissor_rect, sizeof(MTL::ScissorRect))) {
          rec->setScissorRect(scissor);
          rps.last_scissor_rect = scissor;
        }
        this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags &
                                            ~MTL_PIPELINE_STATE_SCISSOR_FLAG);
      }

      /* State: Face winding. */
      if (this->pipeline_state.dirty_flags & MTL_PIPELINE_STATE_FRONT_FACING_FLAG) {
        /* We need to invert the face winding in Metal, to account for the inverted-Y coordinate
         * system. */
        MTL::Winding winding = (this->pipeline_state.front_face == GPU_CLOCKWISE) ?
                                 MTL::WindingClockwise :
                                 MTL::WindingCounterClockwise;
        rec->setFrontFacingWinding(winding);
        this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags &
                                            ~MTL_PIPELINE_STATE_FRONT_FACING_FLAG);
      }

      /* State: cull-mode. */
      if (this->pipeline_state.dirty_flags & MTL_PIPELINE_STATE_CULLMODE_FLAG) {

        MTL::CullMode mode = MTL::CullModeNone;
        if (this->pipeline_state.culling_enabled) {
          switch (this->pipeline_state.cull_mode) {
            case GPU_CULL_NONE:
              mode = MTL::CullModeNone;
              break;
            case GPU_CULL_FRONT:
              mode = MTL::CullModeFront;
              break;
            case GPU_CULL_BACK:
              mode = MTL::CullModeBack;
              break;
            default:
              KLI_assert_unreachable();
              break;
          }
        }
        rec->setCullMode(mode);
        this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags &
                                            ~MTL_PIPELINE_STATE_CULLMODE_FLAG);
      }

      /* Pipeline state is now good. */
      result = true;
    }
    return result;
  }

  /* Bind uniform buffers to an active render command encoder using the rendering state of the
   * current context -> Active shader, Bound UBOs). */
  bool MTLContext::ensure_uniform_buffer_bindings(
    MTL::RenderCommandEncoder *rec,
    const MTLShaderInterface *shader_interface,
    const MTLRenderPipelineStateInstance *pipeline_state_instance)
  {
    /* Fetch Render Pass state. */
    MTLRenderPassState &rps = this->main_command_buffer.get_render_pass_state();

    /* Shader owned push constant block for uniforms.. */
    bool active_shader_changed = (rps.last_bound_shader_state.shader_ !=
                                    this->pipeline_state.active_shader ||
                                  rps.last_bound_shader_state.shader_ == nullptr ||
                                  rps.last_bound_shader_state.pso_index_ !=
                                    pipeline_state_instance->shader_pso_index);

    const MTLShaderUniformBlock &push_constant_block = shader_interface->get_push_constant_block();
    if (push_constant_block.size > 0) {

      /* Fetch uniform buffer base binding index from pipeline_state_instance - There buffer index
       * will be offset by the number of bound VBOs. */
      uint32_t block_size = push_constant_block.size;
      uint32_t buffer_index = pipeline_state_instance->base_uniform_buffer_index +
                              push_constant_block.buffer_index;

      /* Only need to rebind block if push constants have been modified -- or if no data is bound
       * for the current RenderCommandEncoder. */
      if (this->pipeline_state.active_shader->get_push_constant_is_dirty() ||
          active_shader_changed || !rps.cached_vertex_buffer_bindings[buffer_index].is_bytes ||
          !rps.cached_fragment_buffer_bindings[buffer_index].is_bytes || true) {

        /* Bind push constant data. */
        KLI_assert(this->pipeline_state.active_shader->get_push_constant_data() != nullptr);
        rps.bind_vertex_bytes(this->pipeline_state.active_shader->get_push_constant_data(),
                              block_size,
                              buffer_index);
        rps.bind_fragment_bytes(this->pipeline_state.active_shader->get_push_constant_data(),
                                block_size,
                                buffer_index);

        /* Only need to rebind block if it has been modified. */
        this->pipeline_state.active_shader->push_constant_bindstate_mark_dirty(false);
      }
    }
    rps.last_bound_shader_state.set(this->pipeline_state.active_shader,
                                    pipeline_state_instance->shader_pso_index);

    /* Bind Global GPUUniformBuffers */
    /* Iterate through expected UBOs in the shader interface, and check if the globally bound ones
     * match. This is used to support the gpu_uniformbuffer module, where the uniform data is
     * global, and not owned by the shader instance. */
    for (const uint ubo_index : IndexRange(shader_interface->get_total_uniform_blocks())) {
      const MTLShaderUniformBlock &ubo = shader_interface->get_uniform_block(ubo_index);

      if (ubo.buffer_index >= 0) {

        /* Uniform Buffer index offset by 1 as the first shader buffer binding slot is reserved for
         * the uniform PushConstantBlock. */
        const uint32_t buffer_index = ubo.buffer_index + 1;
        int ubo_offset = 0;
        MTL::Buffer *ubo_buffer = nil;
        int ubo_size = 0;

        bool bind_dummy_buffer = false;
        if (this->pipeline_state.ubo_bindings[ubo_index].bound) {

          /* Fetch UBO global-binding properties from slot. */
          ubo_offset = 0;
          ubo_buffer = this->pipeline_state.ubo_bindings[ubo_index].ubo->get_metal_buffer(
            &ubo_offset);
          ubo_size = this->pipeline_state.ubo_bindings[ubo_index].ubo->get_size();

          /* Use dummy zero buffer if no buffer assigned -- this is an optimization to avoid
           * allocating zero buffers. */
          if (ubo_buffer == nil) {
            bind_dummy_buffer = true;
          } else {
            KLI_assert(ubo_buffer != nil);
            KLI_assert(ubo_size > 0);

            if (pipeline_state_instance->reflection_data_available) {
              /* NOTE: While the vertex and fragment stages have different UBOs, the indices in
               * each case will be the same for the same UBO. We also determine expected size and
               * then ensure buffer of the correct size exists in one of the vertex/fragment shader
               * binding tables. This path is used to verify that the size of the bound UBO matches
               * what is expected in the shader. */
              uint32_t expected_size =
                (buffer_index <
                 pipeline_state_instance->buffer_bindings_reflection_data_vert.size()) ?
                  pipeline_state_instance->buffer_bindings_reflection_data_vert[buffer_index]
                    .size :
                  0;
              if (expected_size == 0) {
                expected_size =
                  (buffer_index <
                   pipeline_state_instance->buffer_bindings_reflection_data_frag.size()) ?
                    pipeline_state_instance->buffer_bindings_reflection_data_frag[buffer_index]
                      .size :
                    0;
              }
              KLI_assert_msg(
                expected_size > 0,
                "Shader interface expects UBO, but shader reflection data reports that it "
                "is not present");

              /* If ubo size is smaller than the size expected by the shader, we need to bind the
               * dummy buffer, which will be big enough, to avoid an OOB error. */
              if (ubo_size < expected_size) {
                MTL_LOG_INFO(
                  "[Error][UBO] UBO (UBO Name: %s) bound at index: %d with size %d (Expected size "
                  "%d)  (Shader Name: %s) is too small -- binding NULL buffer. This is likely an "
                  "over-binding, which is not used,  but we need this to avoid validation "
                  "issues\n",
                  shader_interface->get_name_at_offset(ubo.name_offset),
                  buffer_index,
                  ubo_size,
                  expected_size,
                  shader_interface->get_name());
                bind_dummy_buffer = true;
              }
            }
          }
        } else {
          MTL_LOG_INFO(
            "[Warning][UBO] Shader '%s' expected UBO '%s' to be bound at buffer index: %d -- but "
            "nothing was bound -- binding dummy buffer\n",
            shader_interface->get_name(),
            shader_interface->get_name_at_offset(ubo.name_offset),
            buffer_index);
          bind_dummy_buffer = true;
        }

        if (bind_dummy_buffer) {
          /* Perform Dummy binding. */
          ubo_offset = 0;
          ubo_buffer = this->get_null_buffer();
          ubo_size = ubo_buffer->length();
        }

        if (ubo_buffer != nil) {

          uint32_t buffer_bind_index = pipeline_state_instance->base_uniform_buffer_index +
                                       buffer_index;

          /* Bind Vertex UBO. */
          if (bool(ubo.stage_mask & ShaderStage::VERTEX)) {
            KLI_assert(buffer_bind_index >= 0 &&
                       buffer_bind_index < MTL_MAX_UNIFORM_BUFFER_BINDINGS);
            rps.bind_vertex_buffer(ubo_buffer, ubo_offset, buffer_bind_index);
          }

          /* Bind Fragment UBOs. */
          if (bool(ubo.stage_mask & ShaderStage::FRAGMENT)) {
            KLI_assert(buffer_bind_index >= 0 &&
                       buffer_bind_index < MTL_MAX_UNIFORM_BUFFER_BINDINGS);
            rps.bind_fragment_buffer(ubo_buffer, ubo_offset, buffer_bind_index);
          }
        } else {
          MTL_LOG_WARNING(
            "[UBO] Shader '%s' has UBO '%s' bound at buffer index: %d -- but MTLBuffer "
            "is NULL!\n",
            shader_interface->get_name(),
            shader_interface->get_name_at_offset(ubo.name_offset),
            buffer_index);
        }
      }
    }
    return true;
  }

  /* Ensure texture bindings are correct and up to date for current draw call. */
  void MTLContext::ensure_texture_bindings(
    MTL::RenderCommandEncoder *rec,
    MTLShaderInterface *shader_interface,
    const MTLRenderPipelineStateInstance *pipeline_state_instance)
  {
    KLI_assert(shader_interface != nil);
    KLI_assert(rec != nil);

    /* Fetch Render Pass state. */
    MTLRenderPassState &rps = this->main_command_buffer.get_render_pass_state();

    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    int vertex_arg_buffer_bind_index = -1;
    int fragment_arg_buffer_bind_index = -1;

    /* Argument buffers are used for samplers, when the limit of 16 is exceeded. */
    bool use_argument_buffer_for_samplers = shader_interface->get_use_argument_buffer_for_samplers(
      &vertex_arg_buffer_bind_index,
      &fragment_arg_buffer_bind_index);

    /* Loop through expected textures in shader interface and resolve bindings with currently
     * bound textures.. */
    for (const uint t : IndexRange(shader_interface->get_max_texture_index() + 1)) {
      /* Ensure the bound texture is compatible with the shader interface. If the
       * shader does not expect a texture to be bound for the current slot, we skip
       * binding.
       * NOTE: Global texture bindings may be left over from prior draw calls. */
      const MTLShaderTexture &shader_texture_info = shader_interface->get_texture(t);
      if (!shader_texture_info.used) {
        /* Skip unused binding points if explicit indices are specified. */
        continue;
      }

      int slot = shader_texture_info.slot_index;
      if (slot >= 0 && slot < GPU_max_textures()) {
        bool bind_dummy_texture = true;
        if (this->pipeline_state.texture_bindings[slot].used) {
          gpu::MTLTexture *bound_texture =
            this->pipeline_state.texture_bindings[slot].texture_resource;
          MTLSamplerBinding &bound_sampler = this->pipeline_state.sampler_bindings[slot];
          KLI_assert(bound_texture);
          KLI_assert(bound_sampler.used);

          if (shader_texture_info.type == bound_texture->m_type) {
            /* Bind texture and sampler if the bound texture matches the type expected by the
             * shader. */
            MTL::Texture *tex = bound_texture->get_metal_handle();

            if (bool(shader_texture_info.stage_mask & ShaderStage::VERTEX)) {
              rps.bind_vertex_texture(tex, slot);
              rps.bind_vertex_sampler(bound_sampler, use_argument_buffer_for_samplers, slot);
            }

            if (bool(shader_texture_info.stage_mask & ShaderStage::FRAGMENT)) {
              rps.bind_fragment_texture(tex, slot);
              rps.bind_fragment_sampler(bound_sampler, use_argument_buffer_for_samplers, slot);
            }

            /* Texture state resolved, no need to bind dummy texture */
            bind_dummy_texture = false;
          } else {
            /* Texture type for bound texture (e.g. Texture2DArray) does not match what was
             * expected in the shader interface. This is a problem and we will need to bind
             * a dummy texture to ensure correct API usage. */
            MTL_LOG_WARNING(
              "(Shader '%s') Texture %p bound to slot %d is incompatible -- Wrong "
              "texture target type. (Expecting type %d, actual type %d) (binding "
              "name:'%s')(texture name:'%s')\n",
              shader_interface->get_name(),
              bound_texture,
              slot,
              shader_texture_info.type,
              bound_texture->m_type,
              shader_interface->get_name_at_offset(shader_texture_info.name_offset),
              bound_texture->get_name());
          }
        } else {
          MTL_LOG_WARNING(
            "Shader '%s' expected texture to be bound to slot %d -- No texture was "
            "bound. (name:'%s')\n",
            shader_interface->get_name(),
            slot,
            shader_interface->get_name_at_offset(shader_texture_info.name_offset));
        }

        /* Bind Dummy texture -- will temporarily resolve validation issues while incorrect
         * formats are provided -- as certain configurations may not need any binding. These
         * issues should be fixed in the high-level, if problems crop up. */
        if (bind_dummy_texture) {
          if (bool(shader_texture_info.stage_mask & ShaderStage::VERTEX)) {
            rps.bind_vertex_texture(
              get_dummy_texture(shader_texture_info.type)->get_metal_handle(),
              slot);

            /* Bind default sampler state. */
            MTLSamplerBinding default_binding = {true, DEFAULT_SAMPLER_STATE};
            rps.bind_vertex_sampler(default_binding, use_argument_buffer_for_samplers, slot);
          }
          if (bool(shader_texture_info.stage_mask & ShaderStage::FRAGMENT)) {
            rps.bind_fragment_texture(
              get_dummy_texture(shader_texture_info.type)->get_metal_handle(),
              slot);

            /* Bind default sampler state. */
            MTLSamplerBinding default_binding = {true, DEFAULT_SAMPLER_STATE};
            rps.bind_fragment_sampler(default_binding, use_argument_buffer_for_samplers, slot);
          }
        }
      } else {
        MTL_LOG_WARNING(
          "Shader %p expected texture to be bound to slot %d -- Slot exceeds the "
          "hardware/API limit of '%d'. (name:'%s')\n",
          this->pipeline_state.active_shader,
          slot,
          GPU_max_textures(),
          shader_interface->get_name_at_offset(shader_texture_info.name_offset));
      }
    }

    /* Construct and Bind argument buffer.
     * NOTE(Metal): Samplers use an argument buffer when the limit of 16 samplers is exceeded. */
    if (use_argument_buffer_for_samplers) {
#ifndef NDEBUG
      /* Debug check to validate each expected texture in the shader interface has a valid
       * sampler object bound to the context. We will need all of these to be valid
       * when constructing the sampler argument buffer. */
      for (const uint i : IndexRange(shader_interface->get_max_texture_index() + 1)) {
        const MTLShaderTexture &texture = shader_interface->get_texture(i);
        if (texture.used) {
          KLI_assert(this->m_samplers.mtl_sampler[i] != nil);
        }
      }
#endif

      /* Check to ensure the buffer binding index for the argument buffer has been assigned.
       * This PSO property will be set if we expect to use argument buffers, and the shader
       * uses any amount of textures. */
      KLI_assert(vertex_arg_buffer_bind_index >= 0 || fragment_arg_buffer_bind_index >= 0);
      if (vertex_arg_buffer_bind_index >= 0 || fragment_arg_buffer_bind_index >= 0) {
        /* Offset binding index to be relative to the start of static uniform buffer binding
         * slots. The first N slots, prior to
         * `pipeline_state_instance->base_uniform_buffer_index` are used by vertex and index
         * buffer bindings, and the number of buffers present will vary between PSOs. */
        int arg_buffer_idx = (pipeline_state_instance->base_uniform_buffer_index +
                              vertex_arg_buffer_bind_index);
        assert(arg_buffer_idx < 32);
        MTL::ArgumentEncoder *argument_encoder = shader_interface->find_argument_encoder(
          arg_buffer_idx);
        if (argument_encoder == nil) {
          argument_encoder = pipeline_state_instance->vert->newArgumentEncoder(arg_buffer_idx);
          shader_interface->insert_argument_encoder(arg_buffer_idx, argument_encoder);
        }

        /* Generate or Fetch argument buffer sampler configuration.
         * NOTE(Metal): we need to base sampler counts off of the maximal texture
         * index. This is not the most optimal, but in practice, not a use-case
         * when argument buffers are required.
         * This is because with explicit texture indices, the binding indices
         * should match across draws, to allow the high-level to optimize bind-points. */
        gpu::MTLBuffer *encoder_buffer = nullptr;
        this->m_samplers.num_samplers = shader_interface->get_max_texture_index() + 1;

        gpu::MTLBuffer **cached_smp_buffer_search = this->m_cached_sampler_buffers.lookup_ptr(
          this->m_samplers);
        if (cached_smp_buffer_search != nullptr) {
          encoder_buffer = *cached_smp_buffer_search;
        } else {
          /* Populate argument buffer with current global sampler bindings. */
          int size = argument_encoder->encodedLength();
          int alignment = max_uu(argument_encoder->alignment(), 256);
          int size_align_delta = (size % alignment);
          int aligned_alloc_size = ((alignment > 1) && (size_align_delta > 0)) ?
                                     size + (alignment - (size % alignment)) :
                                     size;

          /* Allocate buffer to store encoded sampler arguments. */
          encoder_buffer = MTLContext::get_global_memory_manager().allocate(aligned_alloc_size,
                                                                            true);
          KLI_assert(encoder_buffer);
          KLI_assert(encoder_buffer->get_metal_buffer());

          argument_encoder->setArgumentBuffer(encoder_buffer->get_metal_buffer(), 0);
          argument_encoder->setSamplerStates(
            this->m_samplers.mtl_sampler,
            NS::Range::Make(0, shader_interface->get_max_texture_index() + 1));
          encoder_buffer->flush();

          /* Insert into cache. */
          this->m_cached_sampler_buffers.add_new(this->m_samplers, encoder_buffer);
        }

        KLI_assert(encoder_buffer != nullptr);
        int vert_buffer_index = (pipeline_state_instance->base_uniform_buffer_index +
                                 vertex_arg_buffer_bind_index);
        rps.bind_vertex_buffer(encoder_buffer->get_metal_buffer(), 0, vert_buffer_index);

        /* Fragment shader shares its argument buffer binding with the vertex shader, So no need
         * to re-encode. We can use the same argument buffer. */
        if (fragment_arg_buffer_bind_index >= 0) {
          KLI_assert(fragment_arg_buffer_bind_index);
          int frag_buffer_index = (pipeline_state_instance->base_uniform_buffer_index +
                                   fragment_arg_buffer_bind_index);
          rps.bind_fragment_buffer(encoder_buffer->get_metal_buffer(), 0, frag_buffer_index);
        }
      }
    }

    pool->drain();
    pool = nil;
  }

  /* Encode latest depth-stencil state. */
  void MTLContext::ensure_depth_stencil_state(MTL::PrimitiveType prim_type)
  {
    /* Check if we need to update state. */
    if (!(this->pipeline_state.dirty_flags & MTL_PIPELINE_STATE_DEPTHSTENCIL_FLAG)) {
      return;
    }

    /* Fetch render command encoder. */
    MTL::RenderCommandEncoder *rec = this->main_command_buffer.get_active_render_command_encoder();
    KLI_assert(rec);

    /* Fetch Render Pass state. */
    MTLRenderPassState &rps = this->main_command_buffer.get_render_pass_state();

    /** Prepare Depth-stencil state based on current global pipeline state. */
    MTLFrameBuffer *fb = this->get_current_framebuffer();
    bool hasDepthTarget = fb->has_depth_attachment();
    bool hasStencilTarget = fb->has_stencil_attachment();

    if (hasDepthTarget || hasStencilTarget) {
      /* Update FrameBuffer State. */
      this->pipeline_state.depth_stencil_state.has_depth_target = hasDepthTarget;
      this->pipeline_state.depth_stencil_state.has_stencil_target = hasStencilTarget;

      /* Check if current MTLContextDepthStencilState maps to an existing state object in
       * the Depth-stencil state cache. */
      MTL::DepthStencilState *ds_state = nil;
      MTL::DepthStencilState **depth_stencil_state_lookup =
        this->depth_stencil_state_cache.lookup_ptr(this->pipeline_state.depth_stencil_state);

      /* If not, populate DepthStencil state descriptor. */
      if (depth_stencil_state_lookup == nullptr) {

        MTL::DepthStencilDescriptor *ds_state_desc =
          MTL::DepthStencilDescriptor::alloc()->init()->autorelease();

        if (hasDepthTarget) {
          ds_state_desc->setDepthWriteEnabled(
            this->pipeline_state.depth_stencil_state.depth_write_enable);
          ds_state_desc->setDepthCompareFunction(
            this->pipeline_state.depth_stencil_state.depth_test_enabled ?
              this->pipeline_state.depth_stencil_state.depth_function :
              MTL::CompareFunctionAlways);
        }

        if (hasStencilTarget) {
          ds_state_desc->backFaceStencil()->setReadMask(
            this->pipeline_state.depth_stencil_state.stencil_read_mask);
          ds_state_desc->backFaceStencil()->setWriteMask(
            this->pipeline_state.depth_stencil_state.stencil_write_mask);
          ds_state_desc->backFaceStencil()->setStencilFailureOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_back_stencil_fail);
          ds_state_desc->backFaceStencil()->setDepthFailureOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_back_depth_fail);
          ds_state_desc->backFaceStencil()->setDepthStencilPassOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_back_depthstencil_pass);
          ds_state_desc->backFaceStencil()->setStencilCompareFunction(
            (this->pipeline_state.depth_stencil_state.stencil_test_enabled) ?
              this->pipeline_state.depth_stencil_state.stencil_func :
              MTL::CompareFunctionAlways);

          ds_state_desc->frontFaceStencil()->setReadMask(
            this->pipeline_state.depth_stencil_state.stencil_read_mask);
          ds_state_desc->frontFaceStencil()->setWriteMask(
            this->pipeline_state.depth_stencil_state.stencil_write_mask);
          ds_state_desc->frontFaceStencil()->setStencilFailureOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_front_stencil_fail);
          ds_state_desc->frontFaceStencil()->setDepthFailureOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_front_depth_fail);
          ds_state_desc->frontFaceStencil()->setDepthStencilPassOperation(
            this->pipeline_state.depth_stencil_state.stencil_op_front_depthstencil_pass);
          ds_state_desc->frontFaceStencil()->setStencilCompareFunction(
            (this->pipeline_state.depth_stencil_state.stencil_test_enabled) ?
              this->pipeline_state.depth_stencil_state.stencil_func :
              MTL::CompareFunctionAlways);
        }

        /* Bake new DS state. */
        ds_state = this->device->newDepthStencilState(ds_state_desc);

        /* Store state in cache. */
        KLI_assert(ds_state != nil);
        this->depth_stencil_state_cache.add_new(this->pipeline_state.depth_stencil_state,
                                                ds_state);
      } else {
        ds_state = *depth_stencil_state_lookup;
        KLI_assert(ds_state != nil);
      }

      /* Bind Depth Stencil State to render command encoder. */
      KLI_assert(ds_state != nil);
      if (ds_state != nil) {
        if (rps.bound_ds_state != ds_state) {
          rec->setDepthStencilState(ds_state);
          rps.bound_ds_state = ds_state;
        }
      }

      /* Apply dynamic depth-stencil state on encoder. */
      if (hasStencilTarget) {
        uint32_t stencil_ref_value =
          (this->pipeline_state.depth_stencil_state.stencil_test_enabled) ?
            this->pipeline_state.depth_stencil_state.stencil_ref :
            0;
        if (stencil_ref_value != rps.last_used_stencil_ref_value) {
          rec->setStencilReferenceValue(stencil_ref_value);
          rps.last_used_stencil_ref_value = stencil_ref_value;
        }
      }

      if (hasDepthTarget) {
        bool doBias = false;
        switch (prim_type) {
          case MTL::PrimitiveTypeTriangle:
          case MTL::PrimitiveTypeTriangleStrip:
            doBias = this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_tris;
            break;
          case MTL::PrimitiveTypeLine:
          case MTL::PrimitiveTypeLineStrip:
            doBias = this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_lines;
            break;
          case MTL::PrimitiveTypePoint:
            doBias = this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_points;
            break;
        }
        rec->setDepthBias((doBias) ? this->pipeline_state.depth_stencil_state.depth_bias : 0,
                          (doBias) ? this->pipeline_state.depth_stencil_state.depth_slope_scale :
                                     0,
                          0);
      }
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Global Context State
   * @{ */

  /* Metal Context Pipeline State. */
  void MTLContext::pipeline_state_init()
  {
    /*** Initialize state only once. ***/
    if (!this->pipeline_state.initialised) {
      this->pipeline_state.initialised = true;
      this->pipeline_state.active_shader = nullptr;

      /* Clear bindings state. */
      for (int t = 0; t < GPU_max_textures(); t++) {
        this->pipeline_state.texture_bindings[t].used = false;
        this->pipeline_state.texture_bindings[t].slot_index = -1;
        this->pipeline_state.texture_bindings[t].texture_resource = nullptr;
      }
      for (int s = 0; s < MTL_MAX_SAMPLER_SLOTS; s++) {
        this->pipeline_state.sampler_bindings[s].used = false;
      }
      for (int u = 0; u < MTL_MAX_UNIFORM_BUFFER_BINDINGS; u++) {
        this->pipeline_state.ubo_bindings[u].bound = false;
        this->pipeline_state.ubo_bindings[u].ubo = nullptr;
      }
    }

    /*** State defaults -- restored by GPU_state_init. ***/
    /* Clear blending State. */
    this->pipeline_state.color_write_mask = MTL::ColorWriteMaskRed | MTL::ColorWriteMaskGreen |
                                            MTL::ColorWriteMaskBlue | MTL::ColorWriteMaskAlpha;
    this->pipeline_state.blending_enabled = false;
    this->pipeline_state.alpha_blend_op = MTL::BlendOperationAdd;
    this->pipeline_state.rgb_blend_op = MTL::BlendOperationAdd;
    this->pipeline_state.dest_alpha_blend_factor = MTL::BlendFactorZero;
    this->pipeline_state.dest_rgb_blend_factor = MTL::BlendFactorZero;
    this->pipeline_state.src_alpha_blend_factor = MTL::BlendFactorOne;
    this->pipeline_state.src_rgb_blend_factor = MTL::BlendFactorOne;

    /* Viewport and scissor. */
    this->pipeline_state.viewport_offset_x = 0;
    this->pipeline_state.viewport_offset_y = 0;
    this->pipeline_state.viewport_width = 0;
    this->pipeline_state.viewport_height = 0;
    this->pipeline_state.scissor_x = 0;
    this->pipeline_state.scissor_y = 0;
    this->pipeline_state.scissor_width = 0;
    this->pipeline_state.scissor_height = 0;
    this->pipeline_state.scissor_enabled = false;

    /* Culling State. */
    this->pipeline_state.culling_enabled = false;
    this->pipeline_state.cull_mode = GPU_CULL_NONE;
    this->pipeline_state.front_face = GPU_COUNTERCLOCKWISE;

    /* DATA and IMAGE access state. */
    this->pipeline_state.unpack_row_length = 0;

    /* Depth State. */
    this->pipeline_state.depth_stencil_state.depth_write_enable = false;
    this->pipeline_state.depth_stencil_state.depth_test_enabled = false;
    this->pipeline_state.depth_stencil_state.depth_range_near = 0.0;
    this->pipeline_state.depth_stencil_state.depth_range_far = 1.0;
    this->pipeline_state.depth_stencil_state.depth_function = MTL::CompareFunctionAlways;
    this->pipeline_state.depth_stencil_state.depth_bias = 0.0;
    this->pipeline_state.depth_stencil_state.depth_slope_scale = 0.0;
    this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_points = false;
    this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_lines = false;
    this->pipeline_state.depth_stencil_state.depth_bias_enabled_for_tris = false;

    /* Stencil State. */
    this->pipeline_state.depth_stencil_state.stencil_test_enabled = false;
    this->pipeline_state.depth_stencil_state.stencil_read_mask = 0xFF;
    this->pipeline_state.depth_stencil_state.stencil_write_mask = 0xFF;
    this->pipeline_state.depth_stencil_state.stencil_ref = 0;
    this->pipeline_state.depth_stencil_state.stencil_func = MTL::CompareFunctionAlways;
    this->pipeline_state.depth_stencil_state.stencil_op_front_stencil_fail =
      MTL::StencilOperationKeep;
    this->pipeline_state.depth_stencil_state.stencil_op_front_depth_fail =
      MTL::StencilOperationKeep;
    this->pipeline_state.depth_stencil_state.stencil_op_front_depthstencil_pass =
      MTL::StencilOperationKeep;
    this->pipeline_state.depth_stencil_state.stencil_op_back_stencil_fail =
      MTL::StencilOperationKeep;
    this->pipeline_state.depth_stencil_state.stencil_op_back_depth_fail =
      MTL::StencilOperationKeep;
    this->pipeline_state.depth_stencil_state.stencil_op_back_depthstencil_pass =
      MTL::StencilOperationKeep;
  }

  void MTLContext::set_viewport(int origin_x, int origin_y, int width, int height)
  {
    KLI_assert(this);
    KLI_assert(width > 0);
    KLI_assert(height > 0);
    KLI_assert(origin_x >= 0);
    KLI_assert(origin_y >= 0);
    bool changed = (this->pipeline_state.viewport_offset_x != origin_x) ||
                   (this->pipeline_state.viewport_offset_y != origin_y) ||
                   (this->pipeline_state.viewport_width != width) ||
                   (this->pipeline_state.viewport_height != height);
    this->pipeline_state.viewport_offset_x = origin_x;
    this->pipeline_state.viewport_offset_y = origin_y;
    this->pipeline_state.viewport_width = width;
    this->pipeline_state.viewport_height = height;
    if (changed) {
      this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags |
                                          MTL_PIPELINE_STATE_VIEWPORT_FLAG);
    }
  }

  void MTLContext::set_scissor(int scissor_x, int scissor_y, int scissor_width, int scissor_height)
  {
    KLI_assert(this);
    bool changed = (this->pipeline_state.scissor_x != scissor_x) ||
                   (this->pipeline_state.scissor_y != scissor_y) ||
                   (this->pipeline_state.scissor_width != scissor_width) ||
                   (this->pipeline_state.scissor_height != scissor_height) ||
                   (this->pipeline_state.scissor_enabled != true);
    this->pipeline_state.scissor_x = scissor_x;
    this->pipeline_state.scissor_y = scissor_y;
    this->pipeline_state.scissor_width = scissor_width;
    this->pipeline_state.scissor_height = scissor_height;
    this->pipeline_state.scissor_enabled = (scissor_width > 0 && scissor_height > 0);

    if (changed) {
      this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags |
                                          MTL_PIPELINE_STATE_SCISSOR_FLAG);
    }
  }

  void MTLContext::set_scissor_enabled(bool scissor_enabled)
  {
    /* Only turn on Scissor if requested scissor region is valid */
    scissor_enabled = scissor_enabled && (this->pipeline_state.scissor_width > 0 &&
                                          this->pipeline_state.scissor_height > 0);

    bool changed = (this->pipeline_state.scissor_enabled != scissor_enabled);
    this->pipeline_state.scissor_enabled = scissor_enabled;
    if (changed) {
      this->pipeline_state.dirty_flags = (this->pipeline_state.dirty_flags |
                                          MTL_PIPELINE_STATE_SCISSOR_FLAG);
    }
  }

  /** @} */

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

  /** @} */

}  // namespace kraken::gpu
