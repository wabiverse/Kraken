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
 *
 * GPUBackend derived class contain allocators that do not need a context bound.
 * The backend is init at startup and is accessible using GPU_backend_get()
 */

#include "GPU_vertex_buffer.h"

namespace kraken
{
  namespace gpu
  {

    class Context;

    class Batch;
    class DrawList;
    class FrameBuffer;
    class IndexBuf;
    class QueryPool;
    class Shader;
    class Texture;
    class UniformBuf;
    class StorageBuf;
    class VertBuf;

    class GPUBackend
    {
     public:

      virtual ~GPUBackend() = default;
      virtual void delete_resources() = 0;

      static GPUBackend *get();

      virtual void samplers_update() = 0;
      virtual void compute_dispatch(int groups_x_len, int groups_y_len, int groups_z_len) = 0;
      virtual void compute_dispatch_indirect(StorageBuf *indirect_buf) = 0;

      virtual Context *context_alloc(void *anchor_window, void *anchor_context) = 0;

      virtual Batch *batch_alloc() = 0;
      virtual DrawList *drawlist_alloc(int list_length) = 0;
      virtual FrameBuffer *framebuffer_alloc(const char *name) = 0;
      virtual IndexBuf *indexbuf_alloc() = 0;
      virtual QueryPool *querypool_alloc() = 0;
      virtual Shader *shader_alloc(const char *name) = 0;
      virtual Texture *texture_alloc(const char *name) = 0;
      virtual UniformBuf *uniformbuf_alloc(int size, const char *name) = 0;
      virtual StorageBuf *storagebuf_alloc(int size, GPUUsageType usage, const char *name) = 0;
      virtual VertBuf *vertbuf_alloc() = 0;

      /* Render Frame Coordination --
       * Used for performing per-frame actions globally */
      virtual void render_begin() = 0;
      virtual void render_end() = 0;
      virtual void render_step() = 0;
    };

  }  // namespace gpu
}  // namespace kraken