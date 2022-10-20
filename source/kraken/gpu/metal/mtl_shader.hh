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

#include "GPU_batch.h"
#include "GPU_capabilities.h"
#include "GPU_shader.h"
#include "GPU_vertex_format.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <functional>
#include <unordered_map>

#include <mutex>
#include <thread>

#include "mtl_capabilities.hh"
#include "mtl_framebuffer.hh"
#include "mtl_pso_descriptor_state.hh"
#include "mtl_shader_interface.hh"
#include "mtl_shader_shared.h"
#include "mtl_state.hh"
#include "mtl_texture.hh"

#include "gpu_shader_create_info.hh"
#include "gpu_shader_private.hh"

namespace kraken::gpu
{

  class MTLShaderInterface;
  class MTLContext;

/* Debug control. */
#define MTL_SHADER_DEBUG_EXPORT_SOURCE 1
#define MTL_SHADER_TRANSLATION_DEBUG_OUTPUT 0

/* Separate print used only during development and debugging. */
#if MTL_SHADER_TRANSLATION_DEBUG_OUTPUT
#  define shader_debug_printf printf
#else
#  define shader_debug_printf(...) /* Null print. */
#endif

  /* Desired reflection data for a buffer binding. */
  struct MTLBufferArgumentData
  {
    uint32_t index;
    uint32_t size;
    uint32_t alignment;
    bool active;
  };

  /* Metal Render Pipeline State Instance. */
  struct MTLRenderPipelineStateInstance
  {
    /* Function instances with specialization.
     * Required for argument encoder construction. */
    MTL::Function *vert;
    MTL::Function *frag;

    /* PSO handle. */
    MTL::RenderPipelineState *pso;

    /** Derived information. */
    /* Unique index for PSO variant. */
    uint32_t shader_pso_index;
    /* Base bind index for binding uniform buffers, offset based on other
     * bound buffers such as vertex buffers, as the count can vary. */
    int base_uniform_buffer_index;
    /* buffer bind slot used for null attributes (-1 if not needed). */
    int null_attribute_buffer_index;
    /* buffer bind used for transform feedback output buffer. */
    int transform_feedback_buffer_index;

    /** Reflection Data.
     * Currently used to verify whether uniform buffers of incorrect sizes being bound, due to left
     * over bindings being used for slots that did not need updating for a particular draw. Metal
     * Back-end over-generates bindings due to detecting their presence, though in many cases, the
     * bindings in the source are not all used for a given shader.
     * This information can also be used to eliminate redundant/unused bindings. */
    bool reflection_data_available;
    kraken::Vector<MTLBufferArgumentData> buffer_bindings_reflection_data_vert;
    kraken::Vector<MTLBufferArgumentData> buffer_bindings_reflection_data_frag;
  };

  /* #MTLShaderBuilder source wrapper used during initial compilation. */
  struct MTLShaderBuilder
  {
    NS::String *msl_source_vert_ = NS_STRING_("");
    NS::String *msl_source_frag_ = NS_STRING_("");

    /* Generated GLSL source used during compilation. */
    std::string m_glsl_vertex_source = "";
    std::string m_glsl_fragment_source = "";

    /* Indicates whether source code has been provided via MSL directly. */
    bool m_source_from_msl = false;
  };

  /**
   * #MTLShader implements shader compilation, Pipeline State Object (PSO)
   * creation for rendering and uniform data binding.
   * Shaders can either be created from native MSL, or generated
   * from a GLSL source shader using #GPUShaderCreateInfo.
   *
   * Shader creation process:
   * - Create #MTLShader:
   *    - Convert GLSL to MSL source if required.
   * - set MSL source.
   * - set Vertex/Fragment function names.
   * - Create and populate #MTLShaderInterface.
   **/
  class MTLShader : public Shader
  {
    friend shader::ShaderCreateInfo;
    friend shader::StageInterfaceInfo;

   public:

    /* Cached SSBO vertex fetch attribute uniform locations. */
    int uni_ssbo_input_prim_type_loc = -1;
    int uni_ssbo_input_vert_count_loc = -1;
    int uni_ssbo_uses_indexed_rendering = -1;
    int uni_ssbo_uses_index_mode_u16 = -1;

   private:

    /* Context Handle. */
    MTLContext *m_context = nullptr;

    /** Transform Feedback. */
    /* Transform feedback mode. */
    eGPUShaderTFBType m_transform_feedback_type = GPU_SHADER_TFB_NONE;
    /* Transform feedback outputs written to TFB buffer. */
    kraken::Vector<std::string> m_tf_output_name_list;
    /* Whether transform feedback is currently active. */
    bool m_transform_feedback_active = false;
    /* Vertex buffer to write transform feedback data into. */
    GPUVertBuf *m_transform_feedback_vertbuf = nullptr;

