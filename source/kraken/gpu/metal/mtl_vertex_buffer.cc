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

#include "mtl_vertex_buffer.hh"
#include "mtl_debug.hh"

namespace kraken::gpu
{

  MTLVertBuf::MTLVertBuf() : VertBuf() {}

  MTLVertBuf::~MTLVertBuf()
  {
    this->release_data();
  }

  void MTLVertBuf::acquire_data()
  {
    /* Discard previous data, if any. */
    MEM_SAFE_FREE(data);
    if (usage_ == GPU_USAGE_DEVICE_ONLY) {
      data = nullptr;
    } else {
      data = (uchar *)MEM_mallocN(sizeof(uchar) * this->size_alloc_get(), __func__);
    }
  }

  void MTLVertBuf::resize_data()
  {
    if (usage_ == GPU_USAGE_DEVICE_ONLY) {
      data = nullptr;
    } else {
      data = (uchar *)MEM_reallocN(data, sizeof(uchar) * this->size_alloc_get());
    }
  }

  void MTLVertBuf::release_data()
  {
    if (m_vbo != nullptr) {
      m_vbo->free();
      m_vbo = nullptr;
      m_is_wrapper = false;
    }

    GPU_TEXTURE_FREE_SAFE(m_buffer_texture);

    MEM_SAFE_FREE(data);
  }

  void MTLVertBuf::duplicate_data(VertBuf *dst_)
  {
    KLI_assert(MTLContext::get() != NULL);
    MTLVertBuf *src = this;
    MTLVertBuf *dst = static_cast<MTLVertBuf *>(dst_);

    /* Ensure buffer has been initialized. */
    src->bind();

    if (src->m_vbo) {

      /* Fetch active context. */
      MTLContext *ctx = MTLContext::get();
      KLI_assert(ctx);

      /* Ensure destination does not have an active VBO. */
      KLI_assert(dst->m_vbo == nullptr);

      /* Allocate VBO for destination vertbuf. */
      uint length = src->m_vbo->get_size();
      dst->m_vbo = MTLContext::get_global_memory_manager().allocate(
        length,
        (dst->get_usage_type() != GPU_USAGE_DEVICE_ONLY));
      dst->m_alloc_size = length;

      /* Fetch Metal buffer handles. */
      MTL::Buffer *src_buffer = src->m_vbo->get_metal_buffer();
      MTL::Buffer *dest_buffer = dst->m_vbo->get_metal_buffer();

      /* Use blit encoder to copy data to duplicate buffer allocation. */
      MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
      if (G.debug & G_DEBUG_GPU) {
        enc->insertDebugSignpost(NS_STRING_("VertexBufferDuplicate"));
      }
      enc->copyFromBuffer(src_buffer, 0, dest_buffer, 0, length);

      /* Flush results back to host buffer, if one exists. */
      if (dest_buffer->storageMode() == MTL::StorageModeManaged) {
        enc->synchronizeResource(dest_buffer);
      }

      if (G.debug & G_DEBUG_GPU) {
        enc->insertDebugSignpost(NS_STRING_("VertexBufferDuplicateEnd"));
      }

      /* Mark as in-use, as contents are updated via GPU command. */
      src->flag_used();
    }

    /* Copy raw CPU data. */
    if (data != nullptr) {
      dst->data = (uchar *)MEM_dupallocN(src->data);
    }
  }

  void MTLVertBuf::upload_data()
  {
    this->bind();
  }

