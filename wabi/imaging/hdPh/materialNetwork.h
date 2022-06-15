//
// Copyright 2019 Pixar
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
#ifndef WABI_IMAGING_HD_ST_MATERIAL_NETWORK_H
#define WABI_IMAGING_HD_ST_MATERIAL_NETWORK_H

#include "wabi/imaging/hd/material.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/textureIdentifier.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
using HioGlslfxSharedPtr = std::shared_ptr<class HioGlslfx>;
using HdPh_MaterialParamVector = std::vector<class HdPh_MaterialParam>;

/// \class HdPhMaterialNetwork
///
/// Helps HdPhMaterial process a Hydra material network into shader source code
/// and parameters values.
class HdPhMaterialNetwork final
{
 public:

  HDPH_API
  HdPhMaterialNetwork();

  HDPH_API
  ~HdPhMaterialNetwork();

  /// Process a material network topology and extract all the information we
  /// need from it.
  HDPH_API
  void ProcessMaterialNetwork(SdfPath const &materialId,
                              HdMaterialNetworkMap const &hdNetworkMap,
                              HdPhResourceRegistry *resourceRegistry);

  HDPH_API
  TfToken const &GetMaterialTag() const;

  HDPH_API
  std::string const &GetFragmentCode() const;

  HDPH_API
  std::string const &GetGeometryCode() const;

  HDPH_API
  VtDictionary const &GetMetadata() const;

  HDPH_API
  HdPh_MaterialParamVector const &GetMaterialParams() const;

  // Information necessary to allocate a texture.
  struct TextureDescriptor
  {
    // Name by which the texture will be accessed, i.e., the name
    // of the accesor for thexture will be HdGet_name(...).
    // It is generated from the input name the corresponding texture
    // node is connected to.
    TfToken name;
    HdPhTextureIdentifier textureId;
    HdTextureType type;
    HdSamplerParameters samplerParameters;
    // Memory request in bytes.
    size_t memoryRequest;

    // The texture is not just identified by a file path attribute
    // on the texture prim but there is special API to texture prim
    // to obtain the texture.
    //
    // This is used for draw targets.
    bool useTexturePrimToFindTexture;
    // This is used for draw targets and hashing.
    SdfPath texturePrim;
  };

  using TextureDescriptorVector = std::vector<TextureDescriptor>;

  HDPH_API
  TextureDescriptorVector const &GetTextureDescriptors() const;

 private:

  TfToken _materialTag;
  std::string _fragmentSource;
  std::string _geometrySource;
  VtDictionary _materialMetadata;
  HdPh_MaterialParamVector _materialParams;
  TextureDescriptorVector _textureDescriptors;
  HioGlslfxSharedPtr _surfaceGfx;
  size_t _surfaceGfxHash;
};

WABI_NAMESPACE_END

#endif
