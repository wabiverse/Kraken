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

#include "ANCHOR_api.h"
#include "ANCHOR_debug_codes.h"
#include "ANCHOR_impl_sdl.h"
#include "ANCHOR_impl_vulkan.h"
#include "ANCHOR_vulkan.h"

#include "CKE_version.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <iostream> /* cout */
#include <mutex>    /* once */
#include <stdio.h>  /* printf, fprintf */
#include <stdlib.h> /* abort */
#include <vulkan/vulkan.h>

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/error.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/warning.h>

#include <wabi/imaging/hd/driver.h>
#include <wabi/imaging/hgiVulkan/diagnostic.h>
#include <wabi/imaging/hgiVulkan/hgi.h>
#include <wabi/imaging/hgiVulkan/instance.h>

#include <wabi/usdImaging/usdApollo/engine.h>
#include <wabi/usdImaging/usdApollo/renderParams.h>

WABI_NAMESPACE_USING

/* clang-format off */

/**
 * PIXAR HYDRA GRAPHICS INTERFACE. */
static HgiVulkan         *g_PixarHydra      = nullptr;
static HgiVulkanInstance *g_PixarVkInstance = nullptr;

/**
 * HYDRA RENDERING PARAMS. */
static UsdApolloRenderParams g_HDPARAMS_Apollo;

/**
 * ANCHOR VULKAN GLOBALS. */
static VkAllocationCallbacks   *g_Allocator      = NULL;
static VkInstance               g_Instance       = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device         = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily    = (uint32_t)-1;
static VkQueue                  g_Queue          = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport    = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache  = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ANCHOR_ImplVulkanH_Window g_MainWindowData;
static uint32_t                  g_MinImageCount = 2;
static bool                      g_SwapChainRebuild = false;

/* clang-format on */

