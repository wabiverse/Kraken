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
#ifndef WABI_IMAGING_HD_ST_CODE_GEN_H
#define WABI_IMAGING_HD_ST_CODE_GEN_H

/* clang-format off */
#include "wabi/wabi.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/glslProgram.h"
/* clang-format on */

#include <map>
#include <sstream>
#include <vector>

WABI_NAMESPACE_BEGIN

using HdPhShaderCodeSharedPtr = std::shared_ptr<class HdPhShaderCode>;
using HdPhShaderCodeSharedPtrVector = std::vector<HdPhShaderCodeSharedPtr>;
using HdPh_GeometricShaderPtr = std::shared_ptr<class HdPh_GeometricShader>;

/// \class HdPh_CodeGen
///
/// A utility class to compose glsl shader sources and compile them
/// upon request of HdShaderSpec.
///
class HdPh_CodeGen
{
 public:
  typedef size_t ID;

  /// Constructor.
  HDPH_API
  HdPh_CodeGen(HdPh_GeometricShaderPtr const &geometricShader,
               HdPhShaderCodeSharedPtrVector const &shaders,
               TfToken const &materialTag);

  /// Constructor for non-geometric use cases.
  /// Don't call compile when constructed this way.
  /// Call CompileComputeProgram instead.
  HDPH_API
  HdPh_CodeGen(HdPhShaderCodeSharedPtrVector const &shaders);

  /// Return the hash value of glsl shader to be generated.
  HDPH_API
  ID ComputeHash() const;

  /// Generate shader source and compile it.
  HDPH_API
  HdPhGLSLProgramSharedPtr Compile(HdPhResourceRegistry *const registry);

  /// Generate compute shader source and compile it.
  /// It uses the compute information in the meta data to determine
  /// layouts needed for a compute program.
  /// The caller should have populated the meta data before calling this
  /// using a method like HdPh_ResourceBinder::ResolveBindings.
  ///
  /// The layout and binding information is combined with the compute stage
  /// shader code from the shader vector to form a resolved shader for
  /// compilation.
  ///
  /// The generated code that is compiled is available for diagnostic
  /// purposes from GetComputeShaderSource.
  ///
  /// \see GetComputeShaderSource
  /// \see HdPh_ResourceBinder::ResolveBindings
  HDPH_API
  HdPhGLSLProgramSharedPtr CompileComputeProgram(HdPhResourceRegistry *const registry);

  /// Return the generated vertex shader source
  const std::string &GetVertexShaderSource() const
  {
    return _vsSource;
  }

  /// Return the generated tess control shader source
  const std::string &GetTessControlShaderSource() const
  {
    return _tcsSource;
  }

  /// Return the generated tess eval shader source
  const std::string &GetTessEvalShaderSource() const
  {
    return _tesSource;
  }

  /// Return the generated geometry shader source
  const std::string &GetGeometryShaderSource() const
  {
    return _gsSource;
  }

  /// Return the generated fragment shader source
  const std::string &GetFragmentShaderSource() const
  {
    return _fsSource;
  }

  /// Return the generated compute shader source
  const std::string &GetComputeShaderSource() const
  {
    return _csSource;
  }

  /// Return the pointer of metadata to be populated by resource binder.
  HdPh_ResourceBinder::MetaData *GetMetaData()
  {
    return &_metaData;
  }

 private:
  void _GenerateDrawingCoord();
  void _GenerateConstantPrimvar();
  void _GenerateInstancePrimvar();
  void _GenerateElementPrimvar();
  void _GenerateVertexAndFaceVaryingPrimvar(bool hasGS);
  void _GenerateShaderParameters();
  void _GenerateTopologyVisibilityParameters();

  HdPh_ResourceBinder::MetaData _metaData;
  HdPh_GeometricShaderPtr _geometricShader;
  HdPhShaderCodeSharedPtrVector _shaders;
  TfToken _materialTag;

  // source buckets
  std::stringstream _genCommon, _genVS, _genTCS, _genTES;
  std::stringstream _genGS, _genFS, _genCS;
  std::stringstream _procVS, _procTCS, _procTES, _procGS;

  // generated sources (for diagnostics)
  std::string _vsSource;
  std::string _tcsSource;
  std::string _tesSource;
  std::string _gsSource;
  std::string _fsSource;
  std::string _csSource;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_CODE_GEN_H
