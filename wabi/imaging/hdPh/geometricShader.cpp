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
#include "wabi/imaging/garch/glApi.h"

#include "wabi/imaging/hdPh/geometricShader.h"

#include "wabi/imaging/hdPh/debugCodes.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/shaderKey.h"

#include "wabi/imaging/hd/binding.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/renderPassState.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/hio/glslfx.h"

#include <boost/functional/hash.hpp>

#include <iostream>
#include <string>

WABI_NAMESPACE_BEGIN

HdPh_GeometricShader::HdPh_GeometricShader(std::string const &glslfxString,
                                           PrimitiveType primType,
                                           HdCullStyle cullStyle,
                                           bool useHardwareFaceCulling,
                                           bool hasMirroredTransform,
                                           bool doubleSided,
                                           HdPolygonMode polygonMode,
                                           bool cullingPass,
                                           FvarPatchType fvarPatchType,
                                           SdfPath const &debugId,
                                           float lineWidth)
  : HdPhShaderCode(),
    _primType(primType),
    _cullStyle(cullStyle),
    _useHardwareFaceCulling(useHardwareFaceCulling),
    _hasMirroredTransform(hasMirroredTransform),
    _doubleSided(doubleSided),
    _polygonMode(polygonMode),
    _lineWidth(lineWidth),
    _frustumCullingPass(cullingPass),
    _fvarPatchType(fvarPatchType),
    _hash(0)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  // XXX
  // we will likely move this (the constructor or the entire class) into
  // the base class (HdPhShaderCode) at the end of refactoring, to be able to
  // use same machinery other than geometric shaders.

  if (TfDebug::IsEnabled(HDPH_DUMP_GLSLFX_CONFIG))
  {
    std::cout << debugId << "\n"
              << glslfxString << "\n";
  }

  std::stringstream ss(glslfxString);
  _glslfx.reset(new HioGlslfx(ss));
  boost::hash_combine(_hash, _glslfx->GetHash());
  boost::hash_combine(_hash, cullingPass);
  boost::hash_combine(_hash, primType);
  boost::hash_combine(_hash, fvarPatchType);
  //
  // note: Don't include cullStyle and polygonMode into the hash.
  //      They are independent from the GLSL program.
  //
}

HdPh_GeometricShader::~HdPh_GeometricShader()
{
  // nothing
}

/* virtual */
HdPhShaderCode::ID HdPh_GeometricShader::ComputeHash() const
{
  return _hash;
}

/* virtual */
std::string HdPh_GeometricShader::GetSource(TfToken const &shaderStageKey) const
{
  return _glslfx->GetSource(shaderStageKey);
}

