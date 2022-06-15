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
#ifndef WABI_IMAGING_HD_ST_DRAW_BATCH_H
#define WABI_IMAGING_HD_ST_DRAW_BATCH_H

#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hd/repr.h"
#include "wabi/imaging/hdPh/resourceBinder.h"
#include "wabi/imaging/hdPh/shaderCode.h"

#include <memory>
#include <vector>

WABI_NAMESPACE_BEGIN

class HdPhDrawItem;
class HdPhDrawItemInstance;

using HdPh_DrawBatchSharedPtr = std::shared_ptr<class HdPh_DrawBatch>;
using HdPh_DrawBatchSharedPtrVector = std::vector<HdPh_DrawBatchSharedPtr>;
using HdPh_GeometricShaderSharedPtr = std::shared_ptr<class HdPh_GeometricShader>;
using HdPhGLSLProgramSharedPtr = std::shared_ptr<class HdPhGLSLProgram>;

using HdPhRenderPassStateSharedPtr = std::shared_ptr<class HdPhRenderPassState>;
using HdPhResourceRegistrySharedPtr = std::shared_ptr<class HdPhResourceRegistry>;

/// \class HdPh_DrawBatch
///
/// A drawing batch.
///
/// This is the finest grained element of drawing, representing potentially
/// aggregated drawing resources dispatched with a minimal number of draw
/// calls.
///
class HdPh_DrawBatch
{
 public:

  HDPH_API
  HdPh_DrawBatch(HdPhDrawItemInstance *drawItemInstance);

  HDPH_API
  virtual ~HdPh_DrawBatch();

  /// Attempts to append \a drawItemInstance to the batch, returning \a false
  /// if the item could not be appended, e.g. if there was an aggregation
  /// conflict.
  HDPH_API
  bool Append(HdPhDrawItemInstance *drawItemInstance);

  /// Attempt to rebuild the batch in-place, returns false if draw items are
  /// no longer compatible.
  HDPH_API
  bool Rebuild();

  enum class ValidationResult
  {
    ValidBatch = 0,
    RebuildBatch,
    RebuildAllBatches
  };

  /// Validates whether the  batch can be reused (i.e., submitted) as-is, or
  /// if it needs to be rebuilt, or if all batches need to be rebuilt.
  virtual ValidationResult Validate(bool deepValidation) = 0;

  /// Prepare draw commands and apply view frustum culling for this batch.
  virtual void PrepareDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                           HdPhResourceRegistrySharedPtr const &resourceRegistry) = 0;

  /// Executes the drawing commands for this batch.
  virtual void ExecuteDraw(HdPhRenderPassStateSharedPtr const &renderPassState,
                           HdPhResourceRegistrySharedPtr const &resourceRegistry) = 0;

  /// Let the batch know that one of it's draw item instances has changed
  /// NOTE: This callback is called from multiple threads, so needs to be
  /// threadsafe.
  HDPH_API
  virtual void DrawItemInstanceChanged(HdPhDrawItemInstance const *instance);

  /// Let the batch know whether to use tiny prim culling.
  HDPH_API
  virtual void SetEnableTinyPrimCulling(bool tinyPrimCulling);

 protected:

  HDPH_API
  virtual void _Init(HdPhDrawItemInstance *drawItemInstance);

  /// \class _DrawingProgram
  ///
  /// This wraps glsl code generation and keeps track of
  /// binding assigments for bindable resources.
  ///
  class _DrawingProgram
  {
   public:

    _DrawingProgram() {}

    HDPH_API
    bool CompileShader(HdPhDrawItem const *drawItem,
                       bool indirect,
                       HdPhResourceRegistrySharedPtr const &resourceRegistry);

    HdPhGLSLProgramSharedPtr GetGLSLProgram() const
    {
      return _glslProgram;
    }

    /// Returns the resouce binder, which is used for buffer resource
    /// bindings at draw time.
    const HdPh_ResourceBinder &GetBinder() const
    {
      return _resourceBinder;
    }

    void Reset()
    {
      _glslProgram.reset();
      _surfaceShader.reset();
      _geometricShader.reset();
      _resourceBinder = HdPh_ResourceBinder();
      _shaders.clear();
    }

    void SetSurfaceShader(HdPhShaderCodeSharedPtr shader)
    {
      _surfaceShader = shader;
    }

    const HdPhShaderCodeSharedPtr &GetSurfaceShader()
    {
      return _surfaceShader;
    }

    void SetGeometricShader(HdPh_GeometricShaderSharedPtr shader)
    {
      _geometricShader = shader;
    }

    const HdPh_GeometricShaderSharedPtr &GetGeometricShader()
    {
      return _geometricShader;
    }

    /// Set shaders (lighting/renderpass). In the case of Geometric Shaders
    /// or Surface shaders you can use the specific setters.
    void SetShaders(HdPhShaderCodeSharedPtrVector shaders)
    {
      _shaders = shaders;
    }

    /// Returns array of shaders, this will not include the surface shader
    /// passed via SetSurfaceShader (or the geometric shader).
    const HdPhShaderCodeSharedPtrVector &GetShaders() const
    {
      return _shaders;
    }

    /// Returns array of composed shaders, this include the shaders passed
    /// via SetShaders and the shader passed to SetSurfaceShader.
    HdPhShaderCodeSharedPtrVector GetComposedShaders() const
    {
      HdPhShaderCodeSharedPtrVector shaders = _shaders;
      if (_surfaceShader) {
        shaders.push_back(_surfaceShader);
      }
      return shaders;
    }

   protected:

    // overrides populate customBindings and enableInstanceDraw which
    // will be used to determine if glVertexAttribDivisor needs to be
    // enabled or not.
    HDPH_API
    virtual void _GetCustomBindings(HdBindingRequestVector *customBindings,
                                    bool *enableInstanceDraw) const;

    HDPH_API
    virtual bool _Link(HdPhGLSLProgramSharedPtr const &glslProgram);

   private:

    HdPhGLSLProgramSharedPtr _glslProgram;
    HdPh_ResourceBinder _resourceBinder;
    HdPhShaderCodeSharedPtrVector _shaders;
    HdPh_GeometricShaderSharedPtr _geometricShader;
    HdPhShaderCodeSharedPtr _surfaceShader;
  };

  HDPH_API
  _DrawingProgram &_GetDrawingProgram(HdPhRenderPassStateSharedPtr const &state,
                                      bool indirect,
                                      HdPhResourceRegistrySharedPtr const &resourceRegistry);

 protected:

  HDPH_API
  static bool _IsAggregated(HdPhDrawItem const *drawItem0, HdPhDrawItem const *drawItem1);

  std::vector<HdPhDrawItemInstance const *> _drawItemInstances;

 private:

  _DrawingProgram _program;
  HdPhShaderCode::ID _shaderHash;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_DRAW_BATCH_H
