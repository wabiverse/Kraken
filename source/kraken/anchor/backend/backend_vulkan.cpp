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

#include "ANCHOR_BACKEND_vulkan.h"
#include <stdio.h>

KRAKEN_NAMESPACE_USING

// Reusable buffers used for rendering 1 current in-flight frame, for
// ANCHOR_ImplVulkan_RenderDrawData() [Please zero-clear before use!]
struct ANCHOR_VulkanGPU_FrameRenderBuffers
{
  VkDeviceMemory VertexBufferMemory;
  VkDeviceMemory IndexBufferMemory;
  VkDeviceSize VertexBufferSize;
  VkDeviceSize IndexBufferSize;
  VkBuffer VertexBuffer;
  VkBuffer IndexBuffer;
};

// Each viewport will hold 1 ANCHOR_VulkanGPU_SurfaceRenderBuffers
// [Please zero-clear before use!]
struct ANCHOR_VulkanGPU_SurfaceRenderBuffers
{
  uint32_t Index;
  uint32_t Count;
  ANCHOR_VulkanGPU_FrameRenderBuffers *FrameRenderBuffers;
};

// Vulkan data
static ANCHOR_ImplVulkan_InitInfo g_VulkanInitInfo = {};
static VkRenderPass g_RenderPass = VK_NULL_HANDLE;
static VkDeviceSize g_BufferMemoryAlignment = 256;
static VkPipelineCreateFlags g_PipelineCreateFlags = 0x00;
static VkDescriptorSetLayout g_DescriptorSetLayout = VK_NULL_HANDLE;
static VkPipelineLayout g_PipelineLayout = VK_NULL_HANDLE;
static VkDescriptorSet g_DescriptorSet = VK_NULL_HANDLE;
static VkPipeline g_Pipeline = VK_NULL_HANDLE;
static uint32_t g_Subpass = 0;
static VkShaderModule g_ShaderModuleVert;
static VkShaderModule g_ShaderModuleFrag;
#ifdef VK_NO_PROTOTYPES
static bool g_FunctionsLoaded = false;
#else
static bool g_FunctionsLoaded = true;
#endif

// Font data
static VkSampler g_FontSampler = VK_NULL_HANDLE;
static VkDeviceMemory g_FontMemory = VK_NULL_HANDLE;
static VkImage g_FontImage = VK_NULL_HANDLE;
static VkImageView g_FontView = VK_NULL_HANDLE;
static VkDeviceMemory g_UploadBufferMemory = VK_NULL_HANDLE;
static VkBuffer g_UploadBuffer = VK_NULL_HANDLE;

// Render buffers
static ANCHOR_VulkanGPU_SurfaceRenderBuffers g_MainWindowRenderBuffers;

// Forward Declarations
bool ANCHOR_ImplVulkan_CreateDeviceObjects();
void ANCHOR_ImplVulkan_DestroyDeviceObjects();
void ANCHOR_ImplVulkanH_DestroyFrame(VkDevice device,
                                     ANCHOR_VulkanGPU_Frame *fd,
                                     const VkAllocationCallbacks *allocator);
void ANCHOR_ImplVulkanH_DestroyFrameSemaphores(VkDevice device,
                                               ANCHOR_VulkanGPU_FrameSemaphores *fsd,
                                               const VkAllocationCallbacks *allocator);
void ANCHOR_ImplVulkanH_DestroyFrameRenderBuffers(VkDevice device,
                                                  ANCHOR_VulkanGPU_FrameRenderBuffers *buffers,
                                                  const VkAllocationCallbacks *allocator);
void ANCHOR_ImplVulkanH_DestroyWindowRenderBuffers(VkDevice device,
                                                   ANCHOR_VulkanGPU_SurfaceRenderBuffers *buffers,
                                                   const VkAllocationCallbacks *allocator);
void ANCHOR_ImplVulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device,
                                              VkDevice device,
                                              ANCHOR_VulkanGPU_Surface *wd,
                                              const VkAllocationCallbacks *allocator,
                                              int w,
                                              int h,
                                              uint32_t min_image_count);
void ANCHOR_ImplVulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device,
                                                   VkDevice device,
                                                   ANCHOR_VulkanGPU_Surface *wd,
                                                   uint32_t queue_family,
                                                   const VkAllocationCallbacks *allocator);

// Vulkan prototypes for use with custom loaders
// (see description of ANCHOR_IMPL_VULKAN_NO_PROTOTYPES in anchor_impl_vulkan.h
#ifdef VK_NO_PROTOTYPES
#  define ANCHOR_VULKAN_FUNC_MAP(ANCHOR_VULKAN_FUNC_MAP_MACRO)              \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkAllocateCommandBuffers)                  \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkAllocateDescriptorSets)                  \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkAllocateMemory)                          \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkBindBufferMemory)                        \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkBindImageMemory)                         \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdBindDescriptorSets)                   \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdBindIndexBuffer)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdBindPipeline)                         \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdBindVertexBuffers)                    \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdCopyBufferToImage)                    \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdDrawIndexed)                          \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdPipelineBarrier)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdPushConstants)                        \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdSetScissor)                           \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCmdSetViewport)                          \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateBuffer)                            \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateCommandPool)                       \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateDescriptorSetLayout)               \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateFence)                             \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateFramebuffer)                       \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateGraphicsPipelines)                 \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateImage)                             \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateImageView)                         \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreatePipelineLayout)                    \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateRenderPass)                        \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateSampler)                           \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateSemaphore)                         \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateShaderModule)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkCreateSwapchainKHR)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyBuffer)                           \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyCommandPool)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyDescriptorSetLayout)              \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyFence)                            \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyFramebuffer)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyImage)                            \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyImageView)                        \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyPipeline)                         \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyPipelineLayout)                   \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyRenderPass)                       \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroySampler)                          \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroySemaphore)                        \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroyShaderModule)                     \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroySurfaceKHR)                       \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDestroySwapchainKHR)                     \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkDeviceWaitIdle)                          \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkFlushMappedMemoryRanges)                 \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkFreeCommandBuffers)                      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkFreeMemory)                              \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetBufferMemoryRequirements)             \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetImageMemoryRequirements)              \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceMemoryProperties)       \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceFormatsKHR)      \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfacePresentModesKHR) \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkGetSwapchainImagesKHR)                   \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkMapMemory)                               \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkUnmapMemory)                             \
    ANCHOR_VULKAN_FUNC_MAP_MACRO(vkUpdateDescriptorSets)

