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

#include "kraken/kraken.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

// A wrapper around a MTLBuffer object that knows the last time it was reused
@interface MTLBackendBuffer : NSObject
@property (nonatomic, strong) id<MTLBuffer> buffer;
@property (nonatomic, assign) double        lastReuseTime;
- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer;
@end

// An object that encapsulates the data necessary to uniquely identify a
// render pipeline state. These are used as cache keys.
@interface MTLBackendFramebufferDescriptor : NSObject<NSCopying>
@property (nonatomic, assign) unsigned long  sampleCount;
@property (nonatomic, assign) MTLPixelFormat colorPixelFormat;
@property (nonatomic, assign) MTLPixelFormat depthPixelFormat;
@property (nonatomic, assign) MTLPixelFormat stencilPixelFormat;
- (instancetype)initWithRenderPassDescriptor:(MTLRenderPassDescriptor*)renderPassDescriptor;
@end

// A singleton that stores long-lived objects that are needed by the Metal
// renderer backend. Stores the render pipeline state cache and the default
// font texture, and manages the reusable buffer cache.
@interface MTLBackend : NSObject
@property (nonatomic, strong) id<MTLDevice>                      device;
@property (nonatomic, strong) id<MTLDepthStencilState>           depthStencilState;
@property (nonatomic, strong) MTLBackendFramebufferDescriptor*   framebufferDescriptor; // framebuffer descriptor for current frame; transient
@property (nonatomic, strong) NSMutableDictionary*               renderPipelineStateCache; // pipeline cache; keyed on framebuffer descriptors
@property (nonatomic, strong, nullable) id<MTLTexture>           fontTexture;
@property (nonatomic, strong) NSMutableArray<MTLBackendBuffer*>* bufferCache;
@property (nonatomic, assign) double                             lastBufferCachePurge;
- (MTLBackendBuffer*)dequeueReusableBufferOfLength:(NSUInteger)length device:(id<MTLDevice>)device;
- (id<MTLRenderPipelineState>)renderPipelineStateForFramebufferDescriptor:(MTLBackendFramebufferDescriptor*)descriptor device:(id<MTLDevice>)device;
@end

class AnchorDrawData;

KRAKEN_NAMESPACE_BEGIN

namespace gpu
{
  struct MTLContext
  {
    MTLContext();

    MTLBackend* Shared;
  };

  MTLContext* CreateContext();

  MTLContext* GetContext();

  void DestroyContext();

  static inline CFTimeInterval GetMachAbsoluteTimeInSeconds()       
  { 
    return static_cast<CFTimeInterval>(static_cast<double>(clock_gettime_nsec_np(CLOCK_UPTIME_RAW)) / 1e9); 
  }

  bool InitContext(id<MTLDevice> device);

  void FreeContext();

  void NewFrame(MTLRenderPassDescriptor *desc);

  void ViewUpdate(AnchorDrawData* drawData,
                  id<MTLCommandBuffer> commandBuffer,
                  id<MTLRenderCommandEncoder> commandEncoder, 
                  id<MTLRenderPipelineState> renderPipelineState,
                  MTLBackendBuffer* vertexBuffer, 
                  size_t vertexBufferOffset);

  void ViewDraw(AnchorDrawData* drawData, 
                id<MTLCommandBuffer> commandBuffer, 
                id<MTLRenderCommandEncoder> commandEncoder);

  bool CreateFonts(id<MTLDevice> device);

  void DestroyFonts();

  bool CreateResources(id<MTLDevice> device);

  void DestroyResources();
}

KRAKEN_NAMESPACE_END