  void MTLVertBuf::bind()
  {
    /* Determine allocation size. Set minimum allocation size to be
     * the maximal of a single attribute to avoid validation and
     * correctness errors. */
    uint64_t required_size_raw = sizeof(uchar) * this->size_used_get();
    uint64_t required_size = max_ulul(required_size_raw, 128);

    if (required_size_raw == 0) {
      MTL_LOG_WARNING("Warning: Vertex buffer required_size = 0\n");
    }

    /* If the vertex buffer has already been allocated, but new data is ready,
     * or the usage size has changed, we release the existing buffer and
     * allocate a new buffer to ensure we do not overwrite in-use GPU resources.
     *
     * NOTE: We only need to free the existing allocation if contents have been
     * submitted to the GPU. Otherwise we can simply upload new data to the
     * existing buffer, if it will fit.
     *
     * NOTE: If a buffer is re-sized, but no new data is provided, the previous
     * contents are copied into the newly allocated buffer. */
    bool requires_reallocation = (m_vbo != nullptr) && (m_alloc_size != required_size);
    bool new_data_ready = (this->flag & GPU_VERTBUF_DATA_DIRTY) && this->data;

    gpu::MTLBuffer *prev_vbo = nullptr;
    GPUVertBufStatus prev_flag = this->flag;

    if (m_vbo != nullptr) {
      if (requires_reallocation || (new_data_ready && m_contents_in_flight)) {
        /* Track previous VBO to copy data from. */
        prev_vbo = m_vbo;

        /* Reset current allocation status. */
        m_vbo = nullptr;
        m_is_wrapper = false;
        m_alloc_size = 0;

        /* Flag as requiring data upload. */
        if (requires_reallocation) {
          this->flag &= ~GPU_VERTBUF_DATA_UPLOADED;
        }
      }
    }

    /* Create MTLBuffer of requested size. */
    if (m_vbo == nullptr) {
      m_vbo = MTLContext::get_global_memory_manager().allocate(
        required_size,
        (this->get_usage_type() != GPU_USAGE_DEVICE_ONLY));
      m_vbo->set_label(NS_STRING_("Vertex Buffer"));
      KLI_assert(m_vbo != nullptr);
      KLI_assert(m_vbo->get_metal_buffer() != nil);

      m_is_wrapper = false;
      m_alloc_size = required_size;
      m_contents_in_flight = false;
    }

    /* Upload new data, if provided. */
    if (new_data_ready) {

      /* Only upload data if usage size is greater than zero.
       * Do not upload data for device-only buffers. */
      if (required_size_raw > 0 && usage_ != GPU_USAGE_DEVICE_ONLY) {

        /* Debug: Verify allocation is large enough. */
        KLI_assert(m_vbo->get_size() >= required_size_raw);

        /* Fetch mapped buffer host ptr and upload data. */
        void *dst_data = m_vbo->get_host_ptr();
        memcpy((uint8_t *)dst_data, this->data, required_size_raw);
        m_vbo->flush_range(0, required_size_raw);
      }

      /* If static usage, free host-side data. */
      if (usage_ == GPU_USAGE_STATIC) {
        MEM_SAFE_FREE(data);
      }

      /* Flag data as having been uploaded. */
      this->flag &= ~GPU_VERTBUF_DATA_DIRTY;
      this->flag |= GPU_VERTBUF_DATA_UPLOADED;
    } else if (requires_reallocation) {

      /* If buffer has been re-sized, copy existing data if host
       * data had been previously uploaded. */
      KLI_assert(prev_vbo != nullptr);

      if (prev_flag & GPU_VERTBUF_DATA_UPLOADED) {

        /* Fetch active context. */
        MTLContext *ctx = MTLContext::get();
        KLI_assert(ctx);

        MTL::Buffer *copy_prev_buffer = prev_vbo->get_metal_buffer();
        MTL::Buffer *copy_new_buffer = m_vbo->get_metal_buffer();
        KLI_assert(copy_prev_buffer != nil);
        KLI_assert(copy_new_buffer != nil);

        /* Ensure a blit command encoder is active for buffer copy operation. */
        MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
        enc->copyFromBuffer(copy_prev_buffer,
                            0,
                            copy_new_buffer,
                            0,
                            min_ii(copy_new_buffer->length(), copy_prev_buffer->length()));

        /* Flush newly copied data back to host-side buffer, if one exists.
         * Ensures data and cache coherency for managed MTLBuffers. */
        if (copy_new_buffer->storageMode() == MTL::StorageModeManaged) {
          enc->synchronizeResource(copy_new_buffer);
        }

        /* For VBOs flagged as static, release host data as it will no longer be needed. */
        if (usage_ == GPU_USAGE_STATIC) {
          MEM_SAFE_FREE(data);
        }

        /* Flag data as uploaded. */
        this->flag |= GPU_VERTBUF_DATA_UPLOADED;

        /* Flag as in-use, as contents have been updated via GPU commands. */
        this->flag_used();
      }
    }

    /* Release previous buffer if re-allocated. */
    if (prev_vbo != nullptr) {
      prev_vbo->free();
    }

    /* Ensure buffer has been created. */
    KLI_assert(m_vbo != nullptr);
  }

