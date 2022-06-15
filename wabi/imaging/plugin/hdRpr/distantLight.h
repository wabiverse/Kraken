/************************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
************************************************************************/

#ifndef HDRPR_DISTANT_LIGHT_H
#define HDRPR_DISTANT_LIGHT_H

#include "wabi/wabi.h"

#include "wabi/base/gf/matrix4f.h"
#include "wabi/imaging/hd/sprim.h"
#include "wabi/usd/sdf/path.h"

namespace rpr
{
  class DirectionalLight;
}

WABI_NAMESPACE_BEGIN

class HdRprDistantLight : public HdSprim
{

 public:

  HdRprDistantLight(SdfPath const &id) : HdSprim(id) {}

  void Sync(HdSceneDelegate *sceneDelegate,
            HdRenderParam *renderParam,
            HdDirtyBits *dirtyBits) override;

  HdDirtyBits GetInitialDirtyBitsMask() const override;

  void Finalize(HdRenderParam *renderParam) override;

 protected:

  rpr::DirectionalLight *m_rprLight = nullptr;
  GfMatrix4f m_transform;
};

WABI_NAMESPACE_END

#endif  // HDRPR_DISTANT_LIGHT_H
