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
#include "gpu_uniform_buffer_private.hh"

#include "mtl_context.hh"

namespace kraken::gpu
{

  /**
   * Implementation of Uniform Buffers using Metal.
   **/
  class MTLUniformBuf : public UniformBuf
  {
   private:

    /* Allocation Handle. */
    gpu::MTLBuffer *m_metal_buffer = nullptr;

    /* Whether buffer has contents, if false, no GPU buffer will
     * have yet been allocated. */
    bool m_has_data = false;

    /* Bind-state tracking. */
    int m_bind_slot = -1;
    MTLContext *m_bound_ctx = nullptr;

   public:

    MTLUniformBuf(size_t size, const char *name);
    ~MTLUniformBuf();

    void update(const void *data) override;
    void bind(int slot) override;
    void unbind() override;

    MTL::Buffer *get_metal_buffer(int *r_offset);
    int get_size();
    const char *get_name()
    {
      return m_name;
    }

    MEM_CXX_CLASS_ALLOC_FUNCS("MTLUniformBuf");
  };

}  // namespace kraken::gpu
