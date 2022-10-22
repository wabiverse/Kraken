/* SPDX-License-Identifier: GPL-2.0-or-later */

/** @file
 * \ingroup gpu
 */

#include "KKE_global.h"

#include "KLI_string.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>

#include <cstring>

#include "GPU_platform.h"
#include "GPU_vertex_format.h"

#include "mtl_common.hh"
#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_pso_descriptor_state.hh"
#include "mtl_shader.hh"
#include "mtl_shader_generator.hh"
#include "mtl_shader_interface.hh"
#include "mtl_texture.hh"

extern char datatoc_mtl_shader_common_msl[];

using namespace kraken;
using namespace kraken::gpu;
using namespace kraken::gpu::shader;

namespace kraken::gpu
{

  /* -------------------------------------------------------------------- */
  /** @name Creation / Destruction.
   * @{ */

  /* Create empty shader to be populated later. */
  MTLShader::MTLShader(MTLContext *ctx, const char *name) : Shader(name)
  {
    m_context = ctx;

    /* Create SHD builder to hold temporary resources until compilation is complete. */
    m_shd_builder = new MTLShaderBuilder();

#ifndef NDEBUG
    /* Remove invalid symbols from shader name to ensure debug entry-point function name is valid.
     */
    for (uint i : IndexRange(strlen(this->name))) {
      char c = this->name[i];
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      } else {
        this->name[i] = '_';
      }
    }
#endif
  }

  /* Create shader from MSL source. */
  MTLShader::MTLShader(MTLContext *ctx,
                       MTLShaderInterface *interface,
                       const char *name,
                       NS::String *input_vertex_source,
                       NS::String *input_fragment_source,
                       NS::String *vert_function_name,
                       NS::String *frag_function_name)
    : MTLShader(ctx, name)
  {
    KLI_assert(vert_function_name->length());
    KLI_assert(frag_function_name->length());

    this->set_vertex_function_name(vert_function_name);
    this->set_fragment_function_name(frag_function_name);
    this->shader_source_from_msl(input_vertex_source, input_fragment_source);
    this->set_interface(interface);
    this->finalize(nullptr);
  }

  MTLShader::~MTLShader()
  {
    if (this->is_valid()) {

      /* Free uniform data block. */
      if (m_push_constant_data != nullptr) {
        MEM_freeN(m_push_constant_data);
        m_push_constant_data = nullptr;
      }

      /* Free Metal resources. */
      if (m_shader_library_vert != nil) {
        m_shader_library_vert->release();
        m_shader_library_vert = nil;
      }
      if (m_shader_library_frag != nil) {
        m_shader_library_frag->release();
        m_shader_library_frag = nil;
      }

      if (m_pso_descriptor != nil) {
        m_pso_descriptor->release();
        m_pso_descriptor = nil;
      }

      /* Free Pipeline Cache. */
      for (const MTLRenderPipelineStateInstance *pso_inst : m_pso_cache.values()) {
        if (pso_inst->vert) {
          pso_inst->vert->release();
        }
        if (pso_inst->frag) {
          pso_inst->frag->release();
        }
        if (pso_inst->pso) {
          pso_inst->pso->release();
        }
        delete pso_inst;
      }
      m_pso_cache.clear();

      /* NOTE(Metal): #ShaderInterface deletion is handled in the super destructor `~Shader()`. */
    }
    m_valid = false;

    if (m_shd_builder != nullptr) {
      delete m_shd_builder;
      m_shd_builder = nullptr;
    }
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Shader stage creation.
   * @{ */

  void MTLShader::vertex_shader_from_glsl(MutableSpan<const char *> sources)
  {
    /* Flag source as not being compiled from native MSL. */
    KLI_assert(m_shd_builder != nullptr);
    m_shd_builder->m_source_from_msl = false;

    /* Remove #version tag entry. */
    sources[0] = "";

    /* Consolidate GLSL vertex sources. */
    std::stringstream ss;
    for (int i = 0; i < sources.size(); i++) {
      ss << sources[i] << std::endl;
    }
    m_shd_builder->m_glsl_vertex_source = ss.str();
  }

  void MTLShader::geometry_shader_from_glsl(MutableSpan<const char *> sources)
  {
    MTL_LOG_ERROR("MTLShader::geometry_shader_from_glsl - Geometry shaders unsupported!\n");
  }

  void MTLShader::fragment_shader_from_glsl(MutableSpan<const char *> sources)
  {
    /* Flag source as not being compiled from native MSL. */
    KLI_assert(m_shd_builder != nullptr);
    m_shd_builder->m_source_from_msl = false;

    /* Remove #version tag entry. */
    sources[0] = "";

    /* Consolidate GLSL fragment sources. */
    std::stringstream ss;
    for (int i = 0; i < sources.size(); i++) {
      ss << sources[i] << std::endl;
    }
    m_shd_builder->m_glsl_fragment_source = ss.str();
  }

  void MTLShader::compute_shader_from_glsl(MutableSpan<const char *> sources)
  {
    /* Remove #version tag entry. */
    sources[0] = "";

    /* TODO(Metal): Support compute shaders in Metal. */
    MTL_LOG_WARNING(
      "MTLShader::compute_shader_from_glsl - Compute shaders currently unsupported!\n");
  }

  bool MTLShader::finalize(const shader::ShaderCreateInfo *info)
  {
    /* Check if Shader has already been finalized. */
    if (this->is_valid()) {
      MTL_LOG_ERROR("Shader (%p) '%s' has already been finalized!\n", this, this->name_get());
    }

    /* Perform GLSL to MSL source translation. */
    KLI_assert(m_shd_builder != nullptr);
    if (!m_shd_builder->m_source_from_msl) {
      bool success = generate_msl_from_glsl(info);
      if (!success) {
        /* GLSL to MSL translation has failed, or is unsupported for this shader. */
        m_valid = false;
        KLI_assert_msg(false, "Shader translation from GLSL to MSL has failed. \n");

        /* Create empty interface to allow shader to be silently used. */
        MTLShaderInterface *mtl_interface = new MTLShaderInterface(this->name_get());
        this->set_interface(mtl_interface);

        /* Release temporary compilation resources. */
        delete m_shd_builder;
        m_shd_builder = nullptr;
        return false;
      }
    }

    /* Ensure we have a valid shader interface. */
    MTLShaderInterface *mtl_interface = this->get_interface();
    KLI_assert(mtl_interface != nullptr);

    /* Verify Context handle, fetch device and compile shader. */
    KLI_assert(m_context);
    MTL::Device *device = m_context->device;
    KLI_assert(device != nil);

    /* Ensure source and stage entry-point names are set. */
    KLI_assert(m_vertex_function_name->length() > 0);
    if (m_transform_feedback_type == GPU_SHADER_TFB_NONE) {
      KLI_assert(m_fragment_function_name->length() > 0);
    }
    KLI_assert(m_shd_builder != nullptr);
    KLI_assert(m_shd_builder->m_msl_source_vert->length() > 0);

    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
    MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init()->autorelease();
    options->setLanguageVersion(MTL::LanguageVersion2_2);
    options->setFastMathEnabled(YES);

    NS::String *source_to_compile = m_shd_builder->m_msl_source_vert;
    for (int src_stage = 0; src_stage <= 1; src_stage++) {

      source_to_compile = (src_stage == 0) ? m_shd_builder->m_msl_source_vert :
                                             m_shd_builder->m_msl_source_frag;

      /* Transform feedback, skip compilation. */
      if (src_stage == 1 && (m_transform_feedback_type != GPU_SHADER_TFB_NONE)) {
        m_shader_library_frag = nil;
        break;
      }

      /* Concatenate common source. */
      NS::String *str = NS_STRING_(datatoc_mtl_shader_common_msl);
      NS::String *source_with_header_a = str->stringByAppendingString(source_to_compile);

      /* Inject unique context ID to avoid cross-context shader cache collisions.
       * Required on macOS 11.0. */
      NS::String *source_with_header = source_with_header_a;
#if defined(MAC_OS_VERSION_11_0) && __MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_VERSION_11_0
      /* Pass-through. Availability syntax requirement, expression cannot be negated. */
#else
      char src_def[64];
      snprintf(src_def,
               sizeof(src_def),
               "\n\n#define MTL_CONTEXT_IND %d\n",
               m_context->context_id);
      source_with_header = source_with_header_a->stringByAppendingString(NS_STRING_(src_def));
#endif
      source_with_header->retain();

      /* Prepare Shader Library. */
      NS::Error *error = nil;
      MTL::Library *library = device->newLibrary(source_with_header, options, &error);
      if (error) {
        /* Only exit out if genuine error and not warning. */
        if (error->localizedDescription()
              ->rangeOfString(NS_STRING_("Compilation succeeded"), NS::CaseInsensitiveSearch)
              .location == NS::NotFound) {
          MTL_LOG_ERROR("Compile Error - Metal Shader Library (Stage: %d), error %s \n",
                        src_stage,
                        error->localizedDescription()->utf8String());
          KLI_assert(false);

          /* Release temporary compilation resources. */
          delete m_shd_builder;
          m_shd_builder = nullptr;
          return false;
        }
      }

      MTL_LOG_INFO("Successfully compiled Metal Shader Library (Stage: %d) for shader; %s\n",
                   src_stage,
                   name);
      KLI_assert(library != nil);
      if (src_stage == 0) {
        /* Retain generated library and assign debug name. */
        m_shader_library_vert = library;
        m_shader_library_vert->retain();
        m_shader_library_vert->setLabel(NS_STRING_(this->name));
      } else {
        /* Retain generated library for fragment shader and assign debug name. */
        m_shader_library_frag = library;
        m_shader_library_frag->retain();
        m_shader_library_frag->setLabel(NS_STRING_(this->name));
      }

      source_with_header->autorelease();
    }
    m_pso_descriptor->setLabel(NS_STRING_(this->name));

    /* Prepare descriptor. */
    m_pso_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    m_pso_descriptor->retain();

    /* Shader has successfully been created. */
    m_valid = true;

    /* Prepare backing data storage for local uniforms. */
    const MTLShaderUniformBlock &push_constant_block = mtl_interface->get_push_constant_block();
    if (push_constant_block.size > 0) {
      m_push_constant_data = MEM_callocN(push_constant_block.size, __func__);
      this->push_constant_bindstate_mark_dirty(true);
    } else {
      m_push_constant_data = nullptr;
    }

    pool->drain();
    pool = nil;

    /* Release temporary compilation resources. */
    delete m_shd_builder;
    m_shd_builder = nullptr;
    return true;
  }

  void MTLShader::transform_feedback_names_set(Span<const char *> name_list,
                                               const eGPUShaderTFBType geom_type)
  {
    m_tf_output_name_list.clear();
    for (int i = 0; i < name_list.size(); i++) {
      m_tf_output_name_list.append(std::string(name_list[i]));
    }
    m_transform_feedback_type = geom_type;
  }

  bool MTLShader::transform_feedback_enable(GPUVertBuf *buf)
  {
    KLI_assert(m_transform_feedback_type != GPU_SHADER_TFB_NONE);
    KLI_assert(buf);
    m_transform_feedback_active = true;
    m_transform_feedback_vertbuf = buf;
    /* TODO(Metal): Enable this assertion once #MTLVertBuf lands. */
    // KLI_assert(static_cast<MTLVertBuf *>(unwrap(m_transform_feedback_vertbuf))->get_usage_type()
    // ==
    //            GPU_USAGE_DEVICE_ONLY);
    return true;
  }

  void MTLShader::transform_feedback_disable()
  {
    m_transform_feedback_active = false;
    m_transform_feedback_vertbuf = nullptr;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Shader Binding.
   * @{ */

  void MTLShader::bind()
  {
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    if (interface == nullptr || !this->is_valid()) {
      MTL_LOG_WARNING(
        "MTLShader::bind - Shader '%s' has no valid implementation in Metal, draw calls will be "
        "skipped.\n",
        this->name_get());
    }
    ctx->pipeline_state.active_shader = this;
  }

  void MTLShader::unbind()
  {
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    ctx->pipeline_state.active_shader = nullptr;
  }

  void MTLShader::uniform_float(int location, int comp_len, int array_size, const float *data)
  {
    KLI_assert(this);
    if (!this->is_valid()) {
      return;
    }
    MTLShaderInterface *mtl_interface = get_interface();
    if (location < 0 || location >= mtl_interface->get_total_uniforms()) {
      MTL_LOG_WARNING("Uniform location %d is not valid in Shader %s\n",
                      location,
                      this->name_get());
      return;
    }

    /* Fetch more information about uniform from interface. */
    const MTLShaderUniform &uniform = mtl_interface->get_uniform(location);

    /* Prepare to copy data into local shader push constant memory block. */
    KLI_assert(m_push_constant_data != nullptr);
    uint8_t *dest_ptr = (uint8_t *)m_push_constant_data;
    dest_ptr += uniform.byte_offset;
    uint32_t copy_size = sizeof(float) * comp_len * array_size;

    /* Test per-element size. It is valid to copy less array elements than the total, but each
     * array element needs to match. */
    uint32_t source_per_element_size = sizeof(float) * comp_len;
    uint32_t dest_per_element_size = uniform.size_in_bytes / uniform.array_len;
    KLI_assert_msg(
      source_per_element_size <= dest_per_element_size,
      "source Per-array-element size must be smaller than destination storage capacity for "
      "that data");

    if (source_per_element_size < dest_per_element_size) {
      switch (uniform.type) {

        /* Special case for handling 'vec3' array upload. */
        case MTL_DATATYPE_FLOAT3: {
          int numvecs = uniform.array_len;
          uint8_t *data_c = (uint8_t *)data;

          /* It is more efficient on the host to only modify data if it has changed.
           * Data modifications are small, so memory comparison is cheap.
           * If uniforms have remained unchanged, then we avoid both copying
           * data into the local uniform struct, and upload of the modified uniform
           * contents in the command stream. */
          bool changed = false;
          for (int i = 0; i < numvecs; i++) {
            changed = changed ||
                      (memcmp((void *)dest_ptr, (void *)data_c, sizeof(float) * 3) != 0);
            if (changed) {
              memcpy((void *)dest_ptr, (void *)data_c, sizeof(float) * 3);
            }
            data_c += sizeof(float) * 3;
            dest_ptr += sizeof(float) * 4;
          }
          if (changed) {
            this->push_constant_bindstate_mark_dirty(true);
          }
          return;
        }

        /* Special case for handling 'mat3' upload. */
        case MTL_DATATYPE_FLOAT3x3: {
          int numvecs = 3 * uniform.array_len;
          uint8_t *data_c = (uint8_t *)data;

          /* It is more efficient on the host to only modify data if it has changed.
           * Data modifications are small, so memory comparison is cheap.
           * If uniforms have remained unchanged, then we avoid both copying
           * data into the local uniform struct, and upload of the modified uniform
           * contents in the command stream. */
          bool changed = false;
          for (int i = 0; i < numvecs; i++) {
            changed = changed ||
                      (memcmp((void *)dest_ptr, (void *)data_c, sizeof(float) * 3) != 0);
            if (changed) {
              memcpy((void *)dest_ptr, (void *)data_c, sizeof(float) * 3);
            }
            data_c += sizeof(float) * 3;
            dest_ptr += sizeof(float) * 4;
          }
          if (changed) {
            this->push_constant_bindstate_mark_dirty(true);
          }
          return;
        }
        default:
          shader_debug_printf("INCOMPATIBLE UNIFORM TYPE: %d\n", uniform.type);
          break;
      }
    }

    /* Debug checks. */
    KLI_assert_msg(
      copy_size <= uniform.size_in_bytes,
      "Size of provided uniform data is greater than size specified in Shader interface\n");

    /* Only flag UBO as modified if data is different -- This can avoid re-binding of unmodified
     * local uniform data. */
    bool data_changed = (memcmp((void *)dest_ptr, (void *)data, copy_size) != 0);
    if (data_changed) {
      this->push_constant_bindstate_mark_dirty(true);
      memcpy((void *)dest_ptr, (void *)data, copy_size);
    }
  }

  void MTLShader::uniform_int(int location, int comp_len, int array_size, const int *data)
  {
    KLI_assert(this);
    if (!this->is_valid()) {
      return;
    }

    /* NOTE(Metal): Invalidation warning for uniform re-mapping of texture slots, unsupported in
     * Metal, as we cannot point a texture binding at a different slot. */
    MTLShaderInterface *mtl_interface = this->get_interface();
    if (location >= mtl_interface->get_total_uniforms() &&
        location < (mtl_interface->get_total_uniforms() + mtl_interface->get_total_textures())) {
      MTL_LOG_WARNING(
        "Texture uniform location re-mapping unsupported in Metal. (Possibly also bad uniform "
        "location %d)\n",
        location);
      return;
    }

    if (location < 0 || location >= mtl_interface->get_total_uniforms()) {
      MTL_LOG_WARNING("Uniform is not valid at location %d - Shader %s\n",
                      location,
                      this->name_get());
      return;
    }

    /* Fetch more information about uniform from interface. */
    const MTLShaderUniform &uniform = mtl_interface->get_uniform(location);

    /* Determine data location in uniform block. */
    KLI_assert(m_push_constant_data != nullptr);
    uint8_t *ptr = (uint8_t *)m_push_constant_data;
    ptr += uniform.byte_offset;

    /* Copy data into local block. Only flag UBO as modified if data is different
     * This can avoid re-binding of unmodified local uniform data, reducing
     * the total number of copy operations needed and data transfers between
     * CPU and GPU. */
    bool data_changed = (memcmp((void *)ptr, (void *)data, sizeof(int) * comp_len * array_size) !=
                         0);
    if (data_changed) {
      this->push_constant_bindstate_mark_dirty(true);
      memcpy((void *)ptr, (void *)data, sizeof(int) * comp_len * array_size);
    }
  }

  bool MTLShader::get_push_constant_is_dirty()
  {
    return m_push_constant_modified;
  }

  void MTLShader::push_constant_bindstate_mark_dirty(bool is_dirty)
  {
    m_push_constant_modified = is_dirty;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name METAL Custom Behavior
   * @{ */

  void MTLShader::set_vertex_function_name(NS::String *vert_function_name)
  {
    m_vertex_function_name = vert_function_name;
  }

  void MTLShader::set_fragment_function_name(NS::String *frag_function_name)
  {
    m_fragment_function_name = frag_function_name;
  }

  void MTLShader::shader_source_from_msl(NS::String *input_vertex_source,
                                         NS::String *input_fragment_source)
  {
    KLI_assert(m_shd_builder != nullptr);
    m_shd_builder->m_msl_source_vert = input_vertex_source;
    m_shd_builder->m_msl_source_frag = input_fragment_source;
    m_shd_builder->m_source_from_msl = true;
  }

  void MTLShader::set_interface(MTLShaderInterface *interface)
  {
    /* Assign gpu::Shader super-class interface. */
    Shader::interface = interface;
  }

  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name Bake Pipeline State Objects
   * @{ */

  /**
   * Bakes or fetches a pipeline state using the current
   * #MTLRenderPipelineStateDescriptor state.
   *
   * This state contains information on shader inputs/outputs, such
   * as the vertex descriptor, used to control vertex assembly for
   * current vertex data, and active render target information,
   * describing the output attachment pixel formats.
   *
   * Other rendering parameters such as global point-size, blend state, color mask
   * etc; are also used. See mtl_shader.h for full #MLRenderPipelineStateDescriptor.
   */
  MTLRenderPipelineStateInstance *MTLShader::bake_current_pipeline_state(
    MTLContext *ctx,
    MTL::PrimitiveTopologyClass prim_type)
  {
    /* NOTE(Metal): PSO cache can be accessed from multiple threads, though these operations should
     * be thread-safe due to organization of high-level renderer. If there are any issues, then
     * access can be guarded as appropriate. */
    KLI_assert(this);
    MTLShaderInterface *mtl_interface = this->get_interface();
    KLI_assert(mtl_interface);
    KLI_assert(this->is_valid());

    /* NOTE(Metal): Vertex input assembly description will have been populated externally
     * via #MTLBatch or #MTLImmediate during binding or draw. */

    /* Resolve Context Frame-buffer state. */
    MTLFrameBuffer *framebuffer = ctx->get_current_framebuffer();

    /* Update global pipeline descriptor. */
    MTLStateManager *state_manager = static_cast<MTLStateManager *>(
      MTLContext::get()->state_manager);
    MTLRenderPipelineStateDescriptor &pipeline_descriptor =
      state_manager->get_pipeline_descriptor();

    pipeline_descriptor.num_color_attachments = 0;
    for (int attachment = 0; attachment < GPU_FB_MAX_COLOR_ATTACHMENT; attachment++) {
      MTLAttachment color_attachment = framebuffer->get_color_attachment(attachment);

      if (color_attachment.used) {
        /* If SRGB is disabled and format is SRGB, use color data directly with no conversions
         * between linear and SRGB. */
        MTL::PixelFormat mtl_format = gpu_texture_format_to_metal(
          color_attachment.texture->format_get());
        if (framebuffer->get_is_srgb() && !framebuffer->get_srgb_enabled()) {
          mtl_format = MTL::PixelFormatRGBA8Unorm;
        }
        pipeline_descriptor.color_attachment_format[attachment] = mtl_format;
      } else {
        pipeline_descriptor.color_attachment_format[attachment] = MTL::PixelFormatInvalid;
      }

      pipeline_descriptor.num_color_attachments += (color_attachment.used) ? 1 : 0;
    }
    MTLAttachment depth_attachment = framebuffer->get_depth_attachment();
    MTLAttachment stencil_attachment = framebuffer->get_stencil_attachment();
    pipeline_descriptor.depth_attachment_format = (depth_attachment.used) ?
                                                    gpu_texture_format_to_metal(
                                                      depth_attachment.texture->format_get()) :
                                                    MTL::PixelFormatInvalid;
    pipeline_descriptor.stencil_attachment_format = (stencil_attachment.used) ?
                                                      gpu_texture_format_to_metal(
                                                        stencil_attachment.texture->format_get()) :
                                                      MTL::PixelFormatInvalid;

    /* Resolve Context Pipeline State (required by PSO). */
    pipeline_descriptor.color_write_mask = ctx->pipeline_state.color_write_mask;
    pipeline_descriptor.blending_enabled = ctx->pipeline_state.blending_enabled;
    pipeline_descriptor.alpha_blend_op = ctx->pipeline_state.alpha_blend_op;
    pipeline_descriptor.rgb_blend_op = ctx->pipeline_state.rgb_blend_op;
    pipeline_descriptor.dest_alpha_blend_factor = ctx->pipeline_state.dest_alpha_blend_factor;
    pipeline_descriptor.dest_rgb_blend_factor = ctx->pipeline_state.dest_rgb_blend_factor;
    pipeline_descriptor.src_alpha_blend_factor = ctx->pipeline_state.src_alpha_blend_factor;
    pipeline_descriptor.src_rgb_blend_factor = ctx->pipeline_state.src_rgb_blend_factor;
    pipeline_descriptor.point_size = ctx->pipeline_state.point_size;

    /* Primitive Type -- Primitive topology class needs to be specified for layered rendering. */
    bool requires_specific_topology_class = m_uses_mtl_array_index ||
                                            prim_type == MTL::PrimitiveTopologyClassPoint;
    pipeline_descriptor.vertex_descriptor.prim_topology_class =
      (requires_specific_topology_class) ? prim_type : MTL::PrimitiveTopologyClassUnspecified;

    /* Check if current PSO exists in the cache. */
    MTLRenderPipelineStateInstance **pso_lookup = m_pso_cache.lookup_ptr(pipeline_descriptor);
    MTLRenderPipelineStateInstance *pipeline_state = (pso_lookup) ? *pso_lookup : nullptr;
    if (pipeline_state != nullptr) {
      return pipeline_state;
    }

    shader_debug_printf("Baking new pipeline variant for shader: %s\n", this->name);

    /* Generate new Render Pipeline State Object (PSO). */
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
    /* Prepare Render Pipeline Descriptor. */

    /* Setup function specialization constants, used to modify and optimize
     * generated code based on current render pipeline configuration. */
    MTL::FunctionConstantValues *values =
      MTL::FunctionConstantValues::alloc()->init()->autorelease();

    /* Prepare Vertex descriptor based on current pipeline vertex binding state. */
    MTLRenderPipelineStateDescriptor &current_state = pipeline_descriptor;
    MTL::RenderPipelineDescriptor *desc = m_pso_descriptor;
    desc->reset();
    m_pso_descriptor->setLabel(NS_STRING_(this->name));

    /* Offset the bind index for Uniform buffers such that they begin after the VBO
     * buffer bind slots. `MTL_uniform_buffer_base_index` is passed as a function
     * specialization constant, customized per unique pipeline state permutation.
     *
     * NOTE: For binding point compaction, we could use the number of VBOs present
     * in the current PSO configuration `current_state.vertex_descriptor.num_vert_buffers`).
     * However, it is more efficient to simply offset the uniform buffer base index to the
     * maximal number of VBO bind-points, as then UBO bind-points for similar draw calls
     * will align and avoid the requirement for additional binding. */
    int MTL_uniform_buffer_base_index = GPU_BATCH_VBO_MAX_LEN;

    /* Null buffer index is used if an attribute is not found in the
     * bound VBOs #VertexFormat. */
    int null_buffer_index = current_state.vertex_descriptor.num_vert_buffers;
    bool using_null_buffer = false;

    if (this->get_uses_ssbo_vertex_fetch()) {
      /* If using SSBO Vertex fetch mode, no vertex descriptor is required
       * as we wont be using stage-in. */
      desc->setVertexDescriptor(nil);
      desc->setInputPrimitiveTopology(MTL::PrimitiveTopologyClassUnspecified);

      /* We want to offset the uniform buffer base to allow for sufficient VBO binding slots - We
       * also require +1 slot for the Index buffer. */
      MTL_uniform_buffer_base_index = MTL_SSBO_VERTEX_FETCH_IBO_INDEX + 1;
    } else {
      for (const uint i : IndexRange(current_state.vertex_descriptor.num_attributes)) {

        /* Metal back-end attribute descriptor state. */
        MTLVertexAttributeDescriptorPSO &attribute_desc =
          current_state.vertex_descriptor.attributes[i];

        /* Flag format conversion */
        /* In some cases, Metal cannot implicitly convert between data types.
         * In these instances, the fetch mode #GPUVertFetchMode as provided in the vertex format
         * is passed in, and used to populate function constants named:
         * MTL_AttributeConvert0..15.
         *
         * It is then the responsibility of the vertex shader to perform any necessary type
         * casting.
         *
         * See `mtl_shader.hh` for more information. Relevant Metal API documentation:
         * https://developer.apple.com/documentation/metal/mtlvertexattributedescriptor/1516081-format?language=objc
         */
        if (attribute_desc.format == MTL::VertexFormatInvalid) {
          MTL_LOG_WARNING(
            "MTLShader: baking pipeline state for '%s'- expected input attribute at "
            "index '%d' but none was specified in the current vertex state\n",
            mtl_interface->get_name(),
            i);

          /* Write out null conversion constant if attribute unused. */
          char conv_buf[64];
          int MTL_attribute_conversion_mode = 0;
          snprintf(conv_buf, sizeof(conv_buf), "MTL_AttributeConvert%d", i);
          values->setConstantValue(&MTL_attribute_conversion_mode,
                                   MTL::DataTypeInt,
                                   NS_STRING_(conv_buf));
          continue;
        }

        char conv_buf[64];
        int MTL_attribute_conversion_mode = (int)attribute_desc.format_conversion_mode;
        snprintf(conv_buf, sizeof(conv_buf), "MTL_AttributeConvert%d", i);
        values->setConstantValue(&MTL_attribute_conversion_mode,
                                 MTL::DataTypeInt,
                                 NS_STRING_(conv_buf));
        if (MTL_attribute_conversion_mode == GPU_FETCH_INT_TO_FLOAT_UNIT ||
            MTL_attribute_conversion_mode == GPU_FETCH_INT_TO_FLOAT) {
          shader_debug_printf(
            "TODO(Metal): Shader %s needs to support internal format conversion\n",
            mtl_interface->name);
        }

        /* Copy metal back-end attribute descriptor state into PSO descriptor.
         * NOTE: need to copy each element due to direct assignment restrictions.
         * Also note */
        MTL::VertexAttributeDescriptor *mtl_attribute =
          desc->vertexDescriptor()->attributes()->object(i);

        mtl_attribute->setFormat(attribute_desc.format);
        mtl_attribute->setOffset(attribute_desc.offset);
        mtl_attribute->setBufferIndex(attribute_desc.buffer_index);
      }

      for (const uint i : IndexRange(current_state.vertex_descriptor.num_vert_buffers)) {
        /* Metal back-end state buffer layout. */
        const MTLVertexBufferLayoutDescriptorPSO &buf_layout =
          current_state.vertex_descriptor.buffer_layouts[i];
        /* Copy metal back-end buffer layout state into PSO descriptor.
         * NOTE: need to copy each element due to copying from internal
         * back-end descriptor to Metal API descriptor. */
        MTL::VertexBufferLayoutDescriptor *mtl_buf_layout =
          desc->vertexDescriptor()->layouts()->object(i);

        mtl_buf_layout->setStepFunction(buf_layout.step_function);
        mtl_buf_layout->setStepRate(buf_layout.step_rate);
        mtl_buf_layout->setStride(buf_layout.stride);
      }

      /* Mark empty attribute conversion. */
      for (int i = current_state.vertex_descriptor.num_attributes; i < GPU_VERT_ATTR_MAX_LEN;
           i++) {
        char conv_buf[64];
        int MTL_attribute_conversion_mode = 0;
        snprintf(conv_buf, sizeof(conv_buf), "MTL_AttributeConvert%d", i);
        values->setConstantValue(&MTL_attribute_conversion_mode,
                                 MTL::DataTypeInt,
                                 NS_STRING_(conv_buf));
      }

      /* DEBUG: Missing/empty attributes. */
      /* Attributes are normally mapped as part of the state setting based on the used
       * #GPUVertFormat, however, if attributes have not been set, we can sort them out here. */
      for (const uint i : IndexRange(mtl_interface->get_total_attributes())) {
        const MTLShaderInputAttribute &attribute = mtl_interface->get_attribute(i);
        MTL::VertexAttributeDescriptor *current_attribute =
          desc->vertexDescriptor()->attributes()->object(i);

        if (current_attribute->format() == MTL::VertexFormatInvalid) {
#if MTL_DEBUG_SHADER_ATTRIBUTES == 1
          MTL_LOG_INFO("-> Filling in unbound attribute '%s' for shader PSO '%s' \n",
                       attribute.name,
                       mtl_interface->name);
#endif
          current_attribute->setFormat(attribute.format);
          current_attribute->setOffset(0);
          current_attribute->setBufferIndex(null_buffer_index);

          /* Add Null vert buffer binding for invalid attributes. */
          if (!using_null_buffer) {
            MTL::VertexBufferLayoutDescriptor *null_buf_layout =
              desc->vertexDescriptor()->layouts()->object(null_buffer_index);

            /* Use constant step function such that null buffer can
             * contain just a singular dummy attribute. */
            null_buf_layout->setStepFunction(MTL::VertexStepFunctionConstant);
            null_buf_layout->setStepRate(0);
            null_buf_layout->setStride(max_ii(null_buf_layout->stride(), attribute.size));

            /* If we are using the maximum number of vertex buffers, or tight binding indices,
             * MTL_uniform_buffer_base_index needs shifting to the bind slot after the null
             * buffer index. */
            if (null_buffer_index >= MTL_uniform_buffer_base_index) {
              MTL_uniform_buffer_base_index = null_buffer_index + 1;
            }
            using_null_buffer = true;
#if MTL_DEBUG_SHADER_ATTRIBUTES == 1
            MTL_LOG_INFO("Setting up buffer binding for null attribute with buffer index %d\n",
                         null_buffer_index);
#endif
          }
        }
      }

      /* Primitive Topology */
      desc->setInputPrimitiveTopology(pipeline_descriptor.vertex_descriptor.prim_topology_class);
    }

    /* Update constant value for 'MTL_uniform_buffer_base_index' */
    values->setConstantValue(&MTL_uniform_buffer_base_index,
                             MTL::DataTypeInt,
                             NS_STRING_("MTL_uniform_buffer_base_index"));

    /* Transform feedback constant */
    int MTL_transform_feedback_buffer_index = (this->m_transform_feedback_type !=
                                               GPU_SHADER_TFB_NONE) ?
                                                MTL_uniform_buffer_base_index +
                                                  mtl_interface->get_total_uniform_blocks() :
                                                -1;
    if (this->m_transform_feedback_type != GPU_SHADER_TFB_NONE) {
      values->setConstantValue(&MTL_transform_feedback_buffer_index,
                               MTL::DataTypeInt,
                               NS_STRING_("MTL_transform_feedback_buffer_index"));
    }

    /* gl_PointSize constant */
    bool null_pointsize = true;
    float MTL_pointsize = pipeline_descriptor.point_size;
    if (pipeline_descriptor.vertex_descriptor.prim_topology_class ==
        MTL::PrimitiveTopologyClassPoint) {
      /* `if pointsize is > 0.0`, PROGRAM_POINT_SIZE is enabled, and `gl_PointSize` shader
       * keyword overrides the value. Otherwise, if < 0.0, use global constant point size. */
      if (MTL_pointsize < 0.0) {
        MTL_pointsize = fabsf(MTL_pointsize);
        values->setConstantValue(&MTL_pointsize,
                                 MTL::DataTypeFloat,
                                 NS_STRING_("MTL_global_pointsize"));
        null_pointsize = false;
      }
    }

    if (null_pointsize) {
      MTL_pointsize = 0.0f;
      values->setConstantValue(&MTL_pointsize,
                               MTL::DataTypeFloat,
                               NS_STRING_("MTL_global_pointsize"));
    }

    /* Compile functions */
    NS::Error *error = nullptr;
    desc->setVertexFunction(
      m_shader_library_vert->newFunction(m_vertex_function_name, values, &error));
    if (error) {
      MTL_LOG_ERROR("Compile Error - Metal Shader vertex function, error %s\n",
                    error->localizedDescription()->utf8String());

      /* Only exit out if genuine error and not warning */
      if (error->localizedDescription()
            ->rangeOfString(NS_STRING_("Compilation succeeded"), NS::CaseInsensitiveSearch)
            .location == NS::NotFound) {
        KLI_assert(false);
        return nullptr;
      }
    }

    /* If transform feedback is used, Vertex-only stage */
    if (m_transform_feedback_type == GPU_SHADER_TFB_NONE) {
      desc->setFragmentFunction(
        m_shader_library_frag->newFunction(m_fragment_function_name, values, &error));
      if (error) {
        MTL_LOG_ERROR("Compile Error - Metal Shader fragment function, error %s\n",
                      error->localizedDescription()->utf8String());

        /* Only exit out if genuine error and not warning */
        if (error->localizedDescription()
              ->rangeOfString(NS_STRING_("Compilation succeeded"), NS::CaseInsensitiveSearch)
              .location == NS::NotFound) {
          KLI_assert(false);
          return nullptr;
        }
      }
    } else {
      desc->setFragmentFunction(nil);
      desc->setRasterizationEnabled(false);
    }

    /* Setup pixel format state */
    for (int color_attachment = 0; color_attachment < GPU_FB_MAX_COLOR_ATTACHMENT;
         color_attachment++) {
      /* Fetch color attachment pixel format in back-end pipeline state. */
      MTL::PixelFormat pixel_format = current_state.color_attachment_format[color_attachment];
      /* Populate MTL API PSO attachment descriptor. */
      MTL::RenderPipelineColorAttachmentDescriptor *col_attachment =
        desc->colorAttachments()->object(color_attachment);

      col_attachment->setPixelFormat(pixel_format);
      if (pixel_format != MTL::PixelFormatInvalid) {
        bool format_supports_blending = mtl_format_supports_blending(pixel_format);

        col_attachment->setWriteMask(current_state.color_write_mask);
        col_attachment->setBlendingEnabled(current_state.blending_enabled &&
                                           format_supports_blending);
        if (format_supports_blending && current_state.blending_enabled) {
          col_attachment->setAlphaBlendOperation(current_state.alpha_blend_op);
          col_attachment->setRgbBlendOperation(current_state.rgb_blend_op);
          col_attachment->setDestinationAlphaBlendFactor(current_state.dest_alpha_blend_factor);
          col_attachment->setDestinationRGBBlendFactor(current_state.dest_rgb_blend_factor);
          col_attachment->setSourceAlphaBlendFactor(current_state.src_alpha_blend_factor);
          col_attachment->setSourceRGBBlendFactor(current_state.src_rgb_blend_factor);
        } else {
          if (current_state.blending_enabled && !format_supports_blending) {
            shader_debug_printf(
              "[Warning] Attempting to Bake PSO, but MTL::PixelFormat %d does not support "
              "blending\n",
              *((int *)&pixel_format));
          }
        }
      }
    }
    desc->setDepthAttachmentPixelFormat(current_state.depth_attachment_format);
    desc->setStencilAttachmentPixelFormat(current_state.stencil_attachment_format);

    /* Compile PSO */

    MTL::AutoreleasedRenderPipelineReflection reflection_data;
    MTL::RenderPipelineState *pso = ctx->device->newRenderPipelineState(
      desc,
      MTL::PipelineOptionBufferTypeInfo,
      &reflection_data,
      &error);
    if (error) {
      MTL_LOG_ERROR("Failed to create PSO for shader: %s error %s\n",
                    this->name,
                    error->localizedDescription()->utf8String());
      KLI_assert(false);
      return nullptr;
    } else if (!pso) {
      MTL_LOG_ERROR("Failed to create PSO for shader: %s, but no error was provided!\n",
                    this->name);
      KLI_assert(false);
      return nullptr;
    } else {
      MTL_LOG_ERROR("Successfully compiled PSO for shader: %s (Metal Context: %p)\n",
                    this->name,
                    ctx);
    }

    /* Prepare pipeline state instance. */
    MTLRenderPipelineStateInstance *pso_inst = new MTLRenderPipelineStateInstance();
    pso_inst->vert = desc->vertexFunction();
    pso_inst->frag = desc->fragmentFunction();
    pso_inst->pso = pso;
    pso_inst->base_uniform_buffer_index = MTL_uniform_buffer_base_index;
    pso_inst->null_attribute_buffer_index = (using_null_buffer) ? null_buffer_index : -1;
    pso_inst->transform_feedback_buffer_index = MTL_transform_feedback_buffer_index;
    pso_inst->shader_pso_index = m_pso_cache.size();

    pso_inst->reflection_data_available = (reflection_data != nil);
    if (reflection_data != nil) {
      /* Extract shader reflection data for buffer bindings.
       * This reflection data is used to contrast the binding information
       * we know about in the interface against the bindings in the finalized
       * PSO. This accounts for bindings which have been stripped out during
       * optimization, and allows us to both avoid over-binding and also
       * allows us to verify size-correctness for bindings, to ensure
       * that buffers bound are not smaller than the size of expected data. */
      NS::Array *vert_args = reflection_data->vertexArguments();

      pso_inst->buffer_bindings_reflection_data_vert.clear();
      int buffer_binding_max_ind = 0;

      for (int i = 0; i < vert_args->count(); i++) {
        MTL::Argument *arg = vert_args->object<MTL::Argument>(i);
        if (arg->type() == MTL::ArgumentTypeBuffer) {
          int buf_index = arg->index() - MTL_uniform_buffer_base_index;
          if (buf_index >= 0) {
            buffer_binding_max_ind = max_ii(buffer_binding_max_ind, buf_index);
          }
        }
      }
      pso_inst->buffer_bindings_reflection_data_vert.resize(buffer_binding_max_ind + 1);
      for (int i = 0; i < buffer_binding_max_ind + 1; i++) {
        pso_inst->buffer_bindings_reflection_data_vert[i] = {0, 0, 0, false};
      }

      for (int i = 0; i < vert_args->count(); i++) {
        MTL::Argument *arg = vert_args->object<MTL::Argument>(i);
        if (arg->type() == MTL::ArgumentTypeBuffer) {
          int buf_index = arg->index() - MTL_uniform_buffer_base_index;

          if (buf_index >= 0) {
            pso_inst->buffer_bindings_reflection_data_vert[buf_index] = {
              (uint32_t)(arg->index()),
              (uint32_t)(arg->bufferDataSize()),
              (uint32_t)(arg->bufferAlignment()),
              (arg->active() == YES) ? true : false};
          }
        }
      }

      NS::Array *frag_args = reflection_data->fragmentArguments();

      pso_inst->buffer_bindings_reflection_data_frag.clear();
      buffer_binding_max_ind = 0;

      for (int i = 0; i < frag_args->count(); i++) {
        MTL::Argument *arg = frag_args->object<MTL::Argument>(i);
        if (arg->type() == MTL::ArgumentTypeBuffer) {
          int buf_index = arg->index() - MTL_uniform_buffer_base_index;
          if (buf_index >= 0) {
            buffer_binding_max_ind = max_ii(buffer_binding_max_ind, buf_index);
          }
        }
      }
      pso_inst->buffer_bindings_reflection_data_frag.resize(buffer_binding_max_ind + 1);
      for (int i = 0; i < buffer_binding_max_ind + 1; i++) {
        pso_inst->buffer_bindings_reflection_data_frag[i] = {0, 0, 0, false};
      }

      for (int i = 0; i < frag_args->count(); i++) {
        MTL::Argument *arg = frag_args->object<MTL::Argument>(i);
        if (arg->type() == MTL::ArgumentTypeBuffer) {
          int buf_index = arg->index() - MTL_uniform_buffer_base_index;
          shader_debug_printf(" BUF IND: %d (arg name: %s)\n",
                              buf_index,
                              arg->name()->utf8String());
          if (buf_index >= 0) {
            pso_inst->buffer_bindings_reflection_data_frag[buf_index] = {
              (uint32_t)(arg->index()),
              (uint32_t)(arg->bufferDataSize()),
              (uint32_t)(arg->bufferAlignment()),
              (arg->active() == YES) ? true : false};
          }
        }
      }
    }

    pso_inst->vert->retain();
    pso_inst->frag->retain();
    pso_inst->pso->retain();

    /* Insert into pso cache. */
    m_pso_cache.add(pipeline_descriptor, pso_inst);
    shader_debug_printf("PSO CACHE: Stored new variant in PSO cache for shader '%s'\n",
                        this->name);
    return pso_inst;

    pool->drain();
    pool = nil;
  }
  /** @} */

  /* -------------------------------------------------------------------- */
  /** @name SSBO-vertex-fetch-mode attribute control.
   * @{ */

  int MTLShader::ssbo_vertex_type_to_attr_type(MTL::VertexFormat attribute_type)
  {
    switch (attribute_type) {
      case MTL::VertexFormatFloat:
        return GPU_SHADER_ATTR_TYPE_FLOAT;
      case MTL::VertexFormatInt:
        return GPU_SHADER_ATTR_TYPE_INT;
      case MTL::VertexFormatUInt:
        return GPU_SHADER_ATTR_TYPE_UINT;
      case MTL::VertexFormatShort:
        return GPU_SHADER_ATTR_TYPE_SHORT;
      case MTL::VertexFormatUChar:
        return GPU_SHADER_ATTR_TYPE_CHAR;
      case MTL::VertexFormatUChar2:
        return GPU_SHADER_ATTR_TYPE_CHAR2;
      case MTL::VertexFormatUChar3:
        return GPU_SHADER_ATTR_TYPE_CHAR3;
      case MTL::VertexFormatUChar4:
        return GPU_SHADER_ATTR_TYPE_CHAR4;
      case MTL::VertexFormatFloat2:
        return GPU_SHADER_ATTR_TYPE_VEC2;
      case MTL::VertexFormatFloat3:
        return GPU_SHADER_ATTR_TYPE_VEC3;
      case MTL::VertexFormatFloat4:
        return GPU_SHADER_ATTR_TYPE_VEC4;
      case MTL::VertexFormatUInt2:
        return GPU_SHADER_ATTR_TYPE_UVEC2;
      case MTL::VertexFormatUInt3:
        return GPU_SHADER_ATTR_TYPE_UVEC3;
      case MTL::VertexFormatUInt4:
        return GPU_SHADER_ATTR_TYPE_UVEC4;
      case MTL::VertexFormatInt2:
        return GPU_SHADER_ATTR_TYPE_IVEC2;
      case MTL::VertexFormatInt3:
        return GPU_SHADER_ATTR_TYPE_IVEC3;
      case MTL::VertexFormatInt4:
        return GPU_SHADER_ATTR_TYPE_IVEC4;
      case MTL::VertexFormatUCharNormalized:
        return GPU_SHADER_ATTR_TYPE_UCHAR_NORM;
      case MTL::VertexFormatUChar2Normalized:
        return GPU_SHADER_ATTR_TYPE_UCHAR2_NORM;
      case MTL::VertexFormatUChar3Normalized:
        return GPU_SHADER_ATTR_TYPE_UCHAR3_NORM;
      case MTL::VertexFormatUChar4Normalized:
        return GPU_SHADER_ATTR_TYPE_UCHAR4_NORM;
      case MTL::VertexFormatInt1010102Normalized:
        return GPU_SHADER_ATTR_TYPE_INT1010102_NORM;
      case MTL::VertexFormatShort3Normalized:
        return GPU_SHADER_ATTR_TYPE_SHORT3_NORM;
      default:
        KLI_assert_msg(false,
                       "Not yet supported attribute type for SSBO vertex fetch -- Add entry "
                       "GPU_SHADER_ATTR_TYPE_** to shader defines, and in this table");
        return -1;
    }
    return -1;
  }

  void MTLShader::ssbo_vertex_fetch_bind_attributes_begin()
  {
    MTLShaderInterface *mtl_interface = this->get_interface();
    m_ssbo_vertex_attribute_bind_active = true;
    m_ssbo_vertex_attribute_bind_mask = (1 << mtl_interface->get_total_attributes()) - 1;

    /* Reset tracking of actively used VBO bind slots for SSBO vertex fetch mode. */
    for (int i = 0; i < MTL_SSBO_VERTEX_FETCH_MAX_VBOS; i++) {
      m_ssbo_vbo_slot_used[i] = false;
    }
  }

  void MTLShader::ssbo_vertex_fetch_bind_attribute(const MTLSSBOAttribute &ssbo_attr)
  {
    /* Fetch attribute. */
    MTLShaderInterface *mtl_interface = this->get_interface();
    KLI_assert(ssbo_attr.mtl_attribute_index >= 0 &&
               ssbo_attr.mtl_attribute_index < mtl_interface->get_total_attributes());
    UNUSED_VARS_NDEBUG(mtl_interface);

    /* Update bind-mask to verify this attribute has been used. */
    KLI_assert((m_ssbo_vertex_attribute_bind_mask & (1 << ssbo_attr.mtl_attribute_index)) ==
                 (1 << ssbo_attr.mtl_attribute_index) &&
               "Attribute has already been bound");
    m_ssbo_vertex_attribute_bind_mask &= ~(1 << ssbo_attr.mtl_attribute_index);

    /* Fetch attribute uniform addresses from cache. */
    ShaderSSBOAttributeBinding &cached_ssbo_attribute =
      m_cached_ssbo_attribute_bindings[ssbo_attr.mtl_attribute_index];
    KLI_assert(cached_ssbo_attribute.attribute_index >= 0);

    /* Write attribute descriptor properties to shader uniforms. */
    this->uniform_int(cached_ssbo_attribute.uniform_offset, 1, 1, &ssbo_attr.attribute_offset);
    this->uniform_int(cached_ssbo_attribute.uniform_stride, 1, 1, &ssbo_attr.per_vertex_stride);
    int inst_val = (ssbo_attr.is_instance ? 1 : 0);
    this->uniform_int(cached_ssbo_attribute.uniform_fetchmode, 1, 1, &inst_val);
    this->uniform_int(cached_ssbo_attribute.uniform_vbo_id, 1, 1, &ssbo_attr.vbo_id);
    KLI_assert(ssbo_attr.attribute_format >= 0);
    this->uniform_int(cached_ssbo_attribute.uniform_attr_type, 1, 1, &ssbo_attr.attribute_format);
    m_ssbo_vbo_slot_used[ssbo_attr.vbo_id] = true;
  }

  void MTLShader::ssbo_vertex_fetch_bind_attributes_end(MTL::RenderCommandEncoder *active_encoder)
  {
    m_ssbo_vertex_attribute_bind_active = false;

    /* If our mask is non-zero, we have unassigned attributes. */
    if (m_ssbo_vertex_attribute_bind_mask != 0) {
      MTLShaderInterface *mtl_interface = this->get_interface();

      /* Determine if there is a free slot we can bind the null buffer to -- We should have at
       * least ONE free slot in this instance. */
      int null_attr_buffer_slot = -1;
      for (int i = 0; i < MTL_SSBO_VERTEX_FETCH_MAX_VBOS; i++) {
        if (!m_ssbo_vbo_slot_used[i]) {
          null_attr_buffer_slot = i;
          break;
        }
      }
      KLI_assert_msg(null_attr_buffer_slot >= 0,
                     "No suitable bind location for a NULL buffer was found");

      for (int i = 0; i < mtl_interface->get_total_attributes(); i++) {
        if (m_ssbo_vertex_attribute_bind_mask & (1 << i)) {
          const MTLShaderInputAttribute *mtl_shader_attribute = &mtl_interface->get_attribute(i);
#if MTL_DEBUG_SHADER_ATTRIBUTES == 1
          MTL_LOG_WARNING(
            "SSBO Vertex Fetch missing attribute with index: %d. Shader: %s, Attr "
            "Name: "
            "%s - Null buffer bound\n",
            i,
            this->name_get(),
            mtl_shader_attribute->name);
#endif
          /* Bind Attribute with NULL buffer index and stride zero (for constant access). */
          MTLSSBOAttribute ssbo_attr(i,
                                     null_attr_buffer_slot,
                                     0,
                                     0,
                                     GPU_SHADER_ATTR_TYPE_FLOAT,
                                     false);
          ssbo_vertex_fetch_bind_attribute(ssbo_attr);
          MTL_LOG_WARNING(
            "Unassigned Shader attribute: %s, Attr Name: %s -- Binding NULL BUFFER to "
            "slot %d\n",
            this->name_get(),
            mtl_interface->get_name_at_offset(mtl_shader_attribute->name_offset),
            null_attr_buffer_slot);
        }
      }

      /* Bind NULL buffer to given VBO slot. */
      MTLContext *ctx = reinterpret_cast<MTLContext *>(GPU_context_active_get());
      MTL::Buffer *null_buf = ctx->get_null_attribute_buffer();
      KLI_assert(null_buf);

      MTLRenderPassState &rps = ctx->main_command_buffer.get_render_pass_state();
      rps.bind_vertex_buffer(null_buf, 0, null_attr_buffer_slot);
    }
  }

  GPUVertBuf *MTLShader::get_transform_feedback_active_buffer()
  {
    if (m_transform_feedback_type == GPU_SHADER_TFB_NONE || !m_transform_feedback_active) {
      return nullptr;
    }
    return m_transform_feedback_vertbuf;
  }

  bool MTLShader::has_transform_feedback_varying(std::string str)
  {
    if (this->m_transform_feedback_type == GPU_SHADER_TFB_NONE) {
      return false;
    }

    return (std::find(m_tf_output_name_list.begin(), m_tf_output_name_list.end(), str) !=
            m_tf_output_name_list.end());
  }

}  // namespace kraken::gpu
