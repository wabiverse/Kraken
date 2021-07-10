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
#ifndef WABI_IMAGING_HGIVULKAN_GARBAGE_COLLECTOR_H
#define WABI_IMAGING_HGIVULKAN_GARBAGE_COLLECTOR_H

#include "wabi/base/tf/diagnostic.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgiVulkan/api.h"

#include <mutex>
#include <vector>

WABI_NAMESPACE_BEGIN

class HgiVulkan;
class HgiVulkanDevice;

using HgiVulkanBufferVector = std::vector<class HgiVulkanBuffer *>;
using HgiVulkanTextureVector = std::vector<class HgiVulkanTexture *>;
using HgiVulkanSamplerVector = std::vector<class HgiVulkanSampler *>;
using HgiVulkanShaderFunctionVector = std::vector<class HgiVulkanShaderFunction *>;
using HgiVulkanShaderProgramVector = std::vector<class HgiVulkanShaderProgram *>;
using HgiVulkanResourceBindingsVector = std::vector<class HgiVulkanResourceBindings *>;
using HgiVulkanGraphicsPipelineVector = std::vector<class HgiVulkanGraphicsPipeline *>;
using HgiVulkanComputePipelineVector = std::vector<class HgiVulkanComputePipeline *>;

/// \class HgiVulkanGarbageCollector
///
/// Handles garbage collection of vulkan objects by delaying their destruction
/// until those objects are no longer used.
///
class HgiVulkanGarbageCollector final
{
 public:
  HGIVULKAN_API
  HgiVulkanGarbageCollector(HgiVulkan *hgi);

  HGIVULKAN_API
  ~HgiVulkanGarbageCollector();

  /// Destroys the objects inside the garbage collector.
  /// Thread safety: This call is not thread safe and should only be called
  /// while no other threads are destroying objects (e.g. during EndFrame).
  HGIVULKAN_API
  void PerformGarbageCollection(HgiVulkanDevice *device);

  /// Returns a garbage collection vector for a type of handle.
  /// Thread safety: The returned vector is a thread_local vector so this call
  /// is thread safe as long as the vector is only used by the calling thread.
  HGIVULKAN_API
  HgiVulkanBufferVector *GetBufferList();

  HGIVULKAN_API
  HgiVulkanTextureVector *GetTextureList();

  HGIVULKAN_API
  HgiVulkanSamplerVector *GetSamplerList();

  HGIVULKAN_API
  HgiVulkanShaderFunctionVector *GetShaderFunctionList();

  HGIVULKAN_API
  HgiVulkanShaderProgramVector *GetShaderProgramList();

  HGIVULKAN_API
  HgiVulkanResourceBindingsVector *GetResourceBindingsList();

  HGIVULKAN_API
  HgiVulkanGraphicsPipelineVector *GetGraphicsPipelineList();

  HGIVULKAN_API
  HgiVulkanComputePipelineVector *GetComputePipelineList();

 private:
  HgiVulkanGarbageCollector &operator=(const HgiVulkanGarbageCollector &) = delete;
  HgiVulkanGarbageCollector(const HgiVulkanGarbageCollector &) = delete;

  /// Returns a thread_local vector in which to store a object handle.
  /// Thread safety: The returned vector is a thread_local vector so this call
  /// is thread safe as long as the vector is only used by the calling thread.
  template<class T>
  T *_GetThreadLocalStorageList(std::vector<T *> *collector);

  HgiVulkan *_hgi;

  // List of all the per-thread-vectors of objects that need to be destroyed.
  // The vectors are static (shared across HGIs), because we use thread_local
  // in _GetThreadLocalStorageList which makes us share the garbage collector
  // vectors across Hgi instances.
  static std::vector<HgiVulkanBufferVector *> _bufferList;
  static std::vector<HgiVulkanTextureVector *> _textureList;
  static std::vector<HgiVulkanSamplerVector *> _samplerList;
  static std::vector<HgiVulkanShaderFunctionVector *> _shaderFunctionList;
  static std::vector<HgiVulkanShaderProgramVector *> _shaderProgramList;
  static std::vector<HgiVulkanResourceBindingsVector *> _resourceBindingsList;
  static std::vector<HgiVulkanGraphicsPipelineVector *> _graphicsPipelineList;
  static std::vector<HgiVulkanComputePipelineVector *> _computePipelineList;

  bool _isDestroying;
};

WABI_NAMESPACE_END

#endif
