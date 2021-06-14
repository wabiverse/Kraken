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

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

// ANCHOR: Renderer Backend for Vulkan
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.
// Missing features:
//  [ ] Renderer: User texture binding. Changes of AnchorTextureID aren't supported by this
//  backend! See https://github.com/ocornut/anchor/pull/914

// You can use unmodified anchor_impl_* files in your project. See examples/ folder for examples of
// using this. Prefer including the entire anchor/ repository into your project (either as a copy
// or as a submodule), and only build the backends you need. If you are new to ANCHOR, read
// documentation from the docs/ folder + read the top of anchor.cpp. Read online:
// https://github.com/ocornut/anchor/tree/master/docs

// The aim of anchor_impl_vulkan.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at
// https://github.com/ocornut/anchor/

// Important note to the reader who wish to integrate anchor_impl_vulkan.cpp/.h in their own
// engine/app.
// - Common ANCHOR_ImplVulkan_XXX functions and structures are used to interface with
// anchor_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ANCHOR_ImplVulkanH_XXX functions and structures are only used by this example
// (main.cpp) and by
//   the backend itself (anchor_impl_vulkan.cpp), but should PROBABLY NOT be used by your own
//   engine/app code.
// Read comments in anchor_impl_vulkan.h.

#pragma once
#include "ANCHOR_api.h"  // ANCHOR_IMPL_API
#include "ANCHOR_surface.h"

// [Configuration] in order to use a custom Vulkan function loader:
// (1) You'll need to disable default Vulkan function prototypes.
//     We provide a '#define ANCHOR_IMPL_VULKAN_NO_PROTOTYPES' convenience configuration flag.
//     In order to make sure this is visible from the anchor_impl_vulkan.cpp compilation unit:
//     - Add '#define ANCHOR_IMPL_VULKAN_NO_PROTOTYPES' in your imconfig.h file
//     - Or as a compilation flag in your build system
//     - Or uncomment here (not recommended because you'd be modifying anchor sources!)
//     - Do not simply add it in a .cpp file!
// (2) Call ANCHOR_ImplVulkan_LoadFunctions() before ANCHOR_ImplVulkan_Init() with your custom
// function. If you have no idea what this is, leave it alone!
//#define ANCHOR_IMPL_VULKAN_NO_PROTOTYPES

// Vulkan includes
#if defined(ANCHOR_IMPL_VULKAN_NO_PROTOTYPES) && !defined(VK_NO_PROTOTYPES)
#  define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>

// Initialization data, for ANCHOR_ImplVulkan_Init()
// [Please zero-clear before use!]
struct ANCHOR_ImplVulkan_InitInfo {
  VkInstance Instance;
  VkPhysicalDevice PhysicalDevice;
  VkDevice Device;
  uint32_t QueueFamily;
  VkQueue Queue;
  VkPipelineCache PipelineCache;
  VkDescriptorPool DescriptorPool;
  uint32_t Subpass;
  uint32_t MinImageCount;             // >= 2
  uint32_t ImageCount;                // >= MinImageCount
  VkSampleCountFlagBits MSAASamples;  // >= VK_SAMPLE_COUNT_1_BIT
  const VkAllocationCallbacks *Allocator;
  void (*CheckVkResultFn)(VkResult err);
};

// Called by user code
ANCHOR_IMPL_API bool ANCHOR_ImplVulkan_Init(ANCHOR_ImplVulkan_InitInfo *info,
                                            VkRenderPass render_pass);
ANCHOR_IMPL_API void ANCHOR_ImplVulkan_Shutdown();
ANCHOR_IMPL_API void ANCHOR_ImplVulkan_NewFrame();
ANCHOR_IMPL_API void ANCHOR_ImplVulkan_RenderDrawData(ImDrawData *draw_data,
                                                      VkCommandBuffer command_buffer,
                                                      VkPipeline pipeline = VK_NULL_HANDLE);
