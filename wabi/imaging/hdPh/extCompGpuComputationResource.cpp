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
#include "wabi/imaging/hdPh/extCompGpuComputationResource.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/codeGen.h"
#include "wabi/imaging/hdPh/glslProgram.h"

WABI_NAMESPACE_BEGIN

static size_t _Hash(HdBufferSpecVector const &specs)
{
  size_t result = 0;
  for (HdBufferSpec const &spec : specs) {
    boost::hash_combine(result, spec.Hash());
  }
  return result;
}

HdPhExtCompGpuComputationResource::HdPhExtCompGpuComputationResource(
  HdBufferSpecVector const &outputBufferSpecs,
  HdPh_ExtCompComputeShaderSharedPtr const &kernel,
  HdBufferArrayRangeSharedPtrVector const &inputs,
  HdPhResourceRegistrySharedPtr const &registry)
  : _outputBufferSpecs(outputBufferSpecs),
    _kernel(kernel),
    _registry(registry),
    _shaderSourceHash(),
    _inputs(inputs),
    _computeProgram(),
    _resourceBinder()
{}

bool HdPhExtCompGpuComputationResource::Resolve()
{
  // Non-in-place sources should have been registered as resource registry
  // sources already and Resolved. They go to an internal buffer range that
  // was allocated in AllocateInternalRange
  HdBufferSpecVector inputBufferSpecs;
  for (HdBufferArrayRangeSharedPtr const &input : _inputs) {
    if (TF_VERIFY(input)) {
      input->GetBufferSpecs(&inputBufferSpecs);
    }
  }
  // Once we know the names and sizes of all outputs and inputs and the kernel
  // to use we can codeGen the compute shader to use.

  // We can shortcut the codegen by using a heuristic for determining that
  // the output source would be identical given a certain destination buffer
  // range.
  size_t shaderSourceHash = 0;
  boost::hash_combine(shaderSourceHash, _kernel->ComputeHash());
  boost::hash_combine(shaderSourceHash, _Hash(_outputBufferSpecs));
  boost::hash_combine(shaderSourceHash, _Hash(inputBufferSpecs));

  // XXX we'll need to test for hash collisions as they could be fatal in the
  // case of shader sources. Adjust based on pref vs correctness needs.
  // The new specs and the old specs as well as the new vs old kernel
  // source should be compared for equality if that is the case.
  // if (_shaderSourceHash == shaderSourceHash) {
  //    -- if hash equal but not content equal resolve hash collision --
  //}

  // If the source hash mismatches the saved program from previous executions
  // we are going to have to recompile it here.
  // We save the kernel for future runs to not have to incur the
  // compilation cost each time.
  if (!_computeProgram || _shaderSourceHash != shaderSourceHash) {
    HdPhShaderCodeSharedPtrVector shaders;
    shaders.push_back(_kernel);
    HdPh_CodeGen codeGen(shaders);

    // let resourcebinder resolve bindings and populate metadata
    // which is owned by codegen.
    _resourceBinder.ResolveComputeBindings(_outputBufferSpecs,
                                           inputBufferSpecs,
                                           shaders,
                                           codeGen.GetMetaData());

    HdPhGLSLProgram::ID registryID = codeGen.ComputeHash();

    {
      // ask registry to see if there's already compiled program
      HdInstance<HdPhGLSLProgramSharedPtr> programInstance = _registry->RegisterGLSLProgram(
        registryID);

      if (programInstance.IsFirstInstance()) {
        HdPhGLSLProgramSharedPtr glslProgram = codeGen.CompileComputeProgram(_registry.get());
        if (!TF_VERIFY(glslProgram)) {
          return false;
        }

        if (!glslProgram->Link()) {
          std::string const &logString = glslProgram->GetProgram()->GetCompileErrors();
          TF_WARN("Failed to link compute shader: %s", logString.c_str());
          return false;
        }

        // store the program into the program registry.
        programInstance.SetValue(glslProgram);

        TF_DEBUG(HD_EXT_COMPUTATION_UPDATED)
          .Msg("Compiled and linked compute program for computation %s\n ",
               _kernel->GetExtComputationId().GetText());
      }

      _computeProgram = programInstance.GetValue();
    }

    if (!TF_VERIFY(_computeProgram)) {
      return false;
    }

    _shaderSourceHash = shaderSourceHash;
  }
  return true;
}

WABI_NAMESPACE_END
