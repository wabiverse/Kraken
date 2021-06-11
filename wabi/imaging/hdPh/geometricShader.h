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
#ifndef WABI_IMAGING_HD_ST_GEOMETRIC_SHADER_H
#define WABI_IMAGING_HD_ST_GEOMETRIC_SHADER_H

#include "wabi/imaging/garch/glApi.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/shaderCode.h"
#include "wabi/imaging/hio/glslfx.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

using HdPh_GeometricShaderSharedPtr = std::shared_ptr<class HdPh_GeometricShader>;
struct HdPh_ShaderKey;

/// \class HdPh_GeometricShader
///
/// Phoenix breaks down the concept of a shader program into distinct
/// conceptual pieces that are then stitched together during code generation.
/// The pieces are:
/// (i)   geometric shader
/// (ii)  material shader
/// (iii) lighting shader
/// (iv)  render pass shader
///
/// The geometric shader contains the entry points for the relevant shader
/// stages and uses geometry opinions (such as cullstyle, double sided, etc)
/// to generate shader code variants via mixins.
///
class HdPh_GeometricShader : public HdPhShaderCode {
 public:
  /// Used in HdPh_CodeGen to generate the appropriate shader source
  enum class PrimitiveType {
    PRIM_POINTS,
    PRIM_BASIS_CURVES_LINES,           // when linear (or) non-refined cubic
    PRIM_BASIS_CURVES_LINEAR_PATCHES,  // refined linear curves
    PRIM_BASIS_CURVES_CUBIC_PATCHES,   // refined cubic curves
    PRIM_MESH_COARSE_TRIANGLES,
    PRIM_MESH_REFINED_TRIANGLES,  // e.g: loop subdiv
    PRIM_MESH_COARSE_QUADS,       // e.g: quadrangulation for ptex
    PRIM_MESH_REFINED_QUADS,      // e.g: catmark/bilinear subdiv
    PRIM_MESH_BSPLINE,            // e.g. catmark limit surface patches
    PRIM_MESH_BOXSPLINETRIANGLE,  // e.g. loop limit surface patches
    PRIM_VOLUME                   // Simply draws triangles of bounding
                                  // box of a volume.
  };

  /// static query functions for PrimitiveType
  static inline bool IsPrimTypePoints(PrimitiveType primType)
  {
    return primType == PrimitiveType::PRIM_POINTS;
  }

  static inline bool IsPrimTypeBasisCurves(PrimitiveType primType)
  {
    return (primType == PrimitiveType::PRIM_BASIS_CURVES_LINES ||
            primType == PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES ||
            primType == PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES);
  }

  static inline bool IsPrimTypeMesh(PrimitiveType primType)
  {
    return (primType == PrimitiveType::PRIM_MESH_COARSE_TRIANGLES ||
            primType == PrimitiveType::PRIM_MESH_REFINED_TRIANGLES ||
            primType == PrimitiveType::PRIM_MESH_COARSE_QUADS ||
            primType == PrimitiveType::PRIM_MESH_REFINED_QUADS ||
            primType == PrimitiveType::PRIM_MESH_BSPLINE ||
            primType == PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE);
  }

  static inline bool IsPrimTypeTriangles(PrimitiveType primType)
  {
    return (primType == PrimitiveType::PRIM_MESH_COARSE_TRIANGLES ||
            primType == PrimitiveType::PRIM_MESH_REFINED_TRIANGLES ||
            primType == PrimitiveType::PRIM_VOLUME);
  }

  static inline bool IsPrimTypeQuads(PrimitiveType primType)
  {
    return (primType == PrimitiveType::PRIM_MESH_COARSE_QUADS ||
            primType == PrimitiveType::PRIM_MESH_REFINED_QUADS);
  }

  static inline bool IsPrimTypeRefinedMesh(PrimitiveType primType)
  {
    return (primType == PrimitiveType::PRIM_MESH_REFINED_TRIANGLES ||
            primType == PrimitiveType::PRIM_MESH_REFINED_QUADS ||
            primType == PrimitiveType::PRIM_MESH_BSPLINE ||
            primType == PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE);
  }

