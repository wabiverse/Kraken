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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/imaging/hd/aov.h"
#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/renderBuffer.h"
#include "wabi/imaging/hd/renderIndex.h"
#include "wabi/imaging/hd/renderPassState.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/package.h"
#include "wabi/imaging/hdPh/renderBuffer.h"
#include "wabi/imaging/hdPh/renderPassShader.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/textureBinder.h"
#include "wabi/imaging/hdPh/textureHandle.h"
#include "wabi/imaging/hdPh/textureIdentifier.h"
#include "wabi/imaging/hdPh/textureObject.h"

#include "wabi/imaging/hf/perfLog.h"

#include "wabi/imaging/hio/glslfx.h"

#include <boost/functional/hash.hpp>

#include <string>

WABI_NAMESPACE_BEGIN

namespace {

struct _NamedTextureIdentifier {
  TfToken name;
  HdPhTextureIdentifier id;
};

using _NamedTextureIdentifiers = std::vector<_NamedTextureIdentifier>;

TfToken _GetInputName(const TfToken &aovName)
{
  return TfToken(aovName.GetString() + "Readback");
}

// An AOV is backed by a render buffer. And Phoenix backs a render buffer
// by a texture. The identifier for this texture can be obtained from
// the HdPhRenderBuffer.
_NamedTextureIdentifiers _GetNamedTextureIdentifiers(HdRenderPassAovBindingVector const &aovInputBindings,
                                                     HdRenderIndex *const renderIndex)
{
  _NamedTextureIdentifiers result;
  result.reserve(aovInputBindings.size());

  for (const HdRenderPassAovBinding &aovBinding : aovInputBindings) {
    if (HdPhRenderBuffer *const renderBuffer = dynamic_cast<HdPhRenderBuffer *>(
          renderIndex->GetBprim(HdPrimTypeTokens->renderBuffer, aovBinding.renderBufferId))) {
      result.push_back(_NamedTextureIdentifier{_GetInputName(aovBinding.aovName),
                                               renderBuffer->GetTextureIdentifier(
                                                 /* multiSampled = */ false)});
    }
  }

  return result;
}

// Check whether the given named texture handles match the given
// named texture identifiers.
bool _AreHandlesValid(const HdPhShaderCode::NamedTextureHandleVector &namedTextureHandles,
                      const _NamedTextureIdentifiers &namedTextureIdentifiers)
{
  if (namedTextureHandles.size() != namedTextureIdentifiers.size()) {
    return false;
  }

  for (size_t i = 0; i < namedTextureHandles.size(); i++) {
    const HdPhShaderCode::NamedTextureHandle namedTextureHandle = namedTextureHandles[i];
    const _NamedTextureIdentifier namedTextureIdentifier = namedTextureIdentifiers[i];

    if (namedTextureHandle.name != namedTextureIdentifier.name) {
      return false;
    }
    const HdPhTextureObjectSharedPtr &textureObject = namedTextureHandle.handle->GetTextureObject();
    if (textureObject->GetTextureIdentifier() != namedTextureIdentifier.id) {
      return false;
    }
  }

  return true;
}

}  // namespace

HdPhRenderPassShader::HdPhRenderPassShader() : HdPhRenderPassShader(HdPhPackageRenderPassShader())
{}

HdPhRenderPassShader::HdPhRenderPassShader(TfToken const &glslfxFile)
  : HdPhShaderCode(),
    _glslfxFile(glslfxFile)  // user-defined
    ,
    _glslfx(std::make_unique<HioGlslfx>(glslfxFile)),
    _hash(0),
    _hashValid(false),
    _cullStyle(HdCullStyleNothing)
{}

/*virtual*/
HdPhRenderPassShader::~HdPhRenderPassShader() = default;

/*virtual*/
HdPhRenderPassShader::ID HdPhRenderPassShader::ComputeHash() const
{
  // if nothing changed, returns the cached hash value
  if (_hashValid)
    return _hash;

  _hash = _glslfx->GetHash();

  // cullFaces are dynamic, no need to put in the hash.

  // Custom buffer bindings may vary over time, requiring invalidation
  // of down stream clients.
  TF_FOR_ALL(it, _customBuffers)
  {
    boost::hash_combine(_hash, it->second.ComputeHash());
  }

  for (const HdPhShaderCode::NamedTextureHandle &namedHandle : _namedTextureHandles) {

    // Use name and hash only - not the texture itself as this
    // does not affect the generated shader source.
    boost::hash_combine(_hash, namedHandle.name);
    boost::hash_combine(_hash, namedHandle.hash);
  }

  _hashValid = true;

  return (ID)_hash;
}

/*virtual*/
std::string HdPhRenderPassShader::GetSource(TfToken const &shaderStageKey) const
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  return _glslfx->GetSource(shaderStageKey);
}

/*virtual*/
void HdPhRenderPassShader::BindResources(const int program,
                                         HdPh_ResourceBinder const &binder,
                                         HdRenderPassState const &state)
{
  TF_FOR_ALL(it, _customBuffers)
  {
    binder.Bind(it->second);
  }

  // set fallback states (should be moved to HdRenderPassState::Bind)
  unsigned int cullStyle = _cullStyle;
  binder.BindUniformui(HdShaderTokens->cullStyle, 1, &cullStyle);

  HdPh_TextureBinder::BindResources(binder, /* useBindlessHandles = */ false, _namedTextureHandles);
}