static void check_vk_result(VkResult err)
{
  if (err == 0)
    return;
  TF_CODING_ERROR("[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

/**
 * Display Vulkan Debug Report. */
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags,
                                                   VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char *pLayerPrefix,
                                                   const char *pMessage,
                                                   void *pUserData)
{
  const char *type = (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ? "VULKAN_ERROR" : "VULKAN_MESSAGE";
  TF_UNUSED(object);
  TF_UNUSED(location);
  TF_UNUSED(messageCode);
  TF_UNUSED(pUserData);
  TF_UNUSED(pLayerPrefix);

  TF_WARN("[vulkan] Debug report from ObjectType: %s\nMessage: %s\n\n", type, pMessage);
  return VK_FALSE;
}

/**
 * Main Vulkan Setup and Instantiation Routine. ------------------------------ */
static void SetupVulkan(const char **extensions, uint32_t extensions_count)
{
  VkResult err;

  /**
   * Create Vulkan Instance. */
  {
    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    /**
     * OpenGL Interop :: Memory */
    const char **ext_gl_mem = (const char **)malloc(sizeof(const char *) * (extensions_count + 1));
    memcpy(ext_gl_mem, extensions, extensions_count * sizeof(const char *));
    ext_gl_mem[extensions_count] = VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME;
    extensions                   = ext_gl_mem;
    extensions_count += 1;

    /**
     * OpenGL Interop :: Semaphore */
    const char **ext_gl_sem = (const char **)malloc(sizeof(const char *) * (extensions_count + 1));
    memcpy(ext_gl_sem, extensions, extensions_count * sizeof(const char *));
    ext_gl_sem[extensions_count] = VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME;
    extensions                   = ext_gl_sem;
    extensions_count += 1;

    /**
     * Physical Device Properties 2 */
    const char **ext_dprops_2 = (const char **)malloc(sizeof(const char *) *
                                                      (extensions_count + 1));
    memcpy(ext_dprops_2, extensions, extensions_count * sizeof(const char *));
    ext_dprops_2[extensions_count] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
    extensions                     = ext_dprops_2;
    extensions_count += 1;

    create_info.enabledExtensionCount   = extensions_count;
    create_info.ppEnabledExtensionNames = extensions;

    if (HgiVulkanIsDebugEnabled()) {
      /**
       * Enable validation layers. */
      const char *layers[]            = {"VK_LAYER_KHRONOS_validation"};
      create_info.enabledLayerCount   = 1;
      create_info.ppEnabledLayerNames = layers;

      /**
       * Enable debug report extension (we need additional storage,
       * so we duplicate the user array to add our new extension to it). */
      const char **extensions_ext = (const char **)malloc(sizeof(const char *) *
                                                          (extensions_count + 1));
      memcpy(extensions_ext, extensions, extensions_count * sizeof(const char *));
      extensions_ext[extensions_count]    = "VK_EXT_debug_utils";
      create_info.enabledExtensionCount   = extensions_count + 1;
      create_info.ppEnabledExtensionNames = extensions_ext;

      /**
       * Create Vulkan Instance with debug features. */
      err          = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
      g_PixarHydra = new HgiVulkan(g_PixarVkInstance = new HgiVulkanInstance(g_Instance));
      check_vk_result(err);
      free(extensions_ext);
    }
    else {

      /**
       * Create Vulkan Instance without any debug features. */
      err          = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
      g_PixarHydra = new HgiVulkan(g_PixarVkInstance = new HgiVulkanInstance(g_Instance));
      check_vk_result(err);
    }

    TF_UNUSED(g_DebugReport);

    free(ext_gl_mem);
    free(ext_gl_sem);
    free(ext_dprops_2);
  }

  /**
   * Select GPU. */
  {
    uint32_t gpu_count;
    err = vkEnumeratePhysicalDevices(g_PixarVkInstance->GetVulkanInstance(), &gpu_count, NULL);
    check_vk_result(err);
    ANCHOR_ASSERT(gpu_count > 0);

    VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(g_PixarVkInstance->GetVulkanInstance(), &gpu_count, gpus);
    check_vk_result(err);

    /**
     * If a number >1 of GPUs got reported, find discrete GPU if present,
     * or use first one available. This covers most common cases (multi-gpu
     * & integrated+dedicated graphics).
     *
     * TODO: Handle more complicated setups (multiple dedicated GPUs). */
    int use_gpu = 0;
    for (int i = 0; i < (int)gpu_count; i++) {

      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(gpus[i], &properties);
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        use_gpu = i;
        break;
      }
    }

    g_PhysicalDevice = gpus[use_gpu];
    free(gpus);
  }

  /**
   * Select graphics queue family. */
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(
        sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        g_QueueFamily = i;
        break;
      }
    free(queues);
    ANCHOR_ASSERT(g_QueueFamily != (uint32_t)-1);
  }

  /**
   * Create Logical Device (with 1 queue). */
  {
    int device_extension_count            = 1;
    const char *device_extensions[]       = {"VK_KHR_swapchain"};
    const float queue_priority[]          = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex        = g_QueueFamily;
    queue_info[0].queueCount              = 1;
    queue_info[0].pQueuePriorities        = queue_priority;
    VkDeviceCreateInfo create_info        = {};
    create_info.sType                     = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount      = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos         = queue_info;
    create_info.enabledExtensionCount     = device_extension_count;
    create_info.ppEnabledExtensionNames   = device_extensions;
    err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
    check_vk_result(err);
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
  }

  /**
   * Create Descriptor Pool. */
  {
    /* clang-format off */
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };
    /* clang-format on */
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                    = 1000 * ANCHOR_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount              = (uint32_t)ANCHOR_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes                 = pool_sizes;
    err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
    check_vk_result(err);
  }
}

/**
 * Main Application Window. --------------------------------- */
