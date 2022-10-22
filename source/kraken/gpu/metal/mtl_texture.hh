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

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "KLI_assert.h"
#include "MEM_guardedalloc.h"
#include "gpu_texture_private.hh"

#include "KLI_map.hh"
#include "GPU_texture.h"
#include <mutex>
#include <thread>

struct GPUFrameBuffer;

/* Texture Update system structs. */
struct TextureUpdateRoutineSpecialisation
{

  /* The METAL type of data in input array, e.g. half, float, short, int */
  std::string input_data_type;

  /* The type of the texture data texture2d<T,..>, e.g. T=float, half, int etc. */
  std::string output_data_type;

  /* Number of image channels provided in input texture data array (min=1, max=4). */
  int component_count_input;

  /* Number of channels the destination texture has (min=1, max=4). */
  int component_count_output;

  bool operator==(const TextureUpdateRoutineSpecialisation &other) const
  {
    return ((input_data_type == other.input_data_type) &&
            (output_data_type == other.output_data_type) &&
            (component_count_input == other.component_count_input) &&
            (component_count_output == other.component_count_output));
  }

  uint64_t hash() const
  {
    kraken::DefaultHash<std::string> string_hasher;
    return uint64_t(string_hasher(
      this->input_data_type + this->output_data_type +
      std::to_string((this->component_count_input << 8) + this->component_count_output)));
  }
};

/* Type of data is being written to the depth target:
 * 0 = floating point (0.0 - 1.0)
 * 1 = 24 bit integer (0 - 2^24)
 * 2 = 32 bit integer (0 - 2^32) */

typedef enum
{
  MTL_DEPTH_UPDATE_MODE_FLOAT = 0,
  MTL_DEPTH_UPDATE_MODE_INT24 = 1,
  MTL_DEPTH_UPDATE_MODE_INT32 = 2
} DepthTextureUpdateMode;

struct DepthTextureUpdateRoutineSpecialisation
{
  DepthTextureUpdateMode data_mode;

  bool operator==(const DepthTextureUpdateRoutineSpecialisation &other) const
  {
    return ((data_mode == other.data_mode));
  }

  uint64_t hash() const
  {
    return (uint64_t)(this->data_mode);
  }
};

/* Texture Read system structs. */
struct TextureReadRoutineSpecialisation
{
  std::string input_data_type;
  std::string output_data_type;
  int component_count_input;
  int component_count_output;

  /* Format for depth data.
   * 0 = Not a Depth format,
   * 1 = FLOAT DEPTH,
   * 2 = 24Bit Integer Depth,
   * 4 = 32bit Unsigned-Integer Depth. */
  int depth_format_mode;

  bool operator==(const TextureReadRoutineSpecialisation &other) const
  {
    return ((input_data_type == other.input_data_type) &&
            (output_data_type == other.output_data_type) &&
            (component_count_input == other.component_count_input) &&
            (component_count_output == other.component_count_output) &&
            (depth_format_mode == other.depth_format_mode));
  }

  uint64_t hash() const
  {
    kraken::DefaultHash<std::string> string_hasher;
    return uint64_t(string_hasher(this->input_data_type + this->output_data_type +
                                  std::to_string((this->component_count_input << 8) +
                                                 this->component_count_output +
                                                 (this->depth_format_mode << 28))));
  }
};

namespace kraken::gpu
{

  class MTLContext;
  class MTLVertBuf;

  struct MTLSamplerState;

  /* Metal Texture internal implementation. */
  static const int MTL_MAX_MIPMAP_COUNT = 15; /* Max: 16384x16384 */
  static const int MTL_MAX_FBO_ATTACHED = 16;

  /* Samplers */
  struct MTLSamplerState
  {
    eGPUSamplerState state;

    /* Mip min and mip max on sampler state always the same.
     * Level range now controlled with textureView to be consistent with GL baseLevel. */
    bool operator==(const MTLSamplerState &other) const
    {
      /* Add other parameters as needed. */
      return (this->state == other.state);
    }

    operator uint() const
    {
      return uint(state);
    }

    operator uint64_t() const
    {
      return uint64_t(state);
    }
  };

