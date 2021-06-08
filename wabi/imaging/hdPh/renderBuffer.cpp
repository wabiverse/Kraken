/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#include "wabi/imaging/hdPh/renderBuffer.h"

#include "wabi/imaging/hd/aov.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hdPh/dynamicUvTextureObject.h"
#include "wabi/imaging/hdPh/hgiConversions.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/tokens.h"
#include "wabi/imaging/hgi/blitCmds.h"
#include "wabi/imaging/hgi/blitCmdsOps.h"
#include "wabi/imaging/hgi/texture.h"

WABI_NAMESPACE_BEGIN

static HgiTextureUsage _GetTextureUsage(HdFormat format, TfToken const &name)
{
  if (HdAovHasDepthSemantic(name)) {
    if (format == HdFormatFloat32UInt8) {
      return HgiTextureUsageBitsDepthTarget | HgiTextureUsageBitsStencilTarget;
    }
    return HgiTextureUsageBitsDepthTarget;
  }

  return HgiTextureUsageBitsColorTarget;
}

HdPhRenderBuffer::HdPhRenderBuffer(HdPhResourceRegistry *const resourceRegistry, SdfPath const &id)
    : HdRenderBuffer(id),
      _resourceRegistry(resourceRegistry),
      _format(HdFormatInvalid),
      _msaaSampleCount(4),
      _mappers(0),
      _mappedBuffer()
{}

HdPhRenderBuffer::~HdPhRenderBuffer() = default;

void HdPhRenderBuffer::Sync(HdSceneDelegate *sceneDelegate,
                            HdRenderParam *renderParam,
                            HdDirtyBits *dirtyBits)
{
  // Invoke base class processing for the DirtyDescriptor bit after pulling
  // the MSAA sample count, which is authored for consumption by Phoenix alone.
  if (*dirtyBits & DirtyDescription) {
    VtValue val = sceneDelegate->Get(GetId(), HdPhRenderBufferTokens->phoenixMsaaSampleCount);
    if (val.IsHolding<uint32_t>()) {
      _msaaSampleCount = val.UncheckedGet<uint32_t>();
    }
  }

  HdRenderBuffer::Sync(sceneDelegate, renderParam, dirtyBits);
}

HdPhTextureIdentifier HdPhRenderBuffer::GetTextureIdentifier(const bool multiSampled)
{
  // The texture identifier has to be unique across different render
  // delegates sharing the same resource registry.
  //
  // Thus, we cannot just use the path of the render buffer. Adding
  // "this" pointer to ensure uniqueness.
  //
  std::string idStr = GetId().GetString();
  if (multiSampled) {
    idStr += " [MSAA]";
  }
  idStr += TfStringPrintf("[%p] ", this);

  return HdPhTextureIdentifier(TfToken(idStr),
                               // Tag as texture not being loaded from an asset by
                               // texture registry but populated by us.
                               std::make_unique<HdPhDynamicUvSubtextureIdentifier>());
}

// Debug name for texture.
static std::string _GetDebugName(const HdPhDynamicUvTextureObjectSharedPtr &textureObject)
{
  return textureObject->GetTextureIdentifier().GetFilePath().GetString();
}

static void _CreateTexture(HdPhDynamicUvTextureObjectSharedPtr const &textureObject,
                           const HgiTextureDesc &desc)
{
  HgiTextureHandle const texture = textureObject->GetTexture();
  if (texture && texture->GetDescriptor() == desc) {
    return;
  }

  textureObject->CreateTexture(desc);
}

bool HdPhRenderBuffer::Allocate(GfVec3i const &dimensions,
                                const HdFormat format,
                                const bool multiSampled)
{
  _format = format;

  if (_format == HdFormatInvalid) {
    _textureObject     = nullptr;
    _textureMSAAObject = nullptr;
    return false;
  }

  if (!_textureObject) {
    // Allocate texture object if necessary.
    _textureObject = std::dynamic_pointer_cast<HdPhDynamicUvTextureObject>(
        _resourceRegistry->AllocateTextureObject(GetTextureIdentifier(/*multiSampled = */ false),
                                                 HdTextureType::Uv));
    if (!_textureObject) {
      TF_CODING_ERROR("Expected HdPhDynamicUvTextureObject");
      return false;
    }
  }

  if (multiSampled) {
    if (!_textureMSAAObject) {
      // Allocate texture object if necesssary
      _textureMSAAObject = std::dynamic_pointer_cast<HdPhDynamicUvTextureObject>(
          _resourceRegistry->AllocateTextureObject(GetTextureIdentifier(/*multiSampled = */ true),
                                                   HdTextureType::Uv));
      if (!_textureMSAAObject) {
        TF_CODING_ERROR("Expected HdPhDynamicUvTextureObject");
        return false;
      }
    }
  }
  else {
    // De-allocate texture object
    _textureMSAAObject = nullptr;
  }

  HgiTextureDesc texDesc;
  texDesc.debugName   = _GetDebugName(_textureObject);
  texDesc.dimensions  = dimensions;
  texDesc.type        = (dimensions[2] > 1) ? HgiTextureType3D : HgiTextureType2D;
  texDesc.format      = HdPhHgiConversions::GetHgiFormat(format);
  texDesc.usage       = _GetTextureUsage(format, GetId().GetNameToken());
  texDesc.sampleCount = HgiSampleCount1;

  // Allocate actual GPU resource
  _CreateTexture(_textureObject, texDesc);

  if (multiSampled) {
    texDesc.debugName   = _GetDebugName(_textureMSAAObject);
    texDesc.sampleCount = HgiSampleCount(_msaaSampleCount);

    // Allocate actual GPU resource
    _CreateTexture(_textureMSAAObject, texDesc);
  }

  return true;
}