    /** Shader source code. */
    MTLShaderBuilder *m_shd_builder = nullptr;
    NS::String *m_vertex_function_name = NS_STRING_("");
    NS::String *m_fragment_function_name = NS_STRING_("");

    /** Compiled shader resources. */
    MTL::Library *m_shader_library_vert = nil;
    MTL::Library *m_shader_library_frag = nil;
    bool m_valid = false;

    /** Render pipeline state and PSO caching. */
    /* Metal API Descriptor used for creation of unique PSOs based on rendering state. */
    MTL::RenderPipelineDescriptor *m_pso_descriptor = nil;
    /* Metal backend struct containing all high-level pipeline state parameters
     * which contribute to instantiation of a unique PSO. */
    MTLRenderPipelineStateDescriptor m_current_pipeline_state;
    /* Cache of compiled PipelineStateObjects. */
    kraken::Map<MTLRenderPipelineStateDescriptor, MTLRenderPipelineStateInstance *> m_pso_cache;

    /* True to enable multi-layered rendering support. */
    bool m_uses_mtl_array_index = false;

    /** SSBO Vertex fetch pragma options. */
    /* Indicates whether to pass in VertexBuffer's as regular buffer bindings
     * and perform vertex assembly manually, rather than using Stage-in.
     * This is used to give a vertex shader full access to all of the
     * vertex data.
     * This is primarily used for optimization techniques and
     * alternative solutions for Geometry-shaders which are unsupported
     * by Metal. */
    bool m_use_ssbo_vertex_fetch_mode = false;
    /* Output primitive type when rendering sing ssbo_vertex_fetch. */
    MTL::PrimitiveType m_ssbo_vertex_fetch_output_prim_type;

    /* Output vertices per original vertex shader instance.
     * This number will be multiplied by the number of input primitives
     * from the source draw call. */
    uint32_t m_ssbo_vertex_fetch_output_num_verts = 0;

    bool m_ssbo_vertex_attribute_bind_active = false;
    int m_ssbo_vertex_attribute_bind_mask = 0;
    bool m_ssbo_vbo_slot_used[MTL_SSBO_VERTEX_FETCH_MAX_VBOS];

    struct ShaderSSBOAttributeBinding
    {
      int attribute_index = -1;
      int uniform_stride;
      int uniform_offset;
      int uniform_fetchmode;
      int uniform_vbo_id;
      int uniform_attr_type;
    };
    ShaderSSBOAttributeBinding m_cached_ssbo_attribute_bindings[MTL_MAX_VERTEX_INPUT_ATTRIBUTES] =
      {};

    /* Metal Shader Uniform data store.
     * This blocks is used to store current shader push_constant
     * data before it is submitted to the GPU. This is currently
     * stored per shader instance, though depending on GPU module
     * functionality, this could potentially be a global data store.
     * This data is associated with the PushConstantBlock, which is
     * always at index zero in the UBO list. */
    void *m_push_constant_data = nullptr;
    bool m_push_constant_modified = false;

   public:

    MTLShader(MTLContext *ctx, const char *name);
    MTLShader(MTLContext *ctx,
              MTLShaderInterface *interface,
              const char *name,
              NS::String *input_vertex_source,
              NS::String *input_fragment_source,
              NS::String *vertex_function_name_,
              NS::String *fragment_function_name_);
    ~MTLShader();

    /* Assign GLSL source. */
    void vertex_shader_from_glsl(MutableSpan<const char *> sources) override;
    void geometry_shader_from_glsl(MutableSpan<const char *> sources) override;
    void fragment_shader_from_glsl(MutableSpan<const char *> sources) override;
    void compute_shader_from_glsl(MutableSpan<const char *> sources) override;

    /* Compile and build - Return true if successful. */
    bool finalize(const shader::ShaderCreateInfo *info = nullptr) override;

    /* Utility. */
    bool is_valid()
    {
      return m_valid;
    }
    MTLRenderPipelineStateDescriptor &get_current_pipeline_state()
    {
      return m_current_pipeline_state;
    }
    MTLShaderInterface *get_interface()
    {
      return static_cast<MTLShaderInterface *>(this->interface);
    }
    void *get_push_constant_data()
    {
      return m_push_constant_data;
    }

