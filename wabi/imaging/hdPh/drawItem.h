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
#ifndef WABI_IMAGING_HD_ST_DRAW_ITEM_H
#define WABI_IMAGING_HD_ST_DRAW_ITEM_H

#include "wabi/imaging/hd/drawItem.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

using HdPh_GeometricShaderSharedPtr = std::shared_ptr<class HdPh_GeometricShader>;
using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;

class HdPhDrawItem : public HdDrawItem
{
 public:
  HF_MALLOC_TAG_NEW("new HdPhDrawItem");

  HDPH_API
  HdPhDrawItem(HdRprimSharedData const *sharedData);

  HDPH_API
  ~HdPhDrawItem() override;

  void SetGeometricShader(HdPh_GeometricShaderSharedPtr const &shader)
  {
    _geometricShader = shader;
  }

  HdPh_GeometricShaderSharedPtr const &GetGeometricShader() const
  {
    return _geometricShader;
  }

  HdPhShaderCodeSharedPtr const &GetMaterialShader() const
  {
    return _materialShader;
  }

  void SetMaterialShader(HdPhShaderCodeSharedPtr const &shader)
  {
    _materialShader = shader;
  }

 protected:
  size_t _GetBufferArraysHash() const override;
  size_t _GetElementOffsetsHash() const override;

 private:
  HdPh_GeometricShaderSharedPtr _geometricShader;
  HdPhShaderCodeSharedPtr _materialShader;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_DRAW_ITEM_H
