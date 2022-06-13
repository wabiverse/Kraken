//
// Copyright 2020 Pixar
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

#include "wabi/imaging/hdPh/dynamicUvTextureObject.h"

#include "wabi/imaging/hdPh/dynamicUvTextureImplementation.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/textureHandleRegistry.h"

#include "wabi/imaging/hgi/hgi.h"

WABI_NAMESPACE_BEGIN

HdPhDynamicUvTextureObject::HdPhDynamicUvTextureObject(
  const HdPhTextureIdentifier &textureId,
  HdPh_TextureObjectRegistry *const textureObjectRegistry)
  : HdPhUvTextureObject(textureId, textureObjectRegistry)
{}

HdPhDynamicUvTextureObject::~HdPhDynamicUvTextureObject()
{
  _DestroyTexture();
}

HdPhDynamicUvTextureImplementation *HdPhDynamicUvTextureObject::_GetImpl() const
{
  const HdPhDynamicUvSubtextureIdentifier *const subId =
    dynamic_cast<const HdPhDynamicUvSubtextureIdentifier *>(
      GetTextureIdentifier().GetSubtextureIdentifier());
  if (!TF_VERIFY(subId)) {
    return nullptr;
  }

  return subId->GetTextureImplementation();
}

bool HdPhDynamicUvTextureObject::IsValid() const
{
  if (HdPhDynamicUvTextureImplementation *const impl = _GetImpl()) {
    return impl->IsValid(this);
  }
  return true;
}

void HdPhDynamicUvTextureObject::_Load()
{
  if (HdPhDynamicUvTextureImplementation *const impl = _GetImpl()) {
    impl->Load(this);
  }
}

void HdPhDynamicUvTextureObject::_Commit()
{
  if (HdPhDynamicUvTextureImplementation *const impl = _GetImpl()) {
    impl->Commit(this);
  }
}

WABI_NAMESPACE_END