    /* Shader source generators from create-info.
     * These aren't all used by Metal, as certain parts of source code generation
     * for shader entry-points and resource mapping occur during `finalize`. */
    std::string resources_declare(const shader::ShaderCreateInfo &info) const override;
    std::string vertex_interface_declare(const shader::ShaderCreateInfo &info) const override;
    std::string fragment_interface_declare(const shader::ShaderCreateInfo &info) const override;
    std::string geometry_interface_declare(const shader::ShaderCreateInfo &info) const override;
    std::string geometry_layout_declare(const shader::ShaderCreateInfo &info) const override;
    std::string compute_layout_declare(const shader::ShaderCreateInfo &info) const override;

    void transform_feedback_names_set(Span<const char *> name_list,
                                      const eGPUShaderTFBType geom_type) override;
    bool transform_feedback_enable(GPUVertBuf *buf) override;
    void transform_feedback_disable() override;

    void bind() override;
    void unbind() override;

    void uniform_float(int location, int comp_len, int array_size, const float *data) override;
    void uniform_int(int location, int comp_len, int array_size, const int *data) override;
    bool get_push_constant_is_dirty();
    void push_constant_bindstate_mark_dirty(bool is_dirty);

    /* DEPRECATED: Kept only because of BGL API. (Returning -1 in METAL). */
    int program_handle_get() const override
    {
      return -1;
    }

    bool get_uses_ssbo_vertex_fetch()
    {
      return m_use_ssbo_vertex_fetch_mode;
    }
    MTL::PrimitiveType get_ssbo_vertex_fetch_output_prim_type()
    {
      return m_ssbo_vertex_fetch_output_prim_type;
    }
    uint32_t get_ssbo_vertex_fetch_output_num_verts()
    {
      return m_ssbo_vertex_fetch_output_num_verts;
    }
    static int ssbo_vertex_type_to_attr_type(MTL::VertexFormat attribute_type);
    void prepare_ssbo_vertex_fetch_metadata();

    /* SSBO Vertex Bindings Utility functions. */
    void ssbo_vertex_fetch_bind_attributes_begin();
    void ssbo_vertex_fetch_bind_attribute(const MTLSSBOAttribute &ssbo_attr);
    void ssbo_vertex_fetch_bind_attributes_end(MTL::RenderCommandEncoder *active_encoder);

    /* Metal shader properties and source mapping. */
    void set_vertex_function_name(NS::String *vetex_function_name);
    void set_fragment_function_name(NS::String *fragment_function_name_);
    void shader_source_from_msl(NS::String *input_vertex_source,
                                NS::String *input_fragment_source);
    void set_interface(MTLShaderInterface *interface);
    MTLRenderPipelineStateInstance *bake_current_pipeline_state(
      MTLContext *ctx,
      MTL::PrimitiveTopologyClass prim_type);

    /* Transform Feedback. */
    GPUVertBuf *get_transform_feedback_active_buffer();
    bool has_transform_feedback_varying(std::string str);

   private:

    /* Generate MSL shader from GLSL source. */
    bool generate_msl_from_glsl(const shader::ShaderCreateInfo *info);

    MEM_CXX_CLASS_ALLOC_FUNCS("MTLShader");
  };

  /* Vertex format conversion.
   * Determines whether it is possible to resize a vertex attribute type
   * during input assembly. A conversion is implied by the  difference
   * between the input vertex descriptor (from MTLBatch/MTLImmediate)
   * and the type specified in the shader source.
   *
   * e.g. vec3 to vec4 expansion, or vec4 to vec2 truncation.
   * NOTE: Vector expansion will replace empty elements with the values
   * (0,0,0,1).
   *
   * If implicit format resize is not possible, this function
   * returns false.
   *
   * Implicitly supported conversions in Metal are described here:
   * https://developer.apple.com/documentation/metal/MTL::Vertexattributedescriptor/1516081-format?language=objc
   */
  inline bool mtl_vertex_format_resize(MTL::VertexFormat mtl_format,
                                       uint32_t components,
                                       MTL::VertexFormat *r_convertedFormat)
  {
    MTL::VertexFormat out_vert_format = MTL::VertexFormatInvalid;
    switch (mtl_format) {
      /* Char. */
      case MTL::VertexFormatChar:
      case MTL::VertexFormatChar2:
      case MTL::VertexFormatChar3:
      case MTL::VertexFormatChar4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatChar;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatChar2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatChar3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatChar4;
            break;
        }
        break;

      /* Normalized Char. */
      case MTL::VertexFormatCharNormalized:
      case MTL::VertexFormatChar2Normalized:
      case MTL::VertexFormatChar3Normalized:
      case MTL::VertexFormatChar4Normalized:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatCharNormalized;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatChar2Normalized;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatChar3Normalized;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatChar4Normalized;
            break;
        }
        break;

