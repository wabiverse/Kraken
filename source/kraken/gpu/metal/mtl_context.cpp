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

#include "mtl_context.hh"

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

bool InitContext(MTL::Device *device)
{
  MTLContext* ctx = CreateContext();
  AnchorIO& io = ANCHOR::GetIO();
  io.BackendRendererUserData = (void*)ctx;
  io.BackendRendererName = "kraken.gpu.metal";
  io.BackendFlags |= AnchorBackendFlags_RendererHasVtxOffset;

  MTLBackend::getInstance().device = device;
}

void FreeContext()
{
  DestroyResources();
  DestroyContext();
}

void NewFrame(MTL::RenderPassDescriptor *desc)
{
  MTLContext* ctx = GetContext();
  ANCHOR_ASSERT(MTLBackend::getInstance().device != nullptr && "No Metal context. Did you call InitContext() ?");
  MTLBackend::getInstance().framebufferDescriptor = (new MTLBackendFramebufferDescriptor())->init(desc);

  if (MTLBackend::getInstance().depthStencilState == nil)
    CreateResources(MTLBackend::getInstance().device);
}

void ViewUpdate(AnchorDrawData* drawData,
                MTL::CommandBuffer *commandBuffer,
                MTL::RenderCommandEncoder *commandEncoder, 
                MTL::RenderPipelineState *renderPipelineState,
                MTLBackendBuffer* vertexBuffer, 
                size_t vertexBufferOffset)
{
  TF_UNUSED(commandBuffer);
  MTLContext* ctx = GetContext();
  commandEncoder->setCullMode(MTL::CullModeNone);
  commandEncoder->setDepthStencilState(MTLBackend::getInstance().depthStencilState);

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from drawData->DisplayPos (top left) to
  // drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
  MTL::Viewport viewport =
  {
    .originX = 0.0,
    .originY = 0.0,
    .width = (double)(drawData->DisplaySize[0] * drawData->FramebufferScale[0]),
    .height = (double)(drawData->DisplaySize[1] * drawData->FramebufferScale[1]),
    .znear = 0.0,
    .zfar = 1.0
  };
  commandEncoder->setViewport(viewport);

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
  commandEncoder->setVertexBytes(&ortho_projection, sizeof(ortho_projection), 1);

  commandEncoder->setRenderPipelineState(renderPipelineState);

  commandEncoder->setVertexBuffer(vertexBuffer->buffer, 0, 0);
  commandEncoder->setVertexBufferOffset(vertexBufferOffset, 0);
}

void ViewDraw(AnchorDrawData* drawData, 
              MTL::CommandBuffer *commandBuffer, 
              MTL::RenderCommandEncoder *commandEncoder)
{
  MTLContext* ctx = GetContext();
  MTLBackend* bd = &MTLBackend::getInstance();

  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
  int fb_width = (int)(drawData->DisplaySize[0] * drawData->FramebufferScale[0]);
  int fb_height = (int)(drawData->DisplaySize[1] * drawData->FramebufferScale[1]);
  if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
    return;

  // Try to retrieve a render pipeline state that is compatible with the framebuffer config for this frame
  // The hit rate for this cache should be very near 100%.
  MTL::RenderPipelineState *renderPipelineState = bd->renderPipelineStateCache[bd->framebufferDescriptor];
  if (renderPipelineState == nil)
  {
    // No luck; make a new render pipeline state
    renderPipelineState = bd->newRenderPipelineState(bd->framebufferDescriptor, commandBuffer->device());

    // Cache render pipeline state for later reuse
    bd->renderPipelineStateCache.insert(std::make_pair(bd->framebufferDescriptor, renderPipelineState));
  }

  size_t vertexBufferLength = (size_t)drawData->TotalVtxCount * sizeof(AnchorDrawVert);
  size_t indexBufferLength = (size_t)drawData->TotalIdxCount * sizeof(AnchorDrawIdx);
  MTLBackendBuffer* vertexBuffer = bd->dequeueReusableBuffer(vertexBufferLength, commandBuffer->device());
  MTLBackendBuffer* indexBuffer = bd->dequeueReusableBuffer(indexBufferLength, commandBuffer->device());

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

    memcpy((char*)vertexBuffer->buffer->contents() + vertexBufferOffset, cmd_list->VtxBuffer.Data, (size_t)cmd_list->VtxBuffer.Size * sizeof(AnchorDrawVert));
    memcpy((char*)indexBuffer->buffer->contents() + indexBufferOffset, cmd_list->IdxBuffer.Data, (size_t)cmd_list->IdxBuffer.Size * sizeof(AnchorDrawIdx));

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
        MTL::ScissorRect scissorRect =
        {
          .x = NS::UInteger(clip_min[0]),
          .y = NS::UInteger(clip_min[1]),
          .width = NS::UInteger(clip_max[0] - clip_min[0]),
          .height = NS::UInteger(clip_max[1] - clip_min[1])
        };
        commandEncoder->setScissorRect(scissorRect);

        // Bind texture, Draw
        if (AnchorTextureID tex_id = pcmd->GetTexID())
          commandEncoder->setFragmentTexture((MTL::Texture *)(tex_id), 0);

        commandEncoder->setVertexBufferOffset((vertexBufferOffset + pcmd->VtxOffset * sizeof(AnchorDrawVert)), 0);
        commandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, pcmd->ElemCount, sizeof(AnchorDrawIdx) == 2 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32, indexBuffer->buffer, indexBufferOffset + pcmd->IdxOffset * sizeof(AnchorDrawIdx));
      }
  }

    vertexBufferOffset += (size_t)cmd_list->VtxBuffer.Size * sizeof(AnchorDrawVert);
    indexBufferOffset += (size_t)cmd_list->IdxBuffer.Size * sizeof(AnchorDrawIdx);
  }

  commandBuffer->addCompletedHandler([&](MTL::CommandBuffer *) {
    // dispatch_async(dispatch_get_main_queue(), [&](){
      MTLBackend* bd = &MTLBackend::getInstance();
      if (bd != NULL)
      {
        // @synchronized(bd->bufferCache)
        // {
          bd->bufferCache.push_back(vertexBuffer);
          bd->bufferCache.push_back(indexBuffer);
        // }
      }
    // });
  });
}

