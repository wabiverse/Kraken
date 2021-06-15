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

#ifndef WABI_IMAGING_HGIMETAL_SHADERGENERATOR_H
#define WABI_IMAGING_HGIMETAL_SHADERGENERATOR_H

#include "wabi/base/arch/defines.h"
#include "wabi/base/tf/diagnostic.h"
#include "wabi/imaging/hgi/shaderGenerator.h"
#include "wabi/imaging/hgiMetal/conversions.h"
#include "wabi/imaging/hgiMetal/hgi.h"
#include "wabi/imaging/hgiMetal/shaderSection.h"

WABI_NAMESPACE_BEGIN

// Shader program structure

// Global scope includes
// Global scope macros
// Global scope struct defs
// Global scope member decls
// Global scope function defs
// Stage program scope struct defs
// Stage program scope member decls
// Stage program scope function defs
// Stage program scope main program
// Stage function entryPoint definition
// Stage function implementation

using HgiMetalShaderStageEntryPointUniquePtr = std::unique_ptr<class HgiMetalShaderStageEntryPoint>;

/// \class HgiMetalShaderGenerator
///
/// Takes in a descriptor and spits out metal code through it's Execute function
///

class HgiMetalShaderGenerator final : public HgiShaderGenerator {
 public:
  HGIMETAL_API
  HgiMetalShaderGenerator(const HgiShaderFunctionDesc &descriptor, id<MTLDevice> device);

  HGIMETAL_API
  ~HgiMetalShaderGenerator() override;

  HGIMETAL_API
  HgiMetalShaderSectionUniquePtrVector *GetShaderSections();

  template<typename SectionType, typename... T> SectionType *CreateShaderSection(T &&...t);

 protected:
  HGIMETAL_API
  void _Execute(std::ostream &ss, const std::string &originalShaderShader) override;

 private:
  HgiMetalShaderStageEntryPointUniquePtr _BuildShaderStageEntryPoints(
    const HgiShaderFunctionDesc &descriptor);

  void _BuildTextureShaderSections(const HgiShaderFunctionDesc &descriptor);
  void _BuildBufferShaderSections(const HgiShaderFunctionDesc &descriptor);
  void _BuildKeywordInputShaderSections(const HgiShaderFunctionDesc &descriptor);

  HgiMetalShaderSectionUniquePtrVector _shaderSections;
  HgiMetalShaderStageEntryPointUniquePtr _generatorShaderSections;
};

WABI_NAMESPACE_END

#endif
