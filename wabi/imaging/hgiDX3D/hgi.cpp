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
#include "wabi/imaging/hgiDX3D/hgi.h"

#include "wabi/base/trace/trace.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/registryManager.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_ENV_SETTING(HGIDX3DMAX_FPS, true, "Enable maximum frames per second");

bool HgiDX3DIsMaxFPSEnabled()
{
  static bool _v = TfGetEnvSetting(HGIDX3DMAX_FPS) == true;
  return _v;
}

TF_REGISTRY_FUNCTION(TfType)
{
  TfType t = TfType::Define<HgiDX3D, TfType::Bases<Hgi>>();
  t.SetFactory<HgiFactory<HgiDX3D>>();
}

HgiDX3D::HgiDX3D() : _threadId(std::this_thread::get_id()), _frameDepth(0) {}

HgiDX3D::~HgiDX3D() {}

/* Multi threaded */
HgiGraphicsCmdsUniquePtr HgiDX3D::CreateGraphicsCmds(HgiGraphicsCmdsDesc const &desc)
{
  return HgiGraphicsCmdsUniquePtr(nullptr);
}

/* Multi threaded */
HgiBlitCmdsUniquePtr HgiDX3D::CreateBlitCmds()
{
  return HgiBlitCmdsUniquePtr(nullptr);
}

HgiComputeCmdsUniquePtr HgiDX3D::CreateComputeCmds()
{
  return HgiComputeCmdsUniquePtr(nullptr);
}

/* Multi threaded */
HgiTextureHandle HgiDX3D::CreateTexture(HgiTextureDesc const &desc)
{
  return HgiTextureHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroyTexture(HgiTextureHandle *texHandle) {}

/* Multi threaded */
HgiTextureViewHandle HgiDX3D::CreateTextureView(HgiTextureViewDesc const &desc)
{
  if (!desc.sourceTexture) {
    TF_CODING_ERROR("Source texture is null");
  }

  HgiTextureHandle src = HgiTextureHandle(nullptr, GetUniqueId());
  HgiTextureView *view = new HgiTextureView(desc);
  view->SetViewTexture(src);
  return HgiTextureViewHandle(view, GetUniqueId());
}

void HgiDX3D::DestroyTextureView(HgiTextureViewHandle *viewHandle)
{
  /**
   * Trash the texture inside the view
   * and invalidate the view handle. */
  HgiTextureHandle texHandle = (*viewHandle)->GetViewTexture();
  (*viewHandle)->SetViewTexture(HgiTextureHandle());
  delete viewHandle->Get();
  *viewHandle = HgiTextureViewHandle();
}

/* Multi threaded */
HgiSamplerHandle HgiDX3D::CreateSampler(HgiSamplerDesc const &desc)
{
  return HgiSamplerHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroySampler(HgiSamplerHandle *smpHandle) {}

/* Multi threaded */
HgiBufferHandle HgiDX3D::CreateBuffer(HgiBufferDesc const &desc)
{
  return HgiBufferHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroyBuffer(HgiBufferHandle *bufHandle) {}

/* Multi threaded */
HgiShaderFunctionHandle HgiDX3D::CreateShaderFunction(HgiShaderFunctionDesc const &desc)
{
  return HgiShaderFunctionHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroyShaderFunction(HgiShaderFunctionHandle *shaderFnHandle) {}

/* Multi threaded */
HgiShaderProgramHandle HgiDX3D::CreateShaderProgram(HgiShaderProgramDesc const &desc)
{
  return HgiShaderProgramHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroyShaderProgram(HgiShaderProgramHandle *shaderPrgHandle) {}

/* Multi threaded */
HgiResourceBindingsHandle HgiDX3D::CreateResourceBindings(HgiResourceBindingsDesc const &desc)
{
  return HgiResourceBindingsHandle(nullptr, GetUniqueId());
}

/* Multi threaded */
void HgiDX3D::DestroyResourceBindings(HgiResourceBindingsHandle *resHandle) {}

HgiGraphicsPipelineHandle HgiDX3D::CreateGraphicsPipeline(HgiGraphicsPipelineDesc const &desc)
{
  return HgiGraphicsPipelineHandle(nullptr, GetUniqueId());
}

void HgiDX3D::DestroyGraphicsPipeline(HgiGraphicsPipelineHandle *pipeHandle) {}

HgiComputePipelineHandle HgiDX3D::CreateComputePipeline(HgiComputePipelineDesc const &desc)
{
  return HgiComputePipelineHandle(nullptr, GetUniqueId());
}

void HgiDX3D::DestroyComputePipeline(HgiComputePipelineHandle *pipeHandle) {}

/* Multi threaded */
TfToken const &HgiDX3D::GetAPIName() const
{
  return HgiTokens->DX3D;
}

/* Single threaded */
void HgiDX3D::StartFrame()
{
  /**
   * Please read important usage limitations for Hgi::StartFrame */

  if (_frameDepth++ == 0) {
  }
}

/* Single threaded */
void HgiDX3D::EndFrame()
{
  /**
   * Please read important usage limitations for Hgi::EndFrame */

  if (--_frameDepth == 0) {
    _EndFrameSync();
  }
}

/* Single threaded */
bool HgiDX3D::_SubmitCmds(HgiCmds *cmds, HgiSubmitWaitType wait)
{
  TRACE_FUNCTION();

  /**
   * XXX The device queue is externally synchronized so we would at minimum
   * need a mutex here to ensure only one thread submits cmds at a time.
   * However, since we currently call garbage collection here and because
   * we only have one resource command buffer, we cannot support submitting
   * cmds from secondary threads until those issues are resolved. */
  if (ARCH_UNLIKELY(_threadId != std::this_thread::get_id())) {
    TF_CODING_ERROR("Secondary threads should not submit cmds");
    return false;
  }

  /**
   * Submit Cmds work */
  bool result = false;
  if (cmds) {
    result = Hgi::_SubmitCmds(cmds, wait);
  }

  /**
   * XXX If client does not call StartFrame / EndFrame we perform end of frame
   * cleanup after each SubmitCmds. This is more frequent than ideal and also
   * prevents us from making SubmitCmds thread-safe. */
  if (_frameDepth == 0) {
    _EndFrameSync();
  }

  return result;
}

/* Single threaded */
void HgiDX3D::_EndFrameSync()
{
  /**
   * The garbage collector and command buffer reset must happen on the
   * main-thread when no threads are recording. */
  if (ARCH_UNLIKELY(_threadId != std::this_thread::get_id())) {
    TF_CODING_ERROR("Secondary thread violation");
    return;
  }
}

WABI_NAMESPACE_END