ANCHOR_IMPL_API bool ANCHOR_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
ANCHOR_IMPL_API void ANCHOR_ImplVulkan_DestroyFontUploadObjects();
ANCHOR_IMPL_API void ANCHOR_ImplVulkan_SetMinImageCount(
    uint32_t min_image_count);  // To override MinImageCount after initialization (e.g. if swap
                                // chain is recreated)

// Optional: load Vulkan functions with a custom function loader
// This is only useful with ANCHOR_IMPL_VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES
ANCHOR_IMPL_API bool ANCHOR_ImplVulkan_LoadFunctions(
    PFN_vkVoidFunction (*loader_func)(const char *function_name, void *user_data),
    void *user_data = NULL);

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own
// engine/app.)
//-------------------------------------------------------------------------
// You probably do NOT need to use or care about those functions.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp examples files.
//   2) the upcoming multi-viewport feature will need them internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the backends,
// but it is too much code to duplicate everywhere so we exceptionally expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap chain, render
// pass, frame buffers, etc.). You may read this code to learn about Vulkan, but it is recommended
// you use you own custom tailored code to do equivalent work. (The ANCHOR_ImplVulkanH_XXX
// functions do not interact with any of the state used by the regular ANCHOR_ImplVulkan_XXX
// functions)
//-------------------------------------------------------------------------

struct ANCHOR_ImplVulkanH_Frame;
struct ANCHOR_ImplVulkanH_Window;

// Helpers
ANCHOR_IMPL_API void ANCHOR_ImplVulkanH_CreateOrResizeWindow(
    VkInstance instance,
    VkPhysicalDevice physical_device,
    VkDevice device,
    ANCHOR_ImplVulkanH_Window *wnd,
    uint32_t queue_family,
    const VkAllocationCallbacks *allocator,
    int w,
    int h,
    uint32_t min_image_count);
ANCHOR_IMPL_API void ANCHOR_ImplVulkanH_DestroyWindow(VkInstance instance,
                                                      VkDevice device,
                                                      ANCHOR_ImplVulkanH_Window *wnd,
                                                      const VkAllocationCallbacks *allocator);
ANCHOR_IMPL_API VkSurfaceFormatKHR
ANCHOR_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface,
                                       const VkFormat *request_formats,
                                       int request_formats_count,
                                       VkColorSpaceKHR request_color_space);
ANCHOR_IMPL_API VkPresentModeKHR
ANCHOR_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface,
                                     const VkPresentModeKHR *request_modes,
                                     int request_modes_count);
ANCHOR_IMPL_API int ANCHOR_ImplVulkanH_GetMinImageCountFromPresentMode(
    VkPresentModeKHR present_mode);

// Helper structure to hold the data needed by one rendering frame
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own
// engine/app.) [Please zero-clear before use!]
struct ANCHOR_ImplVulkanH_Frame {
  VkCommandPool CommandPool;
  VkCommandBuffer CommandBuffer;
  VkFence Fence;
  VkImage Backbuffer;
  VkImageView BackbufferView;
  VkFramebuffer Framebuffer;
};

struct ANCHOR_ImplVulkanH_FrameSemaphores {
  VkSemaphore ImageAcquiredSemaphore;
  VkSemaphore RenderCompleteSemaphore;
};

// Helper structure to hold the data needed by one rendering context into one OS window
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own
// engine/app.)
struct ANCHOR_ImplVulkanH_Window : public ANCHOR_Surface {
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
  uint32_t FrameIndex;  // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
  uint32_t ImageCount;  // Number of simultaneous in-flight frames (returned by
                        // vkGetSwapchainImagesKHR, usually derived from min_image_count)
  uint32_t SemaphoreIndex;  // Current set of swapchain wait semaphores we're using (needs to be
                            // distinct from per frame data)
  ANCHOR_ImplVulkanH_Frame *Frames;
  ANCHOR_ImplVulkanH_FrameSemaphores *FrameSemaphores;

  ANCHOR_ImplVulkanH_Window()
  {
    memset(this, 0, sizeof(*this));
    PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    ClearEnable = true;
  }
};