// Define function pointers
#  define ANCHOR_VULKAN_FUNC_DEF(func) static PFN_##func func;
ANCHOR_VULKAN_FUNC_MAP(ANCHOR_VULKAN_FUNC_DEF)
#  undef ANCHOR_VULKAN_FUNC_DEF
#endif  // VK_NO_PROTOTYPES

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t __glsl_shader_vert_spv[] = {
  0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
  0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
  0x000a000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015,
  0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
  0x00000000, 0x00030005, 0x00000009, 0x00000000, 0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43,
  0x00000072, 0x00040006, 0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f,
  0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015, 0x00565561, 0x00060005,
  0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019, 0x00000000,
  0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
  0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368, 0x6e617473, 0x00000074,
  0x00050006, 0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001,
  0x61725475, 0x616c736e, 0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b,
  0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015,
  0x0000001e, 0x00000001, 0x00050048, 0x00000019, 0x00000000, 0x0000000b, 0x00000000, 0x00030047,
  0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e,
  0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001, 0x00000023, 0x00000008,
  0x00030047, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
  0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
  0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020,
  0x0000000a, 0x00000003, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015,
  0x0000000c, 0x00000020, 0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020,
  0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020,
  0x00000011, 0x00000003, 0x00000007, 0x0004002b, 0x0000000c, 0x00000013, 0x00000001, 0x00040020,
  0x00000014, 0x00000001, 0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020,
  0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007, 0x00040020, 0x0000001a,
  0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b, 0x00000014,
  0x0000001c, 0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
  0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009, 0x00040020, 0x00000021,
  0x00000009, 0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b, 0x00000006,
  0x00000029, 0x3f800000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
  0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012,
  0x0000000b, 0x0000000d, 0x0003003e, 0x00000012, 0x00000010, 0x0004003d, 0x00000008, 0x00000016,
  0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
  0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041, 0x00000021, 0x00000022,
  0x00000020, 0x0000000d, 0x0004003d, 0x00000008, 0x00000023, 0x00000022, 0x00050085, 0x00000008,
  0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
  0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024,
  0x00000026, 0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006,
  0x0000002b, 0x00000027, 0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b,
  0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b, 0x0000000d, 0x0003003e,
  0x0000002d, 0x0000002c, 0x000100fd, 0x00010038};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t __glsl_shader_frag_spv[] = {
  0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
  0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
  0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
  0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
  0x00000000, 0x00040005, 0x00000009, 0x6c6f4366, 0x0000726f, 0x00030005, 0x0000000b, 0x00000000,
  0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001,
  0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016, 0x78655473, 0x65727574,
  0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x0000001e,
  0x00000000, 0x00040047, 0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
  0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
  0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
  0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006,
  0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001,
  0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000001, 0x00040015, 0x0000000e, 0x00000020,
  0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001,
  0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
  0x00000001, 0x00000000, 0x0003001b, 0x00000014, 0x00000013, 0x00040020, 0x00000015, 0x00000000,
  0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
  0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004,
  0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d,
  0x0000000f, 0x0004003d, 0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017,
  0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a,
  0x0000001b, 0x0000001a, 0x00050057, 0x00000007, 0x0000001c, 0x00000017, 0x0000001b, 0x00050085,
  0x00000007, 0x0000001d, 0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd,
  0x00010038};

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------

static uint32_t ANCHOR_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  VkPhysicalDeviceMemoryProperties prop;
  vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);
  for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
    if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
      return i;
  return 0xFFFFFFFF;  // Unable to find memoryType
}

static void check_vk_result(VkResult err)
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  if (v->CheckVkResultFn)
    v->CheckVkResultFn(err);
}

static void CreateOrResizeBuffer(VkBuffer &buffer,
                                 VkDeviceMemory &buffer_memory,
                                 VkDeviceSize &p_buffer_size,
                                 size_t new_size,
                                 VkBufferUsageFlagBits usage)
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  VkResult err;
  if (buffer != VK_NULL_HANDLE)
    vkDestroyBuffer(v->Device, buffer, v->Allocator);
  if (buffer_memory != VK_NULL_HANDLE)
    vkFreeMemory(v->Device, buffer_memory, v->Allocator);

  VkDeviceSize vertex_buffer_size_aligned = ((new_size - 1) / g_BufferMemoryAlignment + 1) *
                                            g_BufferMemoryAlignment;
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = vertex_buffer_size_aligned;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &buffer);
  check_vk_result(err);

  VkMemoryRequirements req;
  vkGetBufferMemoryRequirements(v->Device, buffer, &req);
  g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment :
                                                                        req.alignment;
  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = req.size;
  alloc_info.memoryTypeIndex = ANCHOR_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                            req.memoryTypeBits);
  err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &buffer_memory);
  check_vk_result(err);

  err = vkBindBufferMemory(v->Device, buffer, buffer_memory, 0);
  check_vk_result(err);
  p_buffer_size = req.size;
}

static void ANCHOR_ImplVulkan_SetupRenderState(AnchorDrawData *draw_data,
                                               VkPipeline pipeline,
                                               VkCommandBuffer command_buffer,
                                               ANCHOR_VulkanGPU_FrameRenderBuffers *rb,
                                               int fb_width,
                                               int fb_height)
{
  // Bind pipeline and descriptor sets:
  {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    VkDescriptorSet desc_set[1] = {g_DescriptorSet};
    vkCmdBindDescriptorSets(command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            g_PipelineLayout,
                            0,
                            1,
                            desc_set,
                            0,
                            NULL);
  }

  // Bind Vertex And Index Buffer:
  if (draw_data->TotalVtxCount > 0) {
    VkBuffer vertex_buffers[1] = {rb->VertexBuffer};
    VkDeviceSize vertex_offset[1] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, vertex_offset);
    vkCmdBindIndexBuffer(command_buffer,
                         rb->IndexBuffer,
                         0,
                         sizeof(AnchorDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
  }

  // Setup viewport:
  {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)fb_width;
    viewport.height = (float)fb_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  }

  // Setup scale and translation:
  // Our visible anchor space lies from draw_data->DisplayPps (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single
  // viewport apps.
  {
    float scale[2];
    scale[0] = 2.0f / draw_data->DisplaySize[0];
    scale[1] = 2.0f / draw_data->DisplaySize[1];
    float translate[2];
    translate[0] = -1.0f - draw_data->DisplayPos[0] * scale[0];
    translate[1] = -1.0f - draw_data->DisplayPos[1] * scale[1];
    vkCmdPushConstants(command_buffer,
                       g_PipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       sizeof(float) * 0,
                       sizeof(float) * 2,
                       scale);
    vkCmdPushConstants(command_buffer,
                       g_PipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       sizeof(float) * 2,
                       sizeof(float) * 2,
                       translate);
  }
}

