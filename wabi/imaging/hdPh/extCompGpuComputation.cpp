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
#include "wabi/imaging/hdPh/extCompGpuComputation.h"
#include "wabi/base/tf/hash.h"
#include "wabi/imaging/glf/diagnostic.h"
#include "wabi/imaging/hd/compExtCompInputSource.h"
#include "wabi/imaging/hd/extCompCpuComputation.h"
#include "wabi/imaging/hd/extCompPrimvarBufferSource.h"
#include "wabi/imaging/hd/extComputation.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/sceneExtCompInputSource.h"
#include "wabi/imaging/hd/vtBufferSource.h"
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/bufferResource.h"
#include "wabi/imaging/hdPh/extCompGpuComputationBufferSource.h"
#include "wabi/imaging/hdPh/extCompGpuPrimvarBufferSource.h"
#include "wabi/imaging/hdPh/extComputation.h"
#include "wabi/imaging/hdPh/glslProgram.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hgi/computeCmds.h"
#include "wabi/imaging/hgi/computePipeline.h"
#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/shaderProgram.h"
#include "wabi/imaging/hgi/tokens.h"

#include <limits>

WABI_NAMESPACE_BEGIN

static void _AppendResourceBindings(HgiResourceBindingsDesc *resourceDesc,
                                    HgiBufferHandle const &buffer,
                                    uint32_t location)
{
  HgiBufferBindDesc bufBind;
  bufBind.bindingIndex = location;
  bufBind.resourceType = HgiBindResourceTypeStorageBuffer;
  bufBind.stageUsage   = HgiShaderStageCompute;
  bufBind.offsets.push_back(0);
  bufBind.buffers.push_back(buffer);
  resourceDesc->buffers.push_back(std::move(bufBind));
}

static HgiComputePipelineSharedPtr _CreatePipeline(Hgi *hgi,
                                                   uint32_t constantValuesSize,
                                                   HgiShaderProgramHandle const &program)
{
  HgiComputePipelineDesc desc;
  desc.debugName                    = "ExtComputation";
  desc.shaderProgram                = program;
  desc.shaderConstantsDesc.byteSize = constantValuesSize;
  return std::make_shared<HgiComputePipelineHandle>(hgi->CreateComputePipeline(desc));
}

HdPhExtCompGpuComputation::HdPhExtCompGpuComputation(
    SdfPath const &id,
    HdPhExtCompGpuComputationResourceSharedPtr const &resource,
    HdExtComputationPrimvarDescriptorVector const &compPrimvars,
    int dispatchCount,
    int elementCount)
    : HdComputation(),
      _id(id),
      _resource(resource),
      _compPrimvars(compPrimvars),
      _dispatchCount(dispatchCount),
      _elementCount(elementCount)
{}

static std::string _GetDebugPrimvarNames(
    HdExtComputationPrimvarDescriptorVector const &compPrimvars)
{
  std::string result;
  for (HdExtComputationPrimvarDescriptor const &compPrimvar : compPrimvars) {
    result += " '";
    result += compPrimvar.name;
    result += "'";
  }
  return result;
}

