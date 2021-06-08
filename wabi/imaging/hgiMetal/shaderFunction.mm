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
#include "wabi/imaging/hgiMetal/shaderFunction.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/shaderGenerator.h"

#include "wabi/base/arch/defines.h"
#include "wabi/base/tf/diagnostic.h"

#include <unordered_map>

WABI_NAMESPACE_BEGIN

HgiMetalShaderFunction::HgiMetalShaderFunction(HgiMetal *hgi, HgiShaderFunctionDesc const &desc)
    : HgiShaderFunction(desc),
      _shaderId(nil)
{
  if (desc.shaderCode) {
    id<MTLDevice> device = hgi->GetPrimaryDevice();

    HgiMetalShaderGenerator shaderGenerator{desc, device};
    std::stringstream ss;
    shaderGenerator.Execute(ss);
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    options.fastMathEnabled    = YES;
    options.languageVersion    = MTLLanguageVersion2_1;
    options.preprocessorMacros = @{
      @"ARCH_GFX_METAL" : @1,
    };

    NSError *error         = NULL;
    std::string shaderStr  = ss.str();
    id<MTLLibrary> library = [hgi->GetPrimaryDevice() newLibraryWithSource:@(shaderStr.c_str())
                                                                   options:options
                                                                     error:&error];

    NSString *entryPoint = nullptr;
    switch (_descriptor.shaderStage) {
      case HgiShaderStageVertex:
        entryPoint = @"vertexEntryPoint";
        break;
      case HgiShaderStageFragment:
        entryPoint = @"fragmentEntryPoint";
        break;
      case HgiShaderStageCompute:
        entryPoint = @"computeEntryPoint";
        break;
      case HgiShaderStageTessellationControl:
      case HgiShaderStageTessellationEval:
      case HgiShaderStageGeometry:
        TF_CODING_ERROR("Todo: Unsupported shader stage");
        break;
    }

    // Load the function into the library
    _shaderId = [library newFunctionWithName:entryPoint];
    if (!_shaderId) {
      NSString *err = [error localizedDescription];
      TF_WARN("Failed to compile shader: \n%s", [err UTF8String]);
      TF_WARN("%s", shaderStr.c_str());
    }
    else {
      HGIMETAL_DEBUG_LABEL(_shaderId, _descriptor.debugName.c_str());
    }

    [library release];
  }

  _descriptor.shaderCode = nullptr;
}

HgiMetalShaderFunction::~HgiMetalShaderFunction()
{
  [_shaderId release];
  _shaderId = nil;
}

bool HgiMetalShaderFunction::IsValid() const
{
  return _errors.empty();
}

std::string const &HgiMetalShaderFunction::GetCompileErrors()
{
  return _errors;
}

size_t HgiMetalShaderFunction::GetByteSizeOfResource() const
{
  return 0;
}

uint64_t HgiMetalShaderFunction::GetRawResource() const
{
  return (uint64_t)_shaderId;
}

id<MTLFunction> HgiMetalShaderFunction::GetShaderId() const
{
  return _shaderId;
}

WABI_NAMESPACE_END