// Render function
void ANCHOR_ImplVulkan_RenderDrawData(AnchorDrawData *draw_data,
                                      VkCommandBuffer command_buffer,
                                      VkPipeline pipeline)
{
  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates !=
  // framebuffer coordinates)
  int fb_width = (int)(draw_data->DisplaySize[0] * draw_data->FramebufferScale[0]);
  int fb_height = (int)(draw_data->DisplaySize[1] * draw_data->FramebufferScale[1]);
  if (fb_width <= 0 || fb_height <= 0)
    return;

  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  if (pipeline == VK_NULL_HANDLE)
    pipeline = g_Pipeline;

  // Allocate array to store enough vertex/index buffers
  ANCHOR_VulkanGPU_SurfaceRenderBuffers *wrb = &g_MainWindowRenderBuffers;
  if (wrb->FrameRenderBuffers == NULL) {
    wrb->Index = 0;
    wrb->Count = v->ImageCount;
    wrb->FrameRenderBuffers = (ANCHOR_VulkanGPU_FrameRenderBuffers *)ANCHOR_ALLOC(
      sizeof(ANCHOR_VulkanGPU_FrameRenderBuffers) * wrb->Count);
    memset(wrb->FrameRenderBuffers, 0, sizeof(ANCHOR_VulkanGPU_FrameRenderBuffers) * wrb->Count);
  }
  ANCHOR_ASSERT(wrb->Count == v->ImageCount);
  wrb->Index = (wrb->Index + 1) % wrb->Count;
  ANCHOR_VulkanGPU_FrameRenderBuffers *rb = &wrb->FrameRenderBuffers[wrb->Index];

  if (draw_data->TotalVtxCount > 0) {
    // Create or resize the vertex/index buffers
    size_t vertex_size = draw_data->TotalVtxCount * sizeof(AnchorDrawVert);
    size_t index_size = draw_data->TotalIdxCount * sizeof(AnchorDrawIdx);
    if (rb->VertexBuffer == VK_NULL_HANDLE || rb->VertexBufferSize < vertex_size)
      CreateOrResizeBuffer(rb->VertexBuffer,
                           rb->VertexBufferMemory,
                           rb->VertexBufferSize,
                           vertex_size,
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (rb->IndexBuffer == VK_NULL_HANDLE || rb->IndexBufferSize < index_size)
      CreateOrResizeBuffer(rb->IndexBuffer,
                           rb->IndexBufferMemory,
                           rb->IndexBufferSize,
                           index_size,
                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // Upload vertex/index data into a single contiguous GPU buffer
    AnchorDrawVert *vtx_dst = NULL;
    AnchorDrawIdx *idx_dst = NULL;
    VkResult err = vkMapMemory(v->Device,
                               rb->VertexBufferMemory,
                               0,
                               rb->VertexBufferSize,
                               0,
                               (void **)(&vtx_dst));
    check_vk_result(err);
    err = vkMapMemory(v->Device,
                      rb->IndexBufferMemory,
                      0,
                      rb->IndexBufferSize,
                      0,
                      (void **)(&idx_dst));
    check_vk_result(err);
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
      const AnchorDrawList *cmd_list = draw_data->CmdLists[n];
      memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(AnchorDrawVert));
      memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(AnchorDrawIdx));
      vtx_dst += cmd_list->VtxBuffer.Size;
      idx_dst += cmd_list->IdxBuffer.Size;
    }
    VkMappedMemoryRange range[2] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = rb->VertexBufferMemory;
    range[0].size = VK_WHOLE_SIZE;
    range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[1].memory = rb->IndexBufferMemory;
    range[1].size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(v->Device, 2, range);
    check_vk_result(err);
    vkUnmapMemory(v->Device, rb->VertexBufferMemory);
    vkUnmapMemory(v->Device, rb->IndexBufferMemory);
  }

  // Setup desired Vulkan state
  ANCHOR_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);

  // Will project scissor/clipping rectangles into framebuffer space
  GfVec2f clip_off = draw_data->DisplayPos;  // (0,0) unless using multi-viewports
  GfVec2f clip_scale =
    draw_data->FramebufferScale;  // (1,1) unless using retina display which are often (2,2)

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const AnchorDrawList *cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const AnchorDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback != NULL) {
        // User callback, registered via AnchorDrawList::AddCallback()
        // (AnchorDrawCallback_ResetRenderState is a special callback value used by the user to
        // request the renderer to reset render state.)
        if (pcmd->UserCallback == AnchorDrawCallback_ResetRenderState)
          ANCHOR_ImplVulkan_SetupRenderState(draw_data,
                                             pipeline,
                                             command_buffer,
                                             rb,
                                             fb_width,
                                             fb_height);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        GfVec4f clip_rect;
        clip_rect[0] = (pcmd->ClipRect[0] - clip_off[0]) * clip_scale[0];
        clip_rect[1] = (pcmd->ClipRect[1] - clip_off[1]) * clip_scale[1];
        clip_rect[2] = (pcmd->ClipRect[2] - clip_off[0]) * clip_scale[0];
        clip_rect[3] = (pcmd->ClipRect[3] - clip_off[1]) * clip_scale[1];

        if (clip_rect[0] < fb_width && clip_rect[1] < fb_height && clip_rect[2] >= 0.0f &&
            clip_rect[3] >= 0.0f) {
          // Negative offsets are illegal for vkCmdSetScissor
          if (clip_rect[0] < 0.0f)
            clip_rect[0] = 0.0f;
          if (clip_rect[1] < 0.0f)
            clip_rect[1] = 0.0f;

          // Apply scissor/clipping rectangle
          VkRect2D scissor;
          scissor.offset.x = (int32_t)(clip_rect[0]);
          scissor.offset.y = (int32_t)(clip_rect[1]);
          scissor.extent.width = (uint32_t)(clip_rect[2] - clip_rect[0]);
          scissor.extent.height = (uint32_t)(clip_rect[3] - clip_rect[1]);
          vkCmdSetScissor(command_buffer, 0, 1, &scissor);

          // Draw
          vkCmdDrawIndexed(command_buffer,
                           pcmd->ElemCount,
                           1,
                           pcmd->IdxOffset + global_idx_offset,
                           pcmd->VtxOffset + global_vtx_offset,
                           0);
        }
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }
}

