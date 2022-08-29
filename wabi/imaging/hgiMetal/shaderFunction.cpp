//
// Copyright 2020 Pixar
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
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/diagnostic.h"
#include "wabi/imaging/hgiMetal/shaderFunction.h"
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
    MTL::Device *device = hgi->GetPrimaryDevice();

    HgiMetalShaderGenerator shaderGenerator{desc, device};
    shaderGenerator.Execute();
    const char *shaderCode = shaderGenerator.GetGeneratedShaderCode();

    MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init();
    options->setFastMathEnabled(YES);

#ifdef ARCH_OS_MACOS
    if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
          NS::OperatingSystemVersion(10, 15))) {
#elif defined(ARCH_OS_IOS)
    if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
          NS::OperatingSystemVersion(13, 0))) {
#else  /* ARCH_OS_IOS */
    if (false) {
#endif /* ARCH_OS_MACOS */
      options->setLanguageVersion(MTL::LanguageVersion2_2);
    } else {
      options->setLanguageVersion(MTL::LanguageVersion2_1);
    }

    options->setPreprocessorMacros(
      NS::Dictionary::dictionary(NS::String::string("ARCH_GFX_METAL", NS::UTF8StringEncoding),
                                 NS::String::string("1", NS::UTF8StringEncoding)));

    NS::Error *error = NULL;
    MTL::Library *library = hgi->GetPrimaryDevice()->newLibrary(
      NS::String::string(shaderCode, NS::UTF8StringEncoding),
      options,
      &error);

    NS::String *entryPoint = nullptr;
    switch (_descriptor.shaderStage) {
      case HgiShaderStageVertex:
        entryPoint = NS::String::string("vertexEntryPoint", NS::UTF8StringEncoding);
        break;
      case HgiShaderStageFragment:
        entryPoint = NS::String::string("fragmentEntryPoint", NS::UTF8StringEncoding);
        break;
      case HgiShaderStageCompute:
        entryPoint = NS::String::string("computeEntryPoint", NS::UTF8StringEncoding);
        break;
      case HgiShaderStagePostTessellationVertex:
        entryPoint = NS::String::string("vertexEntryPoint", NS::UTF8StringEncoding);
        break;
      case HgiShaderStageTessellationControl:
      case HgiShaderStageTessellationEval:
      case HgiShaderStageGeometry:
        TF_CODING_ERROR("Todo: Unsupported shader stage");
        break;
    }

    // Load the function into the library
    _shaderId = library->newFunction(entryPoint);
    if (!_shaderId) {
      NS::String *err = error->localizedDescription();
      _errors = err->utf8String();
    } else {
      HGIMETAL_DEBUG_LABEL(_shaderId, _descriptor.debugName.c_str());
    }

    library->release();
  }

  // Clear these pointers in our copy of the descriptor since we
  // have to assume they could become invalid after we return.
  _descriptor.shaderCodeDeclarations = nullptr;
  _descriptor.shaderCode = nullptr;
  _descriptor.generatedShaderCodeOut = nullptr;
}

HgiMetalShaderFunction::~HgiMetalShaderFunction()
{
  _shaderId->release();
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

MTL::Function *HgiMetalShaderFunction::GetShaderId() const
{
  return _shaderId;
}

WABI_NAMESPACE_END
