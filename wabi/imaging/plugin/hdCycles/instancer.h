//  Copyright 2020 Tangent Animation
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
//  including without limitation, as related to merchantability and fitness
//  for a particular purpose.
//
//  In no event shall any copyright holder be liable for any damages of any kind
//  arising from the use of this software, whether in contract, tort or otherwise.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef HD_CYCLES_INSTANCER_H
#define HD_CYCLES_INSTANCER_H

#include "hdcycles.h"

#include <mutex>

#include <wabi/base/gf/matrix4d.h>
#include <wabi/base/gf/vec3f.h>
#include <wabi/base/gf/vec4f.h>
#include <wabi/base/vt/array.h>
#include <wabi/imaging/hd/instancer.h>
#include <wabi/imaging/hd/timeSampleArray.h>

WABI_NAMESPACE_BEGIN

class HdSceneDelegate;

/**
 * @brief Properly computes instance transforms for time varying data
 * Heavily inspired by ReadeonProRenderUSD's Instancer.cpp
 *
 */
class HdCyclesInstancer : public HdInstancer {
 public:
  HdCyclesInstancer(HdSceneDelegate *delegate, SdfPath const &id) : HdInstancer(delegate, id)
  {}

  VtMatrix4dArray ComputeTransforms(SdfPath const &prototypeId);

  HdTimeSampleArray<VtMatrix4dArray, HD_CYCLES_MOTION_STEPS> SampleInstanceTransforms(
      SdfPath const &prototypeId);

 private:
  void Sync();

  VtMatrix4dArray m_transform;
  VtVec3fArray m_translate;
  VtVec4fArray m_rotate;
  VtVec3fArray m_scale;

  std::mutex m_syncMutex;
};

WABI_NAMESPACE_END

#endif  // HD_CYCLES_INSTANCER_H
