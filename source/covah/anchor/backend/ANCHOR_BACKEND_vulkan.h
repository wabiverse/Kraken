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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

// ANCHOR: Renderer Backend for Vulkan

#include "ANCHOR_api.h"

// Vulkan includes
#if defined(ANCHOR_IMPL_VULKAN_NO_PROTOTYPES) && !defined(VK_NO_PROTOTYPES)
#  define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>

struct ANCHOR_ImplVulkan_InitInfo
{
  VkInstance Instance;
  VkPhysicalDevice PhysicalDevice;
  VkDevice Device;
  uint32_t QueueFamily;
  VkQueue Queue;
  VkPipelineCache PipelineCache;
  VkDescriptorPool DescriptorPool;
  uint32_t Subpass;
  uint32_t MinImageCount;
  uint32_t ImageCount;
  VkSampleCountFlagBits MSAASamples;
  const VkAllocationCallbacks *Allocator;
  void (*CheckVkResultFn)(VkResult err);
};

// Called by user code
ANCHOR_BACKEND_API bool ANCHOR_ImplVulkan_Init(ANCHOR_ImplVulkan_InitInfo *info, VkRenderPass render_pass);
ANCHOR_BACKEND_API void ANCHOR_ImplVulkan_Shutdown();
ANCHOR_BACKEND_API void ANCHOR_ImplVulkan_NewFrame();
ANCHOR_BACKEND_API void ANCHOR_ImplVulkan_RenderDrawData(ImDrawData *draw_data,
                                                         VkCommandBuffer command_buffer,
                                                         VkPipeline pipeline = VK_NULL_HANDLE);
ANCHOR_BACKEND_API bool ANCHOR_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
ANCHOR_BACKEND_API void ANCHOR_ImplVulkan_DestroyFontUploadObjects();

/**
 * To override MinImageCount after initialization
 * (e.g. if swapchain is recreated) */
ANCHOR_BACKEND_API void ANCHOR_ImplVulkan_SetMinImageCount(uint32_t min_image_count);

/**
 * Optional: load Vulkan functions with a custom function loader
 * This is only useful with ANCHOR_IMPL_VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES */
ANCHOR_BACKEND_API bool ANCHOR_ImplVulkan_LoadFunctions(PFN_vkVoidFunction (*loader_func)(const char *fn_name,
                                                                                          void *data),
                                                        void *data = NULL);

struct ANCHOR_VulkanGPU_Frame;
struct ANCHOR_VulkanGPU_Surface;

// Helpers
ANCHOR_BACKEND_API void ANCHOR_ImplVulkanH_CreateOrResizeWindow(VkInstance instance,
                                                                VkPhysicalDevice physical_device,
                                                                VkDevice device,
                                                                ANCHOR_VulkanGPU_Surface *wnd,
                                                                uint32_t queue_family,
                                                                const VkAllocationCallbacks *allocator,
                                                                int w,
                                                                int h,
                                                                uint32_t min_image_count);
ANCHOR_BACKEND_API void ANCHOR_ImplVulkanH_DestroyWindow(VkInstance instance,
                                                         VkDevice device,
                                                         ANCHOR_VulkanGPU_Surface *wnd,
                                                         const VkAllocationCallbacks *allocator);
ANCHOR_BACKEND_API VkSurfaceFormatKHR
ANCHOR_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface,
                                       const VkFormat *request_formats,
                                       int request_formats_count,
                                       VkColorSpaceKHR request_color_space);
ANCHOR_BACKEND_API VkPresentModeKHR ANCHOR_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device,
                                                                         VkSurfaceKHR surface,
                                                                         const VkPresentModeKHR *request_modes,
                                                                         int request_modes_count);
ANCHOR_BACKEND_API int ANCHOR_ImplVulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode);

// Helper structure to hold the data needed by one rendering frame
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own
// engine/app.) [Please zero-clear before use!]
struct ANCHOR_VulkanGPU_Frame
{
  VkCommandPool CommandPool;
  VkCommandBuffer CommandBuffer;
  VkFence Fence;
  VkImage Backbuffer;
  VkImageView BackbufferView;
  VkFramebuffer Framebuffer;
};

struct ANCHOR_VulkanGPU_FrameSemaphores
{
  VkSemaphore ImageAcquiredSemaphore;
  VkSemaphore RenderCompleteSemaphore;
};

// Helper structure to hold the data needed by one rendering context into one OS window
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own
// engine/app.)
struct ANCHOR_VulkanGPU_Surface
{
  int Width;
  int Height;
  VkSwapchainKHR Swapchain;
  VkSurfaceKHR Surface;
  VkSurfaceFormatKHR SurfaceFormat;
  VkPresentModeKHR PresentMode;
  VkRenderPass RenderPass;
  VkPipeline Pipeline;  // The window pipeline may uses a different VkRenderPass than the one
                        // passed in ANCHOR_ImplVulkan_InitInfo
  bool ClearEnable;
  VkClearValue ClearValue;
  uint32_t FrameIndex;      // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
  uint32_t ImageCount;      // Number of simultaneous in-flight frames (returned by
                            // vkGetSwapchainImagesKHR, usually derived from min_image_count)
  uint32_t SemaphoreIndex;  // Current set of swapchain wait semaphores we're using (needs to be
                            // distinct from per frame data)
  ANCHOR_VulkanGPU_Frame *Frames;
  ANCHOR_VulkanGPU_FrameSemaphores *FrameSemaphores;

  ANCHOR_VulkanGPU_Surface()
  {
    memset(this, 0, sizeof(*this));
    PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    ClearEnable = true;
  }
};
