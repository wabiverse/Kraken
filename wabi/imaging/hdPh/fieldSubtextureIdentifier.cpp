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

#include "wabi/imaging/hdPh/fieldSubtextureIdentifier.h"

WABI_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////
// HdPhOpenVDBAssetSubtextureIdentifier

HdPhOpenVDBAssetSubtextureIdentifier::HdPhOpenVDBAssetSubtextureIdentifier(TfToken const &fieldName,
                                                                           const int fieldIndex)
  : HdPhFieldBaseSubtextureIdentifier(fieldName, fieldIndex)
{}

HdPhOpenVDBAssetSubtextureIdentifier::~HdPhOpenVDBAssetSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhOpenVDBAssetSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhOpenVDBAssetSubtextureIdentifier>(GetFieldName(), GetFieldIndex());
}

HdPhSubtextureIdentifier::ID HdPhOpenVDBAssetSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("vdb"));

  return TfHash::Combine(typeHash, HdPhFieldBaseSubtextureIdentifier::_Hash());
}

////////////////////////////////////////////////////////////////////////////
// HdPhField3DAssetSubtextureIdentifier

HdPhField3DAssetSubtextureIdentifier::HdPhField3DAssetSubtextureIdentifier(TfToken const &fieldName,
                                                                           const int fieldIndex,
                                                                           TfToken const &fieldPurpose)
  : HdPhFieldBaseSubtextureIdentifier(fieldName, fieldIndex),
    _fieldPurpose(fieldPurpose)
{}

HdPhField3DAssetSubtextureIdentifier::~HdPhField3DAssetSubtextureIdentifier() = default;

std::unique_ptr<HdPhSubtextureIdentifier> HdPhField3DAssetSubtextureIdentifier::Clone() const
{
  return std::make_unique<HdPhField3DAssetSubtextureIdentifier>(
    GetFieldName(), GetFieldIndex(), GetFieldPurpose());
}

HdPhSubtextureIdentifier::ID HdPhField3DAssetSubtextureIdentifier::_Hash() const
{
  static ID typeHash = TfHash()(std::string("Field3D"));

  return TfHash::Combine(typeHash, HdPhFieldBaseSubtextureIdentifier::_Hash(), _fieldPurpose);
}

WABI_NAMESPACE_END