/*virtual*/
void HdPhRenderPassShader::UnbindResources(const int program,
                                           HdPh_ResourceBinder const &binder,
                                           HdRenderPassState const &state)
{
  TF_FOR_ALL(it, _customBuffers)
  {
    binder.Unbind(it->second);
  }

  HdPh_TextureBinder::UnbindResources(binder, /* useBindlessHandles = */ false, _namedTextureHandles);

  glActiveTexture(GL_TEXTURE0);
}

void HdPhRenderPassShader::AddBufferBinding(HdBindingRequest const &req)
{
  auto it = _customBuffers.insert({req.GetName(), req});
  // Entry already existed and was equal to what we want to set it.
  if (!it.second && it.first->second == req) {
    return;
  }
  it.first->second = req;
  _hashValid = false;
}

void HdPhRenderPassShader::RemoveBufferBinding(TfToken const &name)
{
  _customBuffers.erase(name);
  _hashValid = false;
}

void HdPhRenderPassShader::ClearBufferBindings()
{
  _customBuffers.clear();
  _hashValid = false;
}

/*virtual*/
void HdPhRenderPassShader::AddBindings(HdBindingRequestVector *customBindings)
{
  // note: be careful, the logic behind this function is tricky.
  //
  // customBindings will be used for two purpose.
  //   1. resouceBinder assigned the binding location and use it
  //      in Bind/UnbindResources. The resourceBinder is held by
  //      drawingProgram in each batch in the renderPass.
  //   2. codeGen generates macros to fill the placeholder of binding location
  //      in glslfx file.
  //
  // To make RenderPassShader work on DrawBatch::Execute(), _customBuffers and
  // other resources should be bound to the right binding locations which were
  // resolved at the compilation time of the drawingProgram.
  //
  // However, if we have 2 or more renderPassStates and if they all share
  // the same shader hash signature, drawingProgram will only be constructed
  // at the first renderPassState and then be reused for the subsequent
  // renderPassStates, because the shaderHash matches in
  // Hd_DrawBatch::_GetDrawingProgram().
  //
  // The shader hash computation must guarantee the consistency such that the
  // resourceBinder held in the drawingProgram is applicable to all other
  // renderPassStates as long as the hash matches.
  //

  customBindings->reserve(customBindings->size() + _customBuffers.size() + 1);
  TF_FOR_ALL(it, _customBuffers)
  {
    customBindings->push_back(it->second);
  }

  // typed binding to emit declaration and accessor.
  customBindings->push_back(HdBindingRequest(HdBinding::UNIFORM, HdShaderTokens->cullStyle, HdTypeUInt32));
}

HdPh_MaterialParamVector const &HdPhRenderPassShader::GetParams() const
{
  return _params;
}

HdPhShaderCode::NamedTextureHandleVector const &HdPhRenderPassShader::GetNamedTextureHandles() const
{
  return _namedTextureHandles;
}

void HdPhRenderPassShader::UpdateAovInputTextures(HdRenderPassAovBindingVector const &aovInputBindings,
                                                  HdRenderIndex *const renderIndex)
{
  TRACE_FUNCTION();

  // Compute the identifiers for the textures backing the requested
  // (resolved) AOVs.
  const _NamedTextureIdentifiers namedTextureIdentifiers = _GetNamedTextureIdentifiers(aovInputBindings,
                                                                                       renderIndex);
  // If the (named) texture handles are up-to-date, there is nothing to do.
  if (_AreHandlesValid(_namedTextureHandles, namedTextureIdentifiers)) {
    return;
  }

  _hashValid = false;

  // Otherwise, we need to (re-)allocate texture handles for the
  // given texture identifiers.
  _namedTextureHandles.clear();
  _params.clear();

  HdPhResourceRegistry *const resourceRegistry = dynamic_cast<HdPhResourceRegistry *>(
    renderIndex->GetResourceRegistry().get());
  if (!TF_VERIFY(resourceRegistry)) {
    return;
  }

  for (const auto &namedTextureIdentifier : namedTextureIdentifiers) {
    static const HdSamplerParameters samplerParameters{
      HdWrapClamp, HdWrapClamp, HdWrapClamp, HdMinFilterNearest, HdMagFilterNearest};

    // Allocate texture handle for given identifier.
    HdPhTextureHandleSharedPtr textureHandle = resourceRegistry->AllocateTextureHandle(
      namedTextureIdentifier.id,
      HdTextureType::Uv,
      samplerParameters,
      /* memoryRequest = */ 0,
      /* createBindlessHandle = */ false,
      shared_from_this());
    // Add to _namedTextureHandles so that the texture will
    // be bound to the shader in BindResources.
    _namedTextureHandles.push_back(HdPhShaderCode::NamedTextureHandle{namedTextureIdentifier.name,
                                                                      HdTextureType::Uv,
                                                                      std::move(textureHandle),
                                                                      /* hash = */ 0});

    // Add a corresponding param so that codegen is
    // generating the accessor HdGet_AOVNAMEReadback().
    _params.emplace_back(
      HdPh_MaterialParam::ParamTypeTexture, namedTextureIdentifier.name, VtValue(GfVec4f(0, 0, 0, 0)));
  }
}

WABI_NAMESPACE_END