static void SetupVulkanWindow(ANCHOR_ImplVulkanH_Window *wd,
                              VkSurfaceKHR surface,
                              int width,
                              int height)
{
  wd->Surface = surface;

  /**
   * Check for WSI support. */
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  /**
   * Select Surface Format. */
  /* clang-format off */
  const VkFormat requestSurfaceImageFormat[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8_UNORM,
    VK_FORMAT_R8G8B8_UNORM
  };
  /* clang-format on */
  const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

  wd->SurfaceFormat = ANCHOR_ImplVulkanH_SelectSurfaceFormat(
      g_PhysicalDevice,
      wd->Surface,
      requestSurfaceImageFormat,
      (size_t)ANCHOR_ARRAYSIZE(requestSurfaceImageFormat),
      requestSurfaceColorSpace);

  /**
   * Render at maximum possible FPS. */
  if (HgiVulkanIsMaxFPSEnabled()) {

    TF_DEBUG(ANCHOR_SDL_VULKAN).Msg("[Anchor] Rendering at maximum possible frames per second.\n");

    /* clang-format off */
    VkPresentModeKHR present_modes[] = {
      /** Removes screen tearing. */
      VK_PRESENT_MODE_MAILBOX_KHR,
      /** Present frames immediately. */
      VK_PRESENT_MODE_IMMEDIATE_KHR,
      /** Required for presentation. */
      VK_PRESENT_MODE_FIFO_KHR
    };
    /* clang-format on */

    wd->PresentMode = ANCHOR_ImplVulkanH_SelectPresentMode(
        g_PhysicalDevice, wd->Surface, &present_modes[0], ANCHOR_ARRAYSIZE(present_modes));
  }
  else { /** Throttled FPS ~75FPS */

    TF_DEBUG(ANCHOR_SDL_VULKAN).Msg("[Anchor] Throttled maximum frames per second.\n");

    /* clang-format off */
    VkPresentModeKHR present_modes[] = {
      /** Required for presentation. */
      VK_PRESENT_MODE_FIFO_KHR
    };
    /* clang-format on */

    wd->PresentMode = ANCHOR_ImplVulkanH_SelectPresentMode(
        g_PhysicalDevice, wd->Surface, &present_modes[0], ANCHOR_ARRAYSIZE(present_modes));
  }

  TF_DEBUG(ANCHOR_SDL_VULKAN).Msg("[Anchor] Selected PresentMode = %d\n", wd->PresentMode);

  /**
   * Create SwapChain, RenderPass, Framebuffer, etc. */
  ANCHOR_ASSERT(g_MinImageCount >= 2);
  ANCHOR_ImplVulkanH_CreateOrResizeWindow(g_PixarVkInstance->GetVulkanInstance(),
                                          g_PhysicalDevice,
                                          g_Device,
                                          wd,
                                          g_QueueFamily,
                                          g_Allocator,
                                          width,
                                          height,
                                          g_MinImageCount);
}

static void CleanupVulkan()
{
  vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

  // if (HgiVulkanIsDebugEnabled()) {
  //   /**
  //    * Remove the debug report callback. */
  //   auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
  //       vkGetInstanceProcAddr(g_PixarVkInstance->GetVulkanInstance(),
  //       "vkDestroyDebugReportCallbackEXT");
  //   vkDestroyDebugReportCallbackEXT(g_PixarVkInstance->GetVulkanInstance(), g_DebugReport,
  //   g_Allocator);
  // }

  vkDestroyDevice(g_Device, g_Allocator);
  vkDestroyInstance(g_PixarVkInstance->GetVulkanInstance(), g_Allocator);
}

static void CleanupVulkanWindow()
{
  ANCHOR_ImplVulkanH_DestroyWindow(
      g_PixarVkInstance->GetVulkanInstance(), g_Device, &g_MainWindowData, g_Allocator);
}

static void FrameRender(ANCHOR_ImplVulkanH_Window *wd, ImDrawData *draw_data)
{
  VkResult err;

  VkSemaphore image_acquired_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(g_Device,
                              wd->Swapchain,
                              UINT64_MAX,
                              image_acquired_semaphore,
                              VK_NULL_HANDLE,
                              &wd->FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);

  ANCHOR_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
  {
    err = vkWaitForFences(g_Device,
                          1,
                          &fd->Fence,
                          VK_TRUE,
                          /* wait indefinitely==**/ UINT64_MAX);
    check_vk_result(err);

    err = vkResetFences(g_Device, 1, &fd->Fence);
    check_vk_result(err);
  }
  {
    err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }
  {
    VkRenderPassBeginInfo info    = {};
    info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass               = wd->RenderPass;
    info.framebuffer              = fd->Framebuffer;
    info.renderArea.extent.width  = wd->Width;
    info.renderArea.extent.height = wd->Height;
    info.clearValueCount          = 1;
    info.pClearValues             = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  /**
   * Record ANCHOR primitives into command buffer. */
  ANCHOR_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

  /**
   * Submit command buffer. */
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info               = {};
    info.sType                      = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount         = 1;
    info.pWaitSemaphores            = &image_acquired_semaphore;
    info.pWaitDstStageMask          = &wait_stage;
    info.commandBufferCount         = 1;
    info.pCommandBuffers            = &fd->CommandBuffer;
    info.signalSemaphoreCount       = 1;
    info.pSignalSemaphores          = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    check_vk_result(err);
    err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
    check_vk_result(err);
  }
}