void HdPhExtCompGpuComputation::Execute(HdBufferArrayRangeSharedPtr const &outputRange,
                                        HdResourceRegistry *resourceRegistry)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  TF_VERIFY(outputRange);
  TF_VERIFY(resourceRegistry);

  TF_DEBUG(HD_EXT_COMPUTATION_UPDATED)
      .Msg("GPU computation '%s' executed for primvars: %s\n",
           _id.GetText(),
           _GetDebugPrimvarNames(_compPrimvars).c_str());

  HdPhResourceRegistry *hdPhResourceRegistry = static_cast<HdPhResourceRegistry *>(
      resourceRegistry);
  HdPhGLSLProgramSharedPtr const &computeProgram = _resource->GetProgram();
  HdPh_ResourceBinder const &binder              = _resource->GetResourceBinder();

  if (!TF_VERIFY(computeProgram)) {
    return;
  }

  HdPhBufferArrayRangeSharedPtr outputBar = std::static_pointer_cast<HdPhBufferArrayRange>(
      outputRange);
  TF_VERIFY(outputBar);

  // Prepare uniform buffer for GPU computation
  // XXX: We'd really prefer to delegate this to the resource binder.
  std::vector<int32_t> _uniforms;
  _uniforms.push_back(outputBar->GetElementOffset());

  // Generate hash for resource bindings and pipeline.
  // XXX Needs fingerprint hash to avoid collisions
  uint64_t rbHash = 0;

  // Bind buffers as SSBOs to the indices matching the layout in the shader
  for (HdExtComputationPrimvarDescriptor const &compPrimvar : _compPrimvars) {
    TfToken const &name                       = compPrimvar.sourceComputationOutputName;
    HdPhBufferResourceSharedPtr const &buffer = outputBar->GetResource(compPrimvar.name);

    HdBinding const &binding = binder.GetBinding(name);
    // These should all be valid as they are required outputs
    if (TF_VERIFY(binding.IsValid()) && TF_VERIFY(buffer->GetHandle())) {
      size_t componentSize = HdDataSizeOfType(HdGetComponentType(buffer->GetTupleType().type));
      _uniforms.push_back(buffer->GetOffset() / componentSize);
      // Assumes non-SSBO allocator for the stride
      _uniforms.push_back(buffer->GetStride() / componentSize);

      rbHash = TfHash::Combine(rbHash, buffer->GetHandle().Get());
    }
  }

  for (HdBufferArrayRangeSharedPtr const &input : _resource->GetInputs()) {
    HdPhBufferArrayRangeSharedPtr const &inputBar = std::static_pointer_cast<HdPhBufferArrayRange>(
        input);

    for (HdPhBufferResourceNamedPair const &it : inputBar->GetResources()) {
      TfToken const &name                       = it.first;
      HdPhBufferResourceSharedPtr const &buffer = it.second;

      HdBinding const &binding = binder.GetBinding(name);
      // These should all be valid as they are required inputs
      if (TF_VERIFY(binding.IsValid())) {
        HdTupleType tupleType = buffer->GetTupleType();
        size_t componentSize  = HdDataSizeOfType(HdGetComponentType(tupleType.type));
        _uniforms.push_back((inputBar->GetByteOffset(name) + buffer->GetOffset()) / componentSize);
        // If allocated with a VBO allocator use the line below instead.
        //_uniforms.push_back(
        //    buffer->GetStride() / buffer->GetComponentSize());
        // This is correct for the SSBO allocator only
        _uniforms.push_back(HdGetComponentCount(tupleType.type));

        if (binding.GetType() != HdBinding::SSBO) {
          TF_RUNTIME_ERROR("Unsupported binding type %d for ExtComputation", binding.GetType());
        }

        rbHash = TfHash::Combine(rbHash, buffer->GetHandle().Get());
      }
    }
  }

  Hgi *hgi = hdPhResourceRegistry->GetHgi();

  // Prepare uniform buffer for GPU computation
  const size_t uboSize = sizeof(int32_t) * _uniforms.size();
  uint64_t pHash       = (uint64_t)TfHash::Combine(computeProgram->GetProgram().Get(), uboSize);

  // Get or add pipeline in registry.
  HdInstance<HgiComputePipelineSharedPtr> computePipelineInstance =
      hdPhResourceRegistry->RegisterComputePipeline(pHash);
  if (computePipelineInstance.IsFirstInstance()) {
    HgiComputePipelineSharedPtr pipe = _CreatePipeline(hgi, uboSize, computeProgram->GetProgram());
    computePipelineInstance.SetValue(pipe);
  }

  HgiComputePipelineSharedPtr const &pipelinePtr = computePipelineInstance.GetValue();
  HgiComputePipelineHandle pipeline              = *pipelinePtr.get();

  // Get or add resource bindings in registry.
  HdInstance<HgiResourceBindingsSharedPtr> resourceBindingsInstance =
      hdPhResourceRegistry->RegisterResourceBindings(rbHash);
  if (resourceBindingsInstance.IsFirstInstance()) {
    // Begin the resource set
    HgiResourceBindingsDesc resourceDesc;
    resourceDesc.debugName = "ExtComputation";

    for (HdExtComputationPrimvarDescriptor const &compPvar : _compPrimvars) {
      TfToken const &name                       = compPvar.sourceComputationOutputName;
      HdPhBufferResourceSharedPtr const &buffer = outputBar->GetResource(compPvar.name);

      HdBinding const &binding = binder.GetBinding(name);
      // These should all be valid as they are required outputs
      if (TF_VERIFY(binding.IsValid()) && TF_VERIFY(buffer->GetHandle())) {
        _AppendResourceBindings(&resourceDesc, buffer->GetHandle(), binding.GetLocation());
      }
    }

    for (HdBufferArrayRangeSharedPtr const &input : _resource->GetInputs()) {
      HdPhBufferArrayRangeSharedPtr const &inputBar =
          std::static_pointer_cast<HdPhBufferArrayRange>(input);

      for (HdPhBufferResourceNamedPair const &it : inputBar->GetResources()) {
        TfToken const &name                       = it.first;
        HdPhBufferResourceSharedPtr const &buffer = it.second;

        HdBinding const &binding = binder.GetBinding(name);
        // These should all be valid as they are required inputs
        if (TF_VERIFY(binding.IsValid())) {
          _AppendResourceBindings(&resourceDesc, buffer->GetHandle(), binding.GetLocation());
        }
      }
    }

    HgiResourceBindingsSharedPtr rb = std::make_shared<HgiResourceBindingsHandle>(
        hgi->CreateResourceBindings(resourceDesc));

    resourceBindingsInstance.SetValue(rb);
  }

  HgiResourceBindingsSharedPtr const &resourceBindindsPtr = resourceBindingsInstance.GetValue();
  HgiResourceBindingsHandle resourceBindings              = *resourceBindindsPtr.get();

  HgiComputeCmds *computeCmds = hdPhResourceRegistry->GetGlobalComputeCmds();

  computeCmds->PushDebugGroup("ExtComputation");
  computeCmds->BindResources(resourceBindings);
  computeCmds->BindPipeline(pipeline);

  // Queue transfer uniform buffer
  computeCmds->SetConstantValues(pipeline, 0, uboSize, &_uniforms[0]);

  // Queue compute work
  computeCmds->Dispatch(GetDispatchCount(), 1);

  computeCmds->PopDebugGroup();
}

