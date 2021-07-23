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

#include "wabi/imaging/hdPh/textureObject.h"

#include "wabi/imaging/hdPh/assetUvTextureCpuData.h"
#include "wabi/imaging/hdPh/fieldSubtextureIdentifier.h"
#include "wabi/imaging/hdPh/fieldTextureCpuData.h"
#include "wabi/imaging/hdPh/ptexTextureObject.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/textureCpuData.h"
#include "wabi/imaging/hdPh/textureIdentifier.h"
#include "wabi/imaging/hdPh/textureObjectRegistry.h"
#include "wabi/imaging/hdPh/tokens.h"
#include "wabi/imaging/hdPh/udimTextureObject.h"

#include "wabi/imaging/hgi/blitCmds.h"
#include "wabi/imaging/hgi/hgi.h"

#include "wabi/imaging/hio/fieldTextureData.h"

#include "wabi/usd/ar/resolver.h"

WABI_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
// HdPhTextureObject

HdPhTextureObject::HdPhTextureObject(const HdPhTextureIdentifier &textureId,
                                     HdPh_TextureObjectRegistry *const textureObjectRegistry)
  : _textureObjectRegistry(textureObjectRegistry),
    _textureId(textureId),
    _targetMemory(0)
{}

void HdPhTextureObject::SetTargetMemory(const size_t targetMemory)
{
  if (_targetMemory == targetMemory)
  {
    return;
  }
  _targetMemory = targetMemory;
  _textureObjectRegistry->MarkTextureObjectDirty(shared_from_this());
}

HdPhResourceRegistry *HdPhTextureObject::_GetResourceRegistry() const
{
  if (!TF_VERIFY(_textureObjectRegistry))
  {
    return nullptr;
  }

  HdPhResourceRegistry *const registry = _textureObjectRegistry->GetResourceRegistry();
  TF_VERIFY(registry);

  return registry;
}

Hgi *HdPhTextureObject::_GetHgi() const
{
  HdPhResourceRegistry *const registry = _GetResourceRegistry();
  if (!TF_VERIFY(registry))
  {
    return nullptr;
  }

  Hgi *const hgi = registry->GetHgi();
  TF_VERIFY(hgi);

  return hgi;
}

void HdPhTextureObject::_AdjustTotalTextureMemory(const int64_t memDiff)
{
  if (TF_VERIFY(_textureObjectRegistry))
  {
    _textureObjectRegistry->AdjustTotalTextureMemory(memDiff);
  }
}

void HdPhTextureObject::_AddToTotalTextureMemory(const HgiTextureHandle &texture)
{
  if (texture)
  {
    _AdjustTotalTextureMemory(texture->GetByteSizeOfResource());
  }
}

void HdPhTextureObject::_SubtractFromTotalTextureMemory(const HgiTextureHandle &texture)
{
  if (texture)
  {
    _AdjustTotalTextureMemory(-texture->GetByteSizeOfResource());
  }
}

HdPhTextureObject::~HdPhTextureObject() = default;

///////////////////////////////////////////////////////////////////////////////
// Helpers

std::string HdPhTextureObject::_GetDebugName(const HdPhTextureIdentifier &textureId) const
{
  const std::string &filePath = textureId.GetFilePath().GetString();
  const HdPhSubtextureIdentifier *const subId = textureId.GetSubtextureIdentifier();

  if (!subId)
  {
    return filePath;
  }

  if (const HdPhOpenVDBAssetSubtextureIdentifier *const vdbSubId =
        dynamic_cast<const HdPhOpenVDBAssetSubtextureIdentifier *>(subId))
  {
    return filePath + " - " + vdbSubId->GetFieldName().GetString();
  }

  if (const HdPhField3DAssetSubtextureIdentifier *const f3dSubId =
        dynamic_cast<const HdPhField3DAssetSubtextureIdentifier *>(subId))
  {
    return filePath + " - " + f3dSubId->GetFieldName().GetString() + " " +
           std::to_string(f3dSubId->GetFieldIndex()) + " " + f3dSubId->GetFieldPurpose().GetString();
  }

  if (const HdPhAssetUvSubtextureIdentifier *const assetUvSubId =
        dynamic_cast<const HdPhAssetUvSubtextureIdentifier *>(subId))
  {
    return filePath + " - flipVertically=" + std::to_string(int(assetUvSubId->GetFlipVertically())) +
           " - premultiplyAlpha=" + std::to_string(int(assetUvSubId->GetPremultiplyAlpha())) +
           " - sourceColorSpace=" + assetUvSubId->GetSourceColorSpace().GetString();
  }

  if (const HdPhPtexSubtextureIdentifier *const ptexSubId =
        dynamic_cast<const HdPhPtexSubtextureIdentifier *>(subId))
  {
    return filePath + " - premultiplyAlpha=" + std::to_string(int(ptexSubId->GetPremultiplyAlpha()));
  }

  if (const HdPhUdimSubtextureIdentifier *const udimSubId =
        dynamic_cast<const HdPhUdimSubtextureIdentifier *>(subId))
  {
    return filePath + +" - premultiplyAlpha=" + std::to_string(int(udimSubId->GetPremultiplyAlpha())) +
           " - sourceColorSpace=" + udimSubId->GetSourceColorSpace().GetString();
  }

  return filePath + " - unknown subtexture identifier";
}