  const MTLSamplerState DEFAULT_SAMPLER_STATE = {GPU_SAMPLER_DEFAULT /*, 0, 9999*/};

  class MTLTexture : public Texture
  {
    friend class MTLContext;
    friend class MTLStateManager;
    friend class MTLFrameBuffer;

   private:

    /* Where the textures data comes from. */
    enum
    {
      MTL_TEXTURE_MODE_DEFAULT,     /* Texture is self-initialized (Standard). */
      MTL_TEXTURE_MODE_EXTERNAL,    /* Texture source from external MTL::Texture handle */
      MTL_TEXTURE_MODE_VBO,         /* Texture source initialized from VBO */
      MTL_TEXTURE_MODE_TEXTURE_VIEW /* Texture is a view into an existing texture. */
    } m_resource_mode;

    /* 'baking' refers to the generation of GPU-backed resources. This flag ensures GPU resources
     * are ready. Baking is generally deferred until as late as possible, to ensure all associated
     * resource state has been specified up-front. */
    bool m_is_baked;
    MTL::TextureDescriptor *m_texture_descriptor;
    MTL::Texture *m_texture;
    MTL::TextureUsage m_usage;

    /* Texture Storage. */
    MTL::Buffer *m_texture_buffer;
    uint m_aligned_w = 0;

    /* Blit Frame-buffer. */
    GPUFrameBuffer *m_blit_fb = nullptr;
    uint m_blit_fb_slice = 0;
    uint m_blit_fb_mip = 0;

    /* Texture view properties */
    /* In Metal, we use texture views to either limit mipmap ranges,
     * , apply a swizzle mask, or both.
     *
     * We apply the mip limit in the view rather than in the sampler, as
     * certain effects and functionality such as textureSize rely on the base level
     * being modified.
     *
     * Texture views can also point to external textures, rather than the owned
     * texture if MTL_TEXTURE_MODE_TEXTURE_VIEW is used.
     * If this mode is used, source_texture points to a GPUTexture from which
     * we pull their texture handle as a root.
     */
    const GPUTexture *m_source_texture = nullptr;

    enum TextureViewDirtyState
    {
      TEXTURE_VIEW_NOT_DIRTY = 0,
      TEXTURE_VIEW_SWIZZLE_DIRTY = (1 << 0),
      TEXTURE_VIEW_MIP_DIRTY = (1 << 1)
    };
    MTL::Texture *m_mip_swizzle_view = nil;
    char m_tex_swizzle_mask[4];
    MTL::TextureSwizzleChannels m_mtl_swizzle_mask;
    bool m_mip_range_dirty = false;

    int m_mip_texture_base_level = 0;
    int m_mip_texture_max_level = 1000;
    int m_mip_texture_base_layer = 0;
    int m_texture_view_dirty_flags = TEXTURE_VIEW_NOT_DIRTY;

    /* Max mip-maps for currently allocated texture resource. */
    int m_mtl_max_mips = 1;

    /* VBO. */
    MTLVertBuf *m_vert_buffer;
    MTL::Buffer *m_vert_buffer_mtl;

    /* Core parameters and sub-resources. */
    eGPUTextureUsage m_gpu_image_usage_flags;

    /* Whether the texture's properties or state has changed (e.g. mipmap range), and re-baking of
     * GPU resource is required. */
    bool m_is_dirty;
    bool m_is_bound;

   public:

    MTLTexture(const char *name);
    MTLTexture(const char *name,
               eGPUTextureFormat format,
               eGPUTextureType type,
               MTL::Texture *metal_texture);
    ~MTLTexture();

    void update_sub(int mip,
                    int offset[3],
                    int extent[3],
                    eGPUDataFormat type,
                    const void *data) override;

    void generate_mipmap() override;
    void copy_to(Texture *dst) override;
    void clear(eGPUDataFormat format, const void *data) override;
    void swizzle_set(const char swizzle_mask[4]) override;
    void stencil_texture_mode_set(bool use_stencil) override{
      /* TODO(Metal): implement. */
    };
    void mip_range_set(int min, int max) override;
    void *read(int mip, eGPUDataFormat type) override;

