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
#ifndef WABI_IMAGING_HD_ST_SHADER_CODE_H
#define WABI_IMAGING_HD_ST_SHADER_CODE_H

#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/path.h"

#include "wabi/base/tf/token.h"

#include <memory>
#include <string>
#include <vector>

WABI_NAMESPACE_BEGIN

using HdBindingRequestVector = std::vector<class HdBindingRequest>;

using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;
using HdPhShaderCodeSharedPtrVector = std::vector<HdPhShaderCodeSharedPtr>;

using HdPh_MaterialParamVector = std::vector<class HdPh_MaterialParam>;
using HdBufferSourceSharedPtr = std::shared_ptr<class HdBufferSource>;
using HdBufferSourceSharedPtrVector = std::vector<HdBufferSourceSharedPtr>;
using HdBufferArrayRangeSharedPtr = std::shared_ptr<class HdBufferArrayRange>;
using HdPhTextureHandleSharedPtr = std::shared_ptr<class HdPhTextureHandle>;
using HdComputationSharedPtr = std::shared_ptr<class HdComputation>;

class HdRenderPassState;
class HdPh_ResourceBinder;
class HdPhResourceRegistry;

/// \class HdPhShaderCode
///
/// A base class representing the implementation (code) of a shader,
/// used in conjunction with HdRenderPass.
///
/// This interface provides a simple way for clients to affect the
/// composition of shading programs used for a render pass.
class HdPhShaderCode : public std::enable_shared_from_this<HdPhShaderCode>
{
 public:
  typedef size_t ID;

  HDPH_API
  HdPhShaderCode();
  HDPH_API
  virtual ~HdPhShaderCode();

  /// Returns the hash value of the shader code and configuration.
  ///
  /// It is computed from the the GLSL code as well as the resource
  /// signature of the shader (as determined from its parameters).
  /// If two shaders have the same hash, the GLSL code as expanded
  /// by codegen should also be the same.
  ///
  virtual ID ComputeHash() const = 0;

  /// Returns the combined hash values of multiple shaders.
  HDPH_API
  static ID ComputeHash(HdPhShaderCodeSharedPtrVector const &shaders);

  /// Returns the hash value of the paths of the texture prims
  /// consumed by this shader.
  ///
  /// Unless textures are bindless, shaders using different textures
  /// cannot be used in the same draw batch. Since textures can be
  /// animated, it can happen that two texture prims use the same
  /// texture at some time but different textures at other times. To
  /// avoid re-computing the draw batches over time, we use the this
  /// hash when grouping the draw batches.
  ///
  virtual ID ComputeTextureSourceHash() const;

  /// Returns the shader source provided by this shader
  /// for \a shaderStageKey
  virtual std::string GetSource(TfToken const &shaderStageKey) const = 0;

  // XXX: Should be pure-virtual
  /// Returns the shader parameters for this shader.
  HDPH_API
  virtual HdPh_MaterialParamVector const &GetParams() const;

  /// Returns whether primvar filtering is enabled for this shader.
  HDPH_API
  virtual bool IsEnabledPrimvarFiltering() const;

  /// Returns the names of primvar that are used by this shader.
  HDPH_API
  virtual TfTokenVector const &GetPrimvarNames() const;

  /// @}

  ///
  /// \name Texture system
  /// @{

  /// Information necessary to bind textures and create accessor
  /// for the texture.
  ///
  struct NamedTextureHandle
  {
    /// Name by which the texture will be accessed, i.e., the name
    /// of the accesor for thexture will be HdGet_name(...).
    ///
    TfToken name;
    /// Equal to handle->GetTextureObject()->GetTextureType().
    /// Saved here for convenience (note that name and type
    /// completely determine the creation of the texture accesor
    /// HdGet_name(...)).
    ///
    HdTextureType type;
    /// The texture.
    HdPhTextureHandleSharedPtr handle;

    /// A hash unique to the corresponding asset; used to
    /// split draw batches when not using bindless textures.
    size_t hash;
  };
  using NamedTextureHandleVector = std::vector<NamedTextureHandle>;

  /// Textures that need to be bound for this shader.
  ///
  HDPH_API
  virtual NamedTextureHandleVector const &GetNamedTextureHandles() const;

  /// @}

  // XXX: Should be pure-virtual
  /// Returns a buffer which stores parameter fallback values and texture
  /// handles.
  HDPH_API
  virtual HdBufferArrayRangeSharedPtr const &GetShaderData() const;

  /// Binds shader-specific resources to \a program
  /// XXX: this interface is meant to be used for bridging
  /// the GlfSimpleLightingContext mechanism, and not for generic use-cases.
  virtual void BindResources(int program,
                             HdPh_ResourceBinder const &binder,
                             HdRenderPassState const &state) = 0;

  /// Unbinds shader-specific resources.
  virtual void UnbindResources(int program,
                               HdPh_ResourceBinder const &binder,
                               HdRenderPassState const &state) = 0;

  /// Add custom bindings (used by codegen)
  virtual void AddBindings(HdBindingRequestVector *customBindings) = 0;

  /// Material tags can be set in the meta-data of a glslfx file to control
  /// what rprim collection that prims using this shader should go into.
  /// E.g. We can use it to split opaque and translucent prims into different
  /// collections. When no material tags are specified in the shader, a empty
  /// token is returned.
  HDPH_API
  virtual TfToken GetMaterialTag() const;

  /// \class ResourceContext
  ///
  /// The context available in implementations of
  /// AddResourcesFromTextures.
  class ResourceContext
  {
   public:
    HDPH_API
    void AddSource(HdBufferArrayRangeSharedPtr const &range, HdBufferSourceSharedPtr const &source);

    HDPH_API
    void AddSources(HdBufferArrayRangeSharedPtr const &range, HdBufferSourceSharedPtrVector &&sources);

    HDPH_API
    void AddComputation(HdBufferArrayRangeSharedPtr const &range,
                        HdComputationSharedPtr const &computation,
                        HdPhComputeQueue const queue);

   private:
    friend class HdPhResourceRegistry;
    ResourceContext(HdPhResourceRegistry *);
    HdPhResourceRegistry *_registry;
  };

  /// This function is called after textures have been allocated and
  /// loaded to add buffer sources and computations to the resource
  /// registry that require texture meta data not available until
  /// the texture is allocated or loaded. For example, the OpenGl
  /// texture sampler handle (in the bindless case) is not available
  /// until after the texture commit phase.
  ///
  HDPH_API
  virtual void AddResourcesFromTextures(ResourceContext &ctx) const;

 private:
  // No copying
  HdPhShaderCode(const HdPhShaderCode &) = delete;
  HdPhShaderCode &operator=(const HdPhShaderCode &) = delete;
};

WABI_NAMESPACE_END

#endif  // HDPH_SHADER_H
