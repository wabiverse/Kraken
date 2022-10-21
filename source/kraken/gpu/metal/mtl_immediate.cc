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

#include "KKE_global.h"

#include "GPU_vertex_format.h"
#include "gpu_context_private.hh"
#include "gpu_shader_private.hh"
#include "gpu_vertex_format_private.h"

#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_immediate.hh"
#include "mtl_primitive.hh"
#include "mtl_shader.hh"

namespace kraken::gpu
{

  MTLImmediate::MTLImmediate(MTLContext *ctx)
  {
    m_context = ctx;
  }

  MTLImmediate::~MTLImmediate() {}

  uchar *MTLImmediate::begin()
  {
    KLI_assert(!m_has_begun);

    /* Determine primitive type. */
    m_metal_primitive_type = gpu_prim_type_to_metal(this->prim_type);
    m_metal_primitive_mode = mtl_prim_type_to_topology_class(m_metal_primitive_type);
    m_has_begun = true;

    /* Allocate a range of data and return host-accessible pointer. */
    const size_t bytes_needed = vertex_buffer_size(&vertex_format, vertex_len);
    m_current_allocation = m_context->get_scratchbuffer_manager()
                             .scratch_buffer_allocate_range_aligned(bytes_needed, 256);
    m_current_allocation.metal_buffer->retain();
    return reinterpret_cast<uchar *>(m_current_allocation.data);
  }

