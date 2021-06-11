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
#ifndef WABI_IMAGING_HD_ST_VOLUME_H
#define WABI_IMAGING_HD_ST_VOLUME_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hd/volume.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class HdPhDrawItem;

/// Represents a Volume Prim.
///
class HdPhVolume final : public HdVolume {
 public:
  HDPH_API
  HdPhVolume(SdfPath const &id);
  HDPH_API
  ~HdPhVolume() override;

  HDPH_API
  HdDirtyBits GetInitialDirtyBitsMask() const override;

  HDPH_API
  void Sync(HdSceneDelegate *delegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits,
            TfToken const &reprToken) override;

  HDPH_API
  void Finalize(HdRenderParam *renderParam) override;

  /// Default step size used for raymarching
  static const float defaultStepSize;

  /// Default step size used for raymarching for lighting computation
  static const float defaultStepSizeLighting;

  /// Default memory limit for a field texture (in Mb) if not
  /// overridden by field prim with textureMemory.
  static const float defaultMaxTextureMemoryPerField;

 protected:
  void _InitRepr(TfToken const &reprToken, HdDirtyBits *dirtyBits) override;

  HdDirtyBits _PropagateDirtyBits(HdDirtyBits bits) const override;

  void _UpdateRepr(HdSceneDelegate *sceneDelegate,
                   HdRenderParam *renderParam,
                   TfToken const &reprToken,
                   HdDirtyBits *dirtyBitsState);

 private:
  void _UpdateDrawItem(HdSceneDelegate *sceneDelegate,
                       HdRenderParam *renderParam,
                       HdPhDrawItem *drawItem,
                       HdDirtyBits *dirtyBits);

  HdReprSharedPtr _volumeRepr;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_VOLUME_H
