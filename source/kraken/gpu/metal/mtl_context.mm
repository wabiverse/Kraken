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

#include "ANCHOR_api.h"
#include "wabi/base/tf/tf.h"

#import "mtl_context.hh"

KRAKEN_NAMESPACE_BEGIN

namespace gpu {

MTLContext::MTLContext()
{ 
  memset(this, 0, sizeof(*this));
}

MTLContext* CreateContext()
{ 
  return ANCHOR_NEW(MTLContext)();
}

MTLContext* GetContext()     
{ 
  return ANCHOR::GetCurrentContext() 
    ? (MTLContext*)ANCHOR::GetIO().BackendRendererUserData 
    : NULL; 
}

void DestroyContext() 
{ 
  ANCHOR_DELETE(GetContext());
}

bool InitContext(id<MTLDevice> device)
{
  MTLContext* ctx = CreateContext();
  AnchorIO& io = ANCHOR::GetIO();
  io.BackendRendererUserData = (void*)ctx;
  io.BackendRendererName = "kraken.gpu.metal";
  io.BackendFlags |= AnchorBackendFlags_RendererHasVtxOffset;

  ctx->Shared = [[MTLBackend alloc] init];
  ctx->Shared.device = device;
}

void FreeContext()
{
  DestroyResources();
  DestroyContext();
}

void NewFrame(MTLRenderPassDescriptor *desc)
{
  MTLContext* ctx = GetContext();
  ANCHOR_ASSERT(ctx->Shared != nil && "No Metal context. Did you call InitContext() ?");
  ctx->Shared.framebufferDescriptor = [[MTLBackendFramebufferDescriptor alloc] initWithRenderPassDescriptor:desc];

  if (ctx->Shared.depthStencilState == nil)
    CreateResources(ctx->Shared.device);
}

void ViewUpdate(AnchorDrawData* drawData,
                id<MTLCommandBuffer> commandBuffer,
                id<MTLRenderCommandEncoder> commandEncoder, 
                id<MTLRenderPipelineState> renderPipelineState,
                MTLBackendBuffer* vertexBuffer, 
                size_t vertexBufferOffset)
{
  TF_UNUSED(commandBuffer);
  MTLContext* ctx = GetContext();
  [commandEncoder setCullMode:MTLCullModeNone];
  [commandEncoder setDepthStencilState:ctx->Shared.depthStencilState];

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from drawData->DisplayPos (top left) to
  // drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
  MTLViewport viewport =
  {
    .originX = 0.0,
    .originY = 0.0,
    .width = (double)(drawData->DisplaySize[0] * drawData->FramebufferScale[0]),
    .height = (double)(drawData->DisplaySize[1] * drawData->FramebufferScale[1]),
    .znear = 0.0,
    .zfar = 1.0
  };
  [commandEncoder setViewport:viewport];

  float L = drawData->DisplayPos[0];
  float R = drawData->DisplayPos[0] + drawData->DisplaySize[0];
  float T = drawData->DisplayPos[1];
  float B = drawData->DisplayPos[1] + drawData->DisplaySize[1];
  float N = (float)viewport.znear;
  float F = (float)viewport.zfar;
  const float ortho_projection[4][4] =
  {
      { 2.0f/(R-L),   0.0f,           0.0f,   0.0f },
      { 0.0f,         2.0f/(T-B),     0.0f,   0.0f },
      { 0.0f,         0.0f,        1/(F-N),   0.0f },
      { (R+L)/(L-R),  (T+B)/(B-T), N/(F-N),   1.0f },
  };
  [commandEncoder setVertexBytes:&ortho_projection length:sizeof(ortho_projection) atIndex:1];

  [commandEncoder setRenderPipelineState:renderPipelineState];

  [commandEncoder setVertexBuffer:vertexBuffer.buffer offset:0 atIndex:0];
  [commandEncoder setVertexBufferOffset:vertexBufferOffset atIndex:0];
}

void ViewDraw(AnchorDrawData* drawData, 
              id<MTLCommandBuffer> commandBuffer, 
              id<MTLRenderCommandEncoder> commandEncoder)
{
  MTLContext* ctx = GetContext();
  MTLBackend* bd = ctx->Shared;

  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
  int fb_width = (int)(drawData->DisplaySize[0] * drawData->FramebufferScale[0]);
  int fb_height = (int)(drawData->DisplaySize[1] * drawData->FramebufferScale[1]);
  if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
    return;

  // Try to retrieve a render pipeline state that is compatible with the framebuffer config for this frame
  // The hit rate for this cache should be very near 100%.
  id<MTLRenderPipelineState> renderPipelineState = bd.renderPipelineStateCache[bd.framebufferDescriptor];
  if (renderPipelineState == nil)
  {
    // No luck; make a new render pipeline state
    renderPipelineState = [bd renderPipelineStateForFramebufferDescriptor:bd.framebufferDescriptor device:commandBuffer.device];

    // Cache render pipeline state for later reuse
    bd.renderPipelineStateCache[bd.framebufferDescriptor] = renderPipelineState;
  }

  size_t vertexBufferLength = (size_t)drawData->TotalVtxCount * sizeof(AnchorDrawVert);
  size_t indexBufferLength = (size_t)drawData->TotalIdxCount * sizeof(AnchorDrawIdx);
  MTLBackendBuffer* vertexBuffer = [bd dequeueReusableBufferOfLength:vertexBufferLength device:commandBuffer.device];
  MTLBackendBuffer* indexBuffer = [bd dequeueReusableBufferOfLength:indexBufferLength device:commandBuffer.device];

  ViewUpdate(drawData, commandBuffer, commandEncoder, renderPipelineState, vertexBuffer, 0);

  // Will project scissor/clipping rectangles into framebuffer space
  wabi::GfVec2f clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
  wabi::GfVec2f clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

  // Render command lists
  size_t vertexBufferOffset = 0;
  size_t indexBufferOffset = 0;
  for (int n = 0; n < drawData->CmdListsCount; n++)
  {
    const AnchorDrawList* cmd_list = drawData->CmdLists[n];

    memcpy((char*)vertexBuffer.buffer.contents + vertexBufferOffset, cmd_list->VtxBuffer.Data, (size_t)cmd_list->VtxBuffer.Size * sizeof(AnchorDrawVert));
    memcpy((char*)indexBuffer.buffer.contents + indexBufferOffset, cmd_list->IdxBuffer.Data, (size_t)cmd_list->IdxBuffer.Size * sizeof(AnchorDrawIdx));

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    {
      const AnchorDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback)
      {
        // User callback, registered via ImDrawList::AddCallback()
        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == AnchorDrawCallback_ResetRenderState)
          ViewUpdate(drawData, commandBuffer, commandEncoder, renderPipelineState, vertexBuffer, vertexBufferOffset);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      }
      else
      {
        // Project scissor/clipping rectangles into framebuffer space
        wabi::GfVec2f clip_min((pcmd->ClipRect[0] - clip_off[0]) * clip_scale[0], (pcmd->ClipRect[1] - clip_off[1]) * clip_scale[1]);
        wabi::GfVec2f clip_max((pcmd->ClipRect[2] - clip_off[0]) * clip_scale[0], (pcmd->ClipRect[3] - clip_off[1]) * clip_scale[1]);

        // Clamp to viewport as setScissorRect() won't accept values that are off bounds
        if (clip_min[0] < 0.0f) { clip_min[0] = 0.0f; }
        if (clip_min[1] < 0.0f) { clip_min[1] = 0.0f; }
        if (clip_max[0] > fb_width) { clip_max[0] = (float)fb_width; }
        if (clip_max[1] > fb_height) { clip_max[1] = (float)fb_height; }
        if (clip_max[0] <= clip_min[0] || clip_max[1] <= clip_min[1])
            continue;
        if (pcmd->ElemCount == 0)
            continue;

        // Apply scissor/clipping rectangle
        MTLScissorRect scissorRect =
        {
          .x = NSUInteger(clip_min[0]),
          .y = NSUInteger(clip_min[1]),
          .width = NSUInteger(clip_max[0] - clip_min[0]),
          .height = NSUInteger(clip_max[1] - clip_min[1])
        };
        [commandEncoder setScissorRect:scissorRect];

        // Bind texture, Draw
        if (AnchorTextureID tex_id = pcmd->GetTexID())
          [commandEncoder setFragmentTexture:(__bridge id<MTLTexture>)(tex_id) atIndex:0];

        [commandEncoder setVertexBufferOffset:(vertexBufferOffset + pcmd->VtxOffset * sizeof(AnchorDrawVert)) atIndex:0];
        [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                   indexCount:pcmd->ElemCount
                                    indexType:sizeof(AnchorDrawIdx) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                                  indexBuffer:indexBuffer.buffer
                            indexBufferOffset:indexBufferOffset + pcmd->IdxOffset * sizeof(AnchorDrawIdx)];
      }
  }

