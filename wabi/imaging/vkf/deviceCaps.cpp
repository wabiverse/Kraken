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

#include "wabi/imaging/vkf/deviceCaps.h"
#include "wabi/imaging/vkf/debugCodes.h"

#include "wabi/base/arch/systemInfo.h"

#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/instantiateSingleton.h"
#include "wabi/base/tf/stringUtils.h"

#include <iostream>
#include <mutex>
#include <optional>

WABI_NAMESPACE_BEGIN

TF_INSTANTIATE_SINGLETON(VkfDeviceCaps);

namespace {

struct _QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;

  bool IsComplete()
  {
    return graphicsFamily.has_value();
  }
};

_QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice device)
{
  _QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    if (indices.IsComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

bool _IsDeviceSuitable(VkPhysicalDevice device)
{
  _QueueFamilyIndices indices = _FindQueueFamilies(device);
  return indices.IsComplete();
}

std::vector<const char *> _GetRequiredExtensions()
{
  std::vector<const char *> extensions
  {
#if defined(ARCH_OS_LINUX)
    "VK_KHR_surface", "VK_KHR_xcb_surface",
#elif defined(ARCH_OS_WINDOWS)
    "VK_KHR_surface", "VK_KHR_win32_surface",
#endif /* ARCH_OS_LINUX */
  };
  return extensions;
}

const char *_GetVendorFromID(uint32_t id)
{
  switch (id) {
    case VK_VENDOR_ID_CODEPLAY:
      return "CODEPLAY";

    case VK_VENDOR_ID_KAZAN:
      return "KAZAN";

    case VK_VENDOR_ID_MESA:
      return "MESA";

    case VK_VENDOR_ID_VIV:
      return "VIV";

    case VK_VENDOR_ID_VSI:
      return "VSI";

    default:
      return "PROPRIETARY";
  }
}

const char *_GetDeviceTypeFromID(uint32_t id)
{
  switch (id) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return "OTHER";

    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return "INTEGRATED GPU";

    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return "DISCRETE GPU";

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return "VIRTUAL GPU";

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return "CPU";

    default:
      return "UNKNOWN";
  }
}

}  // namespace

/**
 * Initialize members to ensure a sane starting state. */
VkfDeviceCaps::VkfDeviceCaps()
    : apiVersion(0),
      driverVersion(0),
      vendorID(0),
      deviceID(0),
      deviceType(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU),
      deviceName(""),
      pipelineCacheUUID({0}),
      limits(),
      sparseProperties()
{}

/* static */
void VkfDeviceCaps::InitInstance()
{
  VkfDeviceCaps &caps = TfSingleton<VkfDeviceCaps>::GetInstance();

  //   GarchVkApiLoad();

  caps.VULKANDEVICE_LoadCaps();
}

/* static */
const VkfDeviceCaps &VkfDeviceCaps::GetInstance()
{
  VkfDeviceCaps &caps = TfSingleton<VkfDeviceCaps>::GetInstance();

  if (caps.apiVersion == 0) {
    TF_CODING_ERROR("VkfDeviceCaps has not been initialized");
  }

  return caps;
}

void VkfDeviceCaps::VULKANDEVICE_LoadCaps()
{
  /**
   * Reset Values to reasonable defaults based of VkDevice mins.
   * So that if we early out, systems can still depend on the
   * caps values being valid.
   *
   * VULKANDEVICE_LoadCaps can also be called multiple times, so
   * not want to mix and match values in the event of an early out. */
  apiVersion       = 0;
  driverVersion    = 0;
  vendorID         = 0;
  deviceID         = 0;
  deviceType       = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU;
  limits           = VkPhysicalDeviceLimits();
  sparseProperties = VkPhysicalDeviceSparseProperties();
  strcpy(deviceName, "N/A");
  memset(pipelineCacheUUID, 0, sizeof(uint8_t) * VK_UUID_SIZE);

  VkApplicationInfo appInfo = {};
  appInfo.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext             = NULL;
  appInfo.pApplicationName  = "Universal Scene Description";
  appInfo.pEngineName       = "Pixar Hydra";
  appInfo.apiVersion        = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext                = NULL;
  createInfo.flags                = 0;
  createInfo.pApplicationInfo     = &appInfo;

  auto extensions                    = _GetRequiredExtensions();
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  vkCreateInstance(&createInfo, NULL, &vkInstance);

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount            = 0;
  vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
  if (!TF_VERIFY(deviceCount != 0))
    TF_CODING_ERROR("Failed to find GPUs with Vulkan support");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
  for (const auto &device : devices) {
    if (_IsDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    TF_CODING_ERROR("Failed to find a suitable GPU");
  }

  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);

  apiVersion       = props.apiVersion;
  driverVersion    = props.driverVersion;
  vendorID         = props.vendorID;
  deviceID         = props.deviceID;
  deviceType       = props.deviceType;
  limits           = props.limits;
  sparseProperties = props.sparseProperties;
  strcpy(deviceName, props.deviceName);
  memcpy(pipelineCacheUUID, props.pipelineCacheUUID, sizeof(uint8_t) * VK_UUID_SIZE);

  if (TfDebug::IsEnabled(VKF_DEBUG_DEVICE_CAPS)) {
    std::cout << "VkfDeviceCaps: \n"
              << "  APPLICATION NAME  = " << TfStringify(appInfo.pApplicationName) << "\n"
              << "  ENGINE NAME       = " << TfStringify(appInfo.pEngineName) << "\n"
              << "  VULKAN API        = " << TfStringify(VK_VERSION_MAJOR(apiVersion)) << "."
              << TfStringify(VK_VERSION_MINOR(apiVersion)) << "."
              << TfStringify(VK_VERSION_PATCH(apiVersion)) << "\n"
              << "  DRIVER VERSION    = " << TfStringify(VK_VERSION_MAJOR(driverVersion)) << "."
              << TfStringify(VK_VERSION_MINOR(driverVersion)) << "."
              << TfStringify(VK_VERSION_PATCH(driverVersion)) << "\n"
              << "  VENDOR ID         = " << TfStringify(_GetVendorFromID(vendorID)) << "\n"
              << "  DEVICE TYPE       = " << TfStringify(_GetDeviceTypeFromID(deviceType)) << "\n"
              << "  DEVICE NAME       = " << TfStringify(deviceName) << "\n";
  }
}

WABI_NAMESPACE_END
