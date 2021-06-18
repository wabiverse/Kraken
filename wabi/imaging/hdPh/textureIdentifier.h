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
#ifndef WABI_IMAGING_HD_ST_TEXTURE_IDENTIFIER_H
#define WABI_IMAGING_HD_ST_TEXTURE_IDENTIFIER_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdPhSubtextureIdentifier;

///
/// \class HdPhTextureIdentifier
///
/// Class to identify a texture file or a texture within the texture file
/// (e.g., a frame in a movie).
///
/// The class has value semantics and uses HdPhSubtextureIdentifier in a
/// polymorphic way.
///
class HdPhTextureIdentifier final
{
 public:
  using ID = size_t;

  HdPhTextureIdentifier();

  /// C'tor for files that can contain only one texture.
  ///
  explicit HdPhTextureIdentifier(const TfToken &filePath);

  /// C'tor for files that can contain more than one texture (e.g.,
  /// frames in a movie, grids in a VDB file).
  ///
  HdPhTextureIdentifier(const TfToken &filePath,
                        std::unique_ptr<const HdPhSubtextureIdentifier> &&subtextureId);

  HdPhTextureIdentifier(const HdPhTextureIdentifier &textureId);

  HdPhTextureIdentifier &operator=(HdPhTextureIdentifier &&textureId);

  HdPhTextureIdentifier &operator=(const HdPhTextureIdentifier &textureId);

  ~HdPhTextureIdentifier();

  /// Get file path of texture file.
  ///
  const TfToken &GetFilePath() const
  {
    return _filePath;
  }

  /// Get additional information identifying a texture in a file that
  /// can contain more than one texture (e.g., a frame in a movie or
  /// a grid in a VDB file).
  ///
  /// nullptr for files (e.g., png) that can contain only one texture.
  ///
  const HdPhSubtextureIdentifier *GetSubtextureIdentifier() const
  {
    return _subtextureId.get();
  }

  bool operator==(const HdPhTextureIdentifier &other) const;
  bool operator!=(const HdPhTextureIdentifier &other) const;

 private:
  TfToken _filePath;
  std::unique_ptr<const HdPhSubtextureIdentifier> _subtextureId;
};

size_t hash_value(const HdPhTextureIdentifier &);

WABI_NAMESPACE_END

#endif
