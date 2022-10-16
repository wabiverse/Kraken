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
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"
#include "ANCHOR_CONTEXT_metal.h"

static void anchor_fatal_error_dialog(const char *msg)
{
  static char errBuf[120] = "";
  NS::String *message;
  NS::AutoreleasePool *pool;

  /* Write the error message. */
  snprintf(errBuf, sizeof(errBuf), "Error opening window:\n%s", msg);

  /** ----------------- @ AUTORELEASEPOOL: Begin ------ */
  pool = NS::AutoreleasePool::alloc()->init();

  message = NS_STRING_(errBuf);

  /* display the error alert. */
  NS::Alert *alert = NS::Alert::alloc()->init();
  alert->addButtonWithTitle(NS_STRING_("Quit"));
  alert->setMessageText(NS_STRING_("Kraken"));
  alert->setInformativeText(message);
  alert->setAlertStyle(NS::AlertStyle::StyleCritical);
  alert->runModal();

  pool->drain();
  pool = nil;
  /** ------------------- @ AUTORELEASEPOOL: End ------ */

  /* kill. */
  exit(1);
}

AnchorContextMetal::AnchorContextMetal(bool stereoVisual,
                                       NS::View *metalView,
                                       CA::MetalLayer *metalLayer)
  : AnchorContext(NULL),
    m_view(metalView),
    m_metalLayer(metalLayer),
    m_queue(nil),
    m_debug(false)
{
  /* Initialize Metal Swap-chain. */
  m_current_swapchain_index = 0;
  for (int i = 0; i < METAL_SWAPCHAIN_SIZE; i++) {
    m_defaultFramebufferMetalTexture[i].texture = nil;
    m_defaultFramebufferMetalTexture[i].index = i;
  }
  if (m_view) {
    m_ownsMetalDevice = false;
    MetalInit();
  } else {
    /* Prepare offscreen GHOST Context Metal device. */
    MTL::Device *metalDevice = MTL::CreateSystemDefaultDevice();

    if (m_debug) {
      printf("Selected Metal Device: %s\n", metalDevice->name()->utf8String());
    }

    m_ownsMetalDevice = true;
    if (metalDevice) {
      m_metalLayer = CA::MetalLayer::layer();
      m_metalLayer->setEdgeAntialiasingMask(0);
      m_metalLayer->setMasksToBounds(NO);
      m_metalLayer->setOpaque(YES);
      m_metalLayer->setFramebufferOnly(YES);
      m_metalLayer->setPresentsWithTransaction(NO);
      m_metalLayer->removeAllAnimations();
      m_metalLayer->setDevice(metalDevice);
      m_metalLayer->allowsNextDrawableTimeout(NO);
      MetalInit();
    } else {
      anchor_fatal_error_dialog(
        "[ERROR] Failed to create Metal device for offscreen ANCHOR Context.\n");
    }
  }

  /* Initialize swap-interval. */
  m_swap_interval = 60;
}

AnchorContextMetal::~AnchorContextMetal()
{
  MetalFree();

  if (m_ownsMetalDevice) {
    if (m_metalLayer) {
      m_metalLayer->release();
      m_metalLayer = nil;
    }
  }
}

static const MTL::PixelFormat METAL_FRAMEBUFFERPIXEL_FORMAT = MTL::PixelFormatBGRA8Unorm;