    /* Remove once no longer required -- will just return 0 for now in MTL path. */
    uint gl_bindcode_get() const override;

    bool texture_is_baked();
    const char *get_name()
    {
      return m_name;
    }

    MTL::Buffer *get_vertex_buffer() const
    {
      if (m_resource_mode == MTL_TEXTURE_MODE_VBO) {
        return m_vert_buffer_mtl;
      }
      return nil;
    }

   protected:

    bool init_internal() override;
    bool init_internal(GPUVertBuf *vbo) override;
    bool init_internal(const GPUTexture *src,
                       int mip_offset,
                       int layer_offset) override; /* Texture View */

   private:

    /* Common Constructor, default initialization. */
    void mtl_texture_init();

    /* Post-construction and member initialization, prior to baking.
     * Called during init_internal */
    void prepare_internal();

    /* Generate Metal GPU resources and upload data if needed */
    void ensure_baked();

    /* Delete associated Metal GPU resources. */
    void reset();
    void ensure_mipmaps(int miplvl);

    /* Flags a given mip level as being used. */
    void add_subresource(uint level);

    void read_internal(int mip,
                       int x_off,
                       int y_off,
                       int z_off,
                       int width,
                       int height,
                       int depth,
                       eGPUDataFormat desired_output_format,
                       int num_output_components,
                       int debug_data_size,
                       void *r_data);
    void bake_mip_swizzle_view();

    MTL::Texture *get_metal_handle();
    MTL::Texture *get_metal_handle_base();
    MTLSamplerState get_sampler_state();
    void blit(MTL::BlitCommandEncoder *blit_encoder,
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
              uint depth);
    void blit(gpu::MTLTexture *dest,
              uint src_x_offset,
              uint src_y_offset,
              uint dst_x_offset,
              uint dst_y_offset,
              uint src_mip,
              uint dst_mip,
              uint dst_slice,
              int width,
              int height);
    GPUFrameBuffer *get_blit_framebuffer(uint dst_slice, uint dst_mip);

    /* Texture Update function Utilities. */
    /* Metal texture updating does not provide the same range of functionality for type conversion
     * and format compatibility as are available in OpenGL. To achieve the same level of
     * functionality, we need to instead use compute kernels to perform texture data conversions
     * where appropriate.
     * There are a number of different inputs which affect permutations and thus require different
     * shaders and PSOs, such as:
     *  - Texture format
     *  - Texture type (e.g. 2D, 3D, 2D Array, Depth etc;)
     *  - Source data format and component count (e.g. floating point)
     *
     * MECHANISM:
     *
     *  kraken::map<INPUT DEFINES STRUCT, compute PSO> update_2d_array_kernel_psos;
     * - Generate compute shader with configured kernel below with variable parameters depending
     *  on input/output format configurations. Do not need to keep source or descriptors around,
     *  just PSO, as same input defines will always generate the same code.
     *
     * - IF datatype IS an exact match e.g. :
     *    - Per-component size matches (e.g. GPU_DATA_UBYTE)
     *                                OR GPU_DATA_10_11_11_REV && GPU_R11G11B10 (equiv)
     *                                OR D24S8 and GPU_DATA_UINT_24_8
     *    We can use BLIT ENCODER.
     *
     * OTHERWISE TRIGGER COMPUTE:
     *  - Compute sizes will vary. Threads per grid WILL match 'extent'.
     *    Dimensions will vary depending on texture type.
     *  - Will use setBytes with 'TextureUpdateParams' struct to pass in useful member params.
     */
    struct TextureUpdateParams
    {
      int mip_index;
      int extent[3];          /* Width, Height, Slice on 2D Array tex. */
      int offset[3];          /* Width, Height, Slice on 2D Array tex. */
      uint unpack_row_length; /* Number of pixels between bytes in input data. */
    };

