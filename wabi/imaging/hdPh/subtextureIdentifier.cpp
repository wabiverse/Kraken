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
#include "wabi/imaging/hdPh/subtextureIdentifier.h"

#include "wabi/base/tf/hash.h"

WABI_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////
// HdPhSubtextureIdentifier

HdPhSubtextureIdentifier::~HdPhSubtextureIdentifier() = default;

size_t hash_value(const HdPhSubtextureIdentifier &subId)
{
  return subId._Hash();
}

////////////////////////////////////////////////////////////////////////////
// HdPhFieldBaseSubtextureIdentifier
HdPhFieldBaseSubtextureIdentifier::HdPhFieldBaseSubtextureIdentifier(TfToken const &fieldName,
                                                                     const int fieldIndex)
  : _fieldName(fieldName),
    _fieldIndex(fieldIndex)
{}

HdPhFieldBaseSubtextureIdentifier::~HdPhFieldBaseSubtextureIdentifier() = default;

HdPhSubtextureIdentifier::ID HdPhFieldBaseSubtextureIdentifier::_Hash() const
{
  return TfHash::Combine(_fieldName, _fieldIndex);
}

////////////////////////////////////////////////////////////////////////////
// HdPhAssetUvSubtextureIdentifier

HdPhAssetUvSubtextureIdentifier::HdPhAssetUvSubtextureIdentifier(const bool flipVertically,
                                                                 const bool premultiplyAlpha,
                                                                 const TfToken &sourceColorSpace)
  : _flipVertically(flipVertically),
    _premultiplyAlpha(premultiplyAlpha),
    _sourceColorSpace(sourceColorSpace)
{}

HdPhAssetUvSubtextureIdentifier::~HdPhAssetUvSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhAssetUvSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhAssetUvSubtextureIdentifier>(GetFlipVertically(),
                                                           GetPremultiplyAlpha(),
                                                           GetSourceColorSpace());
}

HdPhSubtextureIdentifier::ID HdPhAssetUvSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("HdPhAssetUvSubtextureIdentifier"));

  return TfHash::Combine(typeHash, GetFlipVertically(), GetPremultiplyAlpha(), GetSourceColorSpace());
}

////////////////////////////////////////////////////////////////////////////
// HdPhDynamicUvSubtextureIdentifier

HdPhDynamicUvSubtextureIdentifier::HdPhDynamicUvSubtextureIdentifier() = default;

HdPhDynamicUvSubtextureIdentifier::~HdPhDynamicUvSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhDynamicUvSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhDynamicUvSubtextureIdentifier>();
}

HdPhSubtextureIdentifier::ID HdPhDynamicUvSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("HdPhDynamicUvSubtextureIdentifier"));
  return typeHash;
}

HdPhDynamicUvTextureImplementation *HdPhDynamicUvSubtextureIdentifier::GetTextureImplementation() const
{
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////
// HdPhUdimSubtextureIdentifier

HdPhUdimSubtextureIdentifier::HdPhUdimSubtextureIdentifier(const bool premultiplyAlpha,
                                                           const TfToken &sourceColorSpace)
  : _premultiplyAlpha(premultiplyAlpha),
    _sourceColorSpace(sourceColorSpace)
{}

HdPhUdimSubtextureIdentifier::~HdPhUdimSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhUdimSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhUdimSubtextureIdentifier>(GetPremultiplyAlpha(), GetSourceColorSpace());
}

HdPhSubtextureIdentifier::ID HdPhUdimSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("HdPhUdimSubtextureIdentifier"));

  return TfHash::Combine(typeHash, GetPremultiplyAlpha(), GetSourceColorSpace());
}

////////////////////////////////////////////////////////////////////////////
// HdPhPtexSubtextureIdentifier

HdPhPtexSubtextureIdentifier::HdPhPtexSubtextureIdentifier(const bool premultiplyAlpha)
  : _premultiplyAlpha(premultiplyAlpha)
{}

HdPhPtexSubtextureIdentifier::~HdPhPtexSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhPtexSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhPtexSubtextureIdentifier>(GetPremultiplyAlpha());
}

HdPhSubtextureIdentifier::ID HdPhPtexSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("HdPhPtexSubtextureIdentifier"));

  return TfHash::Combine(typeHash, GetPremultiplyAlpha());
}

WABI_NAMESPACE_END