bool ANCHOR_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer)
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  AnchorIO &io = ANCHOR::GetIO();

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  size_t upload_size = width * height * 4 * sizeof(char);

  VkResult err;

  // Create the Image:
  {
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.extent.width = width;
    info.extent.height = height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    err = vkCreateImage(v->Device, &info, v->Allocator, &g_FontImage);
    check_vk_result(err);
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(v->Device, g_FontImage, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = ANCHOR_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                              req.memoryTypeBits);
    err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_FontMemory);
    check_vk_result(err);
    err = vkBindImageMemory(v->Device, g_FontImage, g_FontMemory, 0);
    check_vk_result(err);
  }

  // Create the Image View:
  {
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = g_FontImage;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;
    err = vkCreateImageView(v->Device, &info, v->Allocator, &g_FontView);
    check_vk_result(err);
  }

  // Update the Descriptor Set:
  {
    VkDescriptorImageInfo desc_image[1] = {};
    desc_image[0].sampler = g_FontSampler;
    desc_image[0].imageView = g_FontView;
    desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet write_desc[1] = {};
    write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc[0].dstSet = g_DescriptorSet;
    write_desc[0].descriptorCount = 1;
    write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_desc[0].pImageInfo = desc_image;
    vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, NULL);
  }

  // Create the Upload Buffer:
  {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = upload_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &g_UploadBuffer);
    check_vk_result(err);
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->Device, g_UploadBuffer, &req);
    g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment :
                                                                          req.alignment;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = ANCHOR_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                              req.memoryTypeBits);
    err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_UploadBufferMemory);
    check_vk_result(err);
    err = vkBindBufferMemory(v->Device, g_UploadBuffer, g_UploadBufferMemory, 0);
    check_vk_result(err);
  }

  // Upload to Buffer:
  {
    char *map = NULL;
    err = vkMapMemory(v->Device, g_UploadBufferMemory, 0, upload_size, 0, (void **)(&map));
    check_vk_result(err);
    memcpy(map, pixels, upload_size);
    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = g_UploadBufferMemory;
    range[0].size = upload_size;
    err = vkFlushMappedMemoryRanges(v->Device, 1, range);
    check_vk_result(err);
    vkUnmapMemory(v->Device, g_UploadBufferMemory);
  }

  // Copy to Image:
  {
    VkImageMemoryBarrier copy_barrier[1] = {};
    copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].image = g_FontImage;
    copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier[0].subresourceRange.levelCount = 1;
    copy_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         copy_barrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer,
                           g_UploadBuffer,
                           g_FontImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    VkImageMemoryBarrier use_barrier[1] = {};
    use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].image = g_FontImage;
    use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    use_barrier[0].subresourceRange.levelCount = 1;
    use_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         use_barrier);
  }

  // Store our identifier
  io.Fonts->SetTexID((AnchorTextureID)(intptr_t)g_FontImage);

  return true;
}

static void ANCHOR_ImplVulkan_CreateShaderModules(VkDevice device,
                                                  const VkAllocationCallbacks *allocator)
{
  // Create the shader modules
  if (g_ShaderModuleVert == NULL) {
    VkShaderModuleCreateInfo vert_info = {};
    vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_info.codeSize = sizeof(__glsl_shader_vert_spv);
    vert_info.pCode = (uint32_t *)__glsl_shader_vert_spv;
    VkResult err = vkCreateShaderModule(device, &vert_info, allocator, &g_ShaderModuleVert);
    check_vk_result(err);
  }
  if (g_ShaderModuleFrag == NULL) {
    VkShaderModuleCreateInfo frag_info = {};
    frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = sizeof(__glsl_shader_frag_spv);
    frag_info.pCode = (uint32_t *)__glsl_shader_frag_spv;
    VkResult err = vkCreateShaderModule(device, &frag_info, allocator, &g_ShaderModuleFrag);
    check_vk_result(err);
  }
}

static void ANCHOR_ImplVulkan_CreateFontSampler(VkDevice device,
                                                const VkAllocationCallbacks *allocator)
{
  if (g_FontSampler)
    return;

  VkSamplerCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = VK_FILTER_LINEAR;
  info.minFilter = VK_FILTER_LINEAR;
  info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.minLod = -1000;
  info.maxLod = 1000;
  info.maxAnisotropy = 1.0f;
  VkResult err = vkCreateSampler(device, &info, allocator, &g_FontSampler);
  check_vk_result(err);
}

static void ANCHOR_ImplVulkan_CreateDescriptorSetLayout(VkDevice device,
                                                        const VkAllocationCallbacks *allocator)
{
  if (g_DescriptorSetLayout)
    return;

  ANCHOR_ImplVulkan_CreateFontSampler(device, allocator);
  VkSampler sampler[1] = {g_FontSampler};
  VkDescriptorSetLayoutBinding binding[1] = {};
  binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  binding[0].descriptorCount = 1;
  binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  binding[0].pImmutableSamplers = sampler;
  VkDescriptorSetLayoutCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.bindingCount = 1;
  info.pBindings = binding;
  VkResult err = vkCreateDescriptorSetLayout(device, &info, allocator, &g_DescriptorSetLayout);
  check_vk_result(err);
}

static void ANCHOR_ImplVulkan_CreatePipelineLayout(VkDevice device,
                                                   const VkAllocationCallbacks *allocator)
{
  if (g_PipelineLayout)
    return;

  // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
  ANCHOR_ImplVulkan_CreateDescriptorSetLayout(device, allocator);
  VkPushConstantRange push_constants[1] = {};
  push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constants[0].offset = sizeof(float) * 0;
  push_constants[0].size = sizeof(float) * 4;
  VkDescriptorSetLayout set_layout[1] = {g_DescriptorSetLayout};
  VkPipelineLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.setLayoutCount = 1;
  layout_info.pSetLayouts = set_layout;
  layout_info.pushConstantRangeCount = 1;
  layout_info.pPushConstantRanges = push_constants;
  VkResult err = vkCreatePipelineLayout(device, &layout_info, allocator, &g_PipelineLayout);
  check_vk_result(err);
}

