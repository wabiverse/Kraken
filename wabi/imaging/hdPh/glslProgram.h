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
#ifndef WABI_IMAGING_HD_ST_GLSL_PROGRAM_H
#define WABI_IMAGING_HD_ST_GLSL_PROGRAM_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hgi/buffer.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/imaging/hgi/shaderProgram.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
using HdPhGLSLProgramSharedPtr = std::shared_ptr<class HdPhGLSLProgram>;

using HgiShaderProgramHandle = HgiHandle<class HgiShaderProgram>;

/// \class HdPhGLSLProgram
///
/// An instance of a glsl program.
///
class HdPhGLSLProgram final
{
 public:
  typedef size_t ID;

  HDPH_API
  HdPhGLSLProgram(TfToken const &role, HdPhResourceRegistry *const registry);
  HDPH_API
  ~HdPhGLSLProgram();

  /// Returns the hash value of the program for \a sourceFile
  HDPH_API
  static ID ComputeHash(TfToken const &sourceFile);

  /// Compile shader source for a shader stage.
  HDPH_API
  bool CompileShader(HgiShaderStage stage, std::string const &source);

  /// Link the compiled shaders together.
  HDPH_API
  bool Link();

  /// Validate if this program is a valid progam in the current context.
  HDPH_API
  bool Validate() const;

  /// Returns HdResource of the program object.
  HgiShaderProgramHandle const &GetProgram() const
  {
    return _program;
  }

  /// Convenience method to get a shared compute shader program
  HDPH_API
  static HdPhGLSLProgramSharedPtr GetComputeProgram(TfToken const &shaderToken,
                                                    HdPhResourceRegistry *resourceRegistry);

  HDPH_API
  static HdPhGLSLProgramSharedPtr GetComputeProgram(TfToken const &shaderFileName,
                                                    TfToken const &shaderToken,
                                                    HdPhResourceRegistry *resourceRegistry);

  using PopulateDescriptorCallback = std::function<void(HgiShaderFunctionDesc &computeDesc)>;

  HDPH_API
  static HdPhGLSLProgramSharedPtr GetComputeProgram(TfToken const &shaderToken,
                                                    HdPhResourceRegistry *resourceRegistry,
                                                    PopulateDescriptorCallback populateDescriptor);

  /// Returns the role of the GPU data in this resource.
  TfToken const &GetRole() const
  {
    return _role;
  }

 private:
  HdPhResourceRegistry *const _registry;
  TfToken _role;

  HgiShaderProgramDesc _programDesc;
  HgiShaderProgramHandle _program;

  // An identifier for uniquely identifying the program, for debugging
  // purposes - programs that fail to compile for one reason or another
  // will get deleted, and their GL program IDs reused, so we can't use
  // that to identify it uniquely
  size_t _debugID;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_GLSL_PROGRAM_H
