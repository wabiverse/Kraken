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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#pragma once

#include "kraken/kraken.h"

#ifdef __cplusplus
#  include <Foundation/Foundation.hpp>
#  include <Metal/Metal.hpp>
#  include <QuartzCore/QuartzCore.hpp>
#endif /* __cplusplus */

class AnchorDrawData;

KRAKEN_NAMESPACE_BEGIN

namespace gpu
{
  /**
   * @METAL: Buffer.
   * A wrapper around a Metal Buffer object that knows the last time it was
   * reused. */
  class MTLBackendBuffer
  {
   public:

    MTLBackendBuffer *init(MTL::Buffer *buffer);

    MTL::Buffer *buffer;
    double lastReuseTime;
  };

  /**
   * @METAL: Framebuffer.
   * An object that encapsulates the data necessary to uniquely identify a
   * render pipeline state. These are used as cache keys. */
  class MTLBackendFramebufferDescriptor
  {
   public:

    MTLBackendFramebufferDescriptor *init(MTL::RenderPassDescriptor *desc);

    unsigned long sampleCount;
    MTL::PixelFormat colorPixelFormat;
    MTL::PixelFormat depthPixelFormat;
    MTL::PixelFormat stencilPixelFormat;
  };

  /**
   * @METAL: Backend.
   * A lazy evaluated, correctly destroyed, thread-safe singleton that stores
   * long-lived objects that are needed by the Metal renderer backend. Stores
   * the render pipeline state cache and the default font texture, and manages
   * the reusable buffer cache. */
  class MTLBackend
  {
   public:

    typedef std::vector<MTLBackendBuffer *> MTLBackendBufferCache;
    typedef wabi::TfHashMap<MTLBackendFramebufferDescriptor *, MTL::RenderPipelineState *> MTLRenderPipelineStateCache;

    static MTLBackend &getInstance()
    {
      static MTLBackend instance;
      return instance;
    }

    MTLBackendBuffer *dequeueReusableBuffer(NS::UInteger length, MTL::Device *device);
    MTL::RenderPipelineState *newRenderPipelineState(MTLBackendFramebufferDescriptor *descriptor,
                                                     MTL::Device *device);

   private:

    MTLBackend();

   public:

    MTLBackend(MTLBackend const &) = delete;
    void operator=(MTLBackend const &) = delete;

    MTL::Device *device;
    MTL::DepthStencilState *depthStencilState;

    // framebuffer descriptor for current frame; transient
    MTLBackendFramebufferDescriptor *framebufferDescriptor;
    // pipeline cache; keyed on framebuffer descriptors
    MTLRenderPipelineStateCache renderPipelineStateCache;
    MTL::Texture *fontTexture;
    MTLBackendBufferCache bufferCache;
    double lastBufferCachePurge;
  };

  struct MTLContext
  {
    MTLContext();
  };

  MTLContext *CreateContext();

  MTLContext *GetContext();

  void DestroyContext();

  static inline CFTimeInterval GetMachAbsoluteTimeInSeconds()
  {
    return static_cast<CFTimeInterval>(
      static_cast<double>(clock_gettime_nsec_np(CLOCK_UPTIME_RAW)) / 1e9);
  }

  bool InitContext(MTL::Device *device);

  void FreeContext();

  void NewFrame(MTL::RenderPassDescriptor *desc);

  void ViewUpdate(AnchorDrawData *drawData,
                  MTL::CommandBuffer *commandBuffer,
                  MTL::RenderCommandEncoder *commandEncoder,
                  MTL::RenderPipelineState *renderPipelineState,
                  MTLBackendBuffer *vertexBuffer,
                  size_t vertexBufferOffset);

  void ViewDraw(AnchorDrawData *drawData,
                MTL::CommandBuffer *commandBuffer,
                MTL::RenderCommandEncoder *commandEncoder);

  bool CreateFonts(MTL::Device *device);

  void DestroyFonts();

  bool CreateResources(MTL::Device *device);

  void DestroyResources();
}  // namespace gpu

KRAKEN_NAMESPACE_END