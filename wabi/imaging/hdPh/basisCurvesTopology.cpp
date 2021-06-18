//
// Copyright 2016 Pixar
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
#include "wabi/imaging/hdPh/basisCurvesTopology.h"
#include "wabi/imaging/hdPh/basisCurvesComputations.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/vtBufferSource.h"

WABI_NAMESPACE_BEGIN

// static
HdPh_BasisCurvesTopologySharedPtr HdPh_BasisCurvesTopology::New(const HdBasisCurvesTopology &src)
{
  return HdPh_BasisCurvesTopologySharedPtr(new HdPh_BasisCurvesTopology(src));
}

// explicit
HdPh_BasisCurvesTopology::HdPh_BasisCurvesTopology(const HdBasisCurvesTopology &src)
  : HdBasisCurvesTopology(src)
{}

HdPh_BasisCurvesTopology::~HdPh_BasisCurvesTopology()
{}

HdBufferSourceSharedPtr HdPh_BasisCurvesTopology::GetPointsIndexBuilderComputation()
{
  // This is simple enough to return the result right away, instead of
  // using a computed buffer source.
  const VtIntArray &vertexCounts = GetCurveVertexCounts();
  int numVerts = 0;
  std::vector<int> indices;
  for (int count : vertexCounts)
  {
    numVerts += count;
  }

  int vi = 0;
  while (vi < numVerts)
  {
    indices.emplace_back(vi++);
  }

  VtIntArray finalIndices(indices.size());
  const VtIntArray &curveIndices = GetCurveIndices();
  if (curveIndices.empty())
  {
    std::copy(indices.begin(), indices.end(), finalIndices.begin());
  }
  else
  {
    // If have topology has indices set, map the generated indices
    // with the given indices.
    int maxIndex = curveIndices.size() - 1;
    for (const int &index : indices)
    {
      finalIndices[index] = std::min(curveIndices[index], maxIndex);
    }
  }

  // Note: The primitive param buffer isn't bound.
  return std::make_shared<HdVtBufferSource>(HdTokens->indices, VtValue(finalIndices));
}

HdBufferSourceSharedPtr HdPh_BasisCurvesTopology::GetIndexBuilderComputation(bool forceLines)
{
  return std::make_shared<HdPh_BasisCurvesIndexBuilderComputation>(this, forceLines);
}

WABI_NAMESPACE_END
