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
#ifndef WABI_IMAGING_HD_PH_BASIS_CURVES_COMPUTATIONS_H
#define WABI_IMAGING_HD_PH_BASIS_CURVES_COMPUTATIONS_H

#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/sceneDelegate.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hd/vtBufferSource.h"
#include "wabi/imaging/hdPh/basisCurvesTopology.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/vec3f.h"
#include "wabi/base/vt/array.h"
#include "wabi/base/vt/value.h"

WABI_NAMESPACE_BEGIN

/// \class HdPh_BasisCurvesIndexBuilderComputation
///
/// Compute basis curves indices as a computation on CPU.
///
class HdPh_BasisCurvesIndexBuilderComputation : public HdComputedBufferSource {
 public:
  HdPh_BasisCurvesIndexBuilderComputation(HdBasisCurvesTopology *topology, bool forceLines);
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  virtual bool Resolve() override;

  virtual bool HasChainedBuffer() const override;
  virtual HdBufferSourceSharedPtrVector GetChainedBuffers() const override;

 protected:
  virtual bool _CheckValid() const override;

 public:
  // For building index and primitive index arrays
  struct IndexAndPrimIndex {
    // default constructor results in empty VtValue's
    IndexAndPrimIndex()
    {}

    IndexAndPrimIndex(VtValue indices, VtValue primIndices)
        : _indices(indices),
          _primIndices(primIndices)
    {}

    VtValue _indices;
    VtValue _primIndices;
  };

 private:
  IndexAndPrimIndex _BuildLinesIndexArray();
  IndexAndPrimIndex _BuildLineSegmentIndexArray();
  IndexAndPrimIndex _BuildCubicIndexArray();

  HdBasisCurvesTopology *_topology;
  bool _forceLines;

  HdBufferSourceSharedPtr _primitiveParam;
};

template<typename T>
VtArray<T> HdPh_ExpandVarying(size_t numVerts,
                              VtIntArray const &vertexCounts,
                              const TfToken &wrap,
                              const TfToken &basis,
                              VtArray<T> const &authoredValues)
{
  VtArray<T> outputValues(numVerts);

  size_t srcIndex = 0;
  size_t dstIndex = 0;

  if (wrap == HdTokens->periodic) {
    // XXX : Add support for periodic curves
    TF_WARN("Varying data is only supported for non-periodic curves.");
  }

  for (const int nVerts : vertexCounts) {
    // Handling for the case of potentially incorrect vertex counts
    if (nVerts < 1) {
      continue;
    }

    if (basis == HdTokens->catmullRom || basis == HdTokens->bSpline) {
      // For splines with a vstep of 1, we are doing linear interpolation
      // between segments, so all we do here is duplicate the first and
      // last outputValues. Since these are never acutally used during
      // drawing, it would also work just to set the to 0.
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;
      for (int i = 1; i < nVerts - 2; ++i) {
        outputValues[dstIndex] = authoredValues[srcIndex];
        ++dstIndex;
        ++srcIndex;
      }
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;
      ++srcIndex;
    }
    else if (basis == HdTokens->bezier) {
      // For bezier splines, we map the linear values to cubic values
      // the begin value gets mapped to the first two vertices and
      // the end value gets mapped to the last two vertices in a segment.
      // shaders can choose to access value[1] and value[2] when linearly
      // interpolating a value, which happens to match up with the
      // indexing to use for catmullRom and bSpline basis.
      const int vStep        = 3;
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;  // don't increment the srcIndex
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;
      ++srcIndex;

      // vstep - 1 control points will have an interpolated value
      for (int i = 2; i < nVerts - 2; i += vStep) {
        outputValues[dstIndex] = authoredValues[srcIndex];
        ++dstIndex;  // don't increment the srcIndex
        outputValues[dstIndex] = authoredValues[srcIndex];
        ++dstIndex;  // don't increment the srcIndex
        outputValues[dstIndex] = authoredValues[srcIndex];
        ++dstIndex;
        ++srcIndex;
      }
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;  // don't increment the srcIndex
      outputValues[dstIndex] = authoredValues[srcIndex];
      ++dstIndex;
      ++srcIndex;
    }
    else {
      TF_WARN("Unsupported basis: '%s'", basis.GetText());
    }
  }
  TF_VERIFY(srcIndex == authoredValues.size());
  TF_VERIFY(dstIndex == numVerts);

  return outputValues;
}