void HdPhRenderBuffer::_Deallocate()
{
  _textureObject     = nullptr;
  _textureMSAAObject = nullptr;
}

void *HdPhRenderBuffer::Map()
{
  _mappers.fetch_add(1);

  if (!_textureObject) {
    return nullptr;
  }

  HgiTextureHandle const texture = _textureObject->GetTexture();
  if (!texture) {
    return nullptr;
  }

  const HgiTextureDesc &desc = texture->GetDescriptor();
  const size_t dataByteSize  = desc.dimensions[0] * desc.dimensions[1] * desc.dimensions[2] *
                              HgiGetDataSizeOfFormat(desc.format);

  if (dataByteSize == 0) {
    return nullptr;
  }

  _mappedBuffer.resize(dataByteSize);

  if (!TF_VERIFY(_resourceRegistry)) {
    return nullptr;
  }

  Hgi *const hgi = _resourceRegistry->GetHgi();
  if (!TF_VERIFY(hgi)) {
    return nullptr;
  }

  // Use blit work to record resource copy commands.
  HgiBlitCmdsUniquePtr blitCmds = hgi->CreateBlitCmds();

  {
    HgiTextureGpuToCpuOp copyOp;
    copyOp.gpuSourceTexture          = texture;
    copyOp.sourceTexelOffset         = GfVec3i(0);
    copyOp.mipLevel                  = 0;
    copyOp.cpuDestinationBuffer      = _mappedBuffer.data();
    copyOp.destinationByteOffset     = 0;
    copyOp.destinationBufferByteSize = dataByteSize;
    blitCmds->CopyTextureGpuToCpu(copyOp);
  }

  hgi->SubmitCmds(blitCmds.get(), HgiSubmitWaitTypeWaitUntilCompleted);

  return _mappedBuffer.data();
}

void HdPhRenderBuffer::Unmap()
{
  // XXX We could consider clearing _mappedBuffer here to free RAM.
  //     For now we assume that Map() will be called frequently so we prefer
  //     to avoid the cost of clearing the buffer over memory savings.
  // _mappedBuffer.clear();
  // _mappedBuffer.shrink_to_fit();
  _mappers.fetch_sub(1);
}

void HdPhRenderBuffer::Resolve()
{
  // Textures are resolved at the end of a render pass via the graphicsCmds
  // by supplying the resolve textures to the graphicsCmds descriptor.
}

static VtValue _GetResource(const HdPhDynamicUvTextureObjectSharedPtr &textureObject)
{
  if (textureObject) {
    return VtValue(textureObject->GetTexture());
  }
  else {
    return VtValue();
  }
}

VtValue HdPhRenderBuffer::GetResource(const bool multiSampled) const
{
  if (multiSampled) {
    return _GetResource(_textureMSAAObject);
  }
  else {
    return _GetResource(_textureObject);
  }
}

bool HdPhRenderBuffer::IsMultiSampled() const
{
  return bool(_textureMSAAObject);
}

static const GfVec3i &_GetDimensions(HdPhDynamicUvTextureObjectSharedPtr const &textureObject)
{
  static const GfVec3i invalidDimensions(0, 0, 0);

  if (!textureObject) {
    return invalidDimensions;
  }
  HgiTextureHandle const texture = textureObject->GetTexture();
  if (!texture) {
    return invalidDimensions;
  }
  return texture->GetDescriptor().dimensions;
}

unsigned int HdPhRenderBuffer::GetWidth() const
{
  return _GetDimensions(_textureObject)[0];
}

unsigned int HdPhRenderBuffer::GetHeight() const
{
  return _GetDimensions(_textureObject)[1];
}

unsigned int HdPhRenderBuffer::GetDepth() const
{
  return _GetDimensions(_textureObject)[2];
}

WABI_NAMESPACE_END
