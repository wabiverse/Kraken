//
// Copyright 2018 Pixar
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
#ifndef WABI_IMAGING_HD_FIELD_H
#define WABI_IMAGING_HD_FIELD_H

#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/bprim.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

#define HD_FIELD_TOKENS (filePath)(fieldName)

TF_DECLARE_PUBLIC_TOKENS(HdFieldTokens, HD_API, HD_FIELD_TOKENS);

class HdSceneDelegate;
typedef std::vector<class HdField const *> HdFieldPtrConstVector;

/// \class HdField
///
/// Hydra schema for a USD field primitive. Acts like a texture, combined
/// with other fields to make up a renderable volume.
///
class HdField : public HdBprim
{
 public:
  HD_API
  HdField(SdfPath const &id);
  HD_API
  virtual ~HdField();

  // Change tracking for HdField
  enum DirtyBits : HdDirtyBits
  {
    Clean = 0,
    DirtyTransform = 1 << 0,
    DirtyParams = 1 << 1,
    AllDirty = (DirtyTransform | DirtyParams)
  };
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_FIELD_H
