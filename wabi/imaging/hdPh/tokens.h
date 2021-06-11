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
#ifndef WABI_IMAGING_HD_ST_TOKENS_H
#define WABI_IMAGING_HD_ST_TOKENS_H

#include "wabi/base/tf/staticTokens.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

#define HDPH_GLSL_PROGRAM_TOKENS \
  (smoothNormalsFloatToFloat)(smoothNormalsFloatToPacked)( \
      smoothNormalsDoubleToDouble)(smoothNormalsDoubleToPacked)(flatNormalsTriFloatToFloat)(flatNormalsTriFloatToPacked)(flatNormalsTriDoubleToDouble)(flatNormalsTriDoubleToPacked)(flatNormalsQuadFloatToFloat)(flatNormalsQuadFloatToPacked)(flatNormalsQuadDoubleToDouble)(flatNormalsQuadDoubleToPacked)(quadrangulateFloat)(quadrangulateDouble)

#define HDPH_TOKENS \
  (constantLighting)(packedSmoothNormals)( \
      smoothNormals)(packedFlatNormals)(flatNormals)(scale)(bias)(rotation)(translation)(sRGB)(raw)(( \
      _double, "double"))((_float, "float"))((_int, "int"))(( \
      colorSpaceAuto, "auto"))(fvarIndices)(fvarPatchParam)

#define HDPH_LIGHT_TOKENS (color)

#define HDPH_TEXTURE_TOKENS \
  (wrapS)(wrapT)( \
      wrapR)(black)(clamp)(mirror)(repeat)(useMetadata)(minFilter)(magFilter)(linear)(nearest)(linearMipmapLinear)(linearMipmapNearest)(nearestMipmapLinear)(nearestMipmapNearest)

#define HDPH_RENDER_BUFFER_TOKENS ((phoenixMsaaSampleCount, "phoenix:msaaSampleCount"))

#define HDPH_RENDER_SETTINGS_TOKENS \
  (enableTinyPrimCulling)(volumeRaymarchingStepSize)( \
      volumeRaymarchingStepSizeLighting)(volumeMaxTextureMemoryPerField)

// Material tags help bucket prims into different queues for draw submission.
// The tags supported by Phoenix are:
//    defaultMaterialTag : opaque geometry
//    masked : opaque geometry that uses cutout masks (e.g., foliage)
//    translucentToSelection: opaque geometry that allows occluded selection
//                            to show through
//    additive : transparent geometry (cheap OIT solution w/o sorting)
//    translucent: transparent geometry (OIT solution w/ sorted fragment lists)
//    volume : transparent geoometry (raymarched)
#define HDPH_MATERIAL_TAG_TOKENS \
  (defaultMaterialTag)(masked)(translucentToSelection)(additive)(translucent)(volume)

#define HDPH_SDR_METADATA_TOKENS (swizzle)

#define HDPH_PERF_TOKENS (copyBufferGpuToGpu)(copyBufferCpuToGpu)

TF_DECLARE_PUBLIC_TOKENS(HdPhGLSLProgramTokens, HDPH_API, HDPH_GLSL_PROGRAM_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhTokens, HDPH_API, HDPH_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhLightTokens, HDPH_API, HDPH_LIGHT_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhTextureTokens, HDPH_API, HDPH_TEXTURE_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhRenderBufferTokens, HDPH_API, HDPH_RENDER_BUFFER_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhRenderSettingsTokens, HDPH_API, HDPH_RENDER_SETTINGS_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhMaterialTagTokens, HDPH_API, HDPH_MATERIAL_TAG_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhSdrMetadataTokens, HDPH_API, HDPH_SDR_METADATA_TOKENS);

TF_DECLARE_PUBLIC_TOKENS(HdPhPerfTokens, HDPH_API, HDPH_PERF_TOKENS);

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_TOKENS_H