static void FramePresent(ANCHOR_ImplVulkanH_Window *wd)
{
  if (g_SwapChainRebuild)
    return;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info   = {};
  info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores    = &render_complete_semaphore;
  info.swapchainCount     = 1;
  info.pSwapchains        = &wd->Swapchain;
  info.pImageIndices      = &wd->FrameIndex;
  VkResult err            = vkQueuePresentKHR(g_Queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);
  /**
   * Now we can use the next set of semaphores. */
  wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount;
}

static void SetFont()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  io.Fonts->AddFontDefault();

  const static std::string exe_path = TfGetPathName(ArchGetExecutablePath());

  /* Gotham Font. */
  const static std::string gm_ttf("../datafiles/fonts/GothamPro.ttf");
  const static char *gm_path = TfStringCatPaths(exe_path, gm_ttf).c_str();
  io.Fonts->AddFontFromFileTTF(gm_path, 11.0f);

  /* Dankmono Font. */
  const static std::string dm_ttf("../datafiles/fonts/dankmono.ttf");
  const static char *dm_path = TfStringCatPaths(exe_path, dm_ttf).c_str();
  io.Fonts->AddFontFromFileTTF(dm_path, 12.0f);

  /* San Francisco Font (Default). */
  const static std::string sf_ttf("../datafiles/fonts/SFProText-Medium.ttf");
  const static char *sf_path = TfStringCatPaths(exe_path, sf_ttf).c_str();
  io.FontDefault             = io.Fonts->AddFontFromFileTTF(sf_path, 14.0f);
}

static void SetWindowIcon(SDL_Window *window)
{
  const static std::string exe  = TfGetPathName(ArchGetExecutablePath());
  const static std::string icon = "../datafiles/icons/covah-desktop.png";

  SDL_SetWindowIcon(window, IMG_Load(TfStringCatPaths(exe, icon).c_str()));
}

