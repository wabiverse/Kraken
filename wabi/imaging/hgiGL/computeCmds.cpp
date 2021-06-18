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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/imaging/hgiGL/buffer.h"
#include "wabi/imaging/hgiGL/computeCmds.h"
#include "wabi/imaging/hgiGL/conversions.h"
#include "wabi/imaging/hgiGL/device.h"
#include "wabi/imaging/hgiGL/diagnostic.h"
#include "wabi/imaging/hgiGL/graphicsPipeline.h"
#include "wabi/imaging/hgiGL/ops.h"
#include "wabi/imaging/hgiGL/resourceBindings.h"

WABI_NAMESPACE_BEGIN

HgiGLComputeCmds::HgiGLComputeCmds(HgiGLDevice *device)
  : HgiComputeCmds(),
    _pushStack(0)
{}

HgiGLComputeCmds::~HgiGLComputeCmds() = default;

void HgiGLComputeCmds::BindPipeline(HgiComputePipelineHandle pipeline)
{
  _ops.push_back(HgiGLOps::BindPipeline(pipeline));
}

void HgiGLComputeCmds::BindResources(HgiResourceBindingsHandle res)
{
  _ops.push_back(HgiGLOps::BindResources(res));
}

void HgiGLComputeCmds::SetConstantValues(HgiComputePipelineHandle pipeline,
                                         uint32_t bindIndex,
                                         uint32_t byteSize,
                                         const void *data)
{
  _ops.push_back(HgiGLOps::SetConstantValues(pipeline, bindIndex, byteSize, data));
}

void HgiGLComputeCmds::Dispatch(int dimX, int dimY)
{
  _ops.push_back(HgiGLOps::Dispatch(dimX, dimY));
}

void HgiGLComputeCmds::PushDebugGroup(const char *label)
{
  if (HgiGLDebugEnabled())
  {
    _pushStack++;
    _ops.push_back(HgiGLOps::PushDebugGroup(label));
  }
}

void HgiGLComputeCmds::PopDebugGroup()
{
  if (HgiGLDebugEnabled())
  {
    _pushStack--;
    _ops.push_back(HgiGLOps::PopDebugGroup());
  }
}

void HgiGLComputeCmds::MemoryBarrier(HgiMemoryBarrier barrier)
{
  _ops.push_back(HgiGLOps::MemoryBarrier(barrier));
}

bool HgiGLComputeCmds::_Submit(Hgi *hgi, HgiSubmitWaitType wait)
{
  if (_ops.empty())
  {
    return false;
  }

  TF_VERIFY(_pushStack == 0, "Push and PopDebugGroup do not even out");

  HgiGL *hgiGL = static_cast<HgiGL *>(hgi);
  HgiGLDevice *device = hgiGL->GetPrimaryDevice();
  device->SubmitOps(_ops);
  return true;
}

WABI_NAMESPACE_END
