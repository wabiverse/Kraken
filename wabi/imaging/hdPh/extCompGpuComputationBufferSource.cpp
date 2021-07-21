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
#include "wabi/imaging/hdPh/extCompGpuComputationBufferSource.h"
#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/resourceRegistry.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/vtBufferSource.h"
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/bufferResource.h"
#include "wabi/imaging/hdPh/codeGen.h"
#include "wabi/imaging/hdPh/extCompComputeShader.h"
#include "wabi/imaging/hdPh/extCompGpuComputation.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/shaderCode.h"

#include <limits>

WABI_NAMESPACE_BEGIN

HdPhExtCompGpuComputationBufferSource::HdPhExtCompGpuComputationBufferSource(
  HdBufferSourceSharedPtrVector const &inputs,
  HdPhExtCompGpuComputationResourceSharedPtr const &resource)
  : HdNullBufferSource(),
    _inputs(inputs),
    _resource(resource)
{}

bool HdPhExtCompGpuComputationBufferSource::Resolve()
{
  bool allResolved = true;
  for (size_t i = 0; i < _inputs.size(); ++i)
  {
    HdBufferSourceSharedPtr const &source = _inputs[i];
    if (!source->IsResolved())
    {
      allResolved &= source->Resolve();
    }
  }

  if (!allResolved)
  {
    return false;
  }

  if (!_TryLock())
  {
    return false;
  }

  // Resolve the code gen source code
  _resource->Resolve();

  _SetResolved();

  return true;
}

bool HdPhExtCompGpuComputationBufferSource::_CheckValid() const
{
  return true;
}

WABI_NAMESPACE_END
