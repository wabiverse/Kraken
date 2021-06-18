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
#include "wabi/imaging/hgiMetal/computeCmds.h"
#include "wabi/imaging/hgiMetal/buffer.h"
#include "wabi/imaging/hgiMetal/computePipeline.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/resourceBindings.h"
#include "wabi/imaging/hgiMetal/texture.h"

#include "wabi/base/arch/defines.h"

WABI_NAMESPACE_BEGIN

HgiMetalComputeCmds::HgiMetalComputeCmds(HgiMetal *hgi)
  : HgiComputeCmds(),
    _hgi(hgi),
    _pipelineState(nullptr),
    _commandBuffer(nil),
    _encoder(nil),
    _secondaryCommandBuffer(false)
{
  _CreateEncoder();
}

HgiMetalComputeCmds::~HgiMetalComputeCmds()
{
  TF_VERIFY(_encoder == nil, "Encoder created, but never commited.");
}

void HgiMetalComputeCmds::_CreateEncoder()
{
  if (!_encoder)
  {
    _commandBuffer = _hgi->GetPrimaryCommandBuffer();
    if (_commandBuffer == nil)
    {
      _commandBuffer = _hgi->GetSecondaryCommandBuffer();
      _secondaryCommandBuffer = true;
    }
    _encoder = [_commandBuffer computeCommandEncoder];
  }
}

void HgiMetalComputeCmds::BindPipeline(HgiComputePipelineHandle pipeline)
{
  _CreateEncoder();
  _pipelineState = static_cast<HgiMetalComputePipeline *>(pipeline.Get());
  _pipelineState->BindPipeline(_encoder);
}

void HgiMetalComputeCmds::BindResources(HgiResourceBindingsHandle r)
{
  if (HgiMetalResourceBindings *rb = static_cast<HgiMetalResourceBindings *>(r.Get()))
  {
    _CreateEncoder();
    rb->BindResources(_encoder);
  }
}

void HgiMetalComputeCmds::SetConstantValues(HgiComputePipelineHandle pipeline,
                                            uint32_t bindIndex,
                                            uint32_t byteSize,
                                            const void *data)
{
  _CreateEncoder();
  [_encoder setBytes:data length:byteSize atIndex:bindIndex];
}

void HgiMetalComputeCmds::Dispatch(int dimX, int dimY)
{
  uint32_t maxTotalThreads = [_pipelineState->GetMetalPipelineState() maxTotalThreadsPerThreadgroup];
  uint32_t exeWidth = [_pipelineState->GetMetalPipelineState() threadExecutionWidth];

  uint32_t thread_width, thread_height;
  if (dimY == 1)
  {
    thread_width = (maxTotalThreads / exeWidth) * exeWidth;
    thread_height = 1;
  }
  else
  {
    thread_width = exeWidth;
    thread_height = maxTotalThreads / exeWidth;
  }

  [_encoder dispatchThreads:MTLSizeMake(dimX, dimY, 1)
      threadsPerThreadgroup:MTLSizeMake(MIN(thread_width, dimX), MIN(thread_height, dimY), 1)];

  _hasWork = true;
}

void HgiMetalComputeCmds::PushDebugGroup(const char *label)
{
  _CreateEncoder();
  HGIMETAL_DEBUG_LABEL(_encoder, label)
}

void HgiMetalComputeCmds::PopDebugGroup()
{}

void HgiMetalComputeCmds::MemoryBarrier(HgiMemoryBarrier barrier)
{
  TF_VERIFY(barrier == HgiMemoryBarrierAll, "Unknown barrier");
  // Do nothing. All resource writes performed in a given kernel function
  // are visible in the next kernel function.
}

bool HgiMetalComputeCmds::_Submit(Hgi *hgi, HgiSubmitWaitType wait)
{
  bool submittedWork = false;
  if (_encoder)
  {
    [_encoder endEncoding];
    _encoder = nil;
    submittedWork = true;

    HgiMetal::CommitCommandBufferWaitType waitType;
    switch (wait)
    {
      case HgiSubmitWaitTypeNoWait:
        waitType = HgiMetal::CommitCommandBuffer_NoWait;
        break;
      case HgiSubmitWaitTypeWaitUntilCompleted:
        waitType = HgiMetal::CommitCommandBuffer_WaitUntilCompleted;
        break;
    }

    if (_secondaryCommandBuffer)
    {
      _hgi->CommitSecondaryCommandBuffer(_commandBuffer, waitType);
    }
    else
    {
      _hgi->CommitPrimaryCommandBuffer(waitType);
    }
  }

  if (_secondaryCommandBuffer)
  {
    _hgi->ReleaseSecondaryCommandBuffer(_commandBuffer);
  }
  _commandBuffer = nil;

  return submittedWork;
}

WABI_NAMESPACE_END
