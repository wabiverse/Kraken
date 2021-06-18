/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#include "wabi/imaging/hgiVulkan/instance.h"
#include "wabi/imaging/hgiVulkan/diagnostic.h"

#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/iterator.h"

#include <vector>

WABI_NAMESPACE_BEGIN

HgiVulkanInstance::HgiVulkanInstance()
  : vkDebugMessenger(nullptr),
    vkCreateDebugUtilsMessengerEXT(nullptr),
    vkDestroyDebugUtilsMessengerEXT(nullptr),
    _vkInstance(nullptr)
{
  VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo = &appInfo;

  // Setup instance extensions.
  std::vector<const char *> extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,

// Pick platform specific surface extension
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#else
#  error Unsupported Platform
#endif

    // Extensions for interop with OpenGL
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,

    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
  };

  // Enable validation layers extension.
  // Requires VK_LAYER_PATH to be set.
  if (HgiVulkanIsDebugEnabled())
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    const char *debugLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.ppEnabledLayerNames = debugLayers;
    createInfo.enabledLayerCount = (uint32_t)TfArraySize(debugLayers);
  }

  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledExtensionCount = (uint32_t)extensions.size();

  TF_VERIFY(vkCreateInstance(&createInfo, HgiVulkanAllocator(), &_vkInstance) == VK_SUCCESS);

  HgiVulkanCreateDebug(this);
}

HgiVulkanInstance::HgiVulkanInstance(VkInstance &existingInst)
  : vkDebugMessenger(nullptr),
    vkCreateDebugUtilsMessengerEXT(nullptr),
    vkDestroyDebugUtilsMessengerEXT(nullptr),
    _vkInstance(existingInst)
{
  HgiVulkanCreateDebug(this);
}

HgiVulkanInstance::~HgiVulkanInstance()
{
  HgiVulkanDestroyDebug(this);
  vkDestroyInstance(_vkInstance, HgiVulkanAllocator());
}

VkInstance const &HgiVulkanInstance::GetVulkanInstance() const
{
  return _vkInstance;
}

WABI_NAMESPACE_END
