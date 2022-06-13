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
///
/// \file pxOsd/subdivTags.h
///

#include "wabi/imaging/pxOsd/subdivTags.h"
#include "wabi/base/arch/hash.h"

WABI_NAMESPACE_BEGIN

PxOsdSubdivTags::ID PxOsdSubdivTags::ComputeHash() const
{

  ID hash = 0;

  hash = ArchHash64((const char *)&_vtxInterpolationRule, sizeof(_vtxInterpolationRule), hash);

  hash = ArchHash64((const char *)&_fvarInterpolationRule, sizeof(_fvarInterpolationRule), hash);

  hash = ArchHash64((const char *)&_creaseMethod, sizeof(_creaseMethod), hash);

  hash = ArchHash64((const char *)&_trianglesSubdivision, sizeof(_trianglesSubdivision), hash);

  hash = ArchHash64((const char *)_cornerIndices.cdata(),
                    _cornerIndices.size() * sizeof(int),
                    hash);

  hash = ArchHash64((const char *)_cornerWeights.cdata(),
                    _cornerWeights.size() * sizeof(float),
                    hash);

  hash = ArchHash64((const char *)_creaseIndices.cdata(),
                    _creaseIndices.size() * sizeof(int),
                    hash);

  hash = ArchHash64((const char *)_creaseLengths.cdata(),
                    _creaseLengths.size() * sizeof(int),
                    hash);

  hash = ArchHash64((const char *)_creaseWeights.cdata(),
                    _creaseWeights.size() * sizeof(float),
                    hash);

  return hash;
}

std::ostream &operator<<(std::ostream &out, PxOsdSubdivTags const &st)
{
  out << "(" << st.GetVertexInterpolationRule() << ", " << st.GetFaceVaryingInterpolationRule()
      << ", " << st.GetCreaseMethod() << ", " << st.GetTriangleSubdivision() << ", ("
      << st.GetCreaseIndices() << "), (" << st.GetCreaseLengths() << "), ("
      << st.GetCreaseWeights() << "), (" << st.GetCornerIndices() << "), ("
      << st.GetCornerWeights() << "))";
  return out;
}

bool operator==(const PxOsdSubdivTags &lhs, const PxOsdSubdivTags &rhs)
{
  return lhs.GetVertexInterpolationRule() == rhs.GetVertexInterpolationRule() &&
         lhs.GetFaceVaryingInterpolationRule() == rhs.GetFaceVaryingInterpolationRule() &&
         lhs.GetCreaseMethod() == rhs.GetCreaseMethod() &&
         lhs.GetTriangleSubdivision() == rhs.GetTriangleSubdivision() &&
         lhs.GetCreaseIndices() == rhs.GetCreaseIndices() &&
         lhs.GetCreaseLengths() == rhs.GetCreaseLengths() &&
         lhs.GetCreaseWeights() == rhs.GetCreaseWeights() &&
         lhs.GetCornerIndices() == rhs.GetCornerIndices() &&
         lhs.GetCornerWeights() == rhs.GetCornerWeights();
}

bool operator!=(const PxOsdSubdivTags &lhs, const PxOsdSubdivTags &rhs)
{
  return !(lhs == rhs);
}

WABI_NAMESPACE_END