// Read from the HdPhSubtextureIdentifier whether we need
// to pre-multiply the texture by alpha
//
bool HdPhTextureObject::_GetPremultiplyAlpha(const HdPhSubtextureIdentifier *const subId) const
{
  switch (GetTextureType())
  {
    case HdTextureType::Uv:
      if (const HdPhAssetUvSubtextureIdentifier *const uvSubId =
            dynamic_cast<const HdPhAssetUvSubtextureIdentifier *>(subId))
      {
        return uvSubId->GetPremultiplyAlpha();
      }
      return false;
    case HdTextureType::Ptex:
      if (const HdPhPtexSubtextureIdentifier *const ptexSubId =
            dynamic_cast<const HdPhPtexSubtextureIdentifier *>(subId))
      {
        return ptexSubId->GetPremultiplyAlpha();
      }
      return false;
    case HdTextureType::Udim:
      if (const HdPhUdimSubtextureIdentifier *const udimSubId =
            dynamic_cast<const HdPhUdimSubtextureIdentifier *>(subId))
      {
        return udimSubId->GetPremultiplyAlpha();
      }
      return false;
    default:
      return false;
  }
}

// Read from the HdPhSubtextureIdentifier its source color space
//
HioImage::SourceColorSpace HdPhTextureObject::_GetSourceColorSpace(
  const HdPhSubtextureIdentifier *const subId) const
{
  TfToken sourceColorSpace;
  switch (GetTextureType())
  {
    case HdTextureType::Uv:
      if (const HdPhAssetUvSubtextureIdentifier *const uvSubId =
            dynamic_cast<const HdPhAssetUvSubtextureIdentifier *>(subId))
      {
        sourceColorSpace = uvSubId->GetSourceColorSpace();
      }
      break;
    case HdTextureType::Udim:
      if (const HdPhUdimSubtextureIdentifier *const udimSubId =
            dynamic_cast<const HdPhUdimSubtextureIdentifier *>(subId))
      {
        sourceColorSpace = udimSubId->GetSourceColorSpace();
      }
      break;
    default:
      break;
  }

  if (sourceColorSpace == HdPhTokens->sRGB)
  {
    return HioImage::SRGB;
  }
  if (sourceColorSpace == HdPhTokens->raw)
  {
    return HioImage::Raw;
  }
  return HioImage::Auto;
}

///////////////////////////////////////////////////////////////////////////////
// Uv texture

HdPhUvTextureObject::HdPhUvTextureObject(const HdPhTextureIdentifier &textureId,
                                         HdPh_TextureObjectRegistry *textureObjectRegistry)
  : HdPhTextureObject(textureId, textureObjectRegistry),
    _wrapParameters{HdWrapNoOpinion, HdWrapNoOpinion}
{}

HdTextureType HdPhUvTextureObject::GetTextureType() const
{
  return HdTextureType::Uv;
}

HdPhUvTextureObject::~HdPhUvTextureObject()
{
  _DestroyTexture();
}

void HdPhUvTextureObject::_SetWrapParameters(const std::pair<HdWrap, HdWrap> &wrapParameters)
{
  _wrapParameters = wrapParameters;
}

void HdPhUvTextureObject::_SetCpuData(std::unique_ptr<HdPhTextureCpuData> &&cpuData)
{
  _cpuData = std::move(cpuData);
}