/// Verify the number of authored vertex or varying primvars, expanding the
/// number of varying values when necessary
template<typename T>
class HdPh_BasisCurvesPrimvarInterpolaterComputation : public HdComputedBufferSource {
 public:
  HdPh_BasisCurvesPrimvarInterpolaterComputation(HdPh_BasisCurvesTopologySharedPtr topology,
                                                 const VtArray<T> &authoredPrimvar,
                                                 const TfToken &name,
                                                 HdInterpolation interpolation,
                                                 const T fallbackValue,
                                                 HdType hdType)
      : _topology(topology),
        _authoredPrimvar(authoredPrimvar),
        _name(name),
        _interpolation(interpolation),
        _fallbackValue(fallbackValue),
        _hdType(hdType)
  {}

  virtual bool Resolve() override
  {
    if (!_TryLock())
      return false;

    HD_TRACE_FUNCTION();

    // We need to verify the number of primvars depending on the primvar
    // interpolation type
    const size_t numVerts = _topology->CalculateNeededNumberOfControlPoints();

    VtArray<T> primvars(numVerts);
    const size_t size = _authoredPrimvar.size();

    // Special handling for when points is size 0
    if (size == 0 && _name == HdTokens->points) {
      primvars = _authoredPrimvar;
    }
    else if (_interpolation == HdInterpolationVertex) {
      if (size == 1) {
        for (size_t i = 0; i < numVerts; i++) {
          primvars[i] = _authoredPrimvar[0];
        }
      }
      else if (size == numVerts) {
        primvars = _authoredPrimvar;
      }
      else if (size < numVerts && _topology->HasIndices()) {
        for (size_t i = 0; i < size; ++i) {
          primvars[i] = _authoredPrimvar[i];
        }
      }
      else {
        for (size_t i = 0; i < numVerts; ++i) {
          primvars[i] = _fallbackValue;
        }
        TF_WARN(
            "Incorrect number of primvar %s for vertex "
            "interpolation, using fallback value for rendering",
            _name.GetText());
      }
    }
    else if (_interpolation == HdInterpolationVarying) {
      if (size == 1) {
        for (size_t i = 0; i < numVerts; i++) {
          primvars[i] = _authoredPrimvar[0];
        }
      }
      else if (_topology->GetCurveType() == HdTokens->linear && size == numVerts) {
        primvars = _authoredPrimvar;
      }
      else if (size == _topology->CalculateNeededNumberOfVaryingControlPoints()) {
        primvars = HdPh_ExpandVarying<T>(numVerts,
                                         _topology->GetCurveVertexCounts(),
                                         _topology->GetCurveWrap(),
                                         _topology->GetCurveBasis(),
                                         _authoredPrimvar);
      }
      else {
        for (size_t i = 0; i < numVerts; ++i) {
          primvars[i] = _fallbackValue;
        }
        TF_WARN(
            "Incorrect number of primvar %s for varying "
            "interpolation, using fallback value for rendering",
            _name.GetText());
      }
    }

    _SetResult(
        HdBufferSourceSharedPtr(std::make_shared<HdVtBufferSource>(_name, VtValue(primvars))));

    _SetResolved();
    return true;
  }

  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override
  {
    specs->emplace_back(_name, HdTupleType{_hdType, 1});
  }

 protected:
  virtual bool _CheckValid() const override
  {
    return true;
  }

 private:
  HdPh_BasisCurvesTopologySharedPtr _topology;
  VtArray<T> _authoredPrimvar;
  TfToken _name;
  HdInterpolation _interpolation;
  T _fallbackValue;
  HdType _hdType;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_BASIS_CURVES_COMPUTATIONS_H
