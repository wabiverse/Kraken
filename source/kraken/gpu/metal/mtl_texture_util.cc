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
#include "GPU_platform.h"
#include "GPU_state.h"

#include "mtl_backend.hh"
#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_texture.hh"

/* Utility file for secondary functionality which supports mtl_texture.mm. */

extern char datatoc_compute_texture_update_msl[];
extern char datatoc_compute_texture_read_msl[];

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** \name Texture Utility Functions
   * \{ */

  MTL::PixelFormat gpu_texture_format_to_metal(eGPUTextureFormat tex_format)
  {

    switch (tex_format) {
      /* Formats texture & renderbuffer. */
      case GPU_RGBA8UI:
        return MTL::PixelFormatRGBA8Uint;
      case GPU_RGBA8I:
        return MTL::PixelFormatRGBA8Sint;
      case GPU_RGBA8:
        return MTL::PixelFormatRGBA8Unorm;
      case GPU_RGBA32UI:
        return MTL::PixelFormatRGBA32Uint;
      case GPU_RGBA32I:
        return MTL::PixelFormatRGBA32Sint;
      case GPU_RGBA32F:
        return MTL::PixelFormatRGBA32Float;
      case GPU_RGBA16UI:
        return MTL::PixelFormatRGBA16Uint;
      case GPU_RGBA16I:
        return MTL::PixelFormatRGBA16Sint;
      case GPU_RGBA16F:
        return MTL::PixelFormatRGBA16Float;
      case GPU_RGBA16:
        return MTL::PixelFormatRGBA16Unorm;
      case GPU_RG8UI:
        return MTL::PixelFormatRG8Uint;
      case GPU_RG8I:
        return MTL::PixelFormatRG8Sint;
      case GPU_RG8:
        return MTL::PixelFormatRG8Unorm;
      case GPU_RG32UI:
        return MTL::PixelFormatRG32Uint;
      case GPU_RG32I:
        return MTL::PixelFormatRG32Sint;
      case GPU_RG32F:
        return MTL::PixelFormatRG32Float;
      case GPU_RG16UI:
        return MTL::PixelFormatRG16Uint;
      case GPU_RG16I:
        return MTL::PixelFormatRG16Sint;
      case GPU_RG16F:
        return MTL::PixelFormatRG16Float;
      case GPU_RG16:
        return MTL::PixelFormatRG16Float;
      case GPU_R8UI:
        return MTL::PixelFormatR8Uint;
      case GPU_R8I:
        return MTL::PixelFormatR8Sint;
      case GPU_R8:
        return MTL::PixelFormatR8Unorm;
      case GPU_R32UI:
        return MTL::PixelFormatR32Uint;
      case GPU_R32I:
        return MTL::PixelFormatR32Sint;
      case GPU_R32F:
        return MTL::PixelFormatR32Float;
      case GPU_R16UI:
        return MTL::PixelFormatR16Uint;
      case GPU_R16I:
        return MTL::PixelFormatR16Sint;
      case GPU_R16F:
        return MTL::PixelFormatR16Float;
      case GPU_R16:
        return MTL::PixelFormatR16Snorm;

      /* Special formats texture & renderbuffer. */
      case GPU_R11F_G11F_B10F:
        return MTL::PixelFormatRG11B10Float;
      case GPU_DEPTH32F_STENCIL8:
        return MTL::PixelFormatDepth32Float_Stencil8;
      case GPU_DEPTH24_STENCIL8: {
        KLI_assert(false && "GPU_DEPTH24_STENCIL8 not supported by Apple Silicon.");
        return MTL::PixelFormatDepth24Unorm_Stencil8;
      }
      case GPU_SRGB8_A8:
        return MTL::PixelFormatRGBA8Unorm_sRGB;
      case GPU_RGB16F:
        return MTL::PixelFormatRGBA16Float;

      /* Depth Formats. */
      case GPU_DEPTH_COMPONENT32F:
      case GPU_DEPTH_COMPONENT24:
        return MTL::PixelFormatDepth32Float;
      case GPU_DEPTH_COMPONENT16:
        return MTL::PixelFormatDepth16Unorm;

      default:
        KLI_assert(!"Unrecognized GPU pixel format!\n");
        return MTL::PixelFormatRGBA8Unorm;
    }
  }

  int get_mtl_format_bytesize(MTL::PixelFormat tex_format)
  {

    switch (tex_format) {
      case MTL::PixelFormatRGBA8Uint:
      case MTL::PixelFormatRGBA8Sint:
      case MTL::PixelFormatRGBA8Unorm:
        return 4;
      case MTL::PixelFormatRGBA32Uint:
      case MTL::PixelFormatRGBA32Sint:
      case MTL::PixelFormatRGBA32Float:
        return 16;
      case MTL::PixelFormatRGBA16Uint:
      case MTL::PixelFormatRGBA16Sint:
      case MTL::PixelFormatRGBA16Float:
      case MTL::PixelFormatRGBA16Unorm:
        return 8;
      case MTL::PixelFormatRG8Uint:
      case MTL::PixelFormatRG8Sint:
      case MTL::PixelFormatRG8Unorm:
        return 2;
      case MTL::PixelFormatRG32Uint:
      case MTL::PixelFormatRG32Sint:
      case MTL::PixelFormatRG32Float:
        return 8;
      case MTL::PixelFormatRG16Uint:
      case MTL::PixelFormatRG16Sint:
      case MTL::PixelFormatRG16Float:
        return 4;
      case MTL::PixelFormatR8Uint:
      case MTL::PixelFormatR8Sint:
      case MTL::PixelFormatR8Unorm:
        return 1;
      case MTL::PixelFormatR32Uint:
      case MTL::PixelFormatR32Sint:
      case MTL::PixelFormatR32Float:
        return 4;
      case MTL::PixelFormatR16Uint:
      case MTL::PixelFormatR16Sint:
      case MTL::PixelFormatR16Float:
      case MTL::PixelFormatR16Snorm:
        return 2;
      case MTL::PixelFormatRG11B10Float:
        return 4;
      case MTL::PixelFormatDepth32Float_Stencil8:
        return 8;
      case MTL::PixelFormatRGBA8Unorm_sRGB:
      case MTL::PixelFormatDepth32Float:
      case MTL::PixelFormatDepth24Unorm_Stencil8:
        return 4;
      case MTL::PixelFormatDepth16Unorm:
        return 2;

      default:
        KLI_assert(!"Unrecognized GPU pixel format!\n");
        return 1;
    }
  }

  int get_mtl_format_num_components(MTL::PixelFormat tex_format)
  {

    switch (tex_format) {
      case MTL::PixelFormatRGBA8Uint:
      case MTL::PixelFormatRGBA8Sint:
      case MTL::PixelFormatRGBA8Unorm:
      case MTL::PixelFormatRGBA32Uint:
      case MTL::PixelFormatRGBA32Sint:
      case MTL::PixelFormatRGBA32Float:
      case MTL::PixelFormatRGBA16Uint:
      case MTL::PixelFormatRGBA16Sint:
      case MTL::PixelFormatRGBA16Float:
      case MTL::PixelFormatRGBA16Unorm:
      case MTL::PixelFormatRGBA8Unorm_sRGB:
        return 4;

      case MTL::PixelFormatRG11B10Float:
        return 3;

      case MTL::PixelFormatRG8Uint:
      case MTL::PixelFormatRG8Sint:
      case MTL::PixelFormatRG8Unorm:
      case MTL::PixelFormatRG32Uint:
      case MTL::PixelFormatRG32Sint:
      case MTL::PixelFormatRG32Float:
      case MTL::PixelFormatRG16Uint:
      case MTL::PixelFormatRG16Sint:
      case MTL::PixelFormatRG16Float:
      case MTL::PixelFormatDepth32Float_Stencil8:
        return 2;

      case MTL::PixelFormatR8Uint:
      case MTL::PixelFormatR8Sint:
      case MTL::PixelFormatR8Unorm:
      case MTL::PixelFormatR32Uint:
      case MTL::PixelFormatR32Sint:
      case MTL::PixelFormatR32Float:
      case MTL::PixelFormatR16Uint:
      case MTL::PixelFormatR16Sint:
      case MTL::PixelFormatR16Float:
      case MTL::PixelFormatR16Snorm:
      case MTL::PixelFormatDepth32Float:
      case MTL::PixelFormatDepth16Unorm:
      case MTL::PixelFormatDepth24Unorm_Stencil8:
        /* Treating this format as single-channel for direct data copies -- Stencil component is
         * not addressable. */
        return 1;

      default:
        KLI_assert(!"Unrecognized GPU pixel format!\n");
        return 1;
    }
  }

  bool mtl_format_supports_blending(MTL::PixelFormat format)
  {
    /* Add formats as needed -- Verify platforms. */
    const MTLCapabilities &capabilities = MTLBackend::get_capabilities();

    if (capabilities.supports_family_mac1 || capabilities.supports_family_mac_catalyst1) {

      switch (format) {
        case MTL::PixelFormatA8Unorm:
        case MTL::PixelFormatR8Uint:
        case MTL::PixelFormatR8Sint:
        case MTL::PixelFormatR16Uint:
        case MTL::PixelFormatR16Sint:
        case MTL::PixelFormatRG32Uint:
        case MTL::PixelFormatRG32Sint:
        case MTL::PixelFormatRGBA8Uint:
        case MTL::PixelFormatRGBA8Sint:
        case MTL::PixelFormatRGBA32Uint:
        case MTL::PixelFormatRGBA32Sint:
        case MTL::PixelFormatDepth16Unorm:
        case MTL::PixelFormatDepth32Float:
        case MTL::PixelFormatInvalid:
        case MTL::PixelFormatBGR10A2Unorm:
        case MTL::PixelFormatRGB10A2Uint:
          return false;
        default:
          return true;
      }
    } else {
      switch (format) {
        case MTL::PixelFormatA8Unorm:
        case MTL::PixelFormatR8Uint:
        case MTL::PixelFormatR8Sint:
        case MTL::PixelFormatR16Uint:
        case MTL::PixelFormatR16Sint:
        case MTL::PixelFormatRG32Uint:
        case MTL::PixelFormatRG32Sint:
        case MTL::PixelFormatRGBA8Uint:
        case MTL::PixelFormatRGBA8Sint:
        case MTL::PixelFormatRGBA32Uint:
        case MTL::PixelFormatRGBA32Sint:
        case MTL::PixelFormatRGBA32Float:
        case MTL::PixelFormatDepth16Unorm:
        case MTL::PixelFormatDepth32Float:
        case MTL::PixelFormatInvalid:
        case MTL::PixelFormatBGR10A2Unorm:
        case MTL::PixelFormatRGB10A2Uint:
          return false;
        default:
          return true;
      }
    }
  }

  /** \} */

  /* -------------------------------------------------------------------- */
  /** \name Texture data upload routines
   * \{ */

  MTL::ComputePipelineState *gpu::MTLTexture::mtl_texture_update_impl(
    TextureUpdateRoutineSpecialisation specialization_params,
    kraken::Map<TextureUpdateRoutineSpecialisation, MTL::ComputePipelineState *>
      &specialization_cache,
    eGPUTextureType texture_type)
  {
    /* Check whether the Kernel exists. */
    MTL::ComputePipelineState **result = specialization_cache.lookup_ptr(specialization_params);
    if (result != nullptr) {
      return *result;
    }

    MTL::ComputePipelineState *return_pso = nil;
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* Fetch active context. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /** SOURCE. **/
    NS::String *tex_update_kernel_src = NS_STRING_(datatoc_compute_texture_update_msl);

    /* Prepare options and specializations. */
    MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init()->autorelease();
    options->setLanguageVersion(MTL::LanguageVersion2_2);
    options->setPreprocessorMacros(
      NS::Dictionary::dictionary(NS_STRING_("INPUT_DATA_TYPE"),
                                 NS_STRING_(specialization_params.input_data_type.c_str())));
    options->setPreprocessorMacros(
      NS::Dictionary::dictionary(NS_STRING_("OUTPUT_DATA_TYPE"),
                                 NS_STRING_(specialization_params.output_data_type.c_str())));
    options->setPreprocessorMacros(NS::Dictionary::dictionary(
      NS_STRING_("COMPONENT_COUNT_INPUT"),
      NS::Number::number((int)(specialization_params.component_count_input))));
    options->setPreprocessorMacros(NS::Dictionary::dictionary(
      NS_STRING_("COMPONENT_COUNT_OUTPUT"),
      NS::Number::number((int)(specialization_params.component_count_output))));
    options->setPreprocessorMacros(
      NS::Dictionary::dictionary(NS_STRING_("TEX_TYPE"), NS::Number::number((int)(texture_type))));

    /* Prepare shader library for conversion routine. */
    NS::Error *error = nullptr;
    MTL::Library *temp_lib =
      ctx->device->newLibrary(tex_update_kernel_src, options, &error)->autorelease();
    if (error) {
      MTL_LOG_ERROR("Compile Error - Metal Shader Library error %s ",
                    error->localizedFailureReason()->utf8String());
      KLI_assert(false);
      return nullptr;
    }

    /* Fetch compute function. */
    KLI_assert(temp_lib != nil);
    MTL::Function *temp_compute_function =
      temp_lib->newFunction(NS_STRING_("compute_texture_update"))->autorelease();
    KLI_assert(temp_compute_function);

    /* Otherwise, bake new Kernel. */
    MTL::ComputePipelineState *compute_pso = ctx->device->newComputePipelineState(
      temp_compute_function,
      &error);
    if (error || compute_pso == nil) {
      MTL_LOG_ERROR("Failed to prepare texture_update MTLComputePipelineState %s",
                    error->localizedFailureReason()->utf8String());
      KLI_assert(false);
    }

    /* Store PSO. */
    compute_pso->retain();
    specialization_cache.add_new(specialization_params, compute_pso);
    return_pso = compute_pso;

    pool->drain();
    pool = nil;

    KLI_assert(return_pso != nil);
    return return_pso;
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_update_1d_get_kernel(
    TextureUpdateRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_update_impl(specialization,
                                   mtl_context->get_texture_utils().texture_1d_update_compute_psos,
                                   GPU_TEXTURE_1D);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_update_1d_array_get_kernel(
    TextureUpdateRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_update_impl(
      specialization,
      mtl_context->get_texture_utils().texture_1d_array_update_compute_psos,
      GPU_TEXTURE_1D_ARRAY);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_update_2d_get_kernel(
    TextureUpdateRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_update_impl(specialization,
                                   mtl_context->get_texture_utils().texture_2d_update_compute_psos,
                                   GPU_TEXTURE_2D);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_update_2d_array_get_kernel(
    TextureUpdateRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_update_impl(
      specialization,
      mtl_context->get_texture_utils().texture_2d_array_update_compute_psos,
      GPU_TEXTURE_2D_ARRAY);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_update_3d_get_kernel(
    TextureUpdateRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_update_impl(specialization,
                                   mtl_context->get_texture_utils().texture_3d_update_compute_psos,
                                   GPU_TEXTURE_3D);
  }

  /* TODO(Metal): Data upload routine kernel for texture cube and texture cube array.
   * Currently does not appear to be hit. */

  GPUShader *gpu::MTLTexture::depth_2d_update_sh_get(
    DepthTextureUpdateRoutineSpecialisation specialization)
  {

    /* Check whether the Kernel exists. */
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);

    GPUShader **result = mtl_context->get_texture_utils().depth_2d_update_shaders.lookup_ptr(
      specialization);
    if (result != nullptr) {
      return *result;
    }

    const char *depth_2d_info_variant = nullptr;
    switch (specialization.data_mode) {
      case MTL_DEPTH_UPDATE_MODE_FLOAT:
        depth_2d_info_variant = "depth_2d_update_float";
        break;
      case MTL_DEPTH_UPDATE_MODE_INT24:
        depth_2d_info_variant = "depth_2d_update_int24";
        break;
      case MTL_DEPTH_UPDATE_MODE_INT32:
        depth_2d_info_variant = "depth_2d_update_int32";
        break;
      default:
        KLI_assert(false && "Invalid format mode\n");
        return nullptr;
    }

    GPUShader *shader = GPU_shader_create_from_info_name(depth_2d_info_variant);
    mtl_context->get_texture_utils().depth_2d_update_shaders.add_new(specialization, shader);
    return shader;
  }

  GPUShader *gpu::MTLTexture::fullscreen_blit_sh_get()
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    if (mtl_context->get_texture_utils().fullscreen_blit_shader == nullptr) {
      GPUShader *shader = GPU_shader_create_from_info_name("fullscreen_blit");

      mtl_context->get_texture_utils().fullscreen_blit_shader = shader;
    }
    return mtl_context->get_texture_utils().fullscreen_blit_shader;
  }

  /* Special routine for updating 2D depth textures using the rendering pipeline. */
  void gpu::MTLTexture::update_sub_depth_2d(int mip,
                                            int offset[3],
                                            int extent[3],
                                            eGPUDataFormat type,
                                            const void *data)
  {
    /* Verify we are in a valid configuration. */
    KLI_assert(ELEM(m_format,
                    GPU_DEPTH_COMPONENT24,
                    GPU_DEPTH_COMPONENT32F,
                    GPU_DEPTH_COMPONENT16,
                    GPU_DEPTH24_STENCIL8,
                    GPU_DEPTH32F_STENCIL8));
    KLI_assert(validate_data_format_mtl(m_format, type));
    KLI_assert(ELEM(type, GPU_DATA_FLOAT, GPU_DATA_UINT_24_8, GPU_DATA_UINT));

    /* Determine whether we are in GPU_DATA_UINT_24_8 or GPU_DATA_FLOAT mode. */
    bool is_float = (type == GPU_DATA_FLOAT);
    eGPUTextureFormat format = (is_float) ? GPU_R32F : GPU_R32I;

    /* Shader key - Add parameters here for different configurations. */
    DepthTextureUpdateRoutineSpecialisation specialization;
    switch (type) {
      case GPU_DATA_FLOAT:
        specialization.data_mode = MTL_DEPTH_UPDATE_MODE_FLOAT;
        break;

      case GPU_DATA_UINT_24_8:
        specialization.data_mode = MTL_DEPTH_UPDATE_MODE_INT24;
        break;

      case GPU_DATA_UINT:
        specialization.data_mode = MTL_DEPTH_UPDATE_MODE_INT32;
        break;

      default:
        KLI_assert(false && "Unsupported eGPUDataFormat being passed to depth texture update\n");
        return;
    }

    /* Push contents into an r32_tex and render contents to depth using a shader. */
    GPUTexture *r32_tex_tmp =
      GPU_texture_create_2d("depth_intermediate_copy_tex", m_w, m_h, 1, format, nullptr);
    GPU_texture_filter_mode(r32_tex_tmp, false);
    GPU_texture_wrap_mode(r32_tex_tmp, false, true);
    gpu::MTLTexture *mtl_tex = static_cast<gpu::MTLTexture *>(unwrap(r32_tex_tmp));
    mtl_tex->update_sub(mip, offset, extent, type, data);

    GPUFrameBuffer *restore_fb = GPU_framebuffer_active_get();
    GPUFrameBuffer *depth_fb_temp = GPU_framebuffer_create("depth_intermediate_copy_fb");
    GPU_framebuffer_texture_attach(depth_fb_temp, wrap(static_cast<Texture *>(this)), 0, mip);
    GPU_framebuffer_bind(depth_fb_temp);
    if (extent[0] == m_w && extent[1] == m_h) {
      /* Skip load if the whole texture is being updated. */
      GPU_framebuffer_clear_depth(depth_fb_temp, 0.0);
      GPU_framebuffer_clear_stencil(depth_fb_temp, 0);
    }

    GPUShader *depth_2d_update_sh = depth_2d_update_sh_get(specialization);
    KLI_assert(depth_2d_update_sh != nullptr);
    GPUBatch *quad = GPU_batch_preset_quad();
    GPU_batch_set_shader(quad, depth_2d_update_sh);

    GPU_batch_texture_bind(quad, "source_data", r32_tex_tmp);
    GPU_batch_uniform_1i(quad, "mip", mip);
    GPU_batch_uniform_2f(quad, "extent", (float)extent[0], (float)extent[1]);
    GPU_batch_uniform_2f(quad, "offset", (float)offset[0], (float)offset[1]);
    GPU_batch_uniform_2f(quad, "size", (float)m_w, (float)m_h);

    bool depth_write_prev = GPU_depth_mask_get();
    uint stencil_mask_prev = GPU_stencil_mask_get();
    eGPUDepthTest depth_test_prev = GPU_depth_test_get();
    eGPUStencilTest stencil_test_prev = GPU_stencil_test_get();
    GPU_scissor_test(true);
    GPU_scissor(offset[0], offset[1], extent[0], extent[1]);

    GPU_stencil_write_mask_set(0xFF);
    GPU_stencil_reference_set(0);
    GPU_stencil_test(GPU_STENCIL_ALWAYS);
    GPU_depth_mask(true);
    GPU_depth_test(GPU_DEPTH_ALWAYS);

    GPU_batch_draw(quad);

    GPU_depth_mask(depth_write_prev);
    GPU_stencil_write_mask_set(stencil_mask_prev);
    GPU_stencil_test(stencil_test_prev);
    GPU_depth_test(depth_test_prev);

    if (restore_fb != nullptr) {
      GPU_framebuffer_bind(restore_fb);
    } else {
      GPU_framebuffer_restore();
    }
    GPU_framebuffer_free(depth_fb_temp);
    GPU_texture_free(r32_tex_tmp);
  }
  /** \} */

  /* -------------------------------------------------------------------- */
  /** \name Texture data read  routines
   * \{ */

  MTL::ComputePipelineState *gpu::MTLTexture::mtl_texture_read_impl(
    TextureReadRoutineSpecialisation specialization_params,
    kraken::Map<TextureReadRoutineSpecialisation, MTL::ComputePipelineState *>
      &specialization_cache,
    eGPUTextureType texture_type)
  {
    /* Check whether the Kernel exists. */
    MTL::ComputePipelineState **result = specialization_cache.lookup_ptr(specialization_params);
    if (result != nullptr) {
      return *result;
    }

    MTL::ComputePipelineState *return_pso = nil;
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

    /* Fetch active context. */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);

    /** SOURCE. **/
    NS::String *tex_update_kernel_src = NS_STRING_(datatoc_compute_texture_read_msl);

    /* Defensive Debug Checks. */
    int64_t depth_scale_factor = 1;
    if (specialization_params.depth_format_mode > 0) {
      KLI_assert(specialization_params.component_count_input == 1);
      KLI_assert(specialization_params.component_count_output == 1);
      switch (specialization_params.depth_format_mode) {
        case 1:
          /* FLOAT */
          depth_scale_factor = 1;
          break;
        case 2:
          /* D24 uint */
          depth_scale_factor = 0xFFFFFFu;
          break;
        case 4:
          /* D32 uint */
          depth_scale_factor = 0xFFFFFFFFu;
          break;
        default:
          KLI_assert_msg(0, "Unrecognized mode");
          break;
      }

      pool->drain();
      pool = nil;

      /* Prepare options and specializations. */
      MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init()->autorelease();
      options->setLanguageVersion(MTL::LanguageVersion2_2);
      options->setPreprocessorMacros(
        NS::Dictionary::dictionary(NS_STRING_("INPUT_DATA_TYPE"),
                                   NS_STRING_(specialization_params.input_data_type.c_str())));
      options->setPreprocessorMacros(
        NS::Dictionary::dictionary(NS_STRING_("OUTPUT_DATA_TYPE"),
                                   NS_STRING_(specialization_params.output_data_type.c_str())));
      options->setPreprocessorMacros(NS::Dictionary::dictionary(
        NS_STRING_("COMPONENT_COUNT_INPUT"),
        NS::Number::number((int)(specialization_params.component_count_input))));
      options->setPreprocessorMacros(NS::Dictionary::dictionary(
        NS_STRING_("COMPONENT_COUNT_OUTPUT"),
        NS::Number::number((int)(specialization_params.component_count_output))));
      options->setPreprocessorMacros(NS::Dictionary::dictionary(
        NS_STRING_("WRITE_COMPONENT_COUNT"),
        NS::Number::number((int)(min_ii(specialization_params.component_count_input,
                                        specialization_params.component_count_output)))));
      options->setPreprocessorMacros(NS::Dictionary::dictionary(
        NS_STRING_("IS_DEPTH_FORMAT"),
        NS::Number::number((int)((specialization_params.depth_format_mode > 0) ? 1 : 0))));
      options->setPreprocessorMacros(
        NS::Dictionary::dictionary(NS_STRING_("DEPTH_SCALE_FACTOR"),
                                   NS::Number::number((long long)(depth_scale_factor))));
      options->setPreprocessorMacros(
        NS::Dictionary::dictionary(NS_STRING_("TEX_TYPE"),
                                   NS::Number::number((int)(texture_type))));

      /* Prepare shader library for conversion routine. */
      NS::Error *error = nullptr;
      MTL::Library *temp_lib =
        ctx->device->newLibrary(tex_update_kernel_src, options, &error)->autorelease();
      if (error) {
        MTL_LOG_ERROR("Compile Error - Metal Shader Library error %s ",
                      error->localizedFailureReason()->utf8String());
        KLI_assert(false);
        return nil;
      }

      /* Fetch compute function. */
      KLI_assert(temp_lib != nil);
      MTL::Function *temp_compute_function =
        temp_lib->newFunction(NS_STRING_("compute_texture_read"))->autorelease();
      KLI_assert(temp_compute_function);

      /* Otherwise, bake new Kernel. */
      MTL::ComputePipelineState *compute_pso = ctx->device->newComputePipelineState(
        temp_compute_function,
        &error);
      if (error || compute_pso == nil) {
        MTL_LOG_ERROR("Failed to prepare texture_read MTLComputePipelineState %s",
                      error->localizedFailureReason()->utf8String());
        KLI_assert(false);
        return nil;
      }

      /* Store PSO. */
      compute_pso->retain();
      specialization_cache.add_new(specialization_params, compute_pso);
      return_pso = compute_pso;
    }

    KLI_assert(return_pso != nil);
    return return_pso;
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_read_2d_get_kernel(
    TextureReadRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_read_impl(specialization,
                                 mtl_context->get_texture_utils().texture_2d_read_compute_psos,
                                 GPU_TEXTURE_2D);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_read_2d_array_get_kernel(
    TextureReadRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_read_impl(
      specialization,
      mtl_context->get_texture_utils().texture_2d_array_read_compute_psos,
      GPU_TEXTURE_2D_ARRAY);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_read_1d_get_kernel(
    TextureReadRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_read_impl(specialization,
                                 mtl_context->get_texture_utils().texture_1d_read_compute_psos,
                                 GPU_TEXTURE_1D);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_read_1d_array_get_kernel(
    TextureReadRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_read_impl(
      specialization,
      mtl_context->get_texture_utils().texture_1d_array_read_compute_psos,
      GPU_TEXTURE_1D_ARRAY);
  }

  MTL::ComputePipelineState *gpu::MTLTexture::texture_read_3d_get_kernel(
    TextureReadRoutineSpecialisation specialization)
  {
    MTLContext *mtl_context = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(mtl_context != nullptr);
    return mtl_texture_read_impl(specialization,
                                 mtl_context->get_texture_utils().texture_3d_read_compute_psos,
                                 GPU_TEXTURE_3D);
  }

  /** \} */

}  // namespace kraken::gpu
