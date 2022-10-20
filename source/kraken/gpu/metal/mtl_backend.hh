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
 * @ingroup GPU.
 * Pixel Magic.
 *
 * The Kraken GPU backend - for Apple Metal.
 */

#include "KLI_vector.hh"

#include "gpu_backend.hh"
#include "mtl_capabilities.hh"

namespace kraken::gpu
{

  class Batch;
  class DrawList;
  class FrameBuffer;
  class QueryPool;
  class Shader;
  class UniformBuf;
  class VertBuf;
  class MTLContext;

  class MTLBackend : public GPUBackend
  {
    friend class MTLContext;

   public:

    /* Capabilities. */
    static MTLCapabilities capabilities;

    static MTLCapabilities &get_capabilities()
    {
      return MTLBackend::capabilities;
    }

    ~MTLBackend()
    {
      MTLBackend::platform_exit();
    }

    void delete_resources() override
    {
      /* Delete any resources with context active. */
    }

    static bool metal_is_supported();
    static MTLBackend *get()
    {
      return static_cast<MTLBackend *>(GPUBackend::get());
    }

    void samplers_update() override;
    void compute_dispatch(int groups_x_len, int groups_y_len, int groups_z_len) override
    {
      /* Placeholder */
    }

    void compute_dispatch_indirect(StorageBuf *indirect_buf) override
    {
      /* Placeholder */
    }

    /* MTL Allocators need to be implemented in separate .mm files, due to allocation of
     * Objective-C objects. */
    Context *context_alloc(void *anchor_window, void *anchor_context) override;
    Batch *batch_alloc() override;
    DrawList *drawlist_alloc(int list_length) override;
    FrameBuffer *framebuffer_alloc(const char *name) override;
    IndexBuf *indexbuf_alloc() override;
    QueryPool *querypool_alloc() override;
    Shader *shader_alloc(const char *name) override;
    Texture *texture_alloc(const char *name) override;
    UniformBuf *uniformbuf_alloc(int size, const char *name) override;
    StorageBuf *storagebuf_alloc(int size, GPUUsageType usage, const char *name) override;
    VertBuf *vertbuf_alloc() override;

    /* Render Frame Coordination. */
    void render_begin() override;
    void render_end() override;
    void render_step() override;
    bool is_inside_render_boundary();

   private:

    static void platform_init(MTLContext *ctx);
    static void platform_exit();

    static void capabilities_init(MTLContext *ctx);
  };

}  // namespace kraken::gpu
