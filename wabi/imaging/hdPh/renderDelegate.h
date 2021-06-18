//
// Copyright 2017 Pixar
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
#ifndef WABI_IMAGING_HD_ST_RENDER_DELEGATE_H
#define WABI_IMAGING_HD_ST_RENDER_DELEGATE_H

#include "wabi/imaging/hd/renderDelegate.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include <memory>
#include <mutex>

WABI_NAMESPACE_BEGIN

class Hgi;
class HdPhRenderParam;

using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

///
/// HdPhRenderDelegate
///
/// The Phoenix Render Delegate provides a rasterizer renderer to draw the scene.
/// While it currently has some ties to GL, the goal is to use Hgi to allow
/// it to be graphics API agnostic.
///
class HdPhRenderDelegate final : public HdRenderDelegate
{
 public:
  HDPH_API
  HdPhRenderDelegate();
  HDPH_API
  HdPhRenderDelegate(HdRenderSettingsMap const &settingsMap);

  HDPH_API
  virtual ~HdPhRenderDelegate();

  // ---------------------------------------------------------------------- //
  /// \name HdRenderDelegate virtual API
  // ---------------------------------------------------------------------- //

  HDPH_API
  virtual void SetDrivers(HdDriverVector const &drivers) override;

  HDPH_API
  virtual HdRenderParam *GetRenderParam() const override;

  HDPH_API
  virtual const TfTokenVector &GetSupportedRprimTypes() const override;
  HDPH_API
  virtual const TfTokenVector &GetSupportedSprimTypes() const override;
  HDPH_API
  virtual const TfTokenVector &GetSupportedBprimTypes() const override;
  HDPH_API
  virtual HdResourceRegistrySharedPtr GetResourceRegistry() const override;

  HDPH_API
  virtual HdRenderPassSharedPtr CreateRenderPass(HdRenderIndex *index,
                                                 HdRprimCollection const &collection) override;
  HDPH_API
  virtual HdRenderPassStateSharedPtr CreateRenderPassState() const override;

  HDPH_API
  virtual HdInstancer *CreateInstancer(HdSceneDelegate *delegate, SdfPath const &id) override;

  HDPH_API
  virtual void DestroyInstancer(HdInstancer *instancer) override;

  HDPH_API
  virtual HdRprim *CreateRprim(TfToken const &typeId, SdfPath const &rprimId) override;
  HDPH_API
  virtual void DestroyRprim(HdRprim *rPrim) override;

  HDPH_API
  virtual HdSprim *CreateSprim(TfToken const &typeId, SdfPath const &sprimId) override;
  HDPH_API
  virtual HdSprim *CreateFallbackSprim(TfToken const &typeId) override;
  HDPH_API
  virtual void DestroySprim(HdSprim *sPrim) override;

  HDPH_API
  virtual HdBprim *CreateBprim(TfToken const &typeId, SdfPath const &bprimId) override;
  HDPH_API
  virtual HdBprim *CreateFallbackBprim(TfToken const &typeId) override;
  HDPH_API
  virtual void DestroyBprim(HdBprim *bPrim) override;

  HDPH_API
  virtual void CommitResources(HdChangeTracker *tracker) override;

  HDPH_API
  virtual TfTokenVector GetMaterialRenderContexts() const override;

  HDPH_API
  virtual TfTokenVector GetShaderSourceTypes() const override;

  HDPH_API
  virtual bool IsPrimvarFilteringNeeded() const override;

  HDPH_API
  virtual HdRenderSettingDescriptorList GetRenderSettingDescriptors() const override;

  HDPH_API
  virtual VtDictionary GetRenderStats() const override;

  HDPH_API
  virtual HdAovDescriptor GetDefaultAovDescriptor(TfToken const &name) const override;

  // ---------------------------------------------------------------------- //
  /// \name Misc public API
  // ---------------------------------------------------------------------- //

  // Returns whether or not HdPhRenderDelegate can run on the current
  // hardware.
  HDPH_API
  static bool IsSupported();

  // Returns Hydra graphics interface
  HDPH_API
  Hgi *GetHgi();

 private:
  void _ApplyTextureSettings();
  HdSprim *_CreateFallbackMaterialPrim();

  HdPhRenderDelegate(const HdPhRenderDelegate &) = delete;
  HdPhRenderDelegate &operator=(const HdPhRenderDelegate &) = delete;

  static const TfTokenVector SUPPORTED_RPRIM_TYPES;
  static const TfTokenVector SUPPORTED_SPRIM_TYPES;

  // Resource registry used in this render delegate
  HdPhResourceRegistrySharedPtr _resourceRegistry;

  HdRenderSettingDescriptorList _settingDescriptors;

  Hgi *_hgi;

  std::unique_ptr<HdPhRenderParam> _renderParam;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_RENDER_DELEGATE_H
