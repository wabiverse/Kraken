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
#include "wabi/imaging/hdPh/textureIdentifier.h"

#include "wabi/imaging/hdPh/subtextureIdentifier.h"

WABI_NAMESPACE_BEGIN

static std::unique_ptr<const HdPhSubtextureIdentifier> _CloneSubtextureId(
  std::unique_ptr<const HdPhSubtextureIdentifier> const &subtextureId)
{
  if (subtextureId)
  {
    return subtextureId->Clone();
  }
  return nullptr;
}

HdPhTextureIdentifier::HdPhTextureIdentifier() = default;

HdPhTextureIdentifier::HdPhTextureIdentifier(const TfToken &filePath)
  : _filePath(filePath)
{}

HdPhTextureIdentifier::HdPhTextureIdentifier(const TfToken &filePath,
                                             std::unique_ptr<const HdPhSubtextureIdentifier> &&subtextureId)
  : _filePath(filePath),
    _subtextureId(std::move(subtextureId))
{}

HdPhTextureIdentifier::HdPhTextureIdentifier(const HdPhTextureIdentifier &textureId)
  : _filePath(textureId._filePath),
    _subtextureId(_CloneSubtextureId(textureId._subtextureId))
{}

HdPhTextureIdentifier &HdPhTextureIdentifier::operator=(HdPhTextureIdentifier &&textureId) = default;

HdPhTextureIdentifier &HdPhTextureIdentifier::operator=(const HdPhTextureIdentifier &textureId)
{
  _filePath = textureId._filePath;
  _subtextureId = _CloneSubtextureId(textureId._subtextureId);

  return *this;
}

HdPhTextureIdentifier::~HdPhTextureIdentifier() = default;

static std::pair<bool, HdPhTextureIdentifier::ID> _OptionalSubidentifierHash(const HdPhTextureIdentifier &id)
{
  if (const HdPhSubtextureIdentifier *subId = id.GetSubtextureIdentifier())
  {
    return {true, TfHash()(*subId)};
  }
  return {false, 0};
}

bool HdPhTextureIdentifier::operator==(const HdPhTextureIdentifier &other) const
{
  return _filePath == other._filePath &&
         _OptionalSubidentifierHash(*this) == _OptionalSubidentifierHash(other);
}

bool HdPhTextureIdentifier::operator!=(const HdPhTextureIdentifier &other) const
{
  return !(*this == other);
}

size_t hash_value(const HdPhTextureIdentifier &id)
{
  if (const HdPhSubtextureIdentifier *const subId = id.GetSubtextureIdentifier())
  {
    return TfHash::Combine(id.GetFilePath(), *subId);
  } else
  {
    return TfHash()(id.GetFilePath());
  }
}

WABI_NAMESPACE_END
