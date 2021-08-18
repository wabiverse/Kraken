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
#ifndef WABI_IMAGING_HGIDX3D_HGI_H
#define WABI_IMAGING_HGIDX3D_HGI_H

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hgiDX3D/api.h"
#include "wabi/imaging/hgiDX3D/dx3d.h"

#include <thread>
#include <vector>

WABI_NAMESPACE_BEGIN

/**
 * Returns true if unlimited FPS is enabled
 * (HGIDX3D_MAX_FPS=true). */
HGIDX3D_API
bool HgiDX3DIsMaxFPSEnabled();

/**
 * @class HgiDX3D
 *
 * DirectX implementation of the Hydra Graphics Interface. */
class HgiDX3D final : public Hgi
{
 public:
  HGIDX3D_API
  HgiDX3D();

  HGIDX3D_API
  ~HgiDX3D() override;

  HGIDX3D_API
  HgiGraphicsCmdsUniquePtr CreateGraphicsCmds(HgiGraphicsCmdsDesc const &desc) override;

  HGIDX3D_API
  HgiBlitCmdsUniquePtr CreateBlitCmds() override;

  HGIDX3D_API
  HgiComputeCmdsUniquePtr CreateComputeCmds() override;

  HGIDX3D_API
  HgiTextureHandle CreateTexture(HgiTextureDesc const &desc) override;

  HGIDX3D_API
  void DestroyTexture(HgiTextureHandle *texHandle) override;

  HGIDX3D_API
  HgiTextureViewHandle CreateTextureView(HgiTextureViewDesc const &desc) override;

  HGIDX3D_API
  void DestroyTextureView(HgiTextureViewHandle *viewHandle) override;

  HGIDX3D_API
  HgiSamplerHandle CreateSampler(HgiSamplerDesc const &desc) override;

  HGIDX3D_API
  void DestroySampler(HgiSamplerHandle *smpHandle) override;

  HGIDX3D_API
  HgiBufferHandle CreateBuffer(HgiBufferDesc const &desc) override;

  HGIDX3D_API
  void DestroyBuffer(HgiBufferHandle *bufHandle) override;

  HGIDX3D_API
  HgiShaderFunctionHandle CreateShaderFunction(HgiShaderFunctionDesc const &desc) override;

  HGIDX3D_API
  void DestroyShaderFunction(HgiShaderFunctionHandle *shaderFunctionHandle) override;

  HGIDX3D_API
  HgiShaderProgramHandle CreateShaderProgram(HgiShaderProgramDesc const &desc) override;

  HGIDX3D_API
  void DestroyShaderProgram(HgiShaderProgramHandle *shaderProgramHandle) override;

  HGIDX3D_API
  HgiResourceBindingsHandle CreateResourceBindings(HgiResourceBindingsDesc const &desc) override;

  HGIDX3D_API
  void DestroyResourceBindings(HgiResourceBindingsHandle *resHandle) override;

  HGIDX3D_API
  HgiGraphicsPipelineHandle CreateGraphicsPipeline(HgiGraphicsPipelineDesc const &pipeDesc) override;

  HGIDX3D_API
  void DestroyGraphicsPipeline(HgiGraphicsPipelineHandle *pipeHandle) override;

  HGIDX3D_API
  HgiComputePipelineHandle CreateComputePipeline(HgiComputePipelineDesc const &pipeDesc) override;

  HGIDX3D_API
  void DestroyComputePipeline(HgiComputePipelineHandle *pipeHandle) override;

  HGIDX3D_API
  TfToken const &GetAPIName() const override;

  HGIDX3D_API
  void StartFrame() override;

  HGIDX3D_API
  void EndFrame() override;

 protected:
  HGIDX3D_API
  bool _SubmitCmds(HgiCmds *cmds, HgiSubmitWaitType wait) override;

 private:
  HgiDX3D &operator=(const HgiDX3D &) = delete;
  HgiDX3D(const HgiDX3D &) = delete;

  /**
   * Perform low frequency actions, such as garbage collection.
   * Thread safety: No. Must be called from main thread. */
  void _EndFrameSync();

  std::thread::id _threadId;
  int _frameDepth;
};

WABI_NAMESPACE_END

#endif /* WABI_IMAGING_HGIDX3D_HGI_H */
