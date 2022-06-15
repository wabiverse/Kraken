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

#ifndef HDRPR_INSTANCER_H
#define HDRPR_INSTANCER_H

#include "renderDelegate.h"

#include "wabi/imaging/hd/instancer.h"
#include "wabi/imaging/hd/timeSampleArray.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3f.h"
#include "wabi/base/gf/vec4f.h"
#include "wabi/base/vt/array.h"

#include <mutex>

WABI_NAMESPACE_BEGIN

class HdSceneDelegate;

class HdRprInstancer : public HdInstancer
{
 public:

  HdRprInstancer(HdSceneDelegate *delegate, SdfPath const &id HDRPR_INSTANCER_ID_ARG_DECL)
    : HdInstancer(delegate, id HDRPR_INSTANCER_ID_ARG)
  {}

  VtMatrix4dArray ComputeTransforms(SdfPath const &prototypeId);

  HdTimeSampleArray<VtMatrix4dArray, 2> SampleInstanceTransforms(SdfPath const &prototypeId);

 private:

  void Sync();

  VtMatrix4dArray m_transform;
  VtVec3fArray m_translate;
  VtVec4fArray m_rotate;
  VtVec3fArray m_scale;

  std::mutex m_syncMutex;
};

WABI_NAMESPACE_END

#endif  // HDRPR_INSTANCER_H