HdPhTextureCpuData *HdPhUvTextureObject::_GetCpuData() const
{
  return _cpuData.get();
}

void HdPhUvTextureObject::_CreateTexture(const HgiTextureDesc &desc)
{
  Hgi *const hgi = _GetHgi();
  if (!TF_VERIFY(hgi))
  {
    return;
  }

  _DestroyTexture();

  _gpuTexture = hgi->CreateTexture(desc);
  _AddToTotalTextureMemory(_gpuTexture);
}

void HdPhUvTextureObject::_GenerateMipmaps()
{
  HdPhResourceRegistry *const registry = _GetResourceRegistry();
  if (!TF_VERIFY(registry))
  {
    return;
  }

  if (!_gpuTexture)
  {
    return;
  }

  HgiBlitCmds *const blitCmds = registry->GetGlobalBlitCmds();
  blitCmds->GenerateMipMaps(_gpuTexture);
}

void HdPhUvTextureObject::_DestroyTexture()
{
  if (Hgi *hgi = _GetHgi())
  {
    _SubtractFromTotalTextureMemory(_gpuTexture);
    hgi->DestroyTexture(&_gpuTexture);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Uv asset texture

// Read from the HdPhAssetUvSubtextureIdentifier whether we need
// to flip the image.
//
// This is to support the legacy HwUvTexture_1 shader node which has the
// vertical orientation opposite to UsdUvTexture.
//
static HioImage::ImageOriginLocation _GetImageOriginLocation(const HdPhSubtextureIdentifier *const subId)
{
  using SubId = const HdPhAssetUvSubtextureIdentifier;

  if (SubId *const uvSubId = dynamic_cast<SubId *>(subId))
  {
    if (uvSubId->GetFlipVertically())
    {
      return HioImage::OriginUpperLeft;
    }
  }
  return HioImage::OriginLowerLeft;
}

HdPhAssetUvTextureObject::HdPhAssetUvTextureObject(const HdPhTextureIdentifier &textureId,
                                                   HdPh_TextureObjectRegistry *const textureObjectRegistry)
  : HdPhUvTextureObject(textureId, textureObjectRegistry)
{}

HdPhAssetUvTextureObject::~HdPhAssetUvTextureObject() = default;

void HdPhAssetUvTextureObject::_Load()
{
  TRACE_FUNCTION();

  std::unique_ptr<HdPhAssetUvTextureCpuData> cpuData = std::make_unique<HdPhAssetUvTextureCpuData>(
    GetTextureIdentifier().GetFilePath(),
    GetTargetMemory(),
    _GetPremultiplyAlpha(GetTextureIdentifier().GetSubtextureIdentifier()),
    _GetImageOriginLocation(GetTextureIdentifier().GetSubtextureIdentifier()),
    HdPhTextureObject::_GetSourceColorSpace(GetTextureIdentifier().GetSubtextureIdentifier()));
  _SetWrapParameters(cpuData->GetWrapInfo());
  _SetCpuData(std::move(cpuData));
}

void HdPhAssetUvTextureObject::_Commit()
{
  TRACE_FUNCTION();

  _DestroyTexture();

  if (HdPhTextureCpuData *const cpuData = _GetCpuData())
  {
    if (cpuData->IsValid())
    {
      // Upload to GPU
      _CreateTexture(cpuData->GetTextureDesc());
      if (cpuData->GetGenerateMipmaps())
      {
        _GenerateMipmaps();
      }
    }
  }

  // Free CPU memory after transfer to GPU
  _SetCpuData(nullptr);
}

bool HdPhAssetUvTextureObject::IsValid() const
{
  return bool(GetTexture());
}

///////////////////////////////////////////////////////////////////////////////
// Field texture

// Compute transform mapping GfRange3d to unit box [0,1]^3
static GfMatrix4d _ComputeSamplingTransform(const GfRange3d &range)
{
  const GfVec3d size(range.GetSize());

  const GfVec3d scale(1.0 / size[0], 1.0 / size[1], 1.0 / size[2]);

  return
    // First map range so that min becomes (0,0,0)
    GfMatrix4d(1.0).SetTranslateOnly(-range.GetMin()) *
    // Then scale to unit box
    GfMatrix4d(1.0).SetScale(scale);
}

// Compute transform mapping bounding box to unit box [0,1]^3
static GfMatrix4d _ComputeSamplingTransform(const GfBBox3d &bbox)
{
  return
    // First map so that bounding box goes to its GfRange3d
    bbox.GetInverseMatrix() *
    // Then scale to unit box [0,1]^3
    _ComputeSamplingTransform(bbox.GetRange());
}

static HioFieldTextureDataSharedPtr _ComputeFieldTexData(const HdPhTextureIdentifier &textureId,
                                                         const size_t targetMemory)
{
  const std::string &filePath = textureId.GetFilePath().GetString();
  const HdPhSubtextureIdentifier *const subId = textureId.GetSubtextureIdentifier();

  if (const HdPhOpenVDBAssetSubtextureIdentifier *const vdbSubId =
        dynamic_cast<const HdPhOpenVDBAssetSubtextureIdentifier *>(subId))
  {
    if (vdbSubId->GetFieldIndex() != 0)
    {
      TF_WARN(
        "Support of field index when reading OpenVDB file not yet "
        "implemented (file: %s, field name: %s, field index: %d",
        filePath.c_str(),
        vdbSubId->GetFieldName().GetText(),
        vdbSubId->GetFieldIndex());
    }
    return HioFieldTextureData::New(filePath, vdbSubId->GetFieldName(), 0, std::string(), targetMemory);
  }

  if (const HdPhField3DAssetSubtextureIdentifier *const f3dSubId =
        dynamic_cast<const HdPhField3DAssetSubtextureIdentifier *>(subId))
  {
    return HioFieldTextureData::New(filePath,
                                    f3dSubId->GetFieldName(),
                                    f3dSubId->GetFieldIndex(),
                                    f3dSubId->GetFieldPurpose(),
                                    targetMemory);
  }

  TF_CODING_ERROR("Unsupported field subtexture identifier");

  return nullptr;
}

HdPhFieldTextureObject::HdPhFieldTextureObject(const HdPhTextureIdentifier &textureId,
                                               HdPh_TextureObjectRegistry *const textureObjectRegistry)
  : HdPhTextureObject(textureId, textureObjectRegistry)
{}

HdPhFieldTextureObject::~HdPhFieldTextureObject()
{
  if (Hgi *hgi = _GetHgi())
  {
    _SubtractFromTotalTextureMemory(_gpuTexture);
    hgi->DestroyTexture(&_gpuTexture);
  }
}

void HdPhFieldTextureObject::_Load()
{
  TRACE_FUNCTION();

  HioFieldTextureDataSharedPtr const texData = _ComputeFieldTexData(GetTextureIdentifier(),
                                                                    GetTargetMemory());

  if (!texData)
  {
    return;
  }

  texData->Read();

  _cpuData = std::make_unique<HdPh_FieldTextureCpuData>(texData, _GetDebugName(GetTextureIdentifier()));

  if (_cpuData->IsValid())
  {
    if (_cpuData->GetTextureDesc().type != HgiTextureType3D)
    {
      TF_CODING_ERROR("Wrong texture type for field");
    }

    _bbox = texData->GetBoundingBox();
    _samplingTransform = _ComputeSamplingTransform(_bbox);
  } else
  {
    _bbox = GfBBox3d();
    _samplingTransform = GfMatrix4d(1.0);
  }
}

void HdPhFieldTextureObject::_Commit()
{
  TRACE_FUNCTION();

  Hgi *const hgi = _GetHgi();
  if (!hgi)
  {
    return;
  }

  // Free previously allocated texture
  _SubtractFromTotalTextureMemory(_gpuTexture);
  hgi->DestroyTexture(&_gpuTexture);

  // Upload to GPU only if we have valid CPU data
  if (_cpuData && _cpuData->IsValid())
  {
    _gpuTexture = hgi->CreateTexture(_cpuData->GetTextureDesc());
    _AddToTotalTextureMemory(_gpuTexture);
  }

  // Free CPU memory after transfer to GPU
  _cpuData.reset();
}

bool HdPhFieldTextureObject::IsValid() const
{
  return bool(_gpuTexture);
}

HdTextureType HdPhFieldTextureObject::GetTextureType() const
{
  return HdTextureType::Field;
}

WABI_NAMESPACE_END