  /* Update Sub currently only used by hair */
  void MTLVertBuf::update_sub(uint start, uint len, const void *data)
  {
    /* Fetch and verify active context. */
    MTLContext *ctx = reinterpret_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);
    KLI_assert(ctx->device);

    /* Ensure vertbuf has been created. */
    this->bind();
    KLI_assert(start + len <= m_alloc_size);

    /* Create temporary scratch buffer allocation for sub-range of data. */
    MTLTemporaryBuffer scratch_allocation =
      ctx->get_scratchbuffer_manager().scratch_buffer_allocate_range_aligned(len, 256);
    memcpy(scratch_allocation.data, data, len);
    scratch_allocation.metal_buffer->didModifyRange(
      NS::Range::Make(scratch_allocation.buffer_offset, len));
    MTL::Buffer *data_buffer = scratch_allocation.metal_buffer;
    uint data_buffer_offset = scratch_allocation.buffer_offset;

    KLI_assert(m_vbo != nullptr && data != nullptr);
    KLI_assert((start + len) <= m_vbo->get_size());

    /* Fetch destination buffer. */
    MTL::Buffer *dst_buffer = m_vbo->get_metal_buffer();

    /* Ensure blit command encoder for copying data. */
    MTL::BlitCommandEncoder *enc = ctx->main_command_buffer.ensure_begin_blit_encoder();
    enc->copyFromBuffer(data_buffer, data_buffer_offset, dst_buffer, start, len);

    /* Flush modified buffer back to host buffer, if one exists. */
    if (dst_buffer->storageMode() == MTL::StorageModeManaged) {
      enc->synchronizeResource(dst_buffer);
    }
  }

  void MTLVertBuf::bind_as_ssbo(uint binding)
  {
    /* TODO(Metal): Support binding of buffers as SSBOs.
     * Pending overall compute support for Metal backend. */
    MTL_LOG_WARNING("MTLVertBuf::bind_as_ssbo not yet implemented!\n");
    this->flag_used();
  }

  void MTLVertBuf::bind_as_texture(uint binding)
  {
    /* Ensure allocations are ready, and data uploaded. */
    this->bind();
    KLI_assert(m_vbo != nullptr);

    /* If vertex buffer updated, release existing texture and re-create. */
    MTL::Buffer *buf = this->get_metal_buffer();
    if (m_buffer_texture != nullptr) {
      gpu::MTLTexture *mtl_buffer_tex = static_cast<gpu::MTLTexture *>(
        unwrap(this->m_buffer_texture));
      MTL::Buffer *tex_buf = mtl_buffer_tex->get_vertex_buffer();
      if (tex_buf != buf) {
        GPU_TEXTURE_FREE_SAFE(m_buffer_texture);
        m_buffer_texture = nullptr;
      }
    }

    /* Create texture from vertex buffer. */
    if (m_buffer_texture == nullptr) {
      m_buffer_texture = GPU_texture_create_from_vertbuf("vertbuf_as_texture", wrap(this));
    }

    /* Verify successful creation and bind. */
    KLI_assert(m_buffer_texture != nullptr);
    GPU_texture_bind(m_buffer_texture, binding);
  }

  const void *MTLVertBuf::read() const
  {
    KLI_assert(m_vbo != nullptr);
    KLI_assert(usage_ != GPU_USAGE_DEVICE_ONLY);
    void *return_ptr = m_vbo->get_host_ptr();
    KLI_assert(return_ptr != nullptr);

    return return_ptr;
  }

  void *MTLVertBuf::unmap(const void *mapped_data) const
  {
    void *result = MEM_mallocN(m_alloc_size, __func__);
    memcpy(result, mapped_data, m_alloc_size);
    return result;
  }

  void MTLVertBuf::wrap_handle(uint64_t handle)
  {
    KLI_assert(m_vbo == nullptr);

    /* Attempt to cast to Metal buffer handle. */
    KLI_assert(handle != 0);
    MTL::Buffer *buffer = reinterpret_cast<MTL::Buffer *>((void *)handle);

    m_is_wrapper = true;
    m_vbo = new gpu::MTLBuffer(buffer);

    /* We assume the data is already on the device, so no need to allocate or send it. */
    flag = GPU_VERTBUF_DATA_UPLOADED;
  }

  void MTLVertBuf::flag_used()
  {
    m_contents_in_flight = true;
  }

}  // namespace kraken::gpu
