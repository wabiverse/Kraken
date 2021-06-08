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
#ifndef WABI_IMAGING_HD_FLAT_NORMALS_H
#define WABI_IMAGING_HD_FLAT_NORMALS_H

#include "wabi/imaging/hd/api.h"
#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/computation.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"
#include "wabi/base/tf/token.h"

WABI_NAMESPACE_BEGIN

class HdMeshTopology;

/// \class Hd_FlatNormals
///
/// Hd_FlatNormals encapsulates mesh flat normals information.
/// It uses passed-in face index data and points data to compute
/// flat per-face normals.  It does this by breaking each face into
/// a triangle fan centered at vertex 0, and averaging triangle normals.
///
class Hd_FlatNormals final {
 public:
  /// Computes the flat normals result using the supplied face coord
  /// information and points data. Returns an array of the same size and
  /// type as the source points, with optional packing.
  HD_API
  static VtArray<GfVec3f> ComputeFlatNormals(HdMeshTopology const *topology,
                                             GfVec3f const *pointsPtr);
  HD_API
  static VtArray<GfVec3d> ComputeFlatNormals(HdMeshTopology const *topology,
                                             GfVec3d const *pointsPtr);
  HD_API
  static VtArray<HdVec4f_2_10_10_10_REV> ComputeFlatNormalsPacked(HdMeshTopology const *topology,
                                                                  GfVec3f const *pointsPtr);
  HD_API
  static VtArray<HdVec4f_2_10_10_10_REV> ComputeFlatNormalsPacked(HdMeshTopology const *topology,
                                                                  GfVec3d const *pointsPtr);

 private:
  Hd_FlatNormals()  = delete;
  ~Hd_FlatNormals() = delete;
};

/// \class Hd_FlatNormalsComputation
///
/// Flat normal computation CPU.
///
class Hd_FlatNormalsComputation : public HdComputedBufferSource {
 public:
  HD_API
  Hd_FlatNormalsComputation(HdMeshTopology const *topology,
                            HdBufferSourceSharedPtr const &points,
                            TfToken const &dstName,
                            bool packed);

  /// overrides
  HD_API
  virtual void GetBufferSpecs(HdBufferSpecVector *specs) const override;
  HD_API
  virtual bool Resolve() override;
  HD_API
  virtual TfToken const &GetName() const override;

 protected:
  HD_API
  virtual bool _CheckValid() const override;

 private:
  HdMeshTopology const *_topology;
  HdBufferSourceSharedPtr const _points;
  TfToken _dstName;
  bool _packed;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_FLAT_NORMALS_H