    MTL::ComputePipelineState *texture_update_1d_get_kernel(
      TextureUpdateRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_update_1d_array_get_kernel(
      TextureUpdateRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_update_2d_get_kernel(
      TextureUpdateRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_update_2d_array_get_kernel(
      TextureUpdateRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_update_3d_get_kernel(
      TextureUpdateRoutineSpecialisation specialization);

    MTL::ComputePipelineState *mtl_texture_update_impl(
      TextureUpdateRoutineSpecialisation specialization_params,
      kraken::Map<TextureUpdateRoutineSpecialisation, MTL::ComputePipelineState *>
        &specialization_cache,
      eGPUTextureType texture_type);

    /* Depth Update Utilities */
    /* Depth texture updates are not directly supported with Blit operations, similarly, we cannot
     * use a compute shader to write to depth, so we must instead render to a depth target.
     * These processes use vertex/fragment shaders to render texture data from an intermediate
     * source, in order to prime the depth buffer. */
    GPUShader *depth_2d_update_sh_get(DepthTextureUpdateRoutineSpecialisation specialization);

    void update_sub_depth_2d(int mip,
                             int offset[3],
                             int extent[3],
                             eGPUDataFormat type,
                             const void *data);

    /* Texture Read function utilities -- Follows a similar mechanism to the updating routines */
    struct TextureReadParams
    {
      int mip_index;
      int extent[3]; /* Width, Height, Slice on 2D Array tex. */
      int offset[3]; /* Width, Height, Slice on 2D Array tex. */
    };

    MTL::ComputePipelineState *texture_read_1d_get_kernel(
      TextureReadRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_read_1d_array_get_kernel(
      TextureReadRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_read_2d_get_kernel(
      TextureReadRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_read_2d_array_get_kernel(
      TextureReadRoutineSpecialisation specialization);
    MTL::ComputePipelineState *texture_read_3d_get_kernel(
      TextureReadRoutineSpecialisation specialization);

    MTL::ComputePipelineState *mtl_texture_read_impl(
      TextureReadRoutineSpecialisation specialization_params,
      kraken::Map<TextureReadRoutineSpecialisation, MTL::ComputePipelineState *>
        &specialization_cache,
      eGPUTextureType texture_type);

    /* fullscreen blit utilities. */
    GPUShader *fullscreen_blit_sh_get();

    MEM_CXX_CLASS_ALLOC_FUNCS("MTLTexture")
  };

  /* Utility */
  MTL::PixelFormat gpu_texture_format_to_metal(eGPUTextureFormat tex_format);
  int get_mtl_format_bytesize(MTL::PixelFormat tex_format);
  int get_mtl_format_num_components(MTL::PixelFormat tex_format);
  bool mtl_format_supports_blending(MTL::PixelFormat format);

  /* The type used to define the per-component data in the input buffer. */
  inline std::string tex_data_format_to_msl_type_str(eGPUDataFormat type)
  {
    switch (type) {
      case GPU_DATA_FLOAT:
        return "float";
      case GPU_DATA_HALF_FLOAT:
        return "half";
      case GPU_DATA_INT:
        return "int";
      case GPU_DATA_UINT:
        return "uint";
      case GPU_DATA_UBYTE:
        return "uchar";
      case GPU_DATA_UINT_24_8:
        return "uint"; /* Problematic type - but will match alignment. */
      case GPU_DATA_10_11_11_REV:
        return "float"; /* Problematic type - each component will be read as a float. */
      default:
        KLI_assert(false);
        break;
    }
    return "";
  }

  /* The type T which goes into texture2d<T, access>. */
  inline std::string tex_data_format_to_msl_texture_template_type(eGPUDataFormat type)
  {
    switch (type) {
      case GPU_DATA_FLOAT:
        return "float";
      case GPU_DATA_HALF_FLOAT:
        return "half";
      case GPU_DATA_INT:
        return "int";
      case GPU_DATA_UINT:
        return "uint";
      case GPU_DATA_UBYTE:
        return "ushort";
      case GPU_DATA_UINT_24_8:
        return "uint"; /* Problematic type. */
      case GPU_DATA_10_11_11_REV:
        return "float"; /* Problematic type. */
      default:
        KLI_assert(false);
        break;
    }
    return "";
  }

  /* Determine whether format is writable or not. Use mtl_format_get_writeable_view_format(..) for
   * these. */
  inline bool mtl_format_is_writable(MTL::PixelFormat format)
  {
    switch (format) {
      case MTL::PixelFormatRGBA8Unorm_sRGB:
      case MTL::PixelFormatBGRA8Unorm_sRGB:
      case MTL::PixelFormatDepth16Unorm:
      case MTL::PixelFormatDepth32Float:
      case MTL::PixelFormatDepth32Float_Stencil8:
      case MTL::PixelFormatBGR10A2Unorm:
      case MTL::PixelFormatDepth24Unorm_Stencil8:
        return false;
      default:
        return true;
    }
    return true;
  }

  /* For the cases where a texture format is unwritable, we can create a texture view of a similar
   * format */
  inline MTL::PixelFormat mtl_format_get_writeable_view_format(MTL::PixelFormat format)
  {
    switch (format) {
      case MTL::PixelFormatRGBA8Unorm_sRGB:
        return MTL::PixelFormatRGBA8Unorm;
      case MTL::PixelFormatBGRA8Unorm_sRGB:
        return MTL::PixelFormatBGRA8Unorm;
      case MTL::PixelFormatDepth16Unorm:
        return MTL::PixelFormatR16Unorm;
      case MTL::PixelFormatDepth32Float:
        return MTL::PixelFormatR32Float;
      case MTL::PixelFormatDepth32Float_Stencil8:
        /* return MTL::PixelFormatRG32Float; */
        /* No alternative mirror format. This should not be used for
         * manual data upload */
        return MTL::PixelFormatInvalid;
      case MTL::PixelFormatBGR10A2Unorm:
        /* return MTL::PixelFormatBGRA8Unorm; */
        /* No alternative mirror format. This should not be used for
         * manual data upload */
        return MTL::PixelFormatInvalid;
      case MTL::PixelFormatDepth24Unorm_Stencil8:
        /* No direct format, but we'll just mirror the bytes -- Uint
         * should ensure bytes are not re-normalized or manipulated */
        /* return MTL::PixelFormatR32Uint; */
        return MTL::PixelFormatInvalid;
      default:
        return format;
    }
    return format;
  }

  /* Returns the associated engine data type with a given texture:
   * Definitely not complete, edit according to the METAL specification. */
  inline eGPUDataFormat to_mtl_internal_data_format(eGPUTextureFormat tex_format)
  {
    switch (tex_format) {
      case GPU_RGBA8:
      case GPU_RGBA32F:
      case GPU_RGBA16F:
      case GPU_RGBA16:
      case GPU_RG8:
      case GPU_RG32F:
      case GPU_RG16F:
      case GPU_RG16:
      case GPU_R8:
      case GPU_R32F:
      case GPU_R16F:
      case GPU_R16:
      case GPU_RGB16F:
      case GPU_DEPTH_COMPONENT24:
      case GPU_DEPTH_COMPONENT16:
      case GPU_DEPTH_COMPONENT32F:
      case GPU_SRGB8_A8:
        return GPU_DATA_FLOAT;
      case GPU_DEPTH24_STENCIL8:
      case GPU_DEPTH32F_STENCIL8:
        return GPU_DATA_UINT_24_8;
      case GPU_RGBA8UI:
      case GPU_RGBA32UI:
      case GPU_RGBA16UI:
      case GPU_RG8UI:
      case GPU_RG32UI:
      case GPU_R8UI:
      case GPU_R16UI:
      case GPU_RG16UI:
      case GPU_R32UI:
        return GPU_DATA_UINT;
      case GPU_R8I:
      case GPU_RG8I:
      case GPU_R16I:
      case GPU_R32I:
      case GPU_RG16I:
      case GPU_RGBA8I:
      case GPU_RGBA32I:
      case GPU_RGBA16I:
      case GPU_RG32I:
        return GPU_DATA_INT;
      case GPU_R11F_G11F_B10F:
        return GPU_DATA_10_11_11_REV;
      default:
        KLI_assert(false && "Texture not yet handled");
        return GPU_DATA_FLOAT;
    }
  }

}  // namespace kraken::gpu
