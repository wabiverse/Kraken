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
#ifndef WABI_IMAGING_VKF_DEVICE_CAPS_H
#define WABI_IMAGING_VKF_DEVICE_CAPS_H

#include "wabi/base/tf/singleton.h"
#include "wabi/imaging/vkf/api.h"
#include "wabi/wabi.h"

#include <vulkan/vulkan.h>

WABI_NAMESPACE_BEGIN

/**
 * @class VkfDeviceCaps
 *
 * This class is intended to be a cache of the capabilites
 * (resource limits and features) of the underlying
 * Vulkan Instance.
 *
 * It serves two purposes. Firstly to reduce driver
 * transition overhead of querying these values.
 * Secondly to provide access to the VkDevice, the
 * main handle and represents a logical connection -
 * i.e. 'I am running Vulkan on this GPU'. A VKDevice
 * is equivalent of a GL Context or a D3D11 device.
 *
 * In the event of failure (InitInstance() wasn't called
 * or an issue accessing the VKDevice), a reasonable set
 * of defaults, based on VkDevice Feature minumums, is
 * provided. */

class VkfDeviceCaps {
 public:
  /**
   * InitInstance queries the VkInstance's VkDevice for its properties,
   * features, and capabilities. It should be called by the application
   * before using systems that depend on the caps, such as Hydra. A good
   * example would be to pair the call to initialize after a call to
   * create a VkInstance. */
  VKF_API
  static void InitInstance();

  /**
   * GetInstance() returns the filled capabilities structure.
   * This function will not populate the caps and will issue a
   * coding error if it hasn't been filled. */
  VKF_API
  static const VkfDeviceCaps &GetInstance();

  /**
   * Vulkan version. */
  uint32_t apiVersion;    /** <-- VULKAN API 12.155 (v1.2.155)          */
  uint32_t driverVersion; /** <-- NVIDIA DRIVER 460.29264 (v460.292.64) */
  uint32_t vendorID;
  uint32_t deviceID;

  VkPhysicalDeviceType deviceType;
  char deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];

  uint8_t pipelineCacheUUID[VK_UUID_SIZE];
  VkPhysicalDeviceLimits limits;
  VkPhysicalDeviceSparseProperties sparseProperties;

  VkInstance vkInstance;

 private:
  void VULKANDEVICE_LoadCaps();
  VkfDeviceCaps();
  ~VkfDeviceCaps() = default;

  /** Disallow copies */
  VkfDeviceCaps(const VkfDeviceCaps &) = delete;
  VkfDeviceCaps &operator=(const VkfDeviceCaps &) = delete;

  friend class TfSingleton<VkfDeviceCaps>;
};

VKF_API_TEMPLATE_CLASS(TfSingleton<VkfDeviceCaps>);

WABI_NAMESPACE_END

#endif /* WABI_IMAGING_VKF_DEVICE_CAPS_H */
