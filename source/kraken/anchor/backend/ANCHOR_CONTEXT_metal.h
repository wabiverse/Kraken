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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_internal.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

class AnchorContextMetal : public AnchorContext
{
  AnchorContextMetal(bool stereoVisual, NS::View *metalView, CA::MetalLayer *metalLayer);

  /**
   * Destructor.
   */
  ~AnchorContextMetal();

  /**
   * Initialize metal, create GPU resources.
   */
  void MetalInit();

  /**
   * Break down metal, free GPU resources.
   */
  void MetalFree();

  /**
   * Return a pointer to the Metal command queue used by this context.
   */
  MTL::CommandQueue *GetMetalCommandQueue();

  /**
   * Return a pointer to the Metal device associated with this context.
   */
  MTL::Device *GetMetalDevice();

  /**
   * Register present callback
   */
  void RegisterMetalPresentCallback(void (*callback)(MTL::RenderPassDescriptor *,
                                                     MTL::RenderPipelineState *,
                                                     MTL::Texture *,
                                                     CA::MetalDrawable *));

 private:

  /* ----- Metal Graphics Resources & State. ----- */

  NS::View *m_view;
  CA::MetalLayer *m_metalLayer;
  MTL::CommandQueue *m_queue;
  MTL::RenderPipelineState *m_pipeline;
  bool m_ownsMetalDevice;

  /* ----- The Metal Swapchain. ----- */

  static const int METAL_SWAPCHAIN_SIZE = 3;
  struct MTLSwapchainTexture
  {
    MTL::Texture *texture;
    unsigned int index;
  };
  MTLSwapchainTexture m_defaultFramebufferMetalTexture[METAL_SWAPCHAIN_SIZE];

  unsigned int m_current_swapchain_index = 0;
  int m_swap_interval;
  bool m_debug;

  /**
   * Present callback.
   * We use this such that presentation can be controlled from within the Metal
   * Context. This is required for optimal performance and clean control flow.
   * Also helps ensure flickering does not occur by present being dependent
   * on existing submissions. */
  void (*m_contextPresentCallback)(MTL::RenderPassDescriptor *,
                                   MTL::RenderPipelineState *,
                                   MTL::Texture *,
                                   CA::MetalDrawable *);
};