static void ANCHOR_ImplVulkan_CreatePipeline(VkDevice device,
                                             const VkAllocationCallbacks *allocator,
                                             VkPipelineCache pipelineCache,
                                             VkRenderPass renderPass,
                                             VkSampleCountFlagBits MSAASamples,
                                             VkPipeline *pipeline,
                                             uint32_t subpass)
{
  ANCHOR_ImplVulkan_CreateShaderModules(device, allocator);

  VkPipelineShaderStageCreateInfo stage[2] = {};
  stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stage[0].module = g_ShaderModuleVert;
  stage[0].pName = "main";
  stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stage[1].module = g_ShaderModuleFrag;
  stage[1].pName = "main";

  VkVertexInputBindingDescription binding_desc[1] = {};
  binding_desc[0].stride = sizeof(AnchorDrawVert);
  binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attribute_desc[3] = {};
  attribute_desc[0].location = 0;
  attribute_desc[0].binding = binding_desc[0].binding;
  attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_desc[0].offset = ANCHOR_OFFSETOF(AnchorDrawVert, pos);
  attribute_desc[1].location = 1;
  attribute_desc[1].binding = binding_desc[0].binding;
  attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_desc[1].offset = ANCHOR_OFFSETOF(AnchorDrawVert, uv);
  attribute_desc[2].location = 2;
  attribute_desc[2].binding = binding_desc[0].binding;
  attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
  attribute_desc[2].offset = ANCHOR_OFFSETOF(AnchorDrawVert, col);

  VkPipelineVertexInputStateCreateInfo vertex_info = {};
  vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_info.vertexBindingDescriptionCount = 1;
  vertex_info.pVertexBindingDescriptions = binding_desc;
  vertex_info.vertexAttributeDescriptionCount = 3;
  vertex_info.pVertexAttributeDescriptions = attribute_desc;

  VkPipelineInputAssemblyStateCreateInfo ia_info = {};
  ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineViewportStateCreateInfo viewport_info = {};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo raster_info = {};
  raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.polygonMode = VK_POLYGON_MODE_FILL;
  raster_info.cullMode = VK_CULL_MODE_NONE;
  raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  raster_info.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo ms_info = {};
  ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms_info.rasterizationSamples = (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState color_attachment[1] = {};
  color_attachment[0].blendEnable = VK_TRUE;
  color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
  color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
  color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineDepthStencilStateCreateInfo depth_info = {};
  depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  VkPipelineColorBlendStateCreateInfo blend_info = {};
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = color_attachment;

  VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = (uint32_t)ANCHOR_ARRAYSIZE(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  ANCHOR_ImplVulkan_CreatePipelineLayout(device, allocator);

  VkGraphicsPipelineCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info.flags = g_PipelineCreateFlags;
  info.stageCount = 2;
  info.pStages = stage;
  info.pVertexInputState = &vertex_info;
  info.pInputAssemblyState = &ia_info;
  info.pViewportState = &viewport_info;
  info.pRasterizationState = &raster_info;
  info.pMultisampleState = &ms_info;
  info.pDepthStencilState = &depth_info;
  info.pColorBlendState = &blend_info;
  info.pDynamicState = &dynamic_state;
  info.layout = g_PipelineLayout;
  info.renderPass = renderPass;
  info.subpass = subpass;
  VkResult err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &info, allocator, pipeline);
  check_vk_result(err);
}

bool ANCHOR_ImplVulkan_CreateDeviceObjects()
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  VkResult err;

  if (!g_FontSampler) {
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.minLod = -1000;
    info.maxLod = 1000;
    info.maxAnisotropy = 1.0f;
    err = vkCreateSampler(v->Device, &info, v->Allocator, &g_FontSampler);
    check_vk_result(err);
  }

  if (!g_DescriptorSetLayout) {
    VkSampler sampler[1] = {g_FontSampler};
    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding[0].pImmutableSamplers = sampler;
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;
    err = vkCreateDescriptorSetLayout(v->Device, &info, v->Allocator, &g_DescriptorSetLayout);
    check_vk_result(err);
  }

  // Create Descriptor Set:
  {
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = v->DescriptorPool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &g_DescriptorSetLayout;
    err = vkAllocateDescriptorSets(v->Device, &alloc_info, &g_DescriptorSet);
    check_vk_result(err);
  }

  if (!g_PipelineLayout) {
    // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection
    // matrix
    VkPushConstantRange push_constants[1] = {};
    push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constants[0].offset = sizeof(float) * 0;
    push_constants[0].size = sizeof(float) * 4;
    VkDescriptorSetLayout set_layout[1] = {g_DescriptorSetLayout};
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = set_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = push_constants;
    err = vkCreatePipelineLayout(v->Device, &layout_info, v->Allocator, &g_PipelineLayout);
    check_vk_result(err);
  }

  ANCHOR_ImplVulkan_CreatePipeline(v->Device,
                                   v->Allocator,
                                   v->PipelineCache,
                                   g_RenderPass,
                                   v->MSAASamples,
                                   &g_Pipeline,
                                   g_Subpass);

  return true;
}

void ANCHOR_ImplVulkan_DestroyFontUploadObjects()
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  if (g_UploadBuffer) {
    vkDestroyBuffer(v->Device, g_UploadBuffer, v->Allocator);
    g_UploadBuffer = VK_NULL_HANDLE;
  }
  if (g_UploadBufferMemory) {
    vkFreeMemory(v->Device, g_UploadBufferMemory, v->Allocator);
    g_UploadBufferMemory = VK_NULL_HANDLE;
  }
}

