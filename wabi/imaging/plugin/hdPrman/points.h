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
#ifndef EXT_RMANPKG_23_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_POINTS_H
#define EXT_RMANPKG_23_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_POINTS_H

#include "wabi/base/gf/matrix4f.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/points.h"
#include "wabi/imaging/hd/vertexAdjacency.h"
#include "wabi/imaging/plugin/hdPrman/gprim.h"
#include "wabi/wabi.h"

#include "Riley.h"

WABI_NAMESPACE_BEGIN

class HdPrman_Points final : public HdPrman_Gprim<HdPoints>
{
 public:
  typedef HdPrman_Gprim<HdPoints> BASE;
  HF_MALLOC_TAG_NEW("new HdPrman_Points");
  HdPrman_Points(SdfPath const &id);
  virtual HdDirtyBits GetInitialDirtyBitsMask() const override;

 protected:
  virtual RtPrimVarList
  _ConvertGeometry(HdPrman_Context *context,
                   HdSceneDelegate *sceneDelegate,
                   const SdfPath &id,
                   RtUString *primType,
                   std::vector<HdGeomSubset> *geomSubsets) override;
};

WABI_NAMESPACE_END

#endif  // EXT_RMANPKG_23_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_POINTS_H