bool CreateFonts(MTL::Device *device)
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
  MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm, (NS::UInteger)width, (NS::UInteger)height, NO);
  textureDescriptor->setUsage(MTL::TextureUsageShaderRead);
#if defined(ARCH_OS_MACOS) || TARGET_OS_MACCATALYST
  textureDescriptor->setStorageMode(MTL::StorageModeManaged);
#else
  textureDescriptor->setStorageMode(MTL::StorageModeShared);
#endif
  MTL::Texture *texture = device->newTexture(textureDescriptor);
  texture->replaceRegion(MTL::Region::Make2D(0, 0, (NS::UInteger)width, (NS::UInteger)height), 0, pixels, (NS::UInteger)width * 4);
  MTLBackend::getInstance().fontTexture = texture;
  io.Fonts->SetTexID((void*)MTLBackend::getInstance().fontTexture);

  return (MTLBackend::getInstance().fontTexture != nil);
}

void DestroyFonts()
{
  MTLContext* ctx = GetContext();
  AnchorIO& io = ANCHOR::GetIO();
  MTLBackend::getInstance().fontTexture = nil;
  io.Fonts->SetTexID(nullptr);
}

bool CreateResources(MTL::Device *device)
{
  MTLContext* ctx = GetContext();
  MTL::DepthStencilDescriptor* depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
  depthStencilDescriptor->setDepthWriteEnabled(NO);
  depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
  MTLBackend::getInstance().depthStencilState = device->newDepthStencilState(depthStencilDescriptor);
  CreateFonts(device);

  return true;
}

void DestroyResources()
{
  MTLContext* ctx = GetContext();
  DestroyFonts();
  MTLBackend::getInstance().renderPipelineStateCache.clear();
}

MTLBackendBuffer *MTLBackendBuffer::init(MTL::Buffer *fromBuf)
{
  if (fromBuf)
  {
    buffer = fromBuf;
    lastReuseTime = GetMachAbsoluteTimeInSeconds();
  }
  return this;
}

MTLBackendFramebufferDescriptor *MTLBackendFramebufferDescriptor::init(MTL::RenderPassDescriptor* renderPassDescriptor)
{
  if (MTL::RenderPassDescriptor::alloc()->init())
  {
    sampleCount = renderPassDescriptor->colorAttachments()->object(0)->texture()->sampleCount();
    colorPixelFormat = renderPassDescriptor->colorAttachments()->object(0)->texture()->pixelFormat();
    depthPixelFormat = renderPassDescriptor->depthAttachment()->texture()->pixelFormat();
    stencilPixelFormat = renderPassDescriptor->stencilAttachment()->texture()->pixelFormat();
  }
}