    vertexBufferOffset += (size_t)cmd_list->VtxBuffer.Size * sizeof(AnchorDrawVert);
    indexBufferOffset += (size_t)cmd_list->IdxBuffer.Size * sizeof(AnchorDrawIdx);
  }

  [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>)
  {
    dispatch_async(dispatch_get_main_queue(), ^{
      MTLContext* ctx = GetContext();
      if (ctx != NULL)
      {
          @synchronized(ctx->Shared.bufferCache)
          {
              [ctx->Shared.bufferCache addObject:vertexBuffer];
              [ctx->Shared.bufferCache addObject:indexBuffer];
          }
      }
    });
  }];
}

bool CreateFonts(id<MTLDevice> device)
{
  MTLContext* ctx = GetContext();
  AnchorIO& io = ANCHOR::GetIO();

  // We are retrieving and uploading the font atlas as a 4-channels RGBA texture here.
  // In theory we could call GetTexDataAsAlpha8() and upload a 1-channel texture to save on memory access bandwidth.
  // However, using a shader designed for 1-channel texture would make it less obvious to use the ImTextureID facility to render users own textures.
  // You can make that change in your implementation.
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                width:(NSUInteger)width
                                                                                              height:(NSUInteger)height
                                                                                            mipmapped:NO];
  textureDescriptor.usage = MTLTextureUsageShaderRead;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
  textureDescriptor.storageMode = MTLStorageModeManaged;
