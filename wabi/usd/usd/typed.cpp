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
#include "wabi/wabi.h"
#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usd/prim.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN


// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdTyped, TfType::Bases<UsdSchemaBase>>();
}

UsdTyped::~UsdTyped() {}

/* static */
UsdTyped UsdTyped::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdTyped();
  }
  return UsdTyped(stage->GetPrimAtPath(path));
}


bool UsdTyped::_IsCompatible() const
{
  if (!UsdSchemaBase::_IsCompatible())
    return false;

  return GetPrim().IsA(_GetType());
}

TF_MAKE_STATIC_DATA(TfType, _tfType)
{
  *_tfType = TfType::Find<UsdTyped>();
}
const TfType &UsdTyped::_GetTfType() const
{
  return *_tfType;
}

WABI_NAMESPACE_END