void HdPh_GeometricShader::BindResources(const int program,
                                         HdPh_ResourceBinder const &binder,
                                         HdRenderPassState const &state)
{
  if (_useHardwareFaceCulling)
  {
    switch (_cullStyle)
    {
      case HdCullStyleFront:
        glEnable(GL_CULL_FACE);
        if (_hasMirroredTransform)
        {
          glCullFace(GL_BACK);
        }
        else
        {
          glCullFace(GL_FRONT);
        }
        break;
      case HdCullStyleFrontUnlessDoubleSided:
        if (!_doubleSided)
        {
          glEnable(GL_CULL_FACE);
          if (_hasMirroredTransform)
          {
            glCullFace(GL_BACK);
          }
          else
          {
            glCullFace(GL_FRONT);
          }
        }
        break;
      case HdCullStyleBack:
        glEnable(GL_CULL_FACE);
        if (_hasMirroredTransform)
        {
          glCullFace(GL_FRONT);
        }
        else
        {
          glCullFace(GL_BACK);
        }
        break;
      case HdCullStyleBackUnlessDoubleSided:
        if (!_doubleSided)
        {
          glEnable(GL_CULL_FACE);
          if (_hasMirroredTransform)
          {
            glCullFace(GL_FRONT);
          }
          else
          {
            glCullFace(GL_BACK);
          }
        }
        break;
      case HdCullStyleNothing:
        glDisable(GL_CULL_FACE);
        break;
      case HdCullStyleDontCare:
      default:
        // Fallback to the renderPass opinion, but account for
        // combinations of parameters that require extra handling
        HdCullStyle cullstyle = state.GetCullStyle();
        if (_doubleSided && (cullstyle == HdCullStyleBackUnlessDoubleSided ||
                             cullstyle == HdCullStyleFrontUnlessDoubleSided))
        {
          glDisable(GL_CULL_FACE);
        }
        else if (_hasMirroredTransform &&
                 (cullstyle == HdCullStyleBack || cullstyle == HdCullStyleBackUnlessDoubleSided))
        {
          glEnable(GL_CULL_FACE);
          glCullFace(GL_FRONT);
        }
        else if (_hasMirroredTransform &&
                 (cullstyle == HdCullStyleFront || cullstyle == HdCullStyleFrontUnlessDoubleSided))
        {
          glEnable(GL_CULL_FACE);
          glCullFace(GL_BACK);
        }
        break;
    }
  }
  else
  {
    // Use fragment shader culling via discard.
    glDisable(GL_CULL_FACE);

    if (_cullStyle != HdCullStyleDontCare)
    {
      unsigned int cullStyle = _cullStyle;
      binder.BindUniformui(HdShaderTokens->cullStyle, 1, &cullStyle);
    }
    else
    {
      // don't care -- use renderPass's fallback
    }
  }

  if (GetPrimitiveMode() == GL_PATCHES)
  {
    glPatchParameteri(GL_PATCH_VERTICES, GetPrimitiveIndexSize());
  }

  if (_polygonMode == HdPolygonModeLine)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (_lineWidth > 0)
    {
      glLineWidth(_lineWidth);
    }
  }
}

void HdPh_GeometricShader::UnbindResources(const int program,
                                           HdPh_ResourceBinder const &binder,
                                           HdRenderPassState const &state)
{
  if (_polygonMode == HdPolygonModeLine)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Restore renderPass culling opinions
  HdCullStyle cullstyle = state.GetCullStyle();
  switch (cullstyle)
  {
    case HdCullStyleFront:
    case HdCullStyleFrontUnlessDoubleSided:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      break;
    case HdCullStyleBack:
    case HdCullStyleBackUnlessDoubleSided:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      break;
    case HdCullStyleNothing:
    case HdCullStyleDontCare:
    default:
      glDisable(GL_CULL_FACE);
      break;
  }
}

/*virtual*/
void HdPh_GeometricShader::AddBindings(HdBindingRequestVector *customBindings)
{
  // no-op
}

GLenum HdPh_GeometricShader::GetPrimitiveMode() const
{
  GLenum primMode = GL_POINTS;

  switch (_primType)
  {
    case PrimitiveType::PRIM_POINTS:
      primMode = GL_POINTS;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_LINES:
      primMode = GL_LINES;
      break;
    case PrimitiveType::PRIM_MESH_COARSE_TRIANGLES:
    case PrimitiveType::PRIM_MESH_REFINED_TRIANGLES:
    case PrimitiveType::PRIM_VOLUME:
      primMode = GL_TRIANGLES;
      break;
    case PrimitiveType::PRIM_MESH_COARSE_QUADS:
    case PrimitiveType::PRIM_MESH_REFINED_QUADS:
      primMode = GL_LINES_ADJACENCY;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES:
    case PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES:
    case PrimitiveType::PRIM_MESH_BSPLINE:
    case PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE:
      primMode = GL_PATCHES;
      break;
  }

  return primMode;
}

