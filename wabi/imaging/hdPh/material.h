//
// Copyright 2016 Pixar
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
#ifndef WABI_IMAGING_HD_ST_MATERIAL_H
#define WABI_IMAGING_HD_ST_MATERIAL_H

#include "wabi/imaging/hd/material.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/materialNetwork.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/imaging/hf/perfLog.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdPhSurfaceShaderSharedPtr = std::shared_ptr<class HdPhSurfaceShader>;

class HioGlslfx;

class HdPhMaterial final : public HdMaterial
{
 public:

  HF_MALLOC_TAG_NEW("new HdPhMaterial");

  HDPH_API
  HdPhMaterial(SdfPath const &id);
  HDPH_API
  ~HdPhMaterial() override;

  /// Synchronizes state from the delegate to this object.
  HDPH_API
  void Sync(HdSceneDelegate *sceneDelegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits) override;

  HDPH_API
  void Finalize(HdRenderParam *renderParam) override;

  /// Returns the minimal set of dirty bits to place in the
  /// change tracker for use in the first sync of this prim.
  /// Typically this would be all dirty bits.
  HDPH_API
  HdDirtyBits GetInitialDirtyBitsMask() const override;

  /// Obtains the render delegate specific representation of the shader.
  HDPH_API
  HdPhShaderCodeSharedPtr GetShaderCode() const;

  /// Summary flag. Returns true if the material is bound to one or more
  /// textures and any of those textures is a ptex texture.
  /// If no textures are bound or all textures are uv textures, then
  /// the method returns false.
  inline bool HasPtex() const;

  /// Returns true if the material specifies limit surface evaluation.
  inline bool HasLimitSurfaceEvaluation() const;

  // Returns true if the material has a displacement terminal.
  inline bool HasDisplacement() const;

  // Returns the material's render pass tag.
  inline const TfToken &GetMaterialTag() const;

  /// Replaces the shader code object with an externally created one
  /// Used to set the fallback shader for prim.
  /// This class takes ownership of the passed in object.
  HDPH_API
  void SetSurfaceShader(HdPhSurfaceShaderSharedPtr &shaderCode);

 private:

  // Processes the texture descriptors from a material network to
  // create textures using the Phoenix texture system.
  //
  // Adds buffer specs/sources necessary for textures, e.g., bindless
  // handles or sampling transform for field textures.
  void _ProcessTextureDescriptors(HdSceneDelegate *sceneDelegate,
                                  HdPhResourceRegistrySharedPtr const &resourceRegistry,
                                  std::weak_ptr<HdPhShaderCode> const &shaderCode,
                                  HdPhMaterialNetwork::TextureDescriptorVector const &descs,
                                  HdPhShaderCode::NamedTextureHandleVector *texturesFromPhoenix,
                                  HdBufferSpecVector *specs,
                                  HdBufferSourceSharedPtrVector *sources);

  bool _GetHasLimitSurfaceEvaluation(VtDictionary const &metadata) const;

  void _InitFallbackShader();

  static HioGlslfx *_fallbackGlslfx;

  HdPhSurfaceShaderSharedPtr _surfaceShader;

  bool _isInitialized : 1;
  bool _hasPtex : 1;
  bool _hasLimitSurfaceEvaluation : 1;
  bool _hasDisplacement : 1;

  TfToken _materialTag;
  size_t _textureHash;

  HdPhMaterialNetwork _networkProcessor;
};

inline bool HdPhMaterial::HasPtex() const
{
  return _hasPtex;
}

inline bool HdPhMaterial::HasLimitSurfaceEvaluation() const
{
  return _hasLimitSurfaceEvaluation;
}

inline bool HdPhMaterial::HasDisplacement() const
{
  return _hasDisplacement;
}

inline const TfToken &HdPhMaterial::GetMaterialTag() const
{
  return _materialTag;
}

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_MATERIAL_H
