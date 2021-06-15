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
#ifndef WABI_IMAGING_HGIVULKAN_SHADERFUNCTION_H
#define WABI_IMAGING_HGIVULKAN_SHADERFUNCTION_H

#include "wabi/imaging/hgi/shaderFunction.h"
#include "wabi/imaging/hgiVulkan/api.h"
#include "wabi/imaging/hgiVulkan/shaderCompiler.h"
#include "wabi/imaging/hgiVulkan/vulkan.h"

WABI_NAMESPACE_BEGIN

class HgiVulkan;
class HgiVulkanDevice;

///
/// \class HgiVulkanShaderFunction
///
/// Vulkan implementation of HgiShaderFunction
///
class HgiVulkanShaderFunction final : public HgiShaderFunction {
 public:
  HGIVULKAN_API
  ~HgiVulkanShaderFunction() override;

  HGIVULKAN_API
  bool IsValid() const override;

  HGIVULKAN_API
  std::string const &GetCompileErrors() override;

  HGIVULKAN_API
  size_t GetByteSizeOfResource() const override;

  HGIVULKAN_API
  uint64_t GetRawResource() const override;

  /// Returns the shader stage this function operates in.
  HGIVULKAN_API
  VkShaderStageFlagBits GetShaderStage() const;

  /// Returns the binary shader module of the shader function.
  HGIVULKAN_API
  VkShaderModule GetShaderModule() const;

  /// Returns the shader entry function name (usually "main").
  HGIVULKAN_API
  const char *GetShaderFunctionName() const;

  /// Returns the descriptor set layout information that describe the
  /// resource bindings for this module. The returned info would usually be
  /// merged with info of other shader modules to create a VkPipelineLayout.
  HGIVULKAN_API
  HgiVulkanDescriptorSetInfoVector const &GetDescriptorSetInfo() const;

  /// Returns the device used to create this object.
  HGIVULKAN_API
  HgiVulkanDevice *GetDevice() const;

  /// Returns the (writable) inflight bits of when this object was trashed.
  HGIVULKAN_API
  uint64_t &GetInflightBits();

 protected:
  friend class HgiVulkan;

  HGIVULKAN_API
  HgiVulkanShaderFunction(HgiVulkanDevice *device, HgiShaderFunctionDesc const &desc);

 private:
  HgiVulkanShaderFunction() = delete;
  HgiVulkanShaderFunction &operator=(const HgiVulkanShaderFunction &) = delete;
  HgiVulkanShaderFunction(const HgiVulkanShaderFunction &) = delete;

  HgiVulkanDevice *_device;
  std::string _errors;
  size_t _spirvByteSize;
  VkShaderModule _vkShaderModule;
  HgiVulkanDescriptorSetInfoVector _descriptorSetInfo;
  uint64_t _inflightBits;
};

WABI_NAMESPACE_END

#endif