int HdPh_GeometricShader::GetPrimitiveIndexSize() const
{
  int primIndexSize = 1;

  switch (_primType)
  {
    case PrimitiveType::PRIM_POINTS:
      primIndexSize = 1;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_LINES:
    case PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES:
      primIndexSize = 2;
      break;
    case PrimitiveType::PRIM_MESH_COARSE_TRIANGLES:
    case PrimitiveType::PRIM_MESH_REFINED_TRIANGLES:
    case PrimitiveType::PRIM_VOLUME:
      primIndexSize = 3;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES:
    case PrimitiveType::PRIM_MESH_COARSE_QUADS:
    case PrimitiveType::PRIM_MESH_REFINED_QUADS:
      primIndexSize = 4;
      break;
    case PrimitiveType::PRIM_MESH_BSPLINE:
      primIndexSize = 16;
      break;
    case PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE:
      primIndexSize = 12;
      break;
  }

  return primIndexSize;
}

int HdPh_GeometricShader::GetNumPatchEvalVerts() const
{
  int numPatchEvalVerts = 0;

  switch (_primType)
  {
    case PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES:
      numPatchEvalVerts = 2;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES:
      numPatchEvalVerts = 4;
      break;
    case PrimitiveType::PRIM_MESH_BSPLINE:
      numPatchEvalVerts = 16;
      break;
    case PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE:
      numPatchEvalVerts = 15;
      break;
    default:
      numPatchEvalVerts = 0;
      break;
  }

  return numPatchEvalVerts;
}

int HdPh_GeometricShader::GetNumPrimitiveVertsForGeometryShader() const
{
  int numPrimVerts = 1;

  switch (_primType)
  {
    case PrimitiveType::PRIM_POINTS:
      numPrimVerts = 1;
      break;
    case PrimitiveType::PRIM_BASIS_CURVES_LINES:
      numPrimVerts = 2;
      break;
    case PrimitiveType::PRIM_MESH_COARSE_TRIANGLES:
    case PrimitiveType::PRIM_MESH_REFINED_TRIANGLES:
    case PrimitiveType::PRIM_BASIS_CURVES_LINEAR_PATCHES:
    case PrimitiveType::PRIM_BASIS_CURVES_CUBIC_PATCHES:
    case PrimitiveType::PRIM_MESH_BSPLINE:
    case PrimitiveType::PRIM_MESH_BOXSPLINETRIANGLE:
    // for patches with tesselation, input to GS is still a series of tris
    case PrimitiveType::PRIM_VOLUME:
      numPrimVerts = 3;
      break;
    case PrimitiveType::PRIM_MESH_COARSE_QUADS:
    case PrimitiveType::PRIM_MESH_REFINED_QUADS:
      numPrimVerts = 4;
      break;
  }

  return numPrimVerts;
}

/*static*/
HdPh_GeometricShaderSharedPtr HdPh_GeometricShader::Create(
  HdPh_ShaderKey const &shaderKey,
  HdPhResourceRegistrySharedPtr const &resourceRegistry)
{
  // Use the shaderKey hash to deduplicate geometric shaders.
  HdInstance<HdPh_GeometricShaderSharedPtr> geometricShaderInstance =
    resourceRegistry->RegisterGeometricShader(shaderKey.ComputeHash());

  if (geometricShaderInstance.IsFirstInstance())
  {
    geometricShaderInstance.SetValue(
      std::make_shared<HdPh_GeometricShader>(shaderKey.GetGlslfxString(),
                                             shaderKey.GetPrimitiveType(),
                                             shaderKey.GetCullStyle(),
                                             shaderKey.UseHardwareFaceCulling(),
                                             shaderKey.HasMirroredTransform(),
                                             shaderKey.IsDoubleSided(),
                                             shaderKey.GetPolygonMode(),
                                             shaderKey.IsFrustumCullingPass(),
                                             shaderKey.GetFvarPatchType(),
                                             /*debugId=*/SdfPath(),
                                             shaderKey.GetLineWidth()));
  }
  return geometricShaderInstance.GetValue();
}

WABI_NAMESPACE_END
