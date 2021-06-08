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
#ifndef WABI_IMAGING_HD_UNIT_TEST_NULL_RENDER_DELEGATE_H
#define WABI_IMAGING_HD_UNIT_TEST_NULL_RENDER_DELEGATE_H

#include "wabi/imaging/hd/instancer.h"
#include "wabi/imaging/hd/renderDelegate.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class Hd_UnitTestNullRenderDelegate final : public HdRenderDelegate {
 public:
  Hd_UnitTestNullRenderDelegate()          = default;
  virtual ~Hd_UnitTestNullRenderDelegate() = default;

  virtual const TfTokenVector &GetSupportedRprimTypes() const override;
  virtual const TfTokenVector &GetSupportedSprimTypes() const override;
  virtual const TfTokenVector &GetSupportedBprimTypes() const override;
  virtual HdRenderParam *GetRenderParam() const override;
  virtual HdResourceRegistrySharedPtr GetResourceRegistry() const override;

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Renderpass factory
  ///
  ////////////////////////////////////////////////////////////////////////////

  virtual HdRenderPassSharedPtr CreateRenderPass(HdRenderIndex *index,
                                                 HdRprimCollection const &collection) override;

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Instancer Factory
  ///
  ////////////////////////////////////////////////////////////////////////////

  virtual HdInstancer *CreateInstancer(HdSceneDelegate *delegate, SdfPath const &id) override;

  virtual void DestroyInstancer(HdInstancer *instancer) override;

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Prim Factories
  ///
  ////////////////////////////////////////////////////////////////////////////

  virtual HdRprim *CreateRprim(TfToken const &typeId, SdfPath const &rprimId) override;

  virtual void DestroyRprim(HdRprim *rPrim) override;

  virtual HdSprim *CreateSprim(TfToken const &typeId, SdfPath const &sprimId) override;

  virtual HdSprim *CreateFallbackSprim(TfToken const &typeId) override;
  virtual void DestroySprim(HdSprim *sprim) override;

  virtual HdBprim *CreateBprim(TfToken const &typeId, SdfPath const &bprimId) override;

  virtual HdBprim *CreateFallbackBprim(TfToken const &typeId) override;

  virtual void DestroyBprim(HdBprim *bprim) override;

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Sync, Execute & Dispatch Hooks
  ///
  ////////////////////////////////////////////////////////////////////////////

  virtual void CommitResources(HdChangeTracker *tracker) override;

 private:
  static const TfTokenVector SUPPORTED_RPRIM_TYPES;
  static const TfTokenVector SUPPORTED_SPRIM_TYPES;
  static const TfTokenVector SUPPORTED_BPRIM_TYPES;

  Hd_UnitTestNullRenderDelegate(const Hd_UnitTestNullRenderDelegate &) = delete;
  Hd_UnitTestNullRenderDelegate &operator=(const Hd_UnitTestNullRenderDelegate &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_UNIT_TEST_NULL_RENDER_DELEGATE_H
