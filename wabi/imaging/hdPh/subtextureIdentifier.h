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
#ifndef WABI_IMAGING_HD_ST_SUBTEXTURE_IDENTIFIER_H
#define WABI_IMAGING_HD_ST_SUBTEXTURE_IDENTIFIER_H

#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class HdPhDynamicUvTextureImplementation;

///
/// \class HdPhSubtextureIdentifier
///
/// Base class for additional information to identify a texture in a
/// file that can contain several textures (e.g., frames in a movie or
/// grids in an OpenVDB file).
///
class HdPhSubtextureIdentifier {
 public:
  using ID = size_t;

  HDPH_API
  virtual std::unique_ptr<HdPhSubtextureIdentifier> Clone() const = 0;

  HDPH_API
  virtual ~HdPhSubtextureIdentifier();

 protected:
  HDPH_API
  friend size_t hash_value(const HdPhSubtextureIdentifier &subId);

  virtual ID _Hash() const = 0;
};

HDPH_API
size_t hash_value(const HdPhSubtextureIdentifier &subId);

///
/// \class HdPhFieldBaseSubtextureIdentifier
///
/// Base class for information identifying a grid in a volume field
/// file. Parallels FieldBase in usdVol.
///
class HdPhFieldBaseSubtextureIdentifier : public HdPhSubtextureIdentifier {
 public:
  /// Get field name.
  ///
  HDPH_API
  TfToken const &GetFieldName() const
  {
    return _fieldName;
  }

  /// Get field index.
  ///
  HDPH_API
  int GetFieldIndex() const
  {
    return _fieldIndex;
  }

  HDPH_API
  ~HdPhFieldBaseSubtextureIdentifier() override = 0;

 protected:
  HDPH_API
  HdPhFieldBaseSubtextureIdentifier(TfToken const &fieldName, int fieldIndex);

  HDPH_API
  ID _Hash() const override;

 private:
  TfToken _fieldName;
  int _fieldIndex;
};

///
/// \class HdPhAssetUvSubtextureIdentifier
///
/// Specifies whether a UV texture should be loaded flipped vertically, whether
/// it should be loaded with pre-multiplied alpha values, and the color space
/// in which the texture is encoded.
///
/// The former functionality allows the texture system to support both the
/// legacy HwUvTexture_1 (flipVertically = true) and UsdUvTexture
/// (flipVertically = false) which have opposite conventions for the
/// vertical orientation.
///
class HdPhAssetUvSubtextureIdentifier final : public HdPhSubtextureIdentifier {
 public:
  /// C'tor takes bool whether flipping vertically, whether to pre-multiply
  /// by alpha, and the texture's source color space
  HDPH_API
  explicit HdPhAssetUvSubtextureIdentifier(bool flipVertically,
                                           bool premultiplyAlpha,
                                           const TfToken &sourceColorSpace);

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  HDPH_API
  bool GetFlipVertically() const
  {
    return _flipVertically;
  }

  HDPH_API
  bool GetPremultiplyAlpha() const
  {
    return _premultiplyAlpha;
  }

  HDPH_API
  TfToken GetSourceColorSpace() const
  {
    return _sourceColorSpace;
  }

  HDPH_API
  ~HdPhAssetUvSubtextureIdentifier() override;

 protected:
  HDPH_API
  ID _Hash() const override;

 private:
  bool _flipVertically;
  bool _premultiplyAlpha;
  TfToken _sourceColorSpace;
};

///
/// \class HdPhDynamicUvSubtextureIdentifier
///
/// Used as a tag that the Phoenix texture system returns a
/// HdPhDynamicUvTextureObject that is populated by a client rather
/// than by the Phoenix texture system.
///
/// Clients can subclass this class and provide their own
/// HdPhDynamicUvTextureImplementation to create UV texture with custom
/// load and commit behavior.
///
/// testHdPhDynamicUvTexture.cpp is an example of how custom load and
/// commit behavior can be implemented.
///
/// AOV's are another example. In presto, these are baked by
/// HdPhDynamicUvTextureObject's. In this case, the
/// HdPhDynamicUvTextureObject's do not provide custom load or commit
/// behavior (null-ptr returned by GetTextureImplementation). Instead,
/// GPU memory is allocated by explicitly calling
/// HdPhDynamicUvTextureObject::CreateTexture in
/// HdPhRenderBuffer::Sync/Allocate and the texture is filled by using
/// it as render target in various render passes.
///
class HdPhDynamicUvSubtextureIdentifier : public HdPhSubtextureIdentifier {
 public:
  HDPH_API
  HdPhDynamicUvSubtextureIdentifier();

  HDPH_API
  ~HdPhDynamicUvSubtextureIdentifier() override;

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  /// Textures can return their own HdPhDynamicUvTextureImplementation
  /// to customize the load and commit behavior.
  HDPH_API
  virtual HdPhDynamicUvTextureImplementation *GetTextureImplementation() const;

 protected:
  HDPH_API
  ID _Hash() const override;
};

///
/// \class HdPhPtexSubtextureIdentifier
///
/// Specifies whether a Ptex texture should be loaded with pre-multiplied alpha
/// values.
///
class HdPhPtexSubtextureIdentifier final : public HdPhSubtextureIdentifier {
 public:
  /// C'tor takes bool whether to pre-multiply by alpha
  HDPH_API
  explicit HdPhPtexSubtextureIdentifier(bool premultiplyAlpha);

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  HDPH_API
  bool GetPremultiplyAlpha() const
  {
    return _premultiplyAlpha;
  }

  HDPH_API
  ~HdPhPtexSubtextureIdentifier() override;

 protected:
  HDPH_API
  ID _Hash() const override;

 private:
  bool _premultiplyAlpha;
};

///
/// \class HdPhUdimSubtextureIdentifier
///
/// Specifies whether a Udim texture should be loaded with pre-multiplied alpha
/// values and the color space in which the texture is encoded.
///
class HdPhUdimSubtextureIdentifier final : public HdPhSubtextureIdentifier {
 public:
  /// C'tor takes bool whether to pre-multiply by alpha and the texture's
  /// source color space
  HDPH_API
  explicit HdPhUdimSubtextureIdentifier(bool premultiplyAlpha, const TfToken &sourceColorSpace);

  HDPH_API
  std::unique_ptr<HdPhSubtextureIdentifier> Clone() const override;

  HDPH_API
  bool GetPremultiplyAlpha() const
  {
    return _premultiplyAlpha;
  }

  HDPH_API
  TfToken GetSourceColorSpace() const
  {
    return _sourceColorSpace;
  }

  HDPH_API
  ~HdPhUdimSubtextureIdentifier() override;

 protected:
  HDPH_API
  ID _Hash() const override;

 private:
  bool _premultiplyAlpha;
  TfToken _sourceColorSpace;
};

WABI_NAMESPACE_END

#endif