#else
  textureDescriptor.storageMode = MTLStorageModeShared;
#endif
  id <MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];
  [texture replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)width, (NSUInteger)height) mipmapLevel:0 withBytes:pixels bytesPerRow:(NSUInteger)width * 4];
  ctx->Shared.fontTexture = texture;
  io.Fonts->SetTexID((__bridge void*)ctx->Shared.fontTexture); // ImTextureID == void*

  return (ctx->Shared.fontTexture != nil);
}

void DestroyFonts()
{
  MTLContext* ctx = GetContext();
  AnchorIO& io = ANCHOR::GetIO();
  ctx->Shared.fontTexture = nil;
  io.Fonts->SetTexID(nullptr);
}

bool CreateResources(id<MTLDevice> device)
{
  MTLContext* ctx = GetContext();
  MTLDepthStencilDescriptor* depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
  depthStencilDescriptor.depthWriteEnabled = NO;
  depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
  ctx->Shared.depthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
  CreateFonts(device);

  return true;
}

void DestroyResources()
{
  MTLContext* ctx = GetContext();
  DestroyFonts();
  [ctx->Shared.renderPipelineStateCache removeAllObjects];
}

} /* gpu */

KRAKEN_NAMESPACE_END


using namespace kraken;
using namespace kraken::gpu;

#pragma mark - MTLBackendBuffer implementation

