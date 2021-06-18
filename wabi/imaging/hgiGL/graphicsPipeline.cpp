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
#include "wabi/base/tf/diagnostic.h"

#include "wabi/imaging/hgiGL/conversions.h"
#include "wabi/imaging/hgiGL/diagnostic.h"
#include "wabi/imaging/hgiGL/graphicsPipeline.h"
#include "wabi/imaging/hgiGL/resourceBindings.h"
#include "wabi/imaging/hgiGL/shaderFunction.h"
#include "wabi/imaging/hgiGL/shaderProgram.h"

WABI_NAMESPACE_BEGIN

HgiGLGraphicsPipeline::HgiGLGraphicsPipeline(HgiGraphicsPipelineDesc const &desc)
  : HgiGraphicsPipeline(desc),
    _vao(0)
{
  if (!_descriptor.vertexBuffers.empty())
  {
    glCreateVertexArrays(1, &_vao);

    if (!_descriptor.debugName.empty())
    {
      HgiGLObjectLabel(GL_VERTEX_ARRAY, _vao, _descriptor.debugName);
    }

    // Configure the vertex buffers in the vertex array object.
    for (HgiVertexBufferDesc const &vbo : _descriptor.vertexBuffers)
    {

      HgiVertexAttributeDescVector const &vas = vbo.vertexAttributes;

      // Describe each vertex attribute in the vertex buffer
      for (size_t loc = 0; loc < vas.size(); loc++)
      {
        HgiVertexAttributeDesc const &va = vas[loc];

        uint32_t idx = va.shaderBindLocation;
        glEnableVertexArrayAttrib(_vao, idx);
        glVertexArrayAttribBinding(_vao, idx, vbo.bindingIndex);
        glVertexArrayAttribFormat(_vao,
                                  idx,
                                  HgiGetComponentCount(va.format),
                                  HgiGLConversions::GetFormatType(va.format),
                                  GL_FALSE,
                                  va.offset);
      }
    }
  }

  HGIGL_POST_PENDING_GL_ERRORS();
}

HgiGLGraphicsPipeline::~HgiGLGraphicsPipeline()
{
  if (_vao)
  {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &_vao);
  }
  HGIGL_POST_PENDING_GL_ERRORS();
}

void HgiGLGraphicsPipeline::BindPipeline()
{
  if (_vao)
  {
    glBindVertexArray(_vao);
  }

  //
  // Depth Stencil State
  //
  if (_descriptor.depthState.depthTestEnabled)
  {
    glEnable(GL_DEPTH_TEST);
    GLenum depthFn = HgiGLConversions::GetDepthCompareFunction(_descriptor.depthState.depthCompareFn);
    glDepthFunc(depthFn);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }

  glDepthMask(_descriptor.depthState.depthWriteEnabled ? GL_TRUE : GL_FALSE);

  if (_descriptor.depthState.stencilTestEnabled)
  {
    TF_CODING_ERROR("Missing implementation stencil mask enabled");
  }
  else
  {
    glStencilMaskSeparate(GL_FRONT, 0);
    glStencilMaskSeparate(GL_BACK, 0);
  }

  //
  // Multi sample state
  //
  if (_descriptor.multiSampleState.alphaToCoverageEnable)
  {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_SAMPLE_ALPHA_TO_ONE);
  }
  else
  {
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);
  }

  //
  // Rasterization state
  //
  GLenum cullMode = HgiGLConversions::GetCullMode(_descriptor.rasterizationState.cullMode);
  if (cullMode == GL_NONE)
  {
    glDisable(GL_CULL_FACE);
  }
  else
  {
    glEnable(GL_CULL_FACE);
    glCullFace(cullMode);
  }

  GLenum polygonMode = HgiGLConversions::GetPolygonMode(_descriptor.rasterizationState.polygonMode);
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

  if (_descriptor.rasterizationState.winding == HgiWindingClockwise)
  {
    glFrontFace(GL_CW);
  }
  else
  {
    glFrontFace(GL_CCW);
  }

  if (_descriptor.rasterizationState.lineWidth != 1.0f)
  {
    glLineWidth(_descriptor.rasterizationState.lineWidth);
  }

  if (_descriptor.rasterizationState.rasterizerEnabled)
  {
    glDisable(GL_RASTERIZER_DISCARD);
  }
  else
  {
    glEnable(GL_RASTERIZER_DISCARD);
  }

  //
  // Shader program
  //
  HgiGLShaderProgram *glProgram = static_cast<HgiGLShaderProgram *>(_descriptor.shaderProgram.Get());
  if (glProgram)
  {
    glUseProgram(glProgram->GetProgramId());
  }

  HGIGL_POST_PENDING_GL_ERRORS();
}

WABI_NAMESPACE_END
