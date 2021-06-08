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
#ifndef WABI_IMAGING_HGIVULKAN_SHADERCOMPILER_H
#define WABI_IMAGING_HGIVULKAN_SHADERCOMPILER_H

#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgiVulkan/api.h"
#include "wabi/imaging/hgiVulkan/vulkan.h"
#include "wabi/wabi.h"

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

WABI_NAMESPACE_BEGIN

class HgiVulkanDevice;

struct HgiVulkanDescriptorSetInfo {
  uint32_t setNumber;
  VkDescriptorSetLayoutCreateInfo createInfo;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

using HgiVulkanDescriptorSetInfoVector = std::vector<HgiVulkanDescriptorSetInfo>;

using VkDescriptorSetLayoutVector = std::vector<VkDescriptorSetLayout>;

/// Compiles ascii shader code (glsl) into spirv binary code (spirvOut).
/// Returns true if successful. Errors can optionally be captured.
/// numShaderCodes determines how many strings are provided via shaderCodes.
/// 'name' is purely for debugging compile errors. It can be anything.
HGIVULKAN_API
bool HgiVulkanCompileGLSL(const char *name,
                          const char *shaderCodes[],
                          uint8_t numShaderCodes,
                          HgiShaderStage stage,
                          std::vector<unsigned int> *spirvOUT,
                          std::string *errors = nullptr);

/// Uses spirv-reflection to create new descriptor set layout information for
/// the provided spirv.
/// This information can be merged with the info of the other shader stage
/// modules to create the pipeline layout.
/// During Hgi pipeline layout creation we know the shader modules
/// (HgiShaderProgram), but not the HgiResourceBindings so we must use
/// spirv reflection to discover the descriptorSet info for the module.
HGIVULKAN_API
HgiVulkanDescriptorSetInfoVector HgiVulkanGatherDescriptorSetInfo(
    std::vector<unsigned int> const &spirv);

/// Given all of the DescriptorSetInfos of all of the shader modules in a
/// shader program, this function merges them and creates the descriptorSet
/// layouts needed during pipeline layout creation.
/// The caller takes ownership of the returned layouts and must destroy them.
HGIVULKAN_API
VkDescriptorSetLayoutVector HgiVulkanMakeDescriptorSetLayouts(
    HgiVulkanDevice *device,
    std::vector<HgiVulkanDescriptorSetInfoVector> const &infos,
    std::string const &debugName);

WABI_NAMESPACE_END

#endif