void AnchorContextMetal::MetalInit()
{
  NS::AutoreleasePool *pool;
  MTL::Device *device;
  NS::String *source;

  /** ----------------- @ AUTORELEASEPOOL: Begin ------ */
  pool = NS::AutoreleasePool::alloc()->init();

  device = m_metalLayer->device();

  /* Create a command queue for blit/present operation. */
  m_queue = device->newCommandQueue();
  m_queue->retain();

  /* Create shaders for blit operation. */
  source = NS_STRING_(
    "using namespace metal;\n"
    "\n"
    "struct Vertex {\n"
    "  float4 position [[position]];\n"
    "  float2 texCoord [[attribute(0)]];\n"
    "};\n"
    "\n"
    "vertex Vertex vertex_shader(uint v_id [[vertex_id]]) {\n"
    "  Vertex vtx;\n"
    "\n"
    "  vtx.position.x = float(v_id & 1) * 4.0 - 1.0;\n"
    "  vtx.position.y = float(v_id >> 1) * 4.0 - 1.0\n"
    "  vtx.position.z = 0.0\n"
    "  vtx.position.w = 1.0\n"
    "\n"
    "  vtx.texCoord = vtx.position.xy * 0.5 + 0.5;\n"
    "\n"
    "  return vtx;\n"
    "}\n"
    "\n"
    "constexpr sampler s {};\n"
    "\n"
    "fragment float4 fragment_shader(Vertex v [[stage_in]],\n"
    "                texture2d<float> t [[texture(0)]]) {\n"
    "\n"
    "  /* Final blit should ensure alpha is 1.0. This resolves\n"
    "   * rendering artifacts for blitting of final backbuffer. */\n"
    "  float4 out_tex = t.sample(s, v.texCoord);\n"
    "  out_tex.a = 1.0;\n"
    "  return out_tex;\n"
    "}\n");

  MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init()->autorelease();
  options->setLanguageVersion(MTL::LanguageVersion1_1);

  NS::Error *error = nil;
  MTL::Library *library = device->newLibrary(source, options, &error);
  if (error) {
    anchor_fatal_error_dialog(
      "Metal Initialization failed. Could not compile metal shaders for blit operation.");
  }

  /* Create a render pipeline for blit operation. */
  MTL::RenderPipelineDescriptor *desc =
    MTL::RenderPipelineDescriptor::alloc()->init()->autorelease();

  desc->setFragmentFunction(library->newFunction(NS_STRING_("fragment_shader")));
  desc->setVertexFunction(library->newFunction(NS_STRING_("vertex_shader")));
  /* Ensure library is released. */
  library->autorelease();

  desc->colorAttachments()->object(0)->setPixelFormat(METAL_FRAMEBUFFERPIXEL_FORMAT);

  m_pipeline = device->newRenderPipelineState(desc, &error);
  if (error) {
    anchor_fatal_error_dialog(
      "Metal Initialization failed. Could not create a render pipeline state with the selected "
      "pixel format");
  }

  /* Create a render pipeline to composite things rendered with Metal on top
   * of the frame-buffer contents. Uses the same vertex and fragment shader
   * as the blit above, but with alpha blending enabled. */
  desc->setLabel(NS_STRING_("Metal Overlay"));
  desc->colorAttachments()->object(0)->setBlendingEnabled(YES);
  desc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
  desc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(
    MTL::BlendFactorOneMinusSourceAlpha);

  if (error) {
    anchor_fatal_error_dialog(
      "Metal Initialization failed. Could not create the render overlay pipeline");
  }

  pool->drain();
  pool = nil;
  /** ------------------- @ AUTORELEASEPOOL: End ------ */
}

void AnchorContextMetal::MetalFree()
{
  if (m_queue) {
    m_queue->release();
  }
  if (m_pipeline) {
    m_pipeline->release();
  }

  for (int i = 0; i < METAL_SWAPCHAIN_SIZE; i++) {
    if (m_defaultFramebufferMetalTexture[i].texture) {
      m_defaultFramebufferMetalTexture[i].texture->release();
    }
  }
}

MTL::CommandQueue *AnchorContextMetal::GetMetalCommandQueue()
{
  return m_queue;
}

MTL::Device *AnchorContextMetal::GetMetalDevice()
{
  MTL::Device *device = m_metalLayer->device();
  return device;
}

void AnchorContextMetal::RegisterMetalPresentCallback(void (*callback)(MTL::RenderPassDescriptor *,
                                                                       MTL::RenderPipelineState *,
                                                                       MTL::Texture *,
                                                                       CA::MetalDrawable *))
{
  m_contextPresentCallback = callback;
}
