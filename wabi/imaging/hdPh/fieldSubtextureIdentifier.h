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
#ifndef WABI_IMAGING_HD_ST_FIELD_SUBTEXTURE_IDENTIFIER_H
#define WABI_IMAGING_HD_ST_FIELD_SUBTEXTURE_IDENTIFIER_H

#include "wabi/imaging/hdPh/subtextureIdentifier.h"

WABI_NAMESPACE_BEGIN

///
/// \class HdPhOpenVDBAssetSubtextureIdentifier
///
/// Identifies a grid in an OpenVDB file. Parallels OpenVDBAsset in usdVol.
///
class HdPhOpenVDBAssetSubtextureIdentifier final : public HdPhFieldBaseSubtextureIdentifier
{
 public:
  /// C'tor
  ///
  /// fieldName corresponds to the gridName in the OpenVDB file.
  ///
  HDPH_API
  explicit HdPhOpenVDBAssetSubtextureIdentifier(TfToken const &fieldName, int fieldIndex);

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  HDPH_API
  ~HdPhOpenVDBAssetSubtextureIdentifier() override;

 protected:
  HDPH_API
  ID _Hash() const override;
};

///
/// \class HdPhField3DAssetSubtextureIdentifier
///
/// Identifies the grid in a Field3DAsset file.
/// Parallels Field3DAsset in usdVol.
///
class HdPhField3DAssetSubtextureIdentifier final : public HdPhFieldBaseSubtextureIdentifier
{
 public:
  /// C'tor
  ///
  /// fieldName corresponds (e.g., density) to the
  ///             layer/attribute name in the Field3D file
  /// fieldIndex corresponds to the
  ///             partition index
  /// fieldPurpose (e.g., BigCloud) corresponds to the
  ///             partition name/grouping
  ///
  HDPH_API
  explicit HdPhField3DAssetSubtextureIdentifier(TfToken const &fieldName,
                                                int fieldIndex,
                                                TfToken const &fieldPurpose);

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  HDPH_API
  TfToken const &GetFieldPurpose() const
  {
    return _fieldPurpose;
  }

  HDPH_API
  ~HdPhField3DAssetSubtextureIdentifier() override;

 protected:
  HDPH_API
  ID _Hash() const override;

 private:
  TfToken _fieldPurpose;
};

WABI_NAMESPACE_END

#endif