HANDLE_sdl_vk_win ANCHOR_init_vulkan(VkResult &err)
{
  /**
   * Setup SDL. */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    TF_CODING_ERROR("Error: %s\n", SDL_GetError());
    exit(ANCHOR_ERROR);
  }

  /**
   * Setup Window. */
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                                   SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window *window           = SDL_CreateWindow(
      "Covah", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  SetWindowIcon(window);

  /**
   * Setup Vulkan. */
  uint32_t extensions_count = 0;
  SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, NULL);
  const char **extensions = new const char *[extensions_count];
  SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions);
  SetupVulkan(extensions, extensions_count);
  delete[] extensions;

  /**
   * Create Window Surface. */
  VkSurfaceKHR surface;
  if (SDL_Vulkan_CreateSurface(window, g_PixarVkInstance->GetVulkanInstance(), &surface) == 0) {
    TF_CODING_ERROR("Failed to create Vulkan surface.\n");
    exit(ANCHOR_ERROR);
  }

  /**
   * Create Framebuffers. */
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  ANCHOR_ImplVulkanH_Window *wd = &g_MainWindowData;
  SetupVulkanWindow(wd, surface, w, h);

  /**
   * Setup ANCHOR context. */
  ANCHOR_CHECKVERSION();
  ANCHOR::CreateContext();

  /**
   * Setup Keyboard & Gamepad controls. */
  ANCHOR_IO &io = ANCHOR::GetIO();
  io.ConfigFlags |= ANCHORConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ANCHORConfigFlags_NavEnableGamepad;

  /**
   * Setup Default Covah theme.
   *   Themes::
   *     - ANCHOR::StyleColorsClassic()
   *     - ANCHOR::StyleColorsLight()
   *     - ANCHOR::StyleColorsDark() */
  ANCHOR::StyleColorsClassic();

  /**
   * Setup Platform/Renderer backends. */
  ANCHOR_ImplSDL2_InitForVulkan(window);
  ANCHOR_ImplVulkan_InitInfo init_info = {};
  init_info.Instance                   = g_PixarVkInstance->GetVulkanInstance();
  init_info.PhysicalDevice             = g_PhysicalDevice;
  init_info.Device                     = g_Device;
  init_info.QueueFamily                = g_QueueFamily;
  init_info.Queue                      = g_Queue;
  init_info.PipelineCache              = g_PipelineCache;
  init_info.DescriptorPool             = g_DescriptorPool;
  init_info.Allocator                  = g_Allocator;
  init_info.MinImageCount              = g_MinImageCount;
  init_info.ImageCount                 = wd->ImageCount;
  init_info.CheckVkResultFn            = check_vk_result;
  ANCHOR_ImplVulkan_Init(&init_info, wd->RenderPass);

  /**
   * Create Pixar Hydra Graphics Interface. */
  HdDriver driver;
  HgiUniquePtr hgi = HgiUniquePtr(g_PixarHydra);
  driver.name      = HgiTokens->renderDriver;
  driver.driver    = VtValue(hgi.get());

  /**
   * Setup Pixar Driver & Engine. */
  ANCHOR::GetPixarDriver().name   = driver.name;
  ANCHOR::GetPixarDriver().driver = driver.driver;

  SetFont();

  /**
   * Upload Fonts. */
  {
    /**
     * Use any command queue. */
    VkCommandPool command_pool     = wd->Frames[wd->FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

    err = vkResetCommandPool(g_Device, command_pool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    check_vk_result(err);

    ANCHOR_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info       = {};
    end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers    = &command_buffer;
    err                         = vkEndCommandBuffer(command_buffer);
    check_vk_result(err);
    err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ANCHOR_ImplVulkan_DestroyFontUploadObjects();
  }

  return std::make_pair(window, wd);
}

ANCHOR_Status ANCHOR_run_vulkan(SDL_Window *window, ANCHOR_ImplVulkanH_Window *wd)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ANCHOR_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
      return ANCHOR_SUCCESS;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
      return ANCHOR_SUCCESS;
  }

  /**
   * Resize swap chain? */
  if (g_SwapChainRebuild) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    if (width > 0 && height > 0) {
      ANCHOR_ImplVulkan_SetMinImageCount(g_MinImageCount);
      ANCHOR_ImplVulkanH_CreateOrResizeWindow(g_PixarVkInstance->GetVulkanInstance(),
                                              g_PhysicalDevice,
                                              g_Device,
                                              &g_MainWindowData,
                                              g_QueueFamily,
                                              g_Allocator,
                                              width,
                                              height,
                                              g_MinImageCount);
      g_MainWindowData.FrameIndex = 0;
      g_SwapChainRebuild          = false;
    }
  }

  /**
   * Start the ANCHOR frame. */
  ANCHOR_ImplVulkan_NewFrame();
  ANCHOR_ImplSDL2_NewFrame(window);
  ANCHOR::NewFrame();

  /**
   * *** The Covah Runtime --> To Infinity. *** */

  return ANCHOR_RUN;
}

void ANCHOR_render_vulkan(ANCHOR_ImplVulkanH_Window *wd)
{
  ANCHOR::Render();
  ImDrawData *draw_data   = ANCHOR::GetDrawData();
  const bool is_minimized = (draw_data->DisplaySize[0] <= 0.0f ||
                             draw_data->DisplaySize[1] <= 0.0f);
  if (!is_minimized) {
    wd->ClearValue.color.float32[0] = g_HDPARAMS_Apollo.clearColor[0];
    wd->ClearValue.color.float32[1] = g_HDPARAMS_Apollo.clearColor[1];
    wd->ClearValue.color.float32[2] = g_HDPARAMS_Apollo.clearColor[2];
    wd->ClearValue.color.float32[3] = g_HDPARAMS_Apollo.clearColor[3];
    FrameRender(wd, draw_data);
    FramePresent(wd);
  }
}

void ANCHOR_clean_vulkan(SDL_Window *window, VkResult &err)
{
  err = vkDeviceWaitIdle(g_Device);
  check_vk_result(err);
  ANCHOR_ImplVulkan_Shutdown();
  ANCHOR_ImplSDL2_Shutdown();
  ANCHOR::DestroyContext();

  CleanupVulkanWindow();
  CleanupVulkan();

  SDL_DestroyWindow(window);
  SDL_Quit();
}