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
#ifndef WABI_IMAGING_HGI_COMPUTE_PIPELINE_H
#define WABI_IMAGING_HGI_COMPUTE_PIPELINE_H

#include "wabi/imaging/hgi/api.h"
#include "wabi/imaging/hgi/attachmentDesc.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgi/handle.h"
#include "wabi/imaging/hgi/resourceBindings.h"
#include "wabi/imaging/hgi/shaderProgram.h"
#include "wabi/imaging/hgi/types.h"
#include "wabi/wabi.h"

#include <string>
#include <vector>

WABI_NAMESPACE_BEGIN

/// \struct HgiComputeShaderConstantsDesc
///
/// A small, but fast buffer of uniform data for shaders.
///
/// <ul>
/// <li>byteSize:
///    Size of the constants in bytes. (max 256 bytes)</li>
/// </ul>
///
struct HgiComputeShaderConstantsDesc
{
  HGI_API
  HgiComputeShaderConstantsDesc();

  uint32_t byteSize;
};

HGI_API
bool operator==(const HgiComputeShaderConstantsDesc &lhs, const HgiComputeShaderConstantsDesc &rhs);

HGI_API
bool operator!=(const HgiComputeShaderConstantsDesc &lhs, const HgiComputeShaderConstantsDesc &rhs);

/// \struct HgiComputePipelineDesc
///
/// Describes the properties needed to create a GPU compute pipeline.
///
/// <ul>
/// <li>shaderProgram:
///   Shader function used in this pipeline.</li>
/// <li>shaderConstantsDesc:
///   Describes the shader uniforms.</li>
/// </ul>
///
struct HgiComputePipelineDesc
{
  HGI_API
  HgiComputePipelineDesc();

  std::string debugName;
  HgiShaderProgramHandle shaderProgram;
  HgiComputeShaderConstantsDesc shaderConstantsDesc;
};

HGI_API
bool operator==(const HgiComputePipelineDesc &lhs, const HgiComputePipelineDesc &rhs);

HGI_API
bool operator!=(const HgiComputePipelineDesc &lhs, const HgiComputePipelineDesc &rhs);

///
/// \class HgiComputePipeline
///
/// Represents a graphics platform independent GPU compute pipeline resource.
///
/// Base class for Hgi compute pipelines.
/// To the client (HdPh) compute pipeline resources are referred to via
/// opaque, stateless handles (HgiComputePipelineHandle).
///
class HgiComputePipeline
{
 public:
  HGI_API
  virtual ~HgiComputePipeline();

  /// The descriptor describes the object.
  HGI_API
  HgiComputePipelineDesc const &GetDescriptor() const;

 protected:
  HGI_API
  HgiComputePipeline(HgiComputePipelineDesc const &desc);

  HgiComputePipelineDesc _descriptor;

 private:
  HgiComputePipeline() = delete;
  HgiComputePipeline &operator=(const HgiComputePipeline &) = delete;
  HgiComputePipeline(const HgiComputePipeline &) = delete;
};

using HgiComputePipelineHandle = HgiHandle<class HgiComputePipeline>;
using HgiComputePipelineHandleVector = std::vector<HgiComputePipelineHandle>;

WABI_NAMESPACE_END

#endif