void ANCHOR_ImplVulkan_DestroyDeviceObjects()
{
  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  ANCHOR_ImplVulkanH_DestroyWindowRenderBuffers(v->Device,
                                                &g_MainWindowRenderBuffers,
                                                v->Allocator);
  ANCHOR_ImplVulkan_DestroyFontUploadObjects();

  if (g_ShaderModuleVert) {
    vkDestroyShaderModule(v->Device, g_ShaderModuleVert, v->Allocator);
    g_ShaderModuleVert = VK_NULL_HANDLE;
  }
  if (g_ShaderModuleFrag) {
    vkDestroyShaderModule(v->Device, g_ShaderModuleFrag, v->Allocator);
    g_ShaderModuleFrag = VK_NULL_HANDLE;
  }
  if (g_FontView) {
    vkDestroyImageView(v->Device, g_FontView, v->Allocator);
    g_FontView = VK_NULL_HANDLE;
  }
  if (g_FontImage) {
    vkDestroyImage(v->Device, g_FontImage, v->Allocator);
    g_FontImage = VK_NULL_HANDLE;
  }
  if (g_FontMemory) {
    vkFreeMemory(v->Device, g_FontMemory, v->Allocator);
    g_FontMemory = VK_NULL_HANDLE;
  }
  if (g_FontSampler) {
    vkDestroySampler(v->Device, g_FontSampler, v->Allocator);
    g_FontSampler = VK_NULL_HANDLE;
  }
  if (g_DescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(v->Device, g_DescriptorSetLayout, v->Allocator);
    g_DescriptorSetLayout = VK_NULL_HANDLE;
  }
  if (g_PipelineLayout) {
    vkDestroyPipelineLayout(v->Device, g_PipelineLayout, v->Allocator);
    g_PipelineLayout = VK_NULL_HANDLE;
  }
  if (g_Pipeline) {
    vkDestroyPipeline(v->Device, g_Pipeline, v->Allocator);
    g_Pipeline = VK_NULL_HANDLE;
  }
}

bool ANCHOR_ImplVulkan_LoadFunctions(PFN_vkVoidFunction (*loader_func)(const char *function_name,
                                                                       void *user_data),
                                     void *user_data)
{
  // Load function pointers
  // You can use the default Vulkan loader using:
  //      ANCHOR_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return
  //      vkGetInstanceProcAddr(your_vk_isntance, function_name); });
  // But this would be equivalent to not setting VK_NO_PROTOTYPES.
#ifdef VK_NO_PROTOTYPES
#  define ANCHOR_VULKAN_FUNC_LOAD(func)                                     \
    func = reinterpret_cast<decltype(func)>(loader_func(#func, user_data)); \
    if (func == NULL)                                                       \
      return false;
  ANCHOR_VULKAN_FUNC_MAP(ANCHOR_VULKAN_FUNC_LOAD)
#  undef ANCHOR_VULKAN_FUNC_LOAD
#else
  TF_UNUSED(loader_func);
  TF_UNUSED(user_data);
#endif
  g_FunctionsLoaded = true;
  return true;
}

bool ANCHOR_ImplVulkan_Init(ANCHOR_ImplVulkan_InitInfo *info, VkRenderPass render_pass)
{
  ANCHOR_ASSERT(g_FunctionsLoaded &&
                "Need to call ANCHOR_ImplVulkan_LoadFunctions() if "
                "ANCHOR_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");

  // Setup backend capabilities flags
  AnchorIO &io = ANCHOR::GetIO();
  io.BackendRendererName = "anchor_impl_vulkan";
  io.BackendFlags |=
    AnchorBackendFlags_RendererHasVtxOffset;  // We can honor the AnchorDrawCmd::VtxOffset
                                              // field, allowing for large meshes.

  ANCHOR_ASSERT(info->Instance != VK_NULL_HANDLE);
  ANCHOR_ASSERT(info->PhysicalDevice != VK_NULL_HANDLE);
  ANCHOR_ASSERT(info->Device != VK_NULL_HANDLE);
  ANCHOR_ASSERT(info->Queue != VK_NULL_HANDLE);
  ANCHOR_ASSERT(info->DescriptorPool != VK_NULL_HANDLE);
  ANCHOR_ASSERT(info->MinImageCount >= 2);
  ANCHOR_ASSERT(info->ImageCount >= info->MinImageCount);
  ANCHOR_ASSERT(render_pass != VK_NULL_HANDLE);

  g_VulkanInitInfo = *info;
  g_RenderPass = render_pass;
  g_Subpass = info->Subpass;

  ANCHOR_ImplVulkan_CreateDeviceObjects();

  return true;
}

void ANCHOR_ImplVulkan_Shutdown()
{
  ANCHOR_ImplVulkan_DestroyDeviceObjects();
}

void ANCHOR_ImplVulkan_NewFrame() {}

void ANCHOR_ImplVulkan_SetMinImageCount(uint32_t min_image_count)
{
  ANCHOR_ASSERT(min_image_count >= 2);
  if (g_VulkanInitInfo.MinImageCount == min_image_count)
    return;

  ANCHOR_ImplVulkan_InitInfo *v = &g_VulkanInitInfo;
  VkResult err = vkDeviceWaitIdle(v->Device);
  check_vk_result(err);
  ANCHOR_ImplVulkanH_DestroyWindowRenderBuffers(v->Device,
                                                &g_MainWindowRenderBuffers,
                                                v->Allocator);
  g_VulkanInitInfo.MinImageCount = min_image_count;
}

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own
// app.)
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

VkSurfaceFormatKHR ANCHOR_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device,
                                                          VkSurfaceKHR surface,
                                                          const VkFormat *request_formats,
                                                          int request_formats_count,
                                                          VkColorSpaceKHR request_color_space)
{
  ANCHOR_ASSERT(g_FunctionsLoaded &&
                "Need to call ANCHOR_ImplVulkan_LoadFunctions() if "
                "ANCHOR_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  ANCHOR_ASSERT(request_formats != NULL);
  ANCHOR_ASSERT(request_formats_count > 0);

  // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT
  // was set at image creation Assuming that the default behavior is without setting this bit,
  // there is no need for separate Swapchain image and image view format Additionally several new
  // color spaces were introduced with Vulkan Spec v1.0.40, hence we must make sure that a format
  // with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
  uint32_t avail_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, NULL);
  AnchorVector<VkSurfaceFormatKHR> avail_format;
  avail_format.resize((int)avail_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, avail_format.Data);

  // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any
  // format is available
  if (avail_count == 1) {
    if (avail_format[0].format == VK_FORMAT_UNDEFINED) {
      VkSurfaceFormatKHR ret;
      ret.format = request_formats[0];
      ret.colorSpace = request_color_space;
      return ret;
    } else {
      // No point in searching another format
      return avail_format[0];
    }
  } else {
    // Request several formats, the first found will be used
    for (int request_i = 0; request_i < request_formats_count; request_i++)
      for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
        if (avail_format[avail_i].format == request_formats[request_i] &&
            avail_format[avail_i].colorSpace == request_color_space)
          return avail_format[avail_i];

    // If none of the requested image formats could be found, use the first available
    return avail_format[0];
  }
}

VkPresentModeKHR ANCHOR_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device,
                                                      VkSurfaceKHR surface,
                                                      const VkPresentModeKHR *request_modes,
                                                      int request_modes_count)
{
  ANCHOR_ASSERT(g_FunctionsLoaded &&
                "Need to call ANCHOR_ImplVulkan_LoadFunctions() if "
                "ANCHOR_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  ANCHOR_ASSERT(request_modes != NULL);
  ANCHOR_ASSERT(request_modes_count > 0);

  // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR
  // which is mandatory
  uint32_t avail_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, NULL);
  AnchorVector<VkPresentModeKHR> avail_modes;
  avail_modes.resize((int)avail_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                            surface,
                                            &avail_count,
                                            avail_modes.Data);
  // for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
  //    printf("[vulkan] avail_modes[%d] = %d\n", avail_i, avail_modes[avail_i]);

  for (int request_i = 0; request_i < request_modes_count; request_i++)
    for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
      if (request_modes[request_i] == avail_modes[avail_i])
        return request_modes[request_i];

  return VK_PRESENT_MODE_FIFO_KHR;  // Always available
}