void HdPhExtCompGpuComputation::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  // nothing
}

int HdPhExtCompGpuComputation::GetDispatchCount() const
{
  return _dispatchCount;
}

int HdPhExtCompGpuComputation::GetNumOutputElements() const
{
  return _elementCount;
}

HdPhExtCompGpuComputationResourceSharedPtr const &HdPhExtCompGpuComputation::GetResource() const
{
  return _resource;
}

/* static */
HdPhExtCompGpuComputationSharedPtr HdPhExtCompGpuComputation::CreateGpuComputation(
    HdSceneDelegate *sceneDelegate,
    HdExtComputation const *sourceComp,
    HdExtComputationPrimvarDescriptorVector const &compPrimvars)
{
  TF_DEBUG(HD_EXT_COMPUTATION_UPDATED)
      .Msg("GPU computation '%s' created for primvars: %s\n",
           sourceComp->GetId().GetText(),
           _GetDebugPrimvarNames(compPrimvars).c_str());

  // Downcast the resource registry
  HdRenderIndex &renderIndex = sceneDelegate->GetRenderIndex();
  HdPhResourceRegistrySharedPtr const &resourceRegistry =
      std::dynamic_pointer_cast<HdPhResourceRegistry>(renderIndex.GetResourceRegistry());

  HdPh_ExtCompComputeShaderSharedPtr shader = std::make_shared<HdPh_ExtCompComputeShader>(
      sourceComp);

  // Map the computation outputs onto the destination primvar types
  HdBufferSpecVector outputBufferSpecs;
  outputBufferSpecs.reserve(compPrimvars.size());
  for (HdExtComputationPrimvarDescriptor const &compPrimvar : compPrimvars) {
    outputBufferSpecs.emplace_back(compPrimvar.sourceComputationOutputName, compPrimvar.valueType);
  }

  HdPhExtComputation const *deviceSourceComp = static_cast<HdPhExtComputation const *>(sourceComp);
  if (!TF_VERIFY(deviceSourceComp)) {
    return nullptr;
  }
  HdBufferArrayRangeSharedPtrVector inputs;
  inputs.push_back(deviceSourceComp->GetInputRange());

  for (HdExtComputationInputDescriptor const &desc : sourceComp->GetComputationInputs()) {
    HdPhExtComputation const *deviceInputComp = static_cast<HdPhExtComputation const *>(
        renderIndex.GetSprim(HdPrimTypeTokens->extComputation, desc.sourceComputationId));
    if (deviceInputComp && deviceInputComp->GetInputRange()) {
      HdBufferArrayRangeSharedPtr input = deviceInputComp->GetInputRange();
      // skip duplicate inputs
      if (std::find(inputs.begin(), inputs.end(), input) == inputs.end()) {
        inputs.push_back(deviceInputComp->GetInputRange());
      }
    }
  }

  // There is a companion resource that requires allocation
  // and resolution.
  HdPhExtCompGpuComputationResourceSharedPtr resource(
      new HdPhExtCompGpuComputationResource(outputBufferSpecs, shader, inputs, resourceRegistry));

  return HdPhExtCompGpuComputationSharedPtr(
      new HdPhExtCompGpuComputation(sourceComp->GetId(),
                                    resource,
                                    compPrimvars,
                                    sourceComp->GetDispatchCount(),
                                    sourceComp->GetElementCount()));
}