  void MTLImmediate::end()
  {
    /* Ensure we're between a `imm::begin` / `imm:end` pair. */
    KLI_assert(m_has_begun);
    KLI_assert(prim_type != GPU_PRIM_NONE);

    /* Verify context is valid, vertex data is written and a valid shader is bound. */
    if (m_context && this->vertex_idx > 0 && this->shader) {

      MTLShader *active_mtl_shader = static_cast<MTLShader *>(unwrap(shader));

      /* Skip draw if Metal shader is not valid. */
      if (active_mtl_shader == nullptr || !active_mtl_shader->is_valid() ||
          active_mtl_shader->get_interface() == nullptr) {

        const char *ptr = (active_mtl_shader) ? active_mtl_shader->name_get() : nullptr;
        MTL_LOG_WARNING(
          "MTLImmediate::end -- cannot perform draw as active shader is NULL or invalid (likely "
          "unimplemented) (shader %p '%s')\n",
          active_mtl_shader,
          ptr);
        return;
      }

      /* Ensure we are inside a render pass and fetch active RenderCommandEncoder. */
      MTL::RenderCommandEncoder *rec = m_context->ensure_begin_render_pass();
      KLI_assert(rec != nil);

      /* Fetch active render pipeline state. */
      MTLRenderPassState &rps = m_context->main_command_buffer.get_render_pass_state();

      /* Bind Shader. */
      GPU_shader_bind(this->shader);

      /* Debug markers for frame-capture and detailed error messages. */
      if (G.debug & G_DEBUG_GPU) {
        char debug_msg[64];

        snprintf(debug_msg,
                 sizeof(debug_msg),
                 "immEnd(verts: % d, shader: % s)",
                 this->vertex_idx,
                 active_mtl_shader->get_interface()->get_name());

        rec->pushDebugGroup(NS_STRING_(debug_msg));
        rec->insertDebugSignpost(NS_STRING_(debug_msg));
      }

      /* Populate pipeline state vertex descriptor. */
      MTLStateManager *state_manager = static_cast<MTLStateManager *>(
        MTLContext::get()->state_manager);
      MTLRenderPipelineStateDescriptor &desc = state_manager->get_pipeline_descriptor();
      const MTLShaderInterface *interface = active_mtl_shader->get_interface();

      /* Reset vertex descriptor to default state. */
      desc.reset_vertex_descriptor();

      desc.vertex_descriptor.num_attributes = interface->get_total_attributes();
      desc.vertex_descriptor.num_vert_buffers = 1;

      for (int i = 0; i < desc.vertex_descriptor.num_attributes; i++) {
        desc.vertex_descriptor.attributes[i].format = MTL::VertexFormatInvalid;
      }
      desc.vertex_descriptor.uses_ssbo_vertex_fetch =
        active_mtl_shader->get_uses_ssbo_vertex_fetch();
      desc.vertex_descriptor.num_ssbo_attributes = 0;

      /* SSBO Vertex Fetch -- Verify Attributes. */
      if (active_mtl_shader->get_uses_ssbo_vertex_fetch()) {
        active_mtl_shader->ssbo_vertex_fetch_bind_attributes_begin();

        /* Disable Indexed rendering in SSBO vertex fetch. */
        int uniform_ssbo_use_indexed = active_mtl_shader->uni_ssbo_uses_indexed_rendering;
        KLI_assert_msg(uniform_ssbo_use_indexed != -1,
                       "Expected valid uniform location for ssbo_uses_indexed_rendering.");
        int uses_indexed_rendering = 0;
        active_mtl_shader->uniform_int(uniform_ssbo_use_indexed, 1, 1, &uses_indexed_rendering);
      }

      /* Populate Vertex descriptor and verify attributes.
       * TODO(Metal): Cache this vertex state based on Vertex format and shaders. */
      for (int i = 0; i < interface->get_total_attributes(); i++) {

        /* NOTE: Attribute in VERTEX FORMAT does not necessarily share the same array index as
         * attributes in shader interface. */
        GPUVertAttr *attr = nullptr;
        const MTLShaderInputAttribute &mtl_shader_attribute = interface->get_attribute(i);

        /* Scan through vertex_format attributes until one with a name matching the shader
         * interface is found. */
        for (uint32_t a_idx = 0; a_idx < this->vertex_format.attr_len && attr == nullptr;
             a_idx++) {
          GPUVertAttr *check_attribute = &this->vertex_format.attrs[a_idx];

          /* Attributes can have multiple name aliases associated with them. */
          for (uint32_t n_idx = 0; n_idx < check_attribute->name_len; n_idx++) {
            const char *name = GPU_vertformat_attr_name_get(&this->vertex_format,
                                                            check_attribute,
                                                            n_idx);

            if (strcmp(name, interface->get_name_at_offset(mtl_shader_attribute.name_offset)) ==
                0) {
              attr = check_attribute;
              break;
            }
          }
        }

        KLI_assert_msg(attr != nullptr,
                       "Could not find expected attribute in immediate mode vertex format.");
        if (attr == nullptr) {
          MTL_LOG_ERROR(
            "MTLImmediate::end Could not find matching attribute '%s' from Shader Interface in "
            "Vertex Format! - TODO: Bind Dummy attribute\n",
            interface->get_name_at_offset(mtl_shader_attribute.name_offset));
          return;
        }

        /* Determine whether implicit type conversion between input vertex format
         * and shader interface vertex format is supported. */
        MTL::VertexFormat convertedFormat;
        bool can_use_implicit_conversion = mtl_convert_vertex_format(
          mtl_shader_attribute.format,
          (GPUVertCompType)attr->comp_type,
          attr->comp_len,
          (GPUVertFetchMode)attr->fetch_mode,
          &convertedFormat);

        if (can_use_implicit_conversion) {
          /* Metal API can implicitly convert some formats during vertex assembly:
           * - Converting from a normalized short2 format to float2
           * - Type truncation e.g. Float4 to Float2.
           * - Type expansion from Float3 to Float4.
           * - Note: extra components are filled with the corresponding components of (0,0,0,1).
           * (See
           * https://developer.apple.com/documentation/metal/mtlvertexattributedescriptor/1516081-format)
           */
          bool is_floating_point_format = (attr->comp_type == GPU_COMP_F32);
          desc.vertex_descriptor.attributes[i].format = convertedFormat;
          desc.vertex_descriptor.attributes[i].format_conversion_mode =
            (is_floating_point_format) ? (GPUVertFetchMode)GPU_FETCH_FLOAT :
                                         (GPUVertFetchMode)GPU_FETCH_INT;
          KLI_assert(convertedFormat != MTL::VertexFormatInvalid);
        } else {
          /* Some conversions are NOT valid, e.g. Int4 to Float4
           * - In this case, we need to implement a conversion routine inside the shader.
           * - This is handled using the format_conversion_mode flag
           * - This flag is passed into the PSO as a function specialization,
           *   and will generate an appropriate conversion function when reading the vertex
           * attribute value into local shader storage. (If no explicit conversion is needed, the
           * function specialize to a pass-through). */
          MTL::VertexFormat converted_format;
          bool can_convert = mtl_vertex_format_resize(mtl_shader_attribute.format,
                                                      attr->comp_len,
                                                      &converted_format);
          desc.vertex_descriptor.attributes[i].format = (can_convert) ?
                                                          converted_format :
                                                          mtl_shader_attribute.format;
          desc.vertex_descriptor.attributes[i].format_conversion_mode = (GPUVertFetchMode)
                                                                          attr->fetch_mode;
          KLI_assert(desc.vertex_descriptor.attributes[i].format != MTL::VertexFormatInvalid);
        }
        /* Using attribute offset in vertex format, as this will be correct */
        desc.vertex_descriptor.attributes[i].offset = attr->offset;
        desc.vertex_descriptor.attributes[i].buffer_index = mtl_shader_attribute.buffer_index;

        /* SSBO Vertex Fetch Attribute bind. */
        if (active_mtl_shader->get_uses_ssbo_vertex_fetch()) {
          KLI_assert_msg(mtl_shader_attribute.buffer_index == 0,
                         "All attributes should be in buffer index zero");
          MTLSSBOAttribute ssbo_attr(
            mtl_shader_attribute.index,
            mtl_shader_attribute.buffer_index,
            attr->offset,
            this->vertex_format.stride,
            MTLShader::ssbo_vertex_type_to_attr_type(desc.vertex_descriptor.attributes[i].format),
            false);
          desc.vertex_descriptor.ssbo_attributes[desc.vertex_descriptor.num_ssbo_attributes] =
            ssbo_attr;
          desc.vertex_descriptor.num_ssbo_attributes++;
          active_mtl_shader->ssbo_vertex_fetch_bind_attribute(ssbo_attr);
        }
      }

      /* Buffer bindings for singular vertex buffer. */
      desc.vertex_descriptor.buffer_layouts[0].step_function = MTL::VertexStepFunctionPerVertex;
      desc.vertex_descriptor.buffer_layouts[0].step_rate = 1;
      desc.vertex_descriptor.buffer_layouts[0].stride = this->vertex_format.stride;
      KLI_assert(this->vertex_format.stride > 0);

      /* SSBO Vertex Fetch -- Verify Attributes. */
      if (active_mtl_shader->get_uses_ssbo_vertex_fetch()) {
        active_mtl_shader->ssbo_vertex_fetch_bind_attributes_end(rec);

        /* Set Status uniforms. */
        KLI_assert_msg(active_mtl_shader->uni_ssbo_input_prim_type_loc != -1,
                       "ssbo_input_prim_type uniform location invalid!");
        KLI_assert_msg(active_mtl_shader->uni_ssbo_input_vert_count_loc != -1,
                       "ssbo_input_vert_count uniform location invalid!");
        GPU_shader_uniform_vector_int(reinterpret_cast<GPUShader *>(wrap(active_mtl_shader)),
                                      active_mtl_shader->uni_ssbo_input_prim_type_loc,
                                      1,
                                      1,
                                      (const int *)(&this->prim_type));
        GPU_shader_uniform_vector_int(reinterpret_cast<GPUShader *>(wrap(active_mtl_shader)),
                                      active_mtl_shader->uni_ssbo_input_vert_count_loc,
                                      1,
                                      1,
                                      (const int *)(&this->vertex_idx));
      }

      MTL::PrimitiveType mtl_prim_type = gpu_prim_type_to_metal(this->prim_type);
      if (m_context->ensure_render_pipeline_state(mtl_prim_type)) {

        /* Issue draw call. */
        KLI_assert(this->vertex_idx > 0);

        /* Metal API does not support triangle fan, so we can emulate this
         * input data by generating an index buffer to re-map indices to
         * a TriangleList.
         *
         * NOTE(Metal): Consider caching generated triangle fan index buffers.
         * For immediate mode, generating these is currently very cheap, as we use
         * fast scratch buffer allocations. Though we may benefit from caching of
         * frequently used buffer sizes. */
        if (mtl_needs_topology_emulation(this->prim_type)) {

          /* Debug safety check for SSBO FETCH MODE. */
          if (active_mtl_shader->get_uses_ssbo_vertex_fetch()) {
            KLI_assert(false && "Topology emulation not supported with SSBO Vertex Fetch mode");
          }

          /* Emulate Tri-fan. */
          if (this->prim_type == GPU_PRIM_TRI_FAN) {
            /* Prepare Triangle-Fan emulation index buffer on CPU based on number of input
             * vertices. */
            uint32_t base_vert_count = this->vertex_idx;
            uint32_t num_triangles = max_ii(base_vert_count - 2, 0);
            uint32_t fan_index_count = num_triangles * 3;
            KLI_assert(num_triangles > 0);

            uint32_t alloc_size = sizeof(uint32_t) * fan_index_count;
            uint32_t *index_buffer = nullptr;

            MTLTemporaryBuffer allocation = m_context->get_scratchbuffer_manager()
                                              .scratch_buffer_allocate_range_aligned(alloc_size,
                                                                                     128);
            index_buffer = (uint32_t *)allocation.data;

            int a = 0;
            for (int i = 0; i < num_triangles; i++) {
              index_buffer[a++] = 0;
              index_buffer[a++] = i + 1;
              index_buffer[a++] = i + 2;
            }

            NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

            MTL::Buffer *index_buffer_mtl = nil;
            uint32_t index_buffer_offset = 0;

            /* Region of scratch buffer used for topology emulation element data.
             * NOTE(Metal): We do not need to manually flush as the entire scratch
             * buffer for current command buffer is flushed upon submission. */
            index_buffer_mtl = allocation.metal_buffer;
            index_buffer_offset = allocation.buffer_offset;

            /* Set depth stencil state (requires knowledge of primitive type). */
            m_context->ensure_depth_stencil_state(MTL::PrimitiveTypeTriangle);

            /* Bind Vertex Buffer. */
            rps.bind_vertex_buffer(m_current_allocation.metal_buffer,
                                   m_current_allocation.buffer_offset,
                                   0);

            /* Draw. */
            rec->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                       fan_index_count,
                                       MTL::IndexTypeUInt32,
                                       index_buffer_mtl,
                                       index_buffer_offset);

            pool->drain();
            pool = nil;

          } else {
            /* TODO(Metal): Topology emulation for line loop.
             * NOTE(Metal): This is currently not used anywhere and modified at the high
             * level for efficiency in such cases. */
            KLI_assert_msg(false, "LineLoop requires emulation support in immediate mode.");
          }
        } else {
          MTL::PrimitiveType primitive_type = m_metal_primitive_type;
          int vertex_count = this->vertex_idx;

          /* Bind Vertex Buffer. */
          rps.bind_vertex_buffer(m_current_allocation.metal_buffer,
                                 m_current_allocation.buffer_offset,
                                 0);

          /* Set depth stencil state (requires knowledge of primitive type). */
          m_context->ensure_depth_stencil_state(primitive_type);

          if (active_mtl_shader->get_uses_ssbo_vertex_fetch()) {

            /* Bind Null Buffers for empty/missing bind slots. */
            MTL::Buffer *null_buffer = m_context->get_null_buffer();
            KLI_assert(null_buffer != nil);
            for (int i = 1; i < MTL_SSBO_VERTEX_FETCH_MAX_VBOS; i++) {

              /* We only need to ensure a buffer is bound to the context, its contents do not
               * matter as it will not be used. */
              if (rps.cached_vertex_buffer_bindings[i].metal_buffer == nil) {
                rps.bind_vertex_buffer(null_buffer, 0, i);
              }
            }

            /* SSBO vertex fetch - Nullify elements buffer. */
            if (rps.cached_vertex_buffer_bindings[MTL_SSBO_VERTEX_FETCH_IBO_INDEX].metal_buffer ==
                nil) {
              rps.bind_vertex_buffer(null_buffer, 0, MTL_SSBO_VERTEX_FETCH_IBO_INDEX);
            }

            /* Submit draw call with modified vertex count, which reflects vertices per primitive
             * defined in the USE_SSBO_VERTEX_FETCH `pragma`. */
            int num_input_primitives = gpu_get_prim_count_from_type(vertex_count, this->prim_type);
            int output_num_verts = num_input_primitives *
                                   active_mtl_shader->get_ssbo_vertex_fetch_output_num_verts();
#ifndef NDEBUG
            KLI_assert(
              mtl_vertex_count_fits_primitive_type(
                output_num_verts,
                active_mtl_shader->get_ssbo_vertex_fetch_output_prim_type()) &&
              "Output Vertex count is not compatible with the requested output vertex primitive "
              "type");
#endif
            rec->drawPrimitives(active_mtl_shader->get_ssbo_vertex_fetch_output_prim_type(),
                                NS::UInteger(0),
                                NS::UInteger(output_num_verts));
            m_context->main_command_buffer.register_draw_counters(output_num_verts);
          } else {
            /* Regular draw. */
            rec->drawPrimitives(primitive_type, NS::UInteger(0), NS::UInteger(vertex_count));
            m_context->main_command_buffer.register_draw_counters(vertex_count);
          }
        }
      }
      if (G.debug & G_DEBUG_GPU) {
        rec->popDebugGroup();
      }
    }

    /* Reset allocation after draw submission. */
    m_has_begun = false;
    if (m_current_allocation.metal_buffer) {
      m_current_allocation.metal_buffer->release();
      m_current_allocation.metal_buffer = nil;
    }
  }

}  // namespace kraken::gpu
