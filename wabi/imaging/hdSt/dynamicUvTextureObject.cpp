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

#include "wabi/imaging/hdSt/dynamicUvTextureObject.h"

#include "wabi/imaging/hdSt/dynamicUvTextureImplementation.h"
#include "wabi/imaging/hdSt/resourceRegistry.h"
#include "wabi/imaging/hdSt/textureHandleRegistry.h"
#include "wabi/imaging/hdSt/subtextureIdentifier.h"

#include "wabi/imaging/hgi/hgi.h"

WABI_NAMESPACE_BEGIN

HdStDynamicUvTextureObject::HdStDynamicUvTextureObject(
    const HdStTextureIdentifier &textureId,
    HdSt_TextureObjectRegistry * const textureObjectRegistry)
  : HdStUvTextureObject(textureId, textureObjectRegistry)
{
}

HdStDynamicUvTextureObject::~HdStDynamicUvTextureObject()
{
    _DestroyTexture();
}

HdStDynamicUvTextureImplementation *
HdStDynamicUvTextureObject::_GetImpl() const
{
    const HdStDynamicUvSubtextureIdentifier * const subId =
        dynamic_cast<const HdStDynamicUvSubtextureIdentifier *>(
            GetTextureIdentifier().GetSubtextureIdentifier());
    if (!TF_VERIFY(subId)) {
        return nullptr;
    }

    return subId->GetTextureImplementation();
}

bool
HdStDynamicUvTextureObject::IsValid() const
{
    if (HdStDynamicUvTextureImplementation * const impl = _GetImpl()) {
        return impl->IsValid(this);
    }
    return true;
}

void
HdStDynamicUvTextureObject::_Load()
{
    if (HdStDynamicUvTextureImplementation * const impl = _GetImpl()) {
        impl->Load(this);
    }
}

void
HdStDynamicUvTextureObject::_Commit()
{
    if (HdStDynamicUvTextureImplementation * const impl = _GetImpl()) {
        impl->Commit(this);
    }
}

WABI_NAMESPACE_END
