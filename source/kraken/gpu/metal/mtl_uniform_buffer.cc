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

#include "KLI_string.h"

#include "gpu_backend.hh"
#include "gpu_context_private.hh"

#include "mtl_backend.hh"
#include "mtl_context.hh"
#include "mtl_debug.hh"
#include "mtl_uniform_buffer.hh"

namespace kraken::gpu
{

  MTLUniformBuf::MTLUniformBuf(size_t size, const char *name) : UniformBuf(size, name) {}

  MTLUniformBuf::~MTLUniformBuf()
  {
    if (m_metal_buffer != nullptr) {
      m_metal_buffer->free();
      m_metal_buffer = nullptr;
    }
    m_has_data = false;

    /* Ensure UBO is not bound to active CTX.
     * UBO bindings are reset upon Context-switch so we do not need
     * to check deactivated context's. */
    MTLContext *ctx = MTLContext::get();
    if (ctx) {
      for (int i = 0; i < MTL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
        MTLUniformBufferBinding &slot = ctx->pipeline_state.ubo_bindings[i];
        if (slot.bound && slot.ubo == this) {
          slot.bound = false;
          slot.ubo = nullptr;
        }
      }
    }
  }

  void MTLUniformBuf::update(const void *data)
  {
    KLI_assert(this);
    KLI_assert(m_size_in_bytes > 0);

    /* Free existing allocation.
     * The previous UBO resource will be tracked by the memory manager,
     * in case dependent GPU work is still executing. */
    if (m_metal_buffer != nullptr) {
      m_metal_buffer->free();
      m_metal_buffer = nullptr;
    }

    /* Allocate MTL buffer */
    MTLContext *ctx = static_cast<MTLContext *>(unwrap(GPU_context_active_get()));
    KLI_assert(ctx);
    KLI_assert(ctx->device);
    UNUSED_VARS_NDEBUG(ctx);

    if (data != nullptr) {
      m_metal_buffer = MTLContext::get_global_memory_manager().allocate_with_data(m_size_in_bytes,
                                                                                  true,
                                                                                  data);
      m_has_data = true;

      m_metal_buffer->set_label(NS_STRING_("Uniform Buffer"));
      KLI_assert(m_metal_buffer != nullptr);
      KLI_assert(m_metal_buffer->get_metal_buffer() != nil);
    } else {
      /* If data is not yet present, no buffer will be allocated and MTLContext will use an empty
       * null buffer, containing zeroes, if the UBO is bound. */
      m_metal_buffer = nullptr;
      m_has_data = false;
    }
  }

  void MTLUniformBuf::bind(int slot)
  {
    if (slot < 0) {
      MTL_LOG_WARNING("Failed to bind UBO %p. uniform location %d invalid.\n", this, slot);
      return;
    }

    KLI_assert(slot < MTL_MAX_UNIFORM_BUFFER_BINDINGS);

    /* Bind current UBO to active context. */
    MTLContext *ctx = MTLContext::get();
    KLI_assert(ctx);

    MTLUniformBufferBinding &ctx_ubo_bind_slot = ctx->pipeline_state.ubo_bindings[slot];
    ctx_ubo_bind_slot.ubo = this;
    ctx_ubo_bind_slot.bound = true;

    m_bind_slot = slot;
    m_bound_ctx = ctx;

    /* Check if we have any deferred data to upload. */
    if (m_data != nullptr) {
      this->update(m_data);
      MEM_SAFE_FREE(m_data);
    }

    /* Ensure there is at least an empty dummy buffer. */
    if (m_metal_buffer == nullptr) {
      this->update(nullptr);
    }
  }

  void MTLUniformBuf::unbind()
  {
    /* Unbind in debug mode to validate missing binds.
     * Otherwise, only perform a full unbind upon destruction
     * to ensure no lingering references. */
#ifndef NDEBUG
    if (true) {
#else
    if (G.debug & G_DEBUG_GPU) {
#endif
      if (m_bound_ctx != nullptr && m_bind_slot > -1) {
        MTLUniformBufferBinding &ctx_ubo_bind_slot =
          m_bound_ctx->pipeline_state.ubo_bindings[m_bind_slot];
        if (ctx_ubo_bind_slot.bound && ctx_ubo_bind_slot.ubo == this) {
          ctx_ubo_bind_slot.bound = false;
          ctx_ubo_bind_slot.ubo = nullptr;
        }
      }
    }

    /* Reset bind index. */
    m_bind_slot = -1;
    m_bound_ctx = nullptr;
  }

  MTL::Buffer *MTLUniformBuf::get_metal_buffer(int *r_offset)
  {
    KLI_assert(this);
    *r_offset = 0;
    if (m_metal_buffer != nullptr && m_has_data) {
      *r_offset = 0;
      m_metal_buffer->debug_ensure_used();
      return m_metal_buffer->get_metal_buffer();
    } else {
      *r_offset = 0;
      return nil;
    }
  }

  int MTLUniformBuf::get_size()
  {
    KLI_assert(this);
    return m_size_in_bytes;
  }

}  // namespace kraken::gpu
