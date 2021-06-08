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

#include "wabi/base/gf/homogeneous.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"
#include "wabi/base/gf/vec4d.h"
#include "wabi/base/gf/vec4f.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

GfVec4f GfGetHomogenized(const GfVec4f &v)
{
  GfVec4f ret(v);

  if (ret[3] == 0)
    ret[3] = 1;
  ret /= ret[3];
  return ret;
}

GfVec4f GfHomogeneousCross(const GfVec4f &a, const GfVec4f &b)
{
  GfVec4f ah(GfGetHomogenized(a));
  GfVec4f bh(GfGetHomogenized(b));

  GfVec3f prod = GfCross(GfVec3f(ah[0], ah[1], ah[2]), GfVec3f(bh[0], bh[1], bh[2]));

  return GfVec4f(prod[0], prod[1], prod[2], 1);
}

GfVec4d GfGetHomogenized(const GfVec4d &v)
{
  GfVec4d ret(v);

  if (ret[3] == 0)
    ret[3] = 1;
  ret /= ret[3];
  return ret;
}

GfVec4d GfHomogeneousCross(const GfVec4d &a, const GfVec4d &b)
{
  GfVec4d ah(GfGetHomogenized(a));
  GfVec4d bh(GfGetHomogenized(b));

  GfVec3d prod = GfCross(GfVec3d(ah[0], ah[1], ah[2]), GfVec3d(bh[0], bh[1], bh[2]));

  return GfVec4d(prod[0], prod[1], prod[2], 1);
}

WABI_NAMESPACE_END