      /* Unsigned Char. */
      case MTL::VertexFormatUChar:
      case MTL::VertexFormatUChar2:
      case MTL::VertexFormatUChar3:
      case MTL::VertexFormatUChar4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatUChar;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatUChar2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatUChar3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatUChar4;
            break;
        }
        break;

      /* Normalized Unsigned char */
      case MTL::VertexFormatUCharNormalized:
      case MTL::VertexFormatUChar2Normalized:
      case MTL::VertexFormatUChar3Normalized:
      case MTL::VertexFormatUChar4Normalized:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatUCharNormalized;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatUChar2Normalized;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatUChar3Normalized;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatUChar4Normalized;
            break;
        }
        break;

      /* Short. */
      case MTL::VertexFormatShort:
      case MTL::VertexFormatShort2:
      case MTL::VertexFormatShort3:
      case MTL::VertexFormatShort4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatShort;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatShort2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatShort3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatShort4;
            break;
        }
        break;

      /* Normalized Short. */
      case MTL::VertexFormatShortNormalized:
      case MTL::VertexFormatShort2Normalized:
      case MTL::VertexFormatShort3Normalized:
      case MTL::VertexFormatShort4Normalized:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatShortNormalized;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatShort2Normalized;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatShort3Normalized;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatShort4Normalized;
            break;
        }
        break;

      /* Unsigned Short. */
      case MTL::VertexFormatUShort:
      case MTL::VertexFormatUShort2:
      case MTL::VertexFormatUShort3:
      case MTL::VertexFormatUShort4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatUShort;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatUShort2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatUShort3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatUShort4;
            break;
        }
        break;

      /* Normalized Unsigned Short. */
      case MTL::VertexFormatUShortNormalized:
      case MTL::VertexFormatUShort2Normalized:
      case MTL::VertexFormatUShort3Normalized:
      case MTL::VertexFormatUShort4Normalized:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatUShortNormalized;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatUShort2Normalized;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatUShort3Normalized;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatUShort4Normalized;
            break;
        }
        break;

      /* Integer. */
      case MTL::VertexFormatInt:
      case MTL::VertexFormatInt2:
      case MTL::VertexFormatInt3:
      case MTL::VertexFormatInt4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatInt;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatInt2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatInt3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatInt4;
            break;
        }
        break;

      /* Unsigned Integer. */
      case MTL::VertexFormatUInt:
      case MTL::VertexFormatUInt2:
      case MTL::VertexFormatUInt3:
      case MTL::VertexFormatUInt4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatUInt;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatUInt2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatUInt3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatUInt4;
            break;
        }
        break;

      /* Half. */
      case MTL::VertexFormatHalf:
      case MTL::VertexFormatHalf2:
      case MTL::VertexFormatHalf3:
      case MTL::VertexFormatHalf4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatHalf;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatHalf2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatHalf3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatHalf4;
            break;
        }
        break;

      /* Float. */
      case MTL::VertexFormatFloat:
      case MTL::VertexFormatFloat2:
      case MTL::VertexFormatFloat3:
      case MTL::VertexFormatFloat4:
        switch (components) {
          case 1:
            out_vert_format = MTL::VertexFormatFloat;
            break;
          case 2:
            out_vert_format = MTL::VertexFormatFloat2;
            break;
          case 3:
            out_vert_format = MTL::VertexFormatFloat3;
            break;
          case 4:
            out_vert_format = MTL::VertexFormatFloat4;
            break;
        }
        break;

      /* Other formats */
      default:
        out_vert_format = mtl_format;
        break;
    }
    *r_convertedFormat = out_vert_format;
    return out_vert_format != MTL::VertexFormatInvalid;
  }

  /**
   * Returns whether the METAL API can internally convert between the input type of data in the
   * incoming vertex buffer and the format used by the vertex attribute inside the shader.
   *
   * - Returns TRUE if the type can be converted internally, along with returning the appropriate
   *   type to be passed into the #MTL::VertexAttributeDescriptorPSO.
   *
   * - Returns FALSE if the type cannot be converted internally e.g. casting Int4 to Float4.
   *
   * If implicit conversion is not possible, then we can fallback to performing manual attribute
   * conversion using the special attribute read function specializations in the shader.
   * These functions selectively convert between types based on the specified vertex
   * attribute `GPUVertFetchMode fetch_mode` e.g. `GPU_FETCH_INT`.
   */
  inline bool mtl_convert_vertex_format(MTL::VertexFormat shader_attrib_format,
                                        GPUVertCompType component_type,
                                        uint32_t component_length,
                                        GPUVertFetchMode fetch_mode,
                                        MTL::VertexFormat *r_convertedFormat)
  {
    bool normalized = (fetch_mode == GPU_FETCH_INT_TO_FLOAT_UNIT);
    MTL::VertexFormat out_vert_format = MTL::VertexFormatInvalid;

    switch (component_type) {

      case GPU_COMP_I8:
        switch (fetch_mode) {
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatChar ||
                shader_attrib_format == MTL::VertexFormatChar2 ||
                shader_attrib_format == MTL::VertexFormatChar3 ||
                shader_attrib_format == MTL::VertexFormatChar4) {

              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_type,
                                                          &out_vert_format);

              /* Ensure format resize successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else if (shader_attrib_format == MTL::VertexFormatInt4 && component_length == 4) {
              /* Allow type expansion - Shader expects MTL::VertexFormatInt4, we can supply a type
               * with fewer bytes if component count is the same. Sign must also match original
               * type
               *  -- which is not a problem in this case. */
              out_vert_format = MTL::VertexFormatChar4;
            } else if (shader_attrib_format == MTL::VertexFormatInt3 && component_length == 3) {
              /* Same as above case for matching length and signage (Len=3)*/
              out_vert_format = MTL::VertexFormatChar3;
            } else if (shader_attrib_format == MTL::VertexFormatInt2 && component_length == 2) {
              /* Same as above case for matching length and signage (Len=2)*/
              out_vert_format = MTL::VertexFormatChar2;
            } else if (shader_attrib_format == MTL::VertexFormatInt && component_length == 1) {
              /* Same as above case for matching length and signage (Len=1)*/
              out_vert_format = MTL::VertexFormatChar;
            } else if (shader_attrib_format == MTL::VertexFormatInt && component_length == 4) {
              /* Special case here, format has been specified as GPU_COMP_U8 with 4 components,
               * which is equivalent to an Int -- so data will be compatible with the shader
               * interface. */
              out_vert_format = MTL::VertexFormatInt;
            } else {
              KLI_assert_msg(false,
                             "Source vertex data format is either Char, Char2, Char3, Char4 but "
                             "format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;

          /* Source vertex data is integer type, but shader interface type is floating point.
           * If the input attribute is specified as normalized, we can convert. */
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            if (normalized) {
              switch (component_length) {
                case 1:
                  out_vert_format = MTL::VertexFormatCharNormalized;
                  break;
                case 2:
                  out_vert_format = MTL::VertexFormatChar2Normalized;
                  break;
                case 3:
                  out_vert_format = MTL::VertexFormatChar3Normalized;
                  break;
                case 4:
                  out_vert_format = MTL::VertexFormatChar4Normalized;
                  break;
                default:
                  KLI_assert_msg(false, "invalid vertex format");
                  out_vert_format = MTL::VertexFormatInvalid;
              }
            } else {
              /* Cannot convert. */
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
        }
        break;

      case GPU_COMP_U8:
        switch (fetch_mode) {
          /* Fetching INT: Check backing shader format matches source input. */
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatUChar ||
                shader_attrib_format == MTL::VertexFormatUChar2 ||
                shader_attrib_format == MTL::VertexFormatUChar3 ||
                shader_attrib_format == MTL::VertexFormatUChar4) {

              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Ensure format resize successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
              /* TODO(Metal): Add other format conversions if needed. Currently no attributes hit
               * this path. */
            } else if (shader_attrib_format == MTL::VertexFormatUInt4 && component_length == 4) {
              /* Allow type expansion - Shader expects MTL::VertexFormatUInt4, we can supply a type
               * with fewer bytes if component count is the same. */
              out_vert_format = MTL::VertexFormatUChar4;
            } else if (shader_attrib_format == MTL::VertexFormatUInt3 && component_length == 3) {
              /* Same as above case for matching length and signage (Len=3)*/
              out_vert_format = MTL::VertexFormatUChar3;
            } else if (shader_attrib_format == MTL::VertexFormatUInt2 && component_length == 2) {
              /* Same as above case for matching length and signage (Len=2)*/
              out_vert_format = MTL::VertexFormatUChar2;
            } else if (shader_attrib_format == MTL::VertexFormatUInt && component_length == 1) {
              /* Same as above case for matching length and signage (Len=1)*/
              out_vert_format = MTL::VertexFormatUChar;
            } else if (shader_attrib_format == MTL::VertexFormatInt && component_length == 4) {
              /* Special case here, format has been specified as GPU_COMP_U8 with 4 components,
               * which is equivalent to an Int-- so data will be compatible with shader interface.
               */
              out_vert_format = MTL::VertexFormatInt;
            } else if (shader_attrib_format == MTL::VertexFormatUInt && component_length == 4) {
              /* Special case here, format has been specified as GPU_COMP_U8 with 4 components,
               *which is equivalent to a UInt-- so data will be compatible with shader interface.
               */
              out_vert_format = MTL::VertexFormatUInt;
            } else {
              KLI_assert_msg(
                false,
                "Source vertex data format is either UChar, UChar2, UChar3, UChar4 but "
                "format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;

          /* Source vertex data is integral type, but shader interface type is floating point.
           * If the input attribute is specified as normalized, we can convert. */
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            if (normalized) {
              switch (component_length) {
                case 1:
                  out_vert_format = MTL::VertexFormatUCharNormalized;
                  break;
                case 2:
                  out_vert_format = MTL::VertexFormatUChar2Normalized;
                  break;
                case 3:
                  out_vert_format = MTL::VertexFormatUChar3Normalized;
                  break;
                case 4:
                  out_vert_format = MTL::VertexFormatUChar4Normalized;
                  break;
                default:
                  KLI_assert_msg(false, "invalid vertex format");
                  out_vert_format = MTL::VertexFormatInvalid;
              }
            } else {
              /* Cannot convert. */
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
        }
        break;

      case GPU_COMP_I16:
        switch (fetch_mode) {
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatShort ||
                shader_attrib_format == MTL::VertexFormatShort2 ||
                shader_attrib_format == MTL::VertexFormatShort3 ||
                shader_attrib_format == MTL::VertexFormatShort4) {
              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Ensure conversion successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else {
              KLI_assert_msg(
                false,
                "Source vertex data format is either Short, Short2, Short3, Short4 but "
                "format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;

          /* Source vertex data is integral type, but shader interface type is floating point.
           * If the input attribute is specified as normalized, we can convert. */
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            if (normalized) {
              switch (component_length) {
                case 1:
                  out_vert_format = MTL::VertexFormatShortNormalized;
                  break;
                case 2:
                  out_vert_format = MTL::VertexFormatShort2Normalized;
                  break;
                case 3:
                  out_vert_format = MTL::VertexFormatShort3Normalized;
                  break;
                case 4:
                  out_vert_format = MTL::VertexFormatShort4Normalized;
                  break;
                default:
                  KLI_assert_msg(false, "invalid vertex format");
                  out_vert_format = MTL::VertexFormatInvalid;
              }
            } else {
              /* Cannot convert. */
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
        }
        break;

      case GPU_COMP_U16:
        switch (fetch_mode) {
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatUShort ||
                shader_attrib_format == MTL::VertexFormatUShort2 ||
                shader_attrib_format == MTL::VertexFormatUShort3 ||
                shader_attrib_format == MTL::VertexFormatUShort4) {
              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Ensure format resize successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else {
              KLI_assert_msg(
                false,
                "Source vertex data format is either UShort, UShort2, UShort3, UShort4 "
                "but format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;

          /* Source vertex data is integral type, but shader interface type is floating point.
           * If the input attribute is specified as normalized, we can convert. */
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            if (normalized) {
              switch (component_length) {
                case 1:
                  out_vert_format = MTL::VertexFormatUShortNormalized;
                  break;
                case 2:
                  out_vert_format = MTL::VertexFormatUShort2Normalized;
                  break;
                case 3:
                  out_vert_format = MTL::VertexFormatUShort3Normalized;
                  break;
                case 4:
                  out_vert_format = MTL::VertexFormatUShort4Normalized;
                  break;
                default:
                  KLI_assert_msg(false, "invalid vertex format");
                  out_vert_format = MTL::VertexFormatInvalid;
              }
            } else {
              /* Cannot convert. */
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
        }
        break;

      case GPU_COMP_I32:
        switch (fetch_mode) {
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatInt ||
                shader_attrib_format == MTL::VertexFormatInt2 ||
                shader_attrib_format == MTL::VertexFormatInt3 ||
                shader_attrib_format == MTL::VertexFormatInt4) {
              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Verify conversion successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else {
              KLI_assert_msg(
                false,
                "Source vertex data format is either Int, Int2, Int3, Int4 but format "
                "in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            /* Unfortunately we cannot implicitly convert between Int and Float in METAL. */
            out_vert_format = MTL::VertexFormatInvalid;
            break;
        }
        break;

      case GPU_COMP_U32:
        switch (fetch_mode) {
          case GPU_FETCH_INT:
            if (shader_attrib_format == MTL::VertexFormatUInt ||
                shader_attrib_format == MTL::VertexFormatUInt2 ||
                shader_attrib_format == MTL::VertexFormatUInt3 ||
                shader_attrib_format == MTL::VertexFormatUInt4) {
              /* No conversion Needed (as type matches) - Just a vector resize if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Verify conversion successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else {
              KLI_assert_msg(false,
                             "Source vertex data format is either UInt, UInt2, UInt3, UInt4 but "
                             "format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            /* Unfortunately we cannot convert between UInt and Float in METAL */
            out_vert_format = MTL::VertexFormatInvalid;
            break;
        }
        break;

      case GPU_COMP_F32:
        switch (fetch_mode) {

          /* Source data is float. This will be compatible
           * if type specified in shader is also float. */
          case GPU_FETCH_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT:
          case GPU_FETCH_INT_TO_FLOAT_UNIT:
            if (shader_attrib_format == MTL::VertexFormatFloat ||
                shader_attrib_format == MTL::VertexFormatFloat2 ||
                shader_attrib_format == MTL::VertexFormatFloat3 ||
                shader_attrib_format == MTL::VertexFormatFloat4) {
              /* No conversion Needed (as type matches) - Just a vector resize, if needed. */
              bool can_convert = mtl_vertex_format_resize(shader_attrib_format,
                                                          component_length,
                                                          &out_vert_format);

              /* Verify conversion successful. */
              KLI_assert(can_convert);
              UNUSED_VARS_NDEBUG(can_convert);
            } else {
              KLI_assert_msg(
                false,
                "Source vertex data format is either Float, Float2, Float3, Float4 but "
                "format in shader interface is NOT compatible.\n");
              out_vert_format = MTL::VertexFormatInvalid;
            }
            break;

          case GPU_FETCH_INT:
            /* Unfortunately we cannot convert between Float and Int implicitly in METAL. */
            out_vert_format = MTL::VertexFormatInvalid;
            break;
        }
        break;

      case GPU_COMP_I10:
        out_vert_format = MTL::VertexFormatInt1010102Normalized;
        break;
    }
    *r_convertedFormat = out_vert_format;
    return (out_vert_format != MTL::VertexFormatInvalid);
  }

  inline uint comp_count_from_vert_format(MTL::VertexFormat vert_format)
  {
    switch (vert_format) {
      case MTL::VertexFormatFloat:
      case MTL::VertexFormatInt:
      case MTL::VertexFormatUInt:
      case MTL::VertexFormatShort:
      case MTL::VertexFormatUChar:
      case MTL::VertexFormatUCharNormalized:
        return 1;
      case MTL::VertexFormatUChar2:
      case MTL::VertexFormatUInt2:
      case MTL::VertexFormatFloat2:
      case MTL::VertexFormatInt2:
      case MTL::VertexFormatUChar2Normalized:
        return 2;
      case MTL::VertexFormatUChar3:
      case MTL::VertexFormatUInt3:
      case MTL::VertexFormatFloat3:
      case MTL::VertexFormatInt3:
      case MTL::VertexFormatShort3Normalized:
      case MTL::VertexFormatUChar3Normalized:
        return 3;
      case MTL::VertexFormatUChar4:
      case MTL::VertexFormatFloat4:
      case MTL::VertexFormatUInt4:
      case MTL::VertexFormatInt4:
      case MTL::VertexFormatUChar4Normalized:
      case MTL::VertexFormatInt1010102Normalized:

      default:
        KLI_assert_msg(false, "Unrecognized attribute type. Add types to switch as needed.");
        return 0;
    }
  }

  inline GPUVertFetchMode fetchmode_from_vert_format(MTL::VertexFormat vert_format)
  {
    switch (vert_format) {
      case MTL::VertexFormatFloat:
      case MTL::VertexFormatFloat2:
      case MTL::VertexFormatFloat3:
      case MTL::VertexFormatFloat4:
        return GPU_FETCH_FLOAT;

      case MTL::VertexFormatUChar:
      case MTL::VertexFormatUChar2:
      case MTL::VertexFormatUChar3:
      case MTL::VertexFormatUChar4:
      case MTL::VertexFormatChar:
      case MTL::VertexFormatChar2:
      case MTL::VertexFormatChar3:
      case MTL::VertexFormatChar4:
      case MTL::VertexFormatUShort:
      case MTL::VertexFormatUShort2:
      case MTL::VertexFormatUShort3:
      case MTL::VertexFormatUShort4:
      case MTL::VertexFormatShort:
      case MTL::VertexFormatShort2:
      case MTL::VertexFormatShort3:
      case MTL::VertexFormatShort4:
      case MTL::VertexFormatUInt:
      case MTL::VertexFormatUInt2:
      case MTL::VertexFormatUInt3:
      case MTL::VertexFormatUInt4:
      case MTL::VertexFormatInt:
      case MTL::VertexFormatInt2:
      case MTL::VertexFormatInt3:
      case MTL::VertexFormatInt4:
        return GPU_FETCH_INT;

      case MTL::VertexFormatUCharNormalized:
      case MTL::VertexFormatUChar2Normalized:
      case MTL::VertexFormatUChar3Normalized:
      case MTL::VertexFormatUChar4Normalized:
      case MTL::VertexFormatCharNormalized:
      case MTL::VertexFormatChar2Normalized:
      case MTL::VertexFormatChar3Normalized:
      case MTL::VertexFormatChar4Normalized:
      case MTL::VertexFormatUShortNormalized:
      case MTL::VertexFormatUShort2Normalized:
      case MTL::VertexFormatUShort3Normalized:
      case MTL::VertexFormatUShort4Normalized:
      case MTL::VertexFormatShortNormalized:
      case MTL::VertexFormatShort2Normalized:
      case MTL::VertexFormatShort3Normalized:
      case MTL::VertexFormatShort4Normalized:
      case MTL::VertexFormatInt1010102Normalized:
        return GPU_FETCH_INT_TO_FLOAT_UNIT;

      default:
        KLI_assert_msg(false, "Unrecognized attribute type. Add types to switch as needed.");
        return GPU_FETCH_FLOAT;
    }
  }

  inline GPUVertCompType comp_type_from_vert_format(MTL::VertexFormat vert_format)
  {
    switch (vert_format) {
      case MTL::VertexFormatUChar:
      case MTL::VertexFormatUChar2:
      case MTL::VertexFormatUChar3:
      case MTL::VertexFormatUChar4:
      case MTL::VertexFormatUCharNormalized:
      case MTL::VertexFormatUChar2Normalized:
      case MTL::VertexFormatUChar3Normalized:
      case MTL::VertexFormatUChar4Normalized:
        return GPU_COMP_U8;

      case MTL::VertexFormatChar:
      case MTL::VertexFormatChar2:
      case MTL::VertexFormatChar3:
      case MTL::VertexFormatChar4:
      case MTL::VertexFormatCharNormalized:
      case MTL::VertexFormatChar2Normalized:
      case MTL::VertexFormatChar3Normalized:
      case MTL::VertexFormatChar4Normalized:
        return GPU_COMP_I8;

      case MTL::VertexFormatShort:
      case MTL::VertexFormatShort2:
      case MTL::VertexFormatShort3:
      case MTL::VertexFormatShort4:
      case MTL::VertexFormatShortNormalized:
      case MTL::VertexFormatShort2Normalized:
      case MTL::VertexFormatShort3Normalized:
      case MTL::VertexFormatShort4Normalized:
        return GPU_COMP_I16;

      case MTL::VertexFormatUShort:
      case MTL::VertexFormatUShort2:
      case MTL::VertexFormatUShort3:
      case MTL::VertexFormatUShort4:
      case MTL::VertexFormatUShortNormalized:
      case MTL::VertexFormatUShort2Normalized:
      case MTL::VertexFormatUShort3Normalized:
      case MTL::VertexFormatUShort4Normalized:
        return GPU_COMP_U16;

      case MTL::VertexFormatInt:
      case MTL::VertexFormatInt2:
      case MTL::VertexFormatInt3:
      case MTL::VertexFormatInt4:
        return GPU_COMP_I32;

      case MTL::VertexFormatUInt:
      case MTL::VertexFormatUInt2:
      case MTL::VertexFormatUInt3:
      case MTL::VertexFormatUInt4:
        return GPU_COMP_U32;

      case MTL::VertexFormatFloat:
      case MTL::VertexFormatFloat2:
      case MTL::VertexFormatFloat3:
      case MTL::VertexFormatFloat4:
        return GPU_COMP_F32;

      case MTL::VertexFormatInt1010102Normalized:
        return GPU_COMP_I10;

      default:
        KLI_assert_msg(false, "Unrecognized attribute type. Add types to switch as needed.");
        return GPU_COMP_F32;
    }
  }

}  // namespace kraken::gpu