MTLBackend::MTLBackend()
{
  lastBufferCachePurge = GetMachAbsoluteTimeInSeconds();
}

MTLBackendBuffer *MTLBackend::dequeueReusableBuffer(NS::UInteger length, MTL::Device *device)
{
  uint64_t now = GetMachAbsoluteTimeInSeconds();

  // synchronized(self.bufferCache)
  // {
    // Purge old buffers that haven't been useful for a while
    if (now - lastBufferCachePurge > 1.0)
    {
      for (auto &candidate : bufferCache) {

      
        if (candidate->lastReuseTime > lastBufferCachePurge)
          printf("Todo.\n");
          // survivors->addObject(candidate);
      }
      // self.bufferCache = [survivors mutableCopy];
      lastBufferCachePurge = now;
    }

    // See if we have a buffer we can reuse
    MTLBackendBuffer* bestCandidate = nil;
    for (auto &candidate : bufferCache)
      if (candidate->buffer->length() >= length && (bestCandidate == nil || bestCandidate->lastReuseTime > candidate->lastReuseTime))
        bestCandidate = candidate;

    if (bestCandidate != nil)
    {
      bufferCache.erase(std::remove(bufferCache.begin(), bufferCache.end(), bestCandidate), bufferCache.end());
      bestCandidate->lastReuseTime = now;
      return bestCandidate;
    }
  // }

  // No luck; make a new buffer
  MTL::Buffer *backing = device->newBuffer(length, MTL::ResourceStorageModeShared);
  return (new MTLBackendBuffer())->init(backing);
}

// Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling.
MTL::RenderPipelineState *MTLBackend::newRenderPipelineState(MTLBackendFramebufferDescriptor* descriptor, MTL::Device * device)
{
  NS::Error* error = nil;

  NS::String* shaderSource = NS::String::string(
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
    "}\n", NS::UTF8StringEncoding);

  MTL::Library *library = device->newLibrary(shaderSource, nil, &error);
  if (library == nil) {
    ANCHOR_ASSERT((library == nil) && "Error: failed to create Metal library.");
    return nil;
  }

  MTL::Function *vertexFunction = library->newFunction(NS::String::string("vertex_main", NS::UTF8StringEncoding));
  MTL::Function *fragmentFunction = library->newFunction(NS::String::string("fragment_main", NS::UTF8StringEncoding));

  if (vertexFunction == nil || fragmentFunction == nil)
  {
    ANCHOR_ASSERT((vertexFunction == nil || fragmentFunction == nil) && "Error: failed to find Metal shader functions in library.");
    return nil;
  }

  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
  /* position. */
  vertexDescriptor->attributes()->object(0)->setOffset(ANCHOR_OFFSETOF(AnchorDrawVert, pos));
  vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
  vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

  /* texCoords. */
  vertexDescriptor->attributes()->object(1)->setOffset(ANCHOR_OFFSETOF(AnchorDrawVert, uv));
  vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
  vertexDescriptor->attributes()->object(1)->setBufferIndex(0);

  /* color. */
  vertexDescriptor->attributes()->object(2)->setOffset(ANCHOR_OFFSETOF(AnchorDrawVert, col));
  vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatUChar4);
  vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

  vertexDescriptor->layouts()->object(0)->setStepRate(1);
  vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
  vertexDescriptor->layouts()->object(0)->setStride(sizeof(AnchorDrawVert));

  MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
  pipelineDescriptor->setVertexFunction(vertexFunction);
  pipelineDescriptor->setFragmentFunction(fragmentFunction);
  pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
  pipelineDescriptor->setRasterSampleCount(framebufferDescriptor->sampleCount);
  pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(framebufferDescriptor->colorPixelFormat);
  pipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(YES);
  pipelineDescriptor->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);
  pipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
  pipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  pipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
  pipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
  pipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  pipelineDescriptor->setDepthAttachmentPixelFormat(framebufferDescriptor->depthPixelFormat);
  pipelineDescriptor->setStencilAttachmentPixelFormat(framebufferDescriptor->stencilPixelFormat);

  MTL::RenderPipelineState *renderPipelineState = device->newRenderPipelineState(pipelineDescriptor, &error);
  if (error != nil) {
    ANCHOR_ASSERT((error != nil) && "Error: failed to create Metal pipeline state.");
  }

  return renderPipelineState;
}

} /* gpu */

KRAKEN_NAMESPACE_END