  static inline bool IsPrimTypePatches(PrimitiveType primType)
  {
    return primType == PrimitiveType::PRIM_MESH_BSPLINE ||
           primType == PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE ||
           primType == PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES ||
           primType == PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES;
  }

  // Face-varying patch type
  enum class FvarPatchType {
    PATCH_COARSE_TRIANGLES,
    PATCH_REFINED_TRIANGLES,
    PATCH_COARSE_QUADS,
    PATCH_REFINED_QUADS,
    PATCH_BSPLINE,
    PATCH_BOXSPLINETRIANGLE,
    PATCH_NONE
  };

  HDPH_API
  HdPh_GeometricShader(std::string const &glslfxString,
                       PrimitiveType primType,
                       HdCullStyle cullStyle,
                       bool useHardwareFaceCulling,
                       bool hasMirroredTransform,
                       bool doubleSided,
                       HdPolygonMode polygonMode,
                       bool cullingPass,
                       FvarPatchType fvarPatchType,
                       SdfPath const &debugId = SdfPath(),
                       float lineWidth        = 0);

  HDPH_API
  ~HdPh_GeometricShader() override;

  // HdShader overrides
  HDPH_API
  ID ComputeHash() const override;
  HDPH_API
  std::string GetSource(TfToken const &shaderStageKey) const override;
  HDPH_API
  void BindResources(int program,
                     HdPh_ResourceBinder const &binder,
                     HdRenderPassState const &state) override;

  HDPH_API
  void UnbindResources(int program,
                       HdPh_ResourceBinder const &binder,
                       HdRenderPassState const &state) override;
  HDPH_API
  void AddBindings(HdBindingRequestVector *customBindings) override;

  /// Returns true if this geometric shader is used for GPU frustum culling.
  bool IsFrustumCullingPass() const
  {
    return _frustumCullingPass;
  }

  PrimitiveType GetPrimitiveType() const
  {
    return _primType;
  }

  /// member query functions for PrimitiveType
  inline bool IsPrimTypePoints() const
  {
    return IsPrimTypePoints(_primType);
  }

  inline bool IsPrimTypeBasisCurves() const
  {
    return IsPrimTypeBasisCurves(_primType);
  }

  inline bool IsPrimTypeMesh() const
  {
    return IsPrimTypeMesh(_primType);
  }

  inline bool IsPrimTypeTriangles() const
  {
    return IsPrimTypeTriangles(_primType);
  }

  inline bool IsPrimTypeQuads() const
  {
    return IsPrimTypeQuads(_primType);
  }

  inline bool IsPrimTypePatches() const
  {
    return IsPrimTypePatches(_primType);
  }

  FvarPatchType GetFvarPatchType() const
  {
    return _fvarPatchType;
  }

  /// Return the GL primitive type of the draw item based on _primType
  HDPH_API
  GLenum GetPrimitiveMode() const;

  // Returns the primitive index size based on the primitive mode
  // 3 for triangles, 4 for quads, 16 for regular b-spline patches etc.
  HDPH_API
  int GetPrimitiveIndexSize() const;

  // Returns the number of vertices output for patch evaluation,
  // i.e. the number of tessellation control shader invocations.
  HDPH_API
  int GetNumPatchEvalVerts() const;

  // Returns the primitive index size for the geometry shader shade
  // 1 for points, 2 for lines, 3 for triangles, 4 for lines_adjacency
  HDPH_API
  int GetNumPrimitiveVertsForGeometryShader() const;

  // Factory for convenience.
  static HdPh_GeometricShaderSharedPtr Create(
      HdPh_ShaderKey const &shaderKey,
      HdPhResourceRegistrySharedPtr const &resourceRegistry);

 private:
  PrimitiveType _primType;
  HdCullStyle _cullStyle;
  bool _useHardwareFaceCulling;
  bool _hasMirroredTransform;
  bool _doubleSided;
  HdPolygonMode _polygonMode;
  float _lineWidth;

  std::unique_ptr<HioGlslfx> _glslfx;
  bool _frustumCullingPass;
  FvarPatchType _fvarPatchType;
  ID _hash;

  // No copying
  HdPh_GeometricShader(const HdPh_GeometricShader &) = delete;
  HdPh_GeometricShader &operator=(const HdPh_GeometricShader &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_GEOMETRIC_SHADER_H