void ANCHOR_ImplVulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device,
                                                   VkDevice device,
                                                   ANCHOR_VulkanGPU_Surface *wd,
                                                   uint32_t queue_family,
                                                   const VkAllocationCallbacks *allocator)
{
  ANCHOR_ASSERT(physical_device != VK_NULL_HANDLE && device != VK_NULL_HANDLE);
  (void)physical_device;
  (void)allocator;

  // Create Command Buffers
  VkResult err;
  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    ANCHOR_VulkanGPU_Frame *fd = &wd->Frames[i];
    ANCHOR_VulkanGPU_FrameSemaphores *fsd = &wd->FrameSemaphores[i];
    {
      VkCommandPoolCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      info.queueFamilyIndex = queue_family;
      err = vkCreateCommandPool(device, &info, allocator, &fd->CommandPool);
      check_vk_result(err);
    }
    {
      VkCommandBufferAllocateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      info.commandPool = fd->CommandPool;
      info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      info.commandBufferCount = 1;
      err = vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
      check_vk_result(err);
    }
    {
      VkFenceCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      err = vkCreateFence(device, &info, allocator, &fd->Fence);
      check_vk_result(err);
    }
    {
      VkSemaphoreCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      err = vkCreateSemaphore(device, &info, allocator, &fsd->ImageAcquiredSemaphore);
      check_vk_result(err);
      err = vkCreateSemaphore(device, &info, allocator, &fsd->RenderCompleteSemaphore);
      check_vk_result(err);
    }
  }
}

int ANCHOR_ImplVulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode)
{
  if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
    return 3;
  if (present_mode == VK_PRESENT_MODE_FIFO_KHR || present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
    return 2;
  if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    return 1;
  ANCHOR_ASSERT(0);
  return 1;
}

// Also destroy old swap chain and in-flight frames data, if any.
void ANCHOR_ImplVulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device,
                                              VkDevice device,
                                              ANCHOR_VulkanGPU_Surface *wd,
                                              const VkAllocationCallbacks *allocator,
                                              int w,
                                              int h,
                                              uint32_t min_image_count)
{
  VkResult err;
  VkSwapchainKHR old_swapchain = wd->Swapchain;
  wd->Swapchain = NULL;
  err = vkDeviceWaitIdle(device);
  check_vk_result(err);

  // We don't use ANCHOR_ImplVulkanH_DestroyWindow() because we want to preserve the old swapchain
  // to create the new one. Destroy old Framebuffer
  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    ANCHOR_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    ANCHOR_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
  }
  ANCHOR_FREE(wd->Frames);
  ANCHOR_FREE(wd->FrameSemaphores);
  wd->Frames = NULL;
  wd->FrameSemaphores = NULL;
  wd->ImageCount = 0;
  if (wd->RenderPass)
    vkDestroyRenderPass(device, wd->RenderPass, allocator);
  if (wd->Pipeline)
    vkDestroyPipeline(device, wd->Pipeline, allocator);

  // If min image count was not specified, request different count of images dependent on selected
  // present mode
  if (min_image_count == 0)
    min_image_count = ANCHOR_ImplVulkanH_GetMinImageCountFromPresentMode(wd->PresentMode);

  // Create Swapchain
  {
    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = wd->Surface;
    info.minImageCount = min_image_count;
    info.imageFormat = wd->SurfaceFormat.format;
    info.imageColorSpace = wd->SurfaceFormat.colorSpace;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode =
      VK_SHARING_MODE_EXCLUSIVE;  // Assume that graphics family == present family
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = wd->PresentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = old_swapchain;
    VkSurfaceCapabilitiesKHR cap;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, wd->Surface, &cap);
    check_vk_result(err);
    if (info.minImageCount < cap.minImageCount)
      info.minImageCount = cap.minImageCount;
    else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
      info.minImageCount = cap.maxImageCount;

    if (cap.currentExtent.width == 0xffffffff) {
      info.imageExtent.width = wd->Width = w;
      info.imageExtent.height = wd->Height = h;
    } else {
      info.imageExtent.width = wd->Width = cap.currentExtent.width;
      info.imageExtent.height = wd->Height = cap.currentExtent.height;
    }
    err = vkCreateSwapchainKHR(device, &info, allocator, &wd->Swapchain);
    check_vk_result(err);
    err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, NULL);
    check_vk_result(err);
    VkImage backbuffers[16] = {};
    ANCHOR_ASSERT(wd->ImageCount >= min_image_count);
    ANCHOR_ASSERT(wd->ImageCount < ANCHOR_ARRAYSIZE(backbuffers));
    err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, backbuffers);
    check_vk_result(err);

    ANCHOR_ASSERT(wd->Frames == NULL);
    wd->Frames = (ANCHOR_VulkanGPU_Frame *)ANCHOR_ALLOC(sizeof(ANCHOR_VulkanGPU_Frame) *
                                                        wd->ImageCount);
    wd->FrameSemaphores = (ANCHOR_VulkanGPU_FrameSemaphores *)ANCHOR_ALLOC(
      sizeof(ANCHOR_VulkanGPU_FrameSemaphores) * wd->ImageCount);
    memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
    memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);
    for (uint32_t i = 0; i < wd->ImageCount; i++)
      wd->Frames[i].Backbuffer = backbuffers[i];
  }
  if (old_swapchain)
    vkDestroySwapchainKHR(device, old_swapchain, allocator);

  // Create the Render Pass
  {
    VkAttachmentDescription attachment = {};
    attachment.format = wd->SurfaceFormat.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR :
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    err = vkCreateRenderPass(device, &info, allocator, &wd->RenderPass);
    check_vk_result(err);

    // We do not create a pipeline by default as this is also used by examples' main.cpp,
    // but secondary viewport in multi-viewport mode may want to create one with:
    // ANCHOR_ImplVulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE, wd->RenderPass,
    // VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, g_Subpass);
  }

  // Create The Image Views
  {
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = wd->SurfaceFormat.format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageSubresourceRange image_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    info.subresourceRange = image_range;
    for (uint32_t i = 0; i < wd->ImageCount; i++) {
      ANCHOR_VulkanGPU_Frame *fd = &wd->Frames[i];
      info.image = fd->Backbuffer;
      err = vkCreateImageView(device, &info, allocator, &fd->BackbufferView);
      check_vk_result(err);
    }
  }

  // Create Framebuffer
  {
    VkImageView attachment[1];
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = wd->RenderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = wd->Width;
    info.height = wd->Height;
    info.layers = 1;
    for (uint32_t i = 0; i < wd->ImageCount; i++) {
      ANCHOR_VulkanGPU_Frame *fd = &wd->Frames[i];
      attachment[0] = fd->BackbufferView;
      err = vkCreateFramebuffer(device, &info, allocator, &fd->Framebuffer);
      check_vk_result(err);
    }
  }
}

