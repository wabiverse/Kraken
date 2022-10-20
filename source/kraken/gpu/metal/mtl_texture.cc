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

#include "KKE_global.h"

#include "USD_userdef_types.h"

#include "GPU_batch.h"
#include "GPU_batch_presets.h"
#include "GPU_capabilities.h"
#include "GPU_framebuffer.h"
#include "GPU_immediate.h"
#include "GPU_platform.h"
#include "GPU_state.h"

#include "mtl_backend.hh"
#include "mtl_common.hh"
#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_texture.hh"
#include "mtl_vertex_buffer.hh"

#include "ANCHOR_api.h"

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** \name Creation & Deletion
   * \{ */

  void gpu::MTLTexture::mtl_texture_init()
  {
    KLI_assert(MTLContext::get() != nullptr);

    /* Status. */
    m_is_baked = false;
    m_is_dirty = false;
    m_resource_mode = MTL_TEXTURE_MODE_DEFAULT;
    m_mtl_max_mips = 1;

    /* Metal properties. */
    m_texture = nil;
    m_texture_buffer = nil;
    m_mip_swizzle_view = nil;

    /* Binding information. */
    m_is_bound = false;

    /* VBO. */
    m_vert_buffer = nullptr;
    m_vert_buffer_mtl = nil;

    /* Default Swizzle. */
    m_tex_swizzle_mask[0] = 'r';
    m_tex_swizzle_mask[1] = 'g';
    m_tex_swizzle_mask[2] = 'b';
    m_tex_swizzle_mask[3] = 'a';

    m_mtl_swizzle_mask.red = MTL::TextureSwizzleRed;
    m_mtl_swizzle_mask.green = MTL::TextureSwizzleGreen;
    m_mtl_swizzle_mask.blue = MTL::TextureSwizzleBlue;
    m_mtl_swizzle_mask.alpha = MTL::TextureSwizzleAlpha;

    /* TODO(Metal): Find a way of specifying texture usage externally. */
    m_gpu_image_usage_flags = GPU_TEXTURE_USAGE_SHADER_READ | GPU_TEXTURE_USAGE_ATTACHMENT;
  }

  gpu::MTLTexture::MTLTexture(const char *name) : Texture(name)
  {
    /* Common Initialization. */
    mtl_texture_init();
  }

  gpu::MTLTexture::MTLTexture(const char *name,
                              eGPUTextureFormat format,
                              eGPUTextureType type,
                              MTL::Texture *metal_texture)
    : gpu::Texture(name)
  {
    /* Common Initialization. */
    mtl_texture_init();

    /* Prep texture from METAL handle. */
    KLI_assert(metal_texture != nil);
    KLI_assert(type == GPU_TEXTURE_2D);
    m_type = type;
    init_2D(metal_texture->width(), metal_texture->height(), 0, 1, format);

    /* Assign MTLTexture. */
    m_texture = metal_texture;
    m_texture->retain();

    /* Flag as Baked. */
    m_is_baked = true;
    m_is_dirty = false;
    m_resource_mode = MTL_TEXTURE_MODE_EXTERNAL;
  }

  gpu::MTLTexture::~MTLTexture()
  {
    /* Unbind if bound. */
    if (m_is_bound) {
      MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
      if (ctx != nullptr) {
        ctx->state_manager->texture_unbind(this);
      }
    }

    /* Free memory. */
    this->reset();
  }

  /** \} */

  /* -------------------------------------------------------------------- */
  void gpu::MTLTexture::bake_mip_swizzle_view()
  {
    if (m_texture_view_dirty_flags) {
      /* if a texture view was previously created we release it. */
      if (m_mip_swizzle_view != nil) {
        m_mip_swizzle_view->release();
        m_mip_swizzle_view = nil;
      }

      /* Determine num slices */
      int num_slices = 1;
      switch (m_type) {
        case GPU_TEXTURE_1D_ARRAY:
          num_slices = m_h;
          break;
        case GPU_TEXTURE_2D_ARRAY:
          num_slices = m_d;
          break;
        case GPU_TEXTURE_CUBE:
          num_slices = 6;
          break;
        case GPU_TEXTURE_CUBE_ARRAY:
          /* m_d is equal to array levels * 6, including face count. */
          num_slices = m_d;
          break;
        default:
          num_slices = 1;
          break;
      }

      int range_len = min_ii((m_mip_texture_max_level - m_mip_texture_base_level) + 1,
                             m_texture->mipmapLevelCount());
      KLI_assert(range_len > 0);
      KLI_assert(m_mip_texture_base_level < m_texture->mipmapLevelCount());
      KLI_assert(m_mip_texture_base_layer < num_slices);
      m_mip_swizzle_view = m_texture->newTextureView(
        m_texture->pixelFormat(),
        m_texture->textureType(),
        NS::Range::Make(m_mip_texture_base_level, range_len),
        NS::Range::Make(m_mip_texture_base_layer, num_slices),
        m_mtl_swizzle_mask);
      MTL_LOG_INFO(
        "Updating texture view - MIP TEXTURE BASE LEVEL: %d, MAX LEVEL: %d (Range len: %d)\n",
        m_mip_texture_base_level,
        min_ii(m_mip_texture_max_level, m_texture->mipmapLevelCount()),
        range_len);
      m_mip_swizzle_view->setLabel(m_texture->label());
      m_texture_view_dirty_flags = TEXTURE_VIEW_NOT_DIRTY;
    }
  }

  /** \name Operations
   * \{ */

  MTL::Texture *gpu::MTLTexture::get_metal_handle()
  {

    /* Verify VBO texture shares same buffer. */
    if (m_resource_mode == MTL_TEXTURE_MODE_VBO) {
      MTL::Buffer *buf = m_vert_buffer->get_metal_buffer();

      /* Source vertex buffer has been re-generated, require re-initialization. */
      if (buf != m_vert_buffer_mtl) {
        MTL_LOG_INFO(
          "MTLTexture '%p' using MTL_TEXTURE_MODE_VBO requires re-generation due to updated "
          "Vertex-Buffer.\n",
          this);
        /* Clear state. */
        this->reset();

        /* Re-initialize. */
        this->init_internal(wrap(m_vert_buffer));

        /* Update for assertion check below. */
        buf = m_vert_buffer->get_metal_buffer();
      }

      /* Ensure buffer is valid.
       * Fetch-vert buffer handle directly in-case it changed above. */
      KLI_assert(m_vert_buffer_mtl != nil);
      KLI_assert(m_vert_buffer->get_metal_buffer() == m_vert_buffer_mtl);
    }

    /* ensure up to date and baked. */
    this->ensure_baked();

    if (m_is_baked) {
      /* For explicit texture views, ensure we always return the texture view. */
      if (m_resource_mode == MTL_TEXTURE_MODE_TEXTURE_VIEW) {
        KLI_assert_msg(m_mip_swizzle_view, "Texture view should always have a valid handle.");
      }

      if (m_mip_swizzle_view != nil || m_texture_view_dirty_flags) {
        bake_mip_swizzle_view();
        return m_mip_swizzle_view;
      }
      return m_texture;
    }
    return nil;
  }

  MTL::Texture *gpu::MTLTexture::get_metal_handle_base()
  {

    /* ensure up to date and baked. */
    this->ensure_baked();

    /* For explicit texture views, always return the texture view. */
    if (m_resource_mode == MTL_TEXTURE_MODE_TEXTURE_VIEW) {
      KLI_assert_msg(m_mip_swizzle_view, "Texture view should always have a valid handle.");
      if (m_mip_swizzle_view != nil || m_texture_view_dirty_flags) {
        bake_mip_swizzle_view();
      }
      return m_mip_swizzle_view;
    }

    /* Return base handle. */
    if (m_is_baked) {
      return m_texture;
    }
    return nil;
  }

  void gpu::MTLTexture::blit(MTL::BlitCommandEncoder *blit_encoder,
                             uint src_x_offset,
                             uint src_y_offset,
                             uint src_z_offset,
                             uint src_slice,
                             uint src_mip,
                             gpu::MTLTexture *dest,
                             uint dst_x_offset,
                             uint dst_y_offset,
                             uint dst_z_offset,
                             uint dst_slice,
                             uint dst_mip,
                             uint width,
                             uint height,
                             uint depth)
  {

    KLI_assert(this && dest);
    KLI_assert(width > 0 && height > 0 && depth > 0);
    MTL::Size src_size = MTL::Size::Make(width, height, depth);
    MTL::Origin src_origin = MTL::Origin::Make(src_x_offset, src_y_offset, src_z_offset);
    MTL::Origin dst_origin = MTL::Origin::Make(dst_x_offset, dst_y_offset, dst_z_offset);

    if (this->format_get() != dest->format_get()) {
      MTL_LOG_WARNING(
        "[Warning] gpu::MTLTexture: Cannot copy between two textures of different types using a "
        "blit encoder. TODO: Support this operation\n");
      return;
    }

    /* TODO(Metal): Verify if we want to use the one with modified base-level/texture view
     * or not. */
    blit_encoder->copyFromTexture(this->get_metal_handle_base(),
                                  src_slice,
                                  src_mip,
                                  src_origin,
                                  src_size,
                                  dest->get_metal_handle_base(),
                                  dst_slice,
                                  dst_mip,
                                  dst_origin);
  }

  void gpu::MTLTexture::blit(gpu::MTLTexture *dst,
                             uint src_x_offset,
                             uint src_y_offset,
                             uint dst_x_offset,
                             uint dst_y_offset,
                             uint src_mip,
                             uint dst_mip,
                             uint dst_slice,
                             int width,
                             int height)
  {
    KLI_assert(this->type_get() == dst->type_get());

    GPUShader *shader = fullscreen_blit_sh_get();
    KLI_assert(shader != nullptr);
    KLI_assert(GPU_context_active_get());

    /* Fetch restore framebuffer and blit target framebuffer from destination texture. */
    GPUFrameBuffer *restore_fb = GPU_framebuffer_active_get();
    GPUFrameBuffer *blit_target_fb = dst->get_blit_framebuffer(dst_slice, dst_mip);
    KLI_assert(blit_target_fb);
    GPU_framebuffer_bind(blit_target_fb);

    /* Execute graphics draw call to perform the blit. */
    GPUBatch *quad = GPU_batch_preset_quad();
    GPU_batch_set_shader(quad, shader);

    float w = dst->width_get();
    float h = dst->height_get();

    GPU_shader_uniform_2f(shader, "fullscreen", w, h);
    GPU_shader_uniform_2f(shader, "src_offset", src_x_offset, src_y_offset);
    GPU_shader_uniform_2f(shader, "dst_offset", dst_x_offset, dst_y_offset);
    GPU_shader_uniform_2f(shader, "size", width, height);

    GPU_shader_uniform_1i(shader, "mip", src_mip);
    GPU_batch_texture_bind(quad, "imageTexture", wrap(this));

    /* Caching previous pipeline state. */
    bool depth_write_prev = GPU_depth_mask_get();
    uint stencil_mask_prev = GPU_stencil_mask_get();
    eGPUStencilTest stencil_test_prev = GPU_stencil_test_get();
    eGPUFaceCullTest culling_test_prev = GPU_face_culling_get();
    eGPUBlend blend_prev = GPU_blend_get();
    eGPUDepthTest depth_test_prev = GPU_depth_test_get();
    GPU_scissor_test(false);

    /* Apply state for blit draw call. */
    GPU_stencil_write_mask_set(0xFF);
    GPU_stencil_reference_set(0);
    GPU_face_culling(GPU_CULL_NONE);
    GPU_stencil_test(GPU_STENCIL_ALWAYS);
    GPU_depth_mask(false);
    GPU_blend(GPU_BLEND_NONE);
    GPU_depth_test(GPU_DEPTH_ALWAYS);

    GPU_batch_draw(quad);

    /* restoring old pipeline state. */
    GPU_depth_mask(depth_write_prev);
    GPU_stencil_write_mask_set(stencil_mask_prev);
    GPU_stencil_test(stencil_test_prev);
    GPU_face_culling(culling_test_prev);
    GPU_depth_mask(depth_write_prev);
    GPU_blend(blend_prev);
    GPU_depth_test(depth_test_prev);

    if (restore_fb != nullptr) {
      GPU_framebuffer_bind(restore_fb);
    } else {
      GPU_framebuffer_restore();
    }
  }

  GPUFrameBuffer *gpu::MTLTexture::get_blit_framebuffer(uint dst_slice, uint dst_mip)
  {

    /* Check if layer has changed. */
    bool update_attachments = false;
    if (!m_blit_fb) {
      m_blit_fb = GPU_framebuffer_create("gpu_blit");
      update_attachments = true;
    }

    /* Check if current blit FB has the correct attachment properties. */
    if (m_blit_fb) {
      if (m_blit_fb_slice != dst_slice || m_blit_fb_mip != dst_mip) {
        update_attachments = true;
      }
    }

    if (update_attachments) {
      if (m_format_flag & GPU_FORMAT_DEPTH || m_format_flag & GPU_FORMAT_STENCIL) {
        /* DEPTH TEX */
        GPU_framebuffer_ensure_config(
          &m_blit_fb,
          {GPU_ATTACHMENT_TEXTURE_LAYER_MIP(wrap(static_cast<Texture *>(this)),
                                            static_cast<int>(dst_slice),
                                            static_cast<int>(dst_mip)),
           GPU_ATTACHMENT_NONE});
      } else {
        /* COLOR TEX */
        GPU_framebuffer_ensure_config(
          &m_blit_fb,
          {GPU_ATTACHMENT_NONE,
           GPU_ATTACHMENT_TEXTURE_LAYER_MIP(wrap(static_cast<Texture *>(this)),
                                            static_cast<int>(dst_slice),
                                            static_cast<int>(dst_mip))});
      }
      m_blit_fb_slice = dst_slice;
      m_blit_fb_mip = dst_mip;
    }

    KLI_assert(m_blit_fb);
    return m_blit_fb;
  }

  MTLSamplerState gpu::MTLTexture::get_sampler_state()
  {
    MTLSamplerState sampler_state;
    sampler_state.state = this->sampler_state;
    /* Add more parameters as needed */
    return sampler_state;
  }

  void gpu::MTLTexture::update_sub(int mip,
                                   int offset[3],
                                   int extent[3],
                                   eGPUDataFormat type,
                                   const void *data)
  {
    /* Fetch active context. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /* Do not update texture view. */
    KLI_assert(m_resource_mode != MTL_TEXTURE_MODE_TEXTURE_VIEW);

    /* Ensure mipmaps. */
    this->ensure_mipmaps(mip);

    /* Ensure texture is baked. */
    this->ensure_baked();

    /* Safety checks. */
#if TRUST_NO_ONE
    KLI_assert(mip >= m_mip_min && mip <= m_mip_max);
    KLI_assert(mip < m_texture->mipmapLevelCount());
    KLI_assert(m_texture->mipmapLevelCount() >= m_mip_max);
#endif

    /* DEPTH FLAG - Depth formats cannot use direct BLIT - pass off to their own routine which will
     * do a depth-only render. */
    bool is_depth_format = (m_format_flag & GPU_FORMAT_DEPTH);
    if (is_depth_format) {
      switch (m_type) {

        case GPU_TEXTURE_2D: {
          update_sub_depth_2d(mip, offset, extent, type, data);
          return;
        }
        default:
          MTL_LOG_ERROR(
            "[Error] gpu::MTLTexture::update_sub not yet supported for other depth "
            "configurations\n");
          return;
          return;
      }
    }

    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* Determine totalsize of INPUT Data. */
    int num_channels = to_component_len(m_format);
    int input_bytes_per_pixel = num_channels * to_bytesize(type);
    int totalsize = 0;

    /* If unpack row length is used, size of input data uses the unpack row length, rather than
     * the image length. */
    int expected_update_w = ((ctx->pipeline_state.unpack_row_length == 0) ?
                               extent[0] :
                               ctx->pipeline_state.unpack_row_length);

    /* Ensure calculated total size isn't larger than remaining image data size */
    switch (this->dimensions_count()) {
      case 1:
        totalsize = input_bytes_per_pixel * max_ii(expected_update_w, 1);
        break;
      case 2:
        totalsize = input_bytes_per_pixel * max_ii(expected_update_w, 1) * max_ii(extent[1], 1);
        break;
      case 3:
        totalsize = input_bytes_per_pixel * max_ii(expected_update_w, 1) * max_ii(extent[1], 1) *
                    max_ii(extent[2], 1);
        break;
      default:
        KLI_assert(false);
        break;
    }

    /* When unpack row length is used, provided data does not necessarily contain padding for
     * last row, so we only include up to the end of updated data. */
    if (ctx->pipeline_state.unpack_row_length > 0) {
      KLI_assert(ctx->pipeline_state.unpack_row_length >= extent[0]);
      totalsize -= (ctx->pipeline_state.unpack_row_length - extent[0]) * input_bytes_per_pixel;
    }

    /* Check */
    KLI_assert(totalsize > 0);

    /* Determine expected destination data size. */
    MTL::PixelFormat destination_format = gpu_texture_format_to_metal(m_format);
    int expected_dst_bytes_per_pixel = get_mtl_format_bytesize(destination_format);
    int destination_num_channels = get_mtl_format_num_components(destination_format);

    /* Prepare specialization struct (For texture update routine). */
    TextureUpdateRoutineSpecialisation compute_specialization_kernel = {
      tex_data_format_to_msl_type_str(type),              /* INPUT DATA FORMAT */
      tex_data_format_to_msl_texture_template_type(type), /* TEXTURE DATA FORMAT */
      num_channels,
      destination_num_channels};

    /* Determine whether we can do direct BLIT or not. */
    bool can_use_direct_blit = true;
    if (expected_dst_bytes_per_pixel != input_bytes_per_pixel ||
        num_channels != destination_num_channels) {
      can_use_direct_blit = false;
    }

    if (is_depth_format) {
      if (m_type == GPU_TEXTURE_2D || m_type == GPU_TEXTURE_2D_ARRAY) {
        /* Workaround for crash in validation layer when blitting to depth2D target with
         * dimensions (1, 1, 1); */
        if (extent[0] == 1 && extent[1] == 1 && extent[2] == 1 && totalsize == 4) {
          can_use_direct_blit = false;
        }
      }
    }

    if (m_format == GPU_SRGB8_A8 && !can_use_direct_blit) {
      MTL_LOG_WARNING(
        "SRGB data upload does not work correctly using compute upload. "
        "texname '%s'\n",
        m_name);
    }

    /* Safety Checks. */
    if (type == GPU_DATA_UINT_24_8 || type == GPU_DATA_10_11_11_REV) {
      KLI_assert(can_use_direct_blit &&
                 "Special input data type must be a 1-1 mapping with destination texture as it "
                 "cannot easily be split");
    }

    /* Debug and verification. */
    if (!can_use_direct_blit) {
      MTL_LOG_WARNING(
        "gpu::MTLTexture::update_sub supplied bpp is %d bytes (%d components per "
        "pixel), but backing texture bpp is %d bytes (%d components per pixel) "
        "(TODO(Metal): Channel Conversion needed) (w: %d, h: %d, d: %d)\n",
        input_bytes_per_pixel,
        num_channels,
        expected_dst_bytes_per_pixel,
        destination_num_channels,
        m_w,
        m_h,
        m_d);

      /* Check mip compatibility. */
      if (mip != 0) {
        MTL_LOG_ERROR(
          "[Error]: Updating texture layers other than mip=0 when data is mismatched is not "
          "possible in METAL on macOS using texture->write\n");
        return;
      }

      /* Check Format write-ability. */
      if (mtl_format_get_writeable_view_format(destination_format) == MTL::PixelFormatInvalid) {
        MTL_LOG_ERROR(
          "[Error]: Updating texture -- destination MTL::PixelFormat '%d' does not support write "
          "operations, and no suitable TextureView format exists.\n",
          *(int *)(&destination_format));
        return;
      }
    }

    /* Prepare staging buffer for data. */
    MTL::Buffer *staging_buffer = nil;
    uint64_t staging_buffer_offset = 0;

    /* Fetch allocation from scratch buffer. */
    MTLTemporaryBuffer allocation =
      ctx->get_scratchbuffer_manager().scratch_buffer_allocate_range_aligned(totalsize, 256);
    memcpy(allocation.data, data, totalsize);
    staging_buffer = allocation.metal_buffer;
    staging_buffer_offset = allocation.buffer_offset;

    /* Common Properties. */
    MTL::PixelFormat compatible_write_format = mtl_format_get_writeable_view_format(
      destination_format);

    /* Some texture formats are not writeable so we need to use a texture view. */
    if (compatible_write_format == MTL::PixelFormatInvalid) {
      MTL_LOG_ERROR("Cannot use compute update blit with texture-view format: %d\n",
                    *((int *)&compatible_write_format));
      return;
    }
    MTL::Texture *texture_handle = ((compatible_write_format == destination_format)) ?
                                     m_texture :
                                     m_texture->newTextureView(compatible_write_format);

    /* Prepare command encoders. */
    MTL::BlitCommandEncoder *blit_encoder = nil;
    MTL::ComputeCommandEncoder *compute_encoder = nil;
    if (can_use_direct_blit) {
      blit_encoder = ctx->main_command_buffer.ensure_begin_blit_encoder();
      KLI_assert(blit_encoder != nil);
    } else {
      compute_encoder = ctx->main_command_buffer.ensure_begin_compute_encoder();
      KLI_assert(compute_encoder != nil);
    }

    switch (m_type) {

      /* 1D */
      case GPU_TEXTURE_1D:
      case GPU_TEXTURE_1D_ARRAY: {
        if (can_use_direct_blit) {
          /* Use Blit based update. */
          int bytes_per_row = expected_dst_bytes_per_pixel *
                              ((ctx->pipeline_state.unpack_row_length == 0) ?
                                 extent[0] :
                                 ctx->pipeline_state.unpack_row_length);
          int bytes_per_image = bytes_per_row;
          int max_array_index = ((m_type == GPU_TEXTURE_1D_ARRAY) ? extent[1] : 1);
          for (int array_index = 0; array_index < max_array_index; array_index++) {

            int buffer_array_offset = staging_buffer_offset + (bytes_per_image * array_index);
            blit_encoder->copyFromBuffer(
              staging_buffer,
              buffer_array_offset,
              bytes_per_row,
              bytes_per_image,
              MTL::Size::Make(extent[0], 1, 1),
              texture_handle,
              ((m_type == GPU_TEXTURE_1D_ARRAY) ? (array_index + offset[1]) : 0),
              mip,
              MTL::Origin::Make(offset[0], 0, 0));
          }
        } else {
          /* Use Compute Based update. */
          if (m_type == GPU_TEXTURE_1D) {
            MTL::ComputePipelineState *pso = texture_update_1d_get_kernel(
              compute_specialization_kernel);
            TextureUpdateParams params = {
              mip,
              {extent[0], 1, 1},
              {offset[0], 0, 0},
              ((ctx->pipeline_state.unpack_row_length == 0) ?
                 extent[0] :
                 ctx->pipeline_state.unpack_row_length)
            };
            compute_encoder->setComputePipelineState(pso);
            compute_encoder->setBytes(&params, sizeof(params), 0);
            compute_encoder->setBuffer(staging_buffer, staging_buffer_offset, 1);
            compute_encoder->setTexture(texture_handle, 0);
            compute_encoder->dispatchThreads(
              MTL::Size::Make(extent[0], 1, 1), /* Width, Height, Layer */
              MTL::Size::Make(64, 1, 1));

          } else if (m_type == GPU_TEXTURE_1D_ARRAY) {
            MTL::ComputePipelineState *pso = texture_update_1d_array_get_kernel(
              compute_specialization_kernel);
            TextureUpdateParams params = {
              mip,
              {extent[0], extent[1], 1},
              {offset[0], offset[1], 0},
              ((ctx->pipeline_state.unpack_row_length == 0) ?
                 extent[0] :
                 ctx->pipeline_state.unpack_row_length)
            };
            compute_encoder->setComputePipelineState(pso);
            compute_encoder->setBytes(&params, sizeof(params), 0);
            compute_encoder->setBuffer(staging_buffer, staging_buffer_offset, 1);
            compute_encoder->setTexture(texture_handle, 0);
            compute_encoder->dispatchThreads(
              MTL::Size::Make(extent[0], extent[1], 1), /* Width, layers, nil */
              MTL::Size::Make(8, 8, 1));
          }
        }
      } break;

      /* 2D */
      case GPU_TEXTURE_2D:
      case GPU_TEXTURE_2D_ARRAY: {
        if (can_use_direct_blit) {
          /* Use Blit encoder update. */
          int bytes_per_row = expected_dst_bytes_per_pixel *
                              ((ctx->pipeline_state.unpack_row_length == 0) ?
                                 extent[0] :
                                 ctx->pipeline_state.unpack_row_length);
          int bytes_per_image = bytes_per_row * extent[1];

          int texture_array_relative_offset = 0;
          int base_slice = (m_type == GPU_TEXTURE_2D_ARRAY) ? offset[2] : 0;
          int final_slice = base_slice + ((m_type == GPU_TEXTURE_2D_ARRAY) ? extent[2] : 1);

          for (int array_slice = base_slice; array_slice < final_slice; array_slice++) {

            if (array_slice > 0) {
              KLI_assert(m_type == GPU_TEXTURE_2D_ARRAY);
              KLI_assert(array_slice < m_d);
            }

            blit_encoder->copyFromBuffer(staging_buffer,
                                         staging_buffer_offset + texture_array_relative_offset,
                                         bytes_per_row,
                                         bytes_per_image,
                                         MTL::Size::Make(extent[0], extent[1], 1),
                                         texture_handle,
                                         array_slice,
                                         mip,
                                         MTL::Origin::Make(offset[0], offset[1], 0));

            texture_array_relative_offset += bytes_per_image;
          }
        } else {
          /* Use Compute texture update. */
          if (m_type == GPU_TEXTURE_2D) {
            MTL::ComputePipelineState *pso = texture_update_2d_get_kernel(
              compute_specialization_kernel);
            TextureUpdateParams params = {
              mip,
              {extent[0], extent[1], 1},
              {offset[0], offset[1], 0},
              ((ctx->pipeline_state.unpack_row_length == 0) ?
                 extent[0] :
                 ctx->pipeline_state.unpack_row_length)
            };
            compute_encoder->setComputePipelineState(pso);
            compute_encoder->setBytes(&params, sizeof(params), 0);
            compute_encoder->setBuffer(staging_buffer, staging_buffer_offset, 1);
            compute_encoder->setTexture(texture_handle, 0);
            compute_encoder->dispatchThreads(
              MTL::Size::Make(extent[0], extent[1], 1), /* Width, Height, Layer */
              MTL::Size::Make(8, 8, 1));
          } else if (m_type == GPU_TEXTURE_2D_ARRAY) {
            MTL::ComputePipelineState *pso = texture_update_2d_array_get_kernel(
              compute_specialization_kernel);
            TextureUpdateParams params = {
              mip,
              {extent[0], extent[1], extent[2]},
              {offset[0], offset[1], offset[2]},
              ((ctx->pipeline_state.unpack_row_length == 0) ?
                 extent[0] :
                 ctx->pipeline_state.unpack_row_length)
            };
            compute_encoder->setComputePipelineState(pso);
            compute_encoder->setBytes(&params, sizeof(params), 0);
            compute_encoder->setBuffer(staging_buffer, staging_buffer_offset, 1);
            compute_encoder->setTexture(texture_handle, 0);
            compute_encoder->dispatchThreads(
              MTL::Size::Make(extent[0], extent[1], extent[2]), /* Width, Height, Layer */
              MTL::Size::Make(4, 4, 4));
          }
        }

      } break;

      /* 3D */
      case GPU_TEXTURE_3D: {
        if (can_use_direct_blit) {
          int bytes_per_row = expected_dst_bytes_per_pixel *
                              ((ctx->pipeline_state.unpack_row_length == 0) ?
                                 extent[0] :
                                 ctx->pipeline_state.unpack_row_length);
          int bytes_per_image = bytes_per_row * extent[1];
          blit_encoder->copyFromBuffer(staging_buffer,
                                       staging_buffer_offset,
                                       bytes_per_row,
                                       bytes_per_image,
                                       MTL::Size::Make(extent[0], extent[1], extent[2]),
                                       texture_handle,
                                       0,
                                       mip,
                                       MTL::Origin::Make(offset[0], offset[1], offset[2]));
        } else {
          MTL::ComputePipelineState *pso = texture_update_3d_get_kernel(
            compute_specialization_kernel);
          TextureUpdateParams params = {
            mip,
            {extent[0], extent[1], extent[2]},
            {offset[0], offset[1], offset[2]},
            ((ctx->pipeline_state.unpack_row_length == 0) ?
               extent[0] :
               ctx->pipeline_state.unpack_row_length)
          };
          compute_encoder->setComputePipelineState(pso);
          compute_encoder->setBytes(&params, sizeof(params), 0);
          compute_encoder->setBuffer(staging_buffer, staging_buffer_offset, 1);
          compute_encoder->setTexture(texture_handle, 0);
          compute_encoder->dispatchThreads(
            MTL::Size::Make(extent[0], extent[1], extent[2]), /* Width, Height, Depth */
            MTL::Size::Make(4, 4, 4));
        }
      } break;

      /* CUBE */
      case GPU_TEXTURE_CUBE: {
        if (can_use_direct_blit) {
          int bytes_per_row = expected_dst_bytes_per_pixel *
                              ((ctx->pipeline_state.unpack_row_length == 0) ?
                                 extent[0] :
                                 ctx->pipeline_state.unpack_row_length);
          int bytes_per_image = bytes_per_row * extent[1];

          int texture_array_relative_offset = 0;

          /* Iterate over all cube faces in range (offset[2], offset[2] + extent[2]). */
          for (int i = 0; i < extent[2]; i++) {
            int face_index = offset[2] + i;

            blit_encoder->copyFromBuffer(staging_buffer,
                                         staging_buffer_offset + texture_array_relative_offset,
                                         bytes_per_row,
                                         bytes_per_image,
                                         MTL::Size::Make(extent[0], extent[1], 1),
                                         texture_handle,
                                         face_index /* = cubeFace+arrayIndex*6 */,
                                         mip,
                                         MTL::Origin::Make(offset[0], offset[1], 0));
            texture_array_relative_offset += bytes_per_image;
          }
        } else {
          MTL_LOG_ERROR(
            "TODO(Metal): Support compute texture update for GPU_TEXTURE_CUBE %d, %d, %d\n",
            m_w,
            m_h,
            m_d);
        }
      } break;

      case GPU_TEXTURE_CUBE_ARRAY: {
        if (can_use_direct_blit) {

          int bytes_per_row = expected_dst_bytes_per_pixel *
                              ((ctx->pipeline_state.unpack_row_length == 0) ?
                                 extent[0] :
                                 ctx->pipeline_state.unpack_row_length);
          int bytes_per_image = bytes_per_row * extent[1];

          /* Upload to all faces between offset[2] (which is zero in most cases) AND extent[2].
           */
          int texture_array_relative_offset = 0;
          for (int i = 0; i < extent[2]; i++) {
            int face_index = offset[2] + i;
            blit_encoder->copyFromBuffer(staging_buffer,
                                         staging_buffer_offset + texture_array_relative_offset,
                                         bytes_per_row,
                                         bytes_per_image,
                                         MTL::Size::Make(extent[0], extent[1], 1),
                                         texture_handle,
                                         face_index /* = cubeFace+arrayIndex*6. */,
                                         mip,
                                         MTL::Origin::Make(offset[0], offset[1], 0));
            texture_array_relative_offset += bytes_per_image;
          }
        } else {
          MTL_LOG_ERROR(
            "TODO(Metal): Support compute texture update for GPU_TEXTURE_CUBE_ARRAY %d, %d, "
            "%d\n",
            m_w,
            m_h,
            m_d);
        }
      } break;

      case GPU_TEXTURE_BUFFER: {
        /* TODO(Metal): Support Data upload to TEXTURE BUFFER
         * Data uploads generally happen via GPUVertBuf instead. */
        KLI_assert(false);
      } break;

      case GPU_TEXTURE_ARRAY:
        /* Not an actual format - modifier flag for others. */
        return;
    }

    /* Finalize Blit Encoder. */
    if (can_use_direct_blit) {

      /* Textures which use MTLStorageModeManaged need to have updated contents
       * synced back to CPU to avoid an automatic flush overwriting contents. */
      if (m_texture->storageMode() == MTL::StorageModeManaged) {
        blit_encoder->synchronizeResource(m_texture_buffer);
      }
    } else {
      /* Textures which use MTLStorageModeManaged need to have updated contents
       * synced back to CPU to avoid an automatic flush overwriting contents. */
      if (m_texture->storageMode() == MTL::StorageModeManaged) {
        blit_encoder = ctx->main_command_buffer.ensure_begin_blit_encoder();
        blit_encoder->synchronizeResource(m_texture_buffer);
      }
    }

    pool->drain();
    pool = nil;
  }

  void gpu::MTLTexture::ensure_mipmaps(int miplvl)
  {

    /* Do not update texture view. */
    KLI_assert(m_resource_mode != MTL_TEXTURE_MODE_TEXTURE_VIEW);

    /* Clamp level to maximum. */
    int effective_h = (m_type == GPU_TEXTURE_1D_ARRAY) ? 0 : m_h;
    int effective_d = (m_type != GPU_TEXTURE_3D) ? 0 : m_d;
    int max_dimension = max_iii(m_w, effective_h, effective_d);
    int max_miplvl = floor(log2(max_dimension));
    miplvl = min_ii(max_miplvl, miplvl);

    /* Increase mipmap level. */
    if (m_mipmaps < miplvl) {
      m_mipmaps = miplvl;

      /* Check if baked. */
      if (m_is_baked && m_mipmaps > m_mtl_max_mips) {
        m_is_dirty = true;
        MTL_LOG_WARNING("Texture requires regenerating due to increase in mip-count\n");
      }
    }
    this->mip_range_set(0, m_mipmaps);
  }

  void gpu::MTLTexture::generate_mipmap()
  {
    /* Fetch Active Context. */
    MTLContext *ctx = reinterpret_cast<MTLContext *>(GPU_context_active_get());
    KLI_assert(ctx);

    if (!ctx->device) {
      MTL_LOG_ERROR("Cannot Generate mip-maps -- metal device invalid\n");
      KLI_assert(false);
      return;
    }

    /* Ensure mipmaps. */
    this->ensure_mipmaps(9999);

    /* Ensure texture is baked. */
    this->ensure_baked();
    KLI_assert_msg(m_is_baked && m_texture, "MTLTexture is not valid");

    if (m_mipmaps == 1 || m_mtl_max_mips == 1) {
      MTL_LOG_WARNING("Call to generate mipmaps on texture with 'm_mipmaps=1\n'");
      return;
    }

    /* Verify if we can perform mipmap generation. */
    if (m_format == GPU_DEPTH_COMPONENT32F || m_format == GPU_DEPTH_COMPONENT24 ||
        m_format == GPU_DEPTH_COMPONENT16 || m_format == GPU_DEPTH32F_STENCIL8 ||
        m_format == GPU_DEPTH24_STENCIL8) {
      MTL_LOG_WARNING("Cannot generate mipmaps for textures using DEPTH formats\n");
      return;
    }

    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* Fetch active BlitCommandEncoder. */
    MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
    if (G.debug & G_DEBUG_GPU) {
      enc->insertDebugSignpost(NS_STRING_("Generate MipMaps"));
    }
    enc->generateMipmaps(m_texture);

    pool->drain();
    pool = nil;

    return;
  }

  void gpu::MTLTexture::copy_to(Texture *dst)
  {
    /* Safety Checks. */
    gpu::MTLTexture *mt_src = this;
    gpu::MTLTexture *mt_dst = static_cast<gpu::MTLTexture *>(dst);
    KLI_assert((mt_dst->m_w == mt_src->m_w) && (mt_dst->m_h == mt_src->m_h) &&
               (mt_dst->m_d == mt_src->m_d));
    KLI_assert(mt_dst->m_format == mt_src->m_format);
    KLI_assert(mt_dst->m_type == mt_src->m_type);

    UNUSED_VARS_NDEBUG(mt_src);

    /* Fetch active context. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /* Ensure texture is baked. */
    this->ensure_baked();

    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* Setup blit encoder. */
    MTL::BlitCommandEncoder *blit_encoder = ctx->main_command_buffer.ensure_begin_blit_encoder();
    KLI_assert(blit_encoder != nil);

    /* TODO(Metal): Consider supporting multiple mip levels IF the GL implementation
     * follows, currently it does not. */
    int mip = 0;

    /* NOTE: mip_size_get() won't override any dimension that is equal to 0. */
    int extent[3] = {1, 1, 1};
    this->mip_size_get(mip, extent);

    switch (mt_dst->m_type) {
      case GPU_TEXTURE_2D_ARRAY:
      case GPU_TEXTURE_CUBE_ARRAY:
      case GPU_TEXTURE_3D: {
        /* Do full texture copy for 3D textures */
        KLI_assert(mt_dst->m_d == m_d);
        blit_encoder->copyFromTexture(this->get_metal_handle_base(),
                                      mt_dst->get_metal_handle_base());
      } break;
      default: {
        int slice = 0;
        this->blit(blit_encoder,
                   0,
                   0,
                   0,
                   slice,
                   mip,
                   mt_dst,
                   0,
                   0,
                   0,
                   slice,
                   mip,
                   extent[0],
                   extent[1],
                   extent[2]);
      } break;
    }

    pool->drain();
    pool = nil;
  }

  void gpu::MTLTexture::clear(eGPUDataFormat data_format, const void *data)
  {
    /* Ensure texture is baked. */
    this->ensure_baked();

    /* Create clear framebuffer. */
    GPUFrameBuffer *prev_fb = GPU_framebuffer_active_get();
    FrameBuffer *fb = reinterpret_cast<FrameBuffer *>(this->get_blit_framebuffer(0, 0));
    fb->bind(true);
    fb->clear_attachment(this->attachment_type(0), data_format, data);
    GPU_framebuffer_bind(prev_fb);
  }

  static MTL::TextureSwizzle swizzle_to_mtl(const char swizzle)
  {
    switch (swizzle) {
      default:
      case 'x':
      case 'r':
        return MTL::TextureSwizzleRed;
      case 'y':
      case 'g':
        return MTL::TextureSwizzleGreen;
      case 'z':
      case 'b':
        return MTL::TextureSwizzleBlue;
      case 'w':
      case 'a':
        return MTL::TextureSwizzleAlpha;
      case '0':
        return MTL::TextureSwizzleZero;
      case '1':
        return MTL::TextureSwizzleOne;
    }
  }

  void gpu::MTLTexture::swizzle_set(const char swizzle_mask[4])
  {
    if (memcmp(m_tex_swizzle_mask, swizzle_mask, 4) != 0) {
      memcpy(m_tex_swizzle_mask, swizzle_mask, 4);

      /* Creating the swizzle mask and flagging as dirty if changed. */
      MTL::TextureSwizzleChannels new_swizzle_mask;
      new_swizzle_mask.red = swizzle_to_mtl(swizzle_mask[0]);
      new_swizzle_mask.green = swizzle_to_mtl(swizzle_mask[1]);
      new_swizzle_mask.blue = swizzle_to_mtl(swizzle_mask[2]);
      new_swizzle_mask.alpha = swizzle_to_mtl(swizzle_mask[3]);

      m_mtl_swizzle_mask = new_swizzle_mask;
      m_texture_view_dirty_flags |= TEXTURE_VIEW_SWIZZLE_DIRTY;
    }
  }

  void gpu::MTLTexture::mip_range_set(int min, int max)
  {
    KLI_assert(min <= max && min >= 0 && max <= m_mipmaps);

    /* NOTE:
     * - m_mip_min and m_mip_max are used to Clamp LODs during sampling.
     * - Given functions like Framebuffer::recursive_downsample modifies the mip range
     *   between each layer, we do not want to be re-baking the texture.
     * - For the time being, we are going to just need to generate a FULL mipmap chain
     *   as we do not know ahead of time whether mipmaps will be used.
     *
     *   TODO(Metal): Add texture initialization flag to determine whether mipmaps are used
     *   or not. Will be important for saving memory for big textures. */
    m_mip_min = min;
    m_mip_max = max;

    if ((m_type == GPU_TEXTURE_1D || m_type == GPU_TEXTURE_1D_ARRAY ||
         m_type == GPU_TEXTURE_BUFFER) &&
        max > 1) {

      MTL_LOG_ERROR(
        " MTLTexture of type TEXTURE_1D_ARRAY or TEXTURE_BUFFER cannot have a mipcount "
        "greater than 1\n");
      m_mip_min = 0;
      m_mip_max = 0;
      m_mipmaps = 0;
      KLI_assert(false);
    }

    /* Mip range for texture view. */
    m_mip_texture_base_level = m_mip_min;
    m_mip_texture_max_level = m_mip_max;
    m_texture_view_dirty_flags |= TEXTURE_VIEW_MIP_DIRTY;
  }

  void *gpu::MTLTexture::read(int mip, eGPUDataFormat type)
  {
    /* Prepare Array for return data. */
    KLI_assert(!(m_format_flag & GPU_FORMAT_COMPRESSED));
    KLI_assert(mip <= m_mipmaps);
    KLI_assert(validate_data_format_mtl(m_format, type));

    /* NOTE: mip_size_get() won't override any dimension that is equal to 0. */
    int extent[3] = {1, 1, 1};
    this->mip_size_get(mip, extent);

    size_t sample_len = extent[0] * extent[1] * extent[2];
    size_t sample_size = to_bytesize(m_format, type);
    size_t texture_size = sample_len * sample_size;
    int num_channels = to_component_len(m_format);

    void *data = MEM_mallocN(texture_size + 8, "GPU_texture_read");

    /* Ensure texture is baked. */
    if (m_is_baked) {
      this->read_internal(mip,
                          0,
                          0,
                          0,
                          extent[0],
                          extent[1],
                          extent[2],
                          type,
                          num_channels,
                          texture_size + 8,
                          data);
    } else {
      /* Clear return values? */
      MTL_LOG_WARNING("MTLTexture::read - reading from texture with no image data\n");
    }

    return data;
  }

  /* Fetch the raw buffer data from a texture and copy to CPU host ptr. */
  void gpu::MTLTexture::read_internal(int mip,
                                      int x_off,
                                      int y_off,
                                      int z_off,
                                      int width,
                                      int height,
                                      int depth,
                                      eGPUDataFormat desired_output_format,
                                      int num_output_components,
                                      int debug_data_size,
                                      void *r_data)
  {
    /* Verify textures are baked. */
    if (!m_is_baked) {
      MTL_LOG_WARNING(
        "gpu::MTLTexture::read_internal - Trying to read from a non-baked texture!\n");
      return;
    }
    /* Fetch active context. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /* Calculate Desired output size. */
    int num_channels = to_component_len(m_format);
    KLI_assert(num_output_components <= num_channels);
    uint desired_output_bpp = num_output_components * to_bytesize(desired_output_format);

    /* Calculate Metal data output for trivial copy. */
    uint image_bpp = get_mtl_format_bytesize(m_texture->pixelFormat());
    uint image_components = get_mtl_format_num_components(m_texture->pixelFormat());
    bool is_depth_format = (m_format_flag & GPU_FORMAT_DEPTH);

    /* Verify if we need to use compute read. */
    eGPUDataFormat data_format = to_mtl_internal_data_format(this->format_get());
    bool format_conversion_needed = (data_format != desired_output_format);
    bool can_use_simple_read = (desired_output_bpp == image_bpp) && (!format_conversion_needed) &&
                               (num_output_components == image_components);

    /* Depth must be read using the compute shader -- Some safety checks to verify that params are
     * correct. */
    if (is_depth_format) {
      can_use_simple_read = false;
      /* TODO(Metal): Stencil data write not yet supported, so force components to one. */
      image_components = 1;
      KLI_assert(num_output_components == 1);
      KLI_assert(image_components == 1);
      KLI_assert(data_format == GPU_DATA_FLOAT || data_format == GPU_DATA_UINT_24_8);
      KLI_assert(validate_data_format_mtl(m_format, data_format));
    }

    /* SPECIAL Workaround for R11G11B10 textures requesting a read using: GPU_DATA_10_11_11_REV. */
    if (desired_output_format == GPU_DATA_10_11_11_REV) {
      KLI_assert(m_format == GPU_R11F_G11F_B10F);

      /* override parameters - we'll be able to use simple copy, as bpp will match at 4 bytes. */
      image_bpp = sizeof(int);
      image_components = 1;
      desired_output_bpp = sizeof(int);
      num_output_components = 1;

      data_format = GPU_DATA_INT;
      format_conversion_needed = false;
      can_use_simple_read = true;
    }

    /* Determine size of output data. */
    uint bytes_per_row = desired_output_bpp * width;
    uint bytes_per_image = bytes_per_row * height;
    uint total_bytes = bytes_per_image * depth;

    if (can_use_simple_read) {
      /* DEBUG check that if direct copy is being used, then both the expected output size matches
       * the METAL texture size. */
      KLI_assert(
        ((num_output_components * to_bytesize(desired_output_format)) == desired_output_bpp) &&
        (desired_output_bpp == image_bpp));
    }
    /* DEBUG check that the allocated data size matches the bytes we expect. */
    KLI_assert(total_bytes <= debug_data_size);

    /* Fetch allocation from scratch buffer. */
    MTL::Buffer *destination_buffer = nil;
    uint destination_offset = 0;
    void *destination_buffer_host_ptr = nullptr;

    /* TODO(Metal): Optimize buffer allocation. */
    MTL::ResourceOptions bufferOptions = MTL::ResourceStorageModeManaged;
    destination_buffer = ctx->device->newBuffer(max_ii(total_bytes, 256), bufferOptions);
    destination_offset = 0;
    destination_buffer_host_ptr = (void *)((uint8_t *)(destination_buffer->contents()) +
                                           destination_offset);

    /* Prepare specialization struct (For non-trivial texture read routine). */
    int depth_format_mode = 0;
    if (is_depth_format) {
      depth_format_mode = 1;
      switch (desired_output_format) {
        case GPU_DATA_FLOAT:
          depth_format_mode = 1;
          break;
        case GPU_DATA_UINT_24_8:
          depth_format_mode = 2;
          break;
        case GPU_DATA_UINT:
          depth_format_mode = 4;
          break;
        default:
          KLI_assert_msg(false, "Unhandled depth read format case");
          break;
      }
    }

    TextureReadRoutineSpecialisation compute_specialization_kernel = {
      tex_data_format_to_msl_texture_template_type(data_format), /* TEXTURE DATA TYPE */
      tex_data_format_to_msl_type_str(desired_output_format),    /* OUTPUT DATA TYPE */
      num_channels,                                              /* TEXTURE COMPONENT COUNT */
      num_output_components,                                     /* OUTPUT DATA COMPONENT COUNT */
      depth_format_mode};

    bool copy_successful = false;
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* TODO(Metal): Verify whether we need some form of barrier here to ensure reads
     * happen after work with associated texture is finished. */
    GPU_finish();

    /* Texture View for SRGB special case. */
    MTL::Texture *read_texture = m_texture;
    if (m_format == GPU_SRGB8_A8) {
      read_texture = m_texture->newTextureView(MTL::PixelFormatRGBA8Unorm);
    }

    /* Perform per-texture type read. */
    switch (m_type) {
      case GPU_TEXTURE_2D: {
        if (can_use_simple_read) {
          /* Use Blit Encoder READ. */
          MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
          if (G.debug & G_DEBUG_GPU) {
            enc->insertDebugSignpost(NS_STRING_("GPUTextureRead"));
          }
          enc->copyFromTexture(read_texture,
                               0,
                               mip,
                               MTL::Origin::Make(x_off, y_off, 0),
                               MTL::Size::Make(width, height, 1),
                               destination_buffer,
                               destination_offset,
                               bytes_per_row,
                               bytes_per_image);
          enc->synchronizeResource(destination_buffer);
          copy_successful = true;
        } else {

          /* Use Compute READ. */
          MTL::ComputeCommandEncoder *compute_encoder =
            ctx->main_command_buffer.ensure_begin_compute_encoder();
          MTL::ComputePipelineState *pso = texture_read_2d_get_kernel(
            compute_specialization_kernel);
          TextureReadParams params = {
            mip,
            {width, height, 1},
            {x_off, y_off,  0},
          };
          compute_encoder->setComputePipelineState(pso);
          compute_encoder->setBytes(&params, sizeof(params), 0);
          compute_encoder->setBuffer(destination_buffer, destination_offset, 1);
          compute_encoder->setTexture(read_texture, 0);
          compute_encoder->dispatchThreads(
            MTL::Size::Make(width, height, 1), /* Width, Height, Layer */
            MTL::Size::Make(8, 8, 1));

          /* Use Blit encoder to synchronize results back to CPU. */
          MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
          if (G.debug & G_DEBUG_GPU) {
            enc->insertDebugSignpost(NS_STRING_("GPUTextureRead-syncResource"));
          }
          enc->synchronizeResource(destination_buffer);
          copy_successful = true;
        }
      } break;

      case GPU_TEXTURE_2D_ARRAY: {
        if (can_use_simple_read) {
          /* Use Blit Encoder READ. */
          MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
          if (G.debug & G_DEBUG_GPU) {
            enc->insertDebugSignpost(NS_STRING_("GPUTextureRead"));
          }
          int base_slice = z_off;
          int final_slice = base_slice + depth;
          int texture_array_relative_offset = 0;

          for (int array_slice = base_slice; array_slice < final_slice; array_slice++) {
            enc->copyFromTexture(read_texture,
                                 0,
                                 mip,
                                 MTL::Origin::Make(x_off, y_off, 0),
                                 MTL::Size::Make(width, height, 1),
                                 destination_buffer,
                                 destination_offset + texture_array_relative_offset,
                                 bytes_per_row,
                                 bytes_per_image);
            enc->synchronizeResource(destination_buffer);

            texture_array_relative_offset += bytes_per_image;
          }
          copy_successful = true;
        } else {

          /* Use Compute READ */
          MTL::ComputeCommandEncoder *compute_encoder =
            ctx->main_command_buffer.ensure_begin_compute_encoder();
          MTL::ComputePipelineState *pso = texture_read_2d_array_get_kernel(
            compute_specialization_kernel);
          TextureReadParams params = {
            mip,
            {width, height, depth},
            {x_off, y_off,  z_off},
          };
          compute_encoder->setComputePipelineState(pso);
          compute_encoder->setBytes(&params, sizeof(params), 0);
          compute_encoder->setBuffer(destination_buffer, destination_offset, 1);
          compute_encoder->setTexture(read_texture, 0);
          compute_encoder->dispatchThreads(
            MTL::Size::Make(width, height, depth), /* Width, Height, Layer */
            MTL::Size::Make(8, 8, 1));

          /* Use Blit encoder to synchronize results back to CPU. */
          MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
          if (G.debug & G_DEBUG_GPU) {
            enc->insertDebugSignpost(NS_STRING_("GPUTextureRead-syncResource"));
          }
          enc->synchronizeResource(destination_buffer);
          copy_successful = true;
        }
      } break;

      case GPU_TEXTURE_CUBE_ARRAY: {
        if (can_use_simple_read) {
          MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
          if (G.debug & G_DEBUG_GPU) {
            enc->insertDebugSignpost(NS_STRING_("GPUTextureRead"));
          }
          int base_slice = z_off;
          int final_slice = base_slice + depth;
          int texture_array_relative_offset = 0;

          for (int array_slice = base_slice; array_slice < final_slice; array_slice++) {
            enc->copyFromTexture(read_texture,
                                 array_slice,
                                 mip,
                                 MTL::Origin::Make(x_off, y_off, 0),
                                 MTL::Size::Make(width, height, 1),
                                 destination_buffer,
                                 destination_offset + texture_array_relative_offset,
                                 bytes_per_row,
                                 bytes_per_image);
            enc->synchronizeResource(destination_buffer);

            texture_array_relative_offset += bytes_per_image;
          }
          MTL_LOG_INFO("Copying texture data to buffer GPU_TEXTURE_CUBE_ARRAY\n");
          copy_successful = true;
        } else {
          MTL_LOG_ERROR("TODO(Metal): unsupported compute copy of texture cube array");
        }
      } break;

      default:
        MTL_LOG_WARNING(
          "[Warning] gpu::MTLTexture::read_internal simple-copy not yet supported for texture "
          "type: %d\n",
          (int)m_type);
        break;
    }

    if (copy_successful) {
      /* Ensure GPU copy commands have completed. */
      GPU_finish();

      /* Copy data from Shared Memory into ptr. */
      memcpy(r_data, destination_buffer_host_ptr, total_bytes);
      MTL_LOG_INFO("gpu::MTLTexture::read_internal success! %d bytes read\n", total_bytes);
    } else {
      MTL_LOG_WARNING(
        "[Warning] gpu::MTLTexture::read_internal not yet supported for this config -- data "
        "format different (src %d bytes, dst %d bytes) (src format: %d, dst format: %d), or "
        "varying component counts (src %d, dst %d)\n",
        image_bpp,
        desired_output_bpp,
        (int)data_format,
        (int)desired_output_format,
        image_components,
        num_output_components);
    }

    pool->drain();
    pool = nil;
  }

  /* Remove once no longer required -- will just return 0 for now in MTL path. */
  uint gpu::MTLTexture::gl_bindcode_get() const
  {
    return 0;
  }

  bool gpu::MTLTexture::init_internal()
  {
    if (m_format == GPU_DEPTH24_STENCIL8) {
      /* Apple Silicon requires GPU_DEPTH32F_STENCIL8 instead of GPU_DEPTH24_STENCIL8. */
      m_format = GPU_DEPTH32F_STENCIL8;
    }

    this->prepare_internal();
    return true;
  }

  bool gpu::MTLTexture::init_internal(GPUVertBuf *vbo)
  {
    if (this->m_format == GPU_DEPTH24_STENCIL8) {
      /* Apple Silicon requires GPU_DEPTH32F_STENCIL8 instead of GPU_DEPTH24_STENCIL8. */
      this->m_format = GPU_DEPTH32F_STENCIL8;
    }

    MTL::PixelFormat mtl_format = gpu_texture_format_to_metal(this->m_format);
    m_mtl_max_mips = 1;
    m_mipmaps = 0;
    this->mip_range_set(0, 0);

    /* Create texture from GPUVertBuf's buffer. */
    MTLVertBuf *mtl_vbo = static_cast<MTLVertBuf *>(unwrap(vbo));
    mtl_vbo->bind();
    mtl_vbo->flag_used();

    /* Get Metal Buffer. */
    MTL::Buffer *source_buffer = mtl_vbo->get_metal_buffer();
    KLI_assert(source_buffer);

    /* Verify size. */
    if (m_w <= 0) {
      MTL_LOG_WARNING("Allocating texture buffer of width 0!\n");
      m_w = 1;
    }

    /* Verify Texture and vertex buffer alignment. */
    int bytes_per_pixel = get_mtl_format_bytesize(mtl_format);
    int bytes_per_row = bytes_per_pixel * m_w;

    kraken::gpu::MTLContext *mtl_ctx = kraken::gpu::MTLContext::get();
    uint32_t align_requirement = static_cast<uint32_t>(
      mtl_ctx->device->minimumLinearTextureAlignmentForPixelFormat(mtl_format));

    /* Verify per-vertex size aligns with texture size. */
    const GPUVertFormat *format = GPU_vertbuf_get_format(vbo);
    KLI_assert(bytes_per_pixel == format->stride &&
               "Pixel format stride MUST match the texture format stride -- These being different "
               "is likely caused by Metal's VBO padding to a minimum of 4-bytes per-vertex");
    UNUSED_VARS_NDEBUG(format);

    /* Create texture descriptor. */
    KLI_assert(m_type == GPU_TEXTURE_BUFFER);
    m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
    m_texture_descriptor->setPixelFormat(mtl_format);
    m_texture_descriptor->setTextureType(MTL::TextureTypeTextureBuffer);
    m_texture_descriptor->setWidth(m_w);
    m_texture_descriptor->setHeight(1);
    m_texture_descriptor->setDepth(1);
    m_texture_descriptor->setArrayLength(1);
    m_texture_descriptor->setMipmapLevelCount(m_mtl_max_mips);
    m_texture_descriptor->setUsage(
      MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite |
      MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimise usage flags. */
    m_texture_descriptor->setStorageMode(source_buffer->storageMode());
    m_texture_descriptor->setSampleCount(1);
    m_texture_descriptor->setCpuCacheMode(source_buffer->cpuCacheMode());
    m_texture_descriptor->setHazardTrackingMode(source_buffer->hazardTrackingMode());

    m_texture = source_buffer->newTexture(m_texture_descriptor,
                                          0,
                                          ceil_to_multiple_u(bytes_per_row, align_requirement));
    m_aligned_w = bytes_per_row / bytes_per_pixel;

    KLI_assert(m_texture);
    m_texture->setLabel(NS_STRING_(this->get_name()));
    m_is_baked = true;
    m_is_dirty = false;
    m_resource_mode = MTL_TEXTURE_MODE_VBO;

    /* Track Status. */
    m_vert_buffer = mtl_vbo;
    m_vert_buffer_mtl = source_buffer;
    /* Cleanup. */
    m_texture_descriptor->release();
    m_texture_descriptor = nullptr;

    return true;
  }

  bool gpu::MTLTexture::init_internal(const GPUTexture *src, int mip_offset, int layer_offset)
  {
    KLI_assert(src);

    /* Zero initialize. */
    this->prepare_internal();

    /* Flag as using texture view. */
    m_resource_mode = MTL_TEXTURE_MODE_TEXTURE_VIEW;
    m_source_texture = src;
    m_mip_texture_base_level = mip_offset;
    m_mip_texture_base_layer = layer_offset;

    /* Assign texture as view. */
    const gpu::MTLTexture *mtltex = static_cast<const gpu::MTLTexture *>(unwrap(src));
    m_texture = mtltex->m_texture;
    KLI_assert(m_texture);
    m_texture->retain();

    /* Flag texture as baked -- we do not need explicit initialization. */
    m_is_baked = true;
    m_is_dirty = false;

    /* Bake mip swizzle view. */
    bake_mip_swizzle_view();
    return true;
  }

  /** \} */

  /* -------------------------------------------------------------------- */
  /** \name METAL Resource creation and management
   * \{ */

  bool gpu::MTLTexture::texture_is_baked()
  {
    return m_is_baked;
  }

  /* Prepare texture parameters after initialization, but before baking. */
  void gpu::MTLTexture::prepare_internal()
  {
    /* Derive implicit usage flags for Depth/Stencil attachments. */
    if (m_format_flag & GPU_FORMAT_DEPTH || m_format_flag & GPU_FORMAT_STENCIL) {
      m_gpu_image_usage_flags |= GPU_TEXTURE_USAGE_ATTACHMENT;
    }

    /* Derive maximum number of mip levels by default.
     * TODO(Metal): This can be removed if max mip counts are specified upfront. */
    if (m_type == GPU_TEXTURE_1D || m_type == GPU_TEXTURE_1D_ARRAY ||
        m_type == GPU_TEXTURE_BUFFER) {
      m_mtl_max_mips = 1;
    } else {
      int effective_h = (m_type == GPU_TEXTURE_1D_ARRAY) ? 0 : m_h;
      int effective_d = (m_type != GPU_TEXTURE_3D) ? 0 : m_d;
      int max_dimension = max_iii(m_w, effective_h, effective_d);
      int max_miplvl = max_ii(floor(log2(max_dimension)) + 1, 1);
      m_mtl_max_mips = max_miplvl;
    }
  }

  void gpu::MTLTexture::ensure_baked()
  {

    /* If properties have changed, re-bake. */
    bool copy_previous_contents = false;
    if (m_is_baked && m_is_dirty) {
      copy_previous_contents = true;
      MTL::Texture *previous_texture = m_texture;
      previous_texture->retain();

      this->reset();
    }

    if (!m_is_baked) {
      MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
      KLI_assert(ctx);

      /* Ensure texture mode is valid. */
      KLI_assert(m_resource_mode != MTL_TEXTURE_MODE_EXTERNAL);
      KLI_assert(m_resource_mode != MTL_TEXTURE_MODE_TEXTURE_VIEW);
      KLI_assert(m_resource_mode != MTL_TEXTURE_MODE_VBO);

      /* Format and mip levels (TODO(Metal): Optimize mipmaps counts, specify up-front). */
      MTL::PixelFormat mtl_format = gpu_texture_format_to_metal(m_format);

      /* Create texture descriptor. */
      switch (m_type) {

        /* 1D */
        case GPU_TEXTURE_1D:
        case GPU_TEXTURE_1D_ARRAY: {
          KLI_assert(m_w > 0);
          m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
          m_texture_descriptor->setPixelFormat(mtl_format);
          m_texture_descriptor->setTextureType(
            (m_type == GPU_TEXTURE_1D_ARRAY) ? MTL::TextureType1DArray : MTL::TextureType1D);
          m_texture_descriptor->setWidth(m_w);
          m_texture_descriptor->setHeight(1);
          m_texture_descriptor->setDepth(1);
          m_texture_descriptor->setArrayLength((m_type == GPU_TEXTURE_1D_ARRAY) ? m_h : 1);
          m_texture_descriptor->setMipmapLevelCount((m_mtl_max_mips > 0) ? m_mtl_max_mips : 1);
          m_texture_descriptor->setUsage(
            MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead |
            MTL::TextureUsageShaderWrite |
            MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimize usage flags. */
          m_texture_descriptor->setStorageMode(MTL::StorageModePrivate);
          m_texture_descriptor->setSampleCount(1);
          m_texture_descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
          m_texture_descriptor->setHazardTrackingMode(MTL::HazardTrackingModeDefault);
        } break;

        /* 2D */
        case GPU_TEXTURE_2D:
        case GPU_TEXTURE_2D_ARRAY: {
          KLI_assert(m_w > 0 && m_h > 0);
          m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
          m_texture_descriptor->setPixelFormat(mtl_format);
          m_texture_descriptor->setTextureType(
            (m_type == GPU_TEXTURE_2D_ARRAY) ? MTL::TextureType2DArray : MTL::TextureType2D);
          m_texture_descriptor->setWidth(m_w);
          m_texture_descriptor->setHeight(m_h);
          m_texture_descriptor->setDepth(1);
          m_texture_descriptor->setArrayLength((m_type == GPU_TEXTURE_2D_ARRAY) ? m_h : 1);
          m_texture_descriptor->setMipmapLevelCount((m_mtl_max_mips > 0) ? m_mtl_max_mips : 1);
          m_texture_descriptor->setUsage(
            MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead |
            MTL::TextureUsageShaderWrite |
            MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimize usage flags. */
          m_texture_descriptor->setStorageMode(MTL::StorageModePrivate);
          m_texture_descriptor->setSampleCount(1);
          m_texture_descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
          m_texture_descriptor->setHazardTrackingMode(MTL::HazardTrackingModeDefault);
        } break;

        /* 3D */
        case GPU_TEXTURE_3D: {
          KLI_assert(m_w > 0 && m_h > 0 && m_d > 0);
          m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
          m_texture_descriptor->setPixelFormat(mtl_format);
          m_texture_descriptor->setTextureType(MTL::TextureType3D);
          m_texture_descriptor->setWidth(m_w);
          m_texture_descriptor->setHeight(m_h);
          m_texture_descriptor->setDepth(m_d);
          m_texture_descriptor->setArrayLength(1);
          m_texture_descriptor->setMipmapLevelCount((m_mtl_max_mips > 0) ? m_mtl_max_mips : 1);
          m_texture_descriptor->setUsage(
            MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead |
            MTL::TextureUsageShaderWrite |
            MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimize usage flags. */
          m_texture_descriptor->setStorageMode(MTL::StorageModePrivate);
          m_texture_descriptor->setSampleCount(1);
          m_texture_descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
          m_texture_descriptor->setHazardTrackingMode(MTL::HazardTrackingModeDefault);
        } break;

        /* CUBE TEXTURES */
        case GPU_TEXTURE_CUBE:
        case GPU_TEXTURE_CUBE_ARRAY: {
          /* NOTE: For a cube-map 'Texture::m_d' refers to total number of faces,
           * not just array slices. */
          KLI_assert(m_w > 0 && m_h > 0);
          m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
          m_texture_descriptor->setPixelFormat(mtl_format);
          m_texture_descriptor->setTextureType(
            (m_type == GPU_TEXTURE_CUBE_ARRAY) ? MTL::TextureTypeCubeArray : MTL::TextureTypeCube);
          m_texture_descriptor->setWidth(m_w);
          m_texture_descriptor->setHeight(m_h);
          m_texture_descriptor->setDepth(1);
          m_texture_descriptor->setArrayLength((m_type == GPU_TEXTURE_CUBE_ARRAY) ? m_d / 6 : 1);
          m_texture_descriptor->setMipmapLevelCount((m_mtl_max_mips > 0) ? m_mtl_max_mips : 1);
          m_texture_descriptor->setUsage(
            MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead |
            MTL::TextureUsageShaderWrite |
            MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimize usage flags. */
          m_texture_descriptor->setStorageMode(MTL::StorageModePrivate);
          m_texture_descriptor->setSampleCount(1);
          m_texture_descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
          m_texture_descriptor->setHazardTrackingMode(MTL::HazardTrackingModeDefault);
        } break;

        /* GPU_TEXTURE_BUFFER */
        case GPU_TEXTURE_BUFFER: {
          m_texture_descriptor = MTL::TextureDescriptor::alloc()->init();
          m_texture_descriptor->setPixelFormat(mtl_format);
          m_texture_descriptor->setTextureType(MTL::TextureTypeTextureBuffer);
          m_texture_descriptor->setWidth(m_w);
          m_texture_descriptor->setHeight(1);
          m_texture_descriptor->setDepth(1);
          m_texture_descriptor->setArrayLength(1);
          m_texture_descriptor->setMipmapLevelCount((m_mtl_max_mips > 0) ? m_mtl_max_mips : 1);
          m_texture_descriptor->setUsage(
            MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite |
            MTL::TextureUsagePixelFormatView); /* TODO(Metal): Optimize usage flags. */
          m_texture_descriptor->setStorageMode(MTL::StorageModePrivate);
          m_texture_descriptor->setSampleCount(1);
          m_texture_descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
          m_texture_descriptor->setHazardTrackingMode(MTL::HazardTrackingModeDefault);
        } break;

        default: {
          MTL_LOG_ERROR("[METAL] Error: Cannot create texture with unknown type: %d\n", m_type);
          return;
        } break;
      }

      /* Determine Resource Mode. */
      m_resource_mode = MTL_TEXTURE_MODE_DEFAULT;

      /* Standard texture allocation. */
      m_texture = ctx->device->newTexture(m_texture_descriptor);

      m_texture_descriptor->release();
      m_texture_descriptor = nullptr;
      m_texture->setLabel(NS_STRING_(this->get_name()));
      KLI_assert(m_texture);
      m_is_baked = true;
      m_is_dirty = false;
    }

    /* Re-apply previous contents. */
    if (copy_previous_contents) {
      MTL::Texture *previous_texture;
      /* TODO(Metal): May need to copy previous contents of texture into new texture. */
      /*[previous_texture release]; */
      UNUSED_VARS(previous_texture);
    }
  }

  void gpu::MTLTexture::reset()
  {

    MTL_LOG_INFO("Texture %s reset. Size %d, %d, %d\n", this->get_name(), m_w, m_h, m_d);
    /* Delete associated METAL resources. */
    if (m_texture != nil) {
      m_texture->release();
      m_texture = nil;
      m_is_baked = false;
      m_is_dirty = true;
    }

    if (m_mip_swizzle_view != nil) {
      m_mip_swizzle_view->release();
      m_mip_swizzle_view = nil;
    }

    if (m_texture_buffer != nil) {
      m_texture_buffer->release();
    }

    /* Blit framebuffer. */
    if (m_blit_fb) {
      GPU_framebuffer_free(m_blit_fb);
      m_blit_fb = nullptr;
    }

    KLI_assert(m_texture == nil);
    KLI_assert(m_mip_swizzle_view == nil);
  }

  /** \} */

}  // namespace kraken::gpu
