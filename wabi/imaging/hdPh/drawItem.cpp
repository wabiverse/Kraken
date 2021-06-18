//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "wabi/imaging/hdPh/drawItem.h"
#include "wabi/imaging/hdPh/shaderCode.h"

WABI_NAMESPACE_BEGIN

HdPhDrawItem::HdPhDrawItem(HdRprimSharedData const *sharedData)
  : HdDrawItem(sharedData)
{
  HF_MALLOC_TAG_FUNCTION();
}

HdPhDrawItem::~HdPhDrawItem()
{
  /*NOTHING*/
}

/*virtual*/
size_t HdPhDrawItem::_GetBufferArraysHash() const
{
  if (const HdPhShaderCodeSharedPtr &shader = GetMaterialShader())
  {
    if (const HdBufferArrayRangeSharedPtr &shaderBAR = shader->GetShaderData())
    {
      return shaderBAR->GetVersion();
    }
  }

  return 0;
}

/*virtual*/
size_t HdPhDrawItem::_GetElementOffsetsHash() const
{
  if (const HdPhShaderCodeSharedPtr &shader = GetMaterialShader())
  {
    if (const HdBufferArrayRangeSharedPtr &shaderBAR = shader->GetShaderData())
    {
      return shaderBAR->GetElementOffset();
    }
  }

  return 0;
}

WABI_NAMESPACE_END