@implementation MTLBackendBuffer
- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer
{
  if ((self = [super init]))
  {
    _buffer = buffer;
    _lastReuseTime = GetMachAbsoluteTimeInSeconds();
  }
  return self;
}
@end

#pragma mark - MTLBackendFramebufferDescriptor implementation

@implementation MTLBackendFramebufferDescriptor
- (instancetype)initWithRenderPassDescriptor:(MTLRenderPassDescriptor*)renderPassDescriptor
{
  if ((self = [super init]))
  {
    _sampleCount = renderPassDescriptor.colorAttachments[0].texture.sampleCount;
    _colorPixelFormat = renderPassDescriptor.colorAttachments[0].texture.pixelFormat;
    _depthPixelFormat = renderPassDescriptor.depthAttachment.texture.pixelFormat;
    _stencilPixelFormat = renderPassDescriptor.stencilAttachment.texture.pixelFormat;
  }
  return self;
}

- (nonnull id)copyWithZone:(nullable NSZone*)zone
{
  MTLBackendFramebufferDescriptor* copy = [[MTLBackendFramebufferDescriptor allocWithZone:zone] init];
  copy.sampleCount = self.sampleCount;
  copy.colorPixelFormat = self.colorPixelFormat;
  copy.depthPixelFormat = self.depthPixelFormat;
  copy.stencilPixelFormat = self.stencilPixelFormat;
  return copy;
}

- (NSUInteger)hash
{
  NSUInteger sc = _sampleCount & 0x3;
  NSUInteger cf = _colorPixelFormat & 0x3FF;
  NSUInteger df = _depthPixelFormat & 0x3FF;
  NSUInteger sf = _stencilPixelFormat & 0x3FF;
  NSUInteger hash = (sf << 22) | (df << 12) | (cf << 2) | sc;
  return hash;
}

- (BOOL)isEqual:(id)object
{
  MTLBackendFramebufferDescriptor* other = object;
  if (![other isKindOfClass:[MTLBackendFramebufferDescriptor class]])
    return NO;
  return other.sampleCount == self.sampleCount      &&
  other.colorPixelFormat   == self.colorPixelFormat &&
  other.depthPixelFormat   == self.depthPixelFormat &&
  other.stencilPixelFormat == self.stencilPixelFormat;
}

@end

#pragma mark - MTLBackend implementation

@implementation MTLBackend
- (instancetype)init
{
  if ((self = [super init]))
  {
    self.renderPipelineStateCache = [NSMutableDictionary dictionary];
    self.bufferCache = [NSMutableArray array];
    _lastBufferCachePurge = GetMachAbsoluteTimeInSeconds();
  }
  return self;
}

- (MTLBackendBuffer*)dequeueReusableBufferOfLength:(NSUInteger)length device:(id<MTLDevice>)device
{
  uint64_t now = GetMachAbsoluteTimeInSeconds();

  @synchronized(self.bufferCache)
  {
    // Purge old buffers that haven't been useful for a while
    if (now - self.lastBufferCachePurge > 1.0)
    {
      NSMutableArray* survivors = [NSMutableArray array];
      for (MTLBackendBuffer* candidate in self.bufferCache)
        if (candidate.lastReuseTime > self.lastBufferCachePurge)
          [survivors addObject:candidate];
      self.bufferCache = [survivors mutableCopy];
      self.lastBufferCachePurge = now;
    }

    // See if we have a buffer we can reuse
    MTLBackendBuffer* bestCandidate = nil;
    for (MTLBackendBuffer* candidate in self.bufferCache)
      if (candidate.buffer.length >= length && (bestCandidate == nil || bestCandidate.lastReuseTime > candidate.lastReuseTime))
        bestCandidate = candidate;

    if (bestCandidate != nil)
    {
      [self.bufferCache removeObject:bestCandidate];
      bestCandidate.lastReuseTime = now;
      return bestCandidate;
    }
  }

  // No luck; make a new buffer
  id<MTLBuffer> backing = [device newBufferWithLength:length options:MTLResourceStorageModeShared];
  return [[MTLBackendBuffer alloc] initWithBuffer:backing];
}

// Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling.
- (id<MTLRenderPipelineState>)renderPipelineStateForFramebufferDescriptor:(MTLBackendFramebufferDescriptor*)descriptor device:(id<MTLDevice>)device
{
  NSError* error = nil;

  NSString* shaderSource = @""
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "\n"
  "struct Uniforms {\n"
  "    float4x4 projectionMatrix;\n"
  "};\n"
  "\n"
  "struct VertexIn {\n"
  "    float2 position  [[attribute(0)]];\n"
  "    float2 texCoords [[attribute(1)]];\n"
  "    uchar4 color     [[attribute(2)]];\n"
  "};\n"
  "\n"
  "struct VertexOut {\n"
  "    float4 position [[position]];\n"
  "    float2 texCoords;\n"
  "    float4 color;\n"
  "};\n"
  "\n"
  "vertex VertexOut vertex_main(VertexIn in                 [[stage_in]],\n"
  "                             constant Uniforms &uniforms [[buffer(1)]]) {\n"
  "    VertexOut out;\n"
  "    out.position = uniforms.projectionMatrix * float4(in.position, 0, 1);\n"
  "    out.texCoords = in.texCoords;\n"
  "    out.color = float4(in.color) / float4(255.0);\n"
  "    return out;\n"
  "}\n"
  "\n"
  "fragment half4 fragment_main(VertexOut in [[stage_in]],\n"
  "                             texture2d<half, access::sample> texture [[texture(0)]]) {\n"
  "    constexpr sampler linearSampler(coord::normalized, min_filter::linear, mag_filter::linear, mip_filter::linear);\n"
  "    half4 texColor = texture.sample(linearSampler, in.texCoords);\n"
  "    return half4(in.color) * texColor;\n"
  "}\n";

  id<MTLLibrary> library = [device newLibraryWithSource:shaderSource options:nil error:&error];
  if (library == nil)
  {
    NSLog(@"Error: failed to create Metal library: %@", error);
    return nil;
  }

  id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
  id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

  if (vertexFunction == nil || fragmentFunction == nil)
  {
    NSLog(@"Error: failed to find Metal shader functions in library: %@", error);
    return nil;
  }

  MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
  vertexDescriptor.attributes[0].offset = ANCHOR_OFFSETOF(AnchorDrawVert, pos);
  vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2; // position
  vertexDescriptor.attributes[0].bufferIndex = 0;
  vertexDescriptor.attributes[1].offset = ANCHOR_OFFSETOF(AnchorDrawVert, uv);
  vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2; // texCoords
  vertexDescriptor.attributes[1].bufferIndex = 0;
  vertexDescriptor.attributes[2].offset = ANCHOR_OFFSETOF(AnchorDrawVert, col);
  vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4; // color
  vertexDescriptor.attributes[2].bufferIndex = 0;
  vertexDescriptor.layouts[0].stepRate = 1;
  vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  vertexDescriptor.layouts[0].stride = sizeof(AnchorDrawVert);

  MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
  pipelineDescriptor.vertexFunction = vertexFunction;
  pipelineDescriptor.fragmentFunction = fragmentFunction;
  pipelineDescriptor.vertexDescriptor = vertexDescriptor;
  pipelineDescriptor.rasterSampleCount = self.framebufferDescriptor.sampleCount;
  pipelineDescriptor.colorAttachments[0].pixelFormat = self.framebufferDescriptor.colorPixelFormat;
  pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
  pipelineDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
  pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
  pipelineDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
  pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
  pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
  pipelineDescriptor.depthAttachmentPixelFormat = self.framebufferDescriptor.depthPixelFormat;
  pipelineDescriptor.stencilAttachmentPixelFormat = self.framebufferDescriptor.stencilPixelFormat;

  id<MTLRenderPipelineState> renderPipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
  if (error != nil)
    NSLog(@"Error: failed to create Metal pipeline state: %@", error);

  return renderPipelineState;
}

@end