void HdPh_GetExtComputationPrimvarsComputations(
    SdfPath const &id,
    HdSceneDelegate *sceneDelegate,
    HdExtComputationPrimvarDescriptorVector const &allCompPrimvars,
    HdDirtyBits dirtyBits,
    HdBufferSourceSharedPtrVector *sources,
    HdBufferSourceSharedPtrVector *reserveOnlySources,
    HdBufferSourceSharedPtrVector *separateComputationSources,
    HdPhComputationSharedPtrVector *computations)
{
  TF_VERIFY(sources);
  TF_VERIFY(reserveOnlySources);
  TF_VERIFY(separateComputationSources);
  TF_VERIFY(computations);

  HdRenderIndex &renderIndex = sceneDelegate->GetRenderIndex();

  // Group computation primvars by source computation
  typedef std::map<SdfPath, HdExtComputationPrimvarDescriptorVector> CompPrimvarsByComputation;
  CompPrimvarsByComputation byComputation;
  for (HdExtComputationPrimvarDescriptor const &compPrimvar : allCompPrimvars) {
    byComputation[compPrimvar.sourceComputationId].push_back(compPrimvar);
  }

  // Create computation primvar buffer sources by source computation
  for (CompPrimvarsByComputation::value_type it : byComputation) {
    SdfPath const &computationId                                = it.first;
    HdExtComputationPrimvarDescriptorVector const &compPrimvars = it.second;

    HdExtComputation const *sourceComp = static_cast<HdExtComputation const *>(
        renderIndex.GetSprim(HdPrimTypeTokens->extComputation, computationId));

    if (!(sourceComp && sourceComp->GetElementCount() > 0)) {
      continue;
    }

    if (!sourceComp->GetGpuKernelSource().empty()) {

      HdPhExtCompGpuComputationSharedPtr gpuComputation;
      for (HdExtComputationPrimvarDescriptor const &compPrimvar : compPrimvars) {

        if (HdChangeTracker::IsPrimvarDirty(dirtyBits, id, compPrimvar.name)) {

          if (!gpuComputation) {
            // Create the computation for the first dirty primvar
            gpuComputation = HdPhExtCompGpuComputation::CreateGpuComputation(
                sceneDelegate, sourceComp, compPrimvars);

            HdBufferSourceSharedPtr gpuComputationSource(new HdPhExtCompGpuComputationBufferSource(
                HdBufferSourceSharedPtrVector(), gpuComputation->GetResource()));

            separateComputationSources->push_back(gpuComputationSource);
            // Assume there are no dependencies between ExtComp so
            // put all of them in queue zero.
            computations->emplace_back(gpuComputation, HdPhComputeQueueZero);
          }

          // Create a primvar buffer source for the computation
          HdBufferSourceSharedPtr primvarBufferSource(
              new HdPhExtCompGpuPrimvarBufferSource(compPrimvar.name,
                                                    compPrimvar.valueType,
                                                    sourceComp->GetElementCount(),
                                                    sourceComp->GetId()));

          // Gpu primvar sources only need to reserve space
          reserveOnlySources->push_back(primvarBufferSource);
        }
      }
    }
    else {

      HdExtCompCpuComputationSharedPtr cpuComputation;
      for (HdExtComputationPrimvarDescriptor const &compPrimvar : compPrimvars) {

        if (HdChangeTracker::IsPrimvarDirty(dirtyBits, id, compPrimvar.name)) {

          if (!cpuComputation) {
            // Create the computation for the first dirty primvar
            cpuComputation = HdExtCompCpuComputation::CreateComputation(
                sceneDelegate, *sourceComp, separateComputationSources);
          }

          // Create a primvar buffer source for the computation
          HdBufferSourceSharedPtr primvarBufferSource(
              new HdExtCompPrimvarBufferSource(compPrimvar.name,
                                               cpuComputation,
                                               compPrimvar.sourceComputationOutputName,
                                               compPrimvar.valueType));

          // Cpu primvar sources need to allocate and commit data
          sources->push_back(primvarBufferSource);
        }
      }
    }
  }
}

WABI_NAMESPACE_END