// Create or resize window
void ANCHOR_ImplVulkanH_CreateOrResizeWindow(VkInstance instance,
                                             VkPhysicalDevice physical_device,
                                             VkDevice device,
                                             ANCHOR_VulkanGPU_Surface *wd,
                                             uint32_t queue_family,
                                             const VkAllocationCallbacks *allocator,
                                             int width,
                                             int height,
                                             uint32_t min_image_count)
{
  ANCHOR_ASSERT(g_FunctionsLoaded &&
                "Need to call ANCHOR_ImplVulkan_LoadFunctions() if "
                "ANCHOR_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  (void)instance;
  ANCHOR_ImplVulkanH_CreateWindowSwapChain(physical_device,
                                           device,
                                           wd,
                                           allocator,
                                           width,
                                           height,
                                           min_image_count);
  ANCHOR_ImplVulkanH_CreateWindowCommandBuffers(physical_device,
                                                device,
                                                wd,
                                                queue_family,
                                                allocator);
}

void ANCHOR_ImplVulkanH_DestroyWindow(VkInstance instance,
                                      VkDevice device,
                                      ANCHOR_VulkanGPU_Surface *wd,
                                      const VkAllocationCallbacks *allocator)
{
  vkDeviceWaitIdle(device);  // FIXME: We could wait on the Queue if we had the queue in wd->
                             // (otherwise VulkanH functions can't use globals)
  // vkQueueWaitIdle(g_Queue);

  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    ANCHOR_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    ANCHOR_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
  }
  ANCHOR_FREE(wd->Frames);
  ANCHOR_FREE(wd->FrameSemaphores);
  wd->Frames = NULL;
  wd->FrameSemaphores = NULL;
  vkDestroyPipeline(device, wd->Pipeline, allocator);
  vkDestroyRenderPass(device, wd->RenderPass, allocator);
  vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
  vkDestroySurfaceKHR(instance, wd->Surface, allocator);

  *wd = ANCHOR_VulkanGPU_Surface();
}

void ANCHOR_ImplVulkanH_DestroyFrame(VkDevice device,
                                     ANCHOR_VulkanGPU_Frame *fd,
                                     const VkAllocationCallbacks *allocator)
{
  vkDestroyFence(device, fd->Fence, allocator);
  vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
  vkDestroyCommandPool(device, fd->CommandPool, allocator);
  fd->Fence = VK_NULL_HANDLE;
  fd->CommandBuffer = VK_NULL_HANDLE;
  fd->CommandPool = VK_NULL_HANDLE;

  vkDestroyImageView(device, fd->BackbufferView, allocator);
  vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
}

void ANCHOR_ImplVulkanH_DestroyFrameSemaphores(VkDevice device,
                                               ANCHOR_VulkanGPU_FrameSemaphores *fsd,
                                               const VkAllocationCallbacks *allocator)
{
  vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
  vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
  fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
}

void ANCHOR_ImplVulkanH_DestroyFrameRenderBuffers(VkDevice device,
                                                  ANCHOR_VulkanGPU_FrameRenderBuffers *buffers,
                                                  const VkAllocationCallbacks *allocator)
{
  if (buffers->VertexBuffer) {
    vkDestroyBuffer(device, buffers->VertexBuffer, allocator);
    buffers->VertexBuffer = VK_NULL_HANDLE;
  }
  if (buffers->VertexBufferMemory) {
    vkFreeMemory(device, buffers->VertexBufferMemory, allocator);
    buffers->VertexBufferMemory = VK_NULL_HANDLE;
  }
  if (buffers->IndexBuffer) {
    vkDestroyBuffer(device, buffers->IndexBuffer, allocator);
    buffers->IndexBuffer = VK_NULL_HANDLE;
  }
  if (buffers->IndexBufferMemory) {
    vkFreeMemory(device, buffers->IndexBufferMemory, allocator);
    buffers->IndexBufferMemory = VK_NULL_HANDLE;
  }
  buffers->VertexBufferSize = 0;
  buffers->IndexBufferSize = 0;
}

void ANCHOR_ImplVulkanH_DestroyWindowRenderBuffers(VkDevice device,
                                                   ANCHOR_VulkanGPU_SurfaceRenderBuffers *buffers,
                                                   const VkAllocationCallbacks *allocator)
{
  for (uint32_t n = 0; n < buffers->Count; n++)
    ANCHOR_ImplVulkanH_DestroyFrameRenderBuffers(device,
                                                 &buffers->FrameRenderBuffers[n],
                                                 allocator);
  ANCHOR_FREE(buffers->FrameRenderBuffers);
  buffers->FrameRenderBuffers = NULL;
  buffers->Index = 0;
  buffers->Count = 0;
}
