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

#include "MEM_guardedalloc.h"

#include "GPU_vertex_buffer.h"
#include "gpu_vertex_buffer_private.hh"
#include "mtl_context.hh"

namespace kraken::gpu
{

  class MTLVertBuf : public VertBuf
  {
    friend class gpu::MTLTexture; /* For buffer texture. */
    friend class MTLShader;       /* For transform feedback. */
    friend class MTLBatch;
    friend class MTLContext; /* For transform feedback. */

   private:

    /** Metal buffer allocation. **/
    gpu::MTLBuffer *m_vbo = nullptr;
    /** Texture used if the buffer is bound as buffer texture. Init on first use. */
    struct ::GPUTexture *m_buffer_texture = nullptr;
    /** Defines whether the buffer handle is wrapped by this MTLVertBuf, i.e. we do not own it and
     * should not free it. */
    bool m_is_wrapper = false;
    /** Requested allocation size for Metal buffer.
     * Differs from raw buffer size as alignment is not included. */
    uint64_t m_alloc_size = 0;
    /** Whether existing allocation has been submitted for use by the GPU. */
    bool m_contents_in_flight = false;

    /* Fetch Metal buffer and offset into allocation if necessary.
     * Access limited to friend classes. */
    MTL::Buffer *get_metal_buffer()
    {
      m_vbo->debug_ensure_used();
      return m_vbo->get_metal_buffer();
    }

   public:

    MTLVertBuf();
    ~MTLVertBuf();

    void bind();
    void flag_used();

    void update_sub(uint start, uint len, const void *data) override;

    const void *read() const override;
    void *unmap(const void *mapped_data) const override;

    void wrap_handle(uint64_t handle) override;

   protected:

    void acquire_data() override;
    void resize_data() override;
    void release_data() override;
    void upload_data() override;
    void duplicate_data(VertBuf *dst) override;
    void bind_as_ssbo(uint binding) override;
    void bind_as_texture(uint binding) override;

    MEM_CXX_CLASS_ALLOC_FUNCS("MTLVertBuf");
  };

}  // namespace kraken::gpu
