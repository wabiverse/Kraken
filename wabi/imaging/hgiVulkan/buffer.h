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
#ifndef WABI_IMAGING_HGIVULKAN_BUFFER_H
#define WABI_IMAGING_HGIVULKAN_BUFFER_H

#include "wabi/imaging/hgi/buffer.h"
#include "wabi/imaging/hgiVulkan/api.h"
#include "wabi/imaging/hgiVulkan/vulkan.h"

WABI_NAMESPACE_BEGIN

class HgiVulkan;
class HgiVulkanCommandBuffer;
class HgiVulkanDevice;

///
/// \class HgiVulkanBuffer
///
/// Vulkan implementation of HgiBuffer
///
class HgiVulkanBuffer final : public HgiBuffer
{
 public:

  HGIVULKAN_API
  ~HgiVulkanBuffer() override;

  HGIVULKAN_API
  size_t GetByteSizeOfResource() const override;

  HGIVULKAN_API
  uint64_t GetRawResource() const override;

  HGIVULKAN_API
  void *GetCPUStagingAddress() override;

  /// Returns true if the provided ptr matches the address of staging buffer.
  HGIVULKAN_API
  bool IsCPUStagingAddress(const void *address) const;

  /// Returns the vulkan buffer.
  HGIVULKAN_API
  VkBuffer GetVulkanBuffer() const;

  /// Returns the memory allocation
  HGIVULKAN_API
  VmaAllocation GetVulkanMemoryAllocation() const;

  /// Returns the staging buffer.
  HGIVULKAN_API
  HgiVulkanBuffer *GetStagingBuffer() const;

  /// Returns the device used to create this object.
  HGIVULKAN_API
  HgiVulkanDevice *GetDevice() const;

  /// Returns the (writable) inflight bits of when this object was trashed.
  HGIVULKAN_API
  uint64_t &GetInflightBits();

  /// Creates a staging buffer.
  /// The caller is responsible for the lifetime (destruction) of the buffer.
  HGIVULKAN_API
  static HgiVulkanBuffer *CreateStagingBuffer(HgiVulkanDevice *device, HgiBufferDesc const &desc);

 protected:

  friend class HgiVulkan;

  // Constructor for making buffers
  HGIVULKAN_API
  HgiVulkanBuffer(HgiVulkan *hgi, HgiVulkanDevice *device, HgiBufferDesc const &desc);

  // Constructor for making staging buffers
  HGIVULKAN_API
  HgiVulkanBuffer(HgiVulkanDevice *device,
                  VkBuffer vkBuffer,
                  VmaAllocation vmaAllocation,
                  HgiBufferDesc const &desc);

 private:

  HgiVulkanBuffer() = delete;
  HgiVulkanBuffer &operator=(const HgiVulkanBuffer &) = delete;
  HgiVulkanBuffer(const HgiVulkanBuffer &) = delete;

  HgiVulkanDevice *_device;
  VkBuffer _vkBuffer;
  VmaAllocation _vmaAllocation;
  uint64_t _inflightBits;
  HgiVulkanBuffer *_stagingBuffer;
  void *_cpuStagingAddress;
};

WABI_NAMESPACE_END

#endif
