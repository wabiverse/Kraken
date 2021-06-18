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

#ifndef USDCYCLES_TOKENS_H
#define USDCYCLES_TOKENS_H

#include "api.h"
#include "wabi/base/tf/staticData.h"
#include "wabi/base/tf/token.h"
#include "wabi/wabi.h"
#include <vector>

WABI_NAMESPACE_BEGIN

struct HdCyclesTokensType
{

  HDCYCLES_API
  HdCyclesTokensType();

  const TfToken cpu;
  const TfToken gpu;

  const TfToken svm;
  const TfToken osl;

  const TfToken hilbert_spiral;
  const TfToken center;
  const TfToken right_to_left;
  const TfToken left_to_right;
  const TfToken top_to_bottom;
  const TfToken bottom_to_top;

  const TfToken bvh_dynamic;
  const TfToken bvh_static;

  const TfToken none;
  const TfToken optix;
  const TfToken openimagedenoise;

  const TfToken rgb_albedo_normal;
  const TfToken rgb;
  const TfToken rgb_albedo;

  const TfToken branched_path;
  const TfToken path;

  const TfToken sobol;
  const TfToken cmj;
  const TfToken pmj;

  const TfToken box;
  const TfToken gaussian;
  const TfToken blackman_harris;

  const TfToken start;
  const TfToken end;

  const TfToken top;
  const TfToken left;
  const TfToken right;

  const TfToken equirectangular;
  const TfToken fisheye_equidistant;
  const TfToken fisheye_equisolid;
  const TfToken mirrorball;

  const TfToken displacement_bump;
  const TfToken displacement_true;
  const TfToken displacement_both;

  const TfToken volume_sampling_distance;
  const TfToken volume_sampling_equiangular;
  const TfToken volume_sampling_multiple_importance;

  const TfToken volume_interpolation_linear;
  const TfToken volume_interpolation_cubic;

  const TfToken ribbon;
  const TfToken thick;

  const TfToken disc;
  const TfToken disc_oriented;
  const TfToken sphere;

  const TfToken combined;
  const TfToken ambient_occlusion;
  const TfToken shadow;
  const TfToken normal;
  const TfToken uv;
  const TfToken roughness;
  const TfToken emit;
  const TfToken environment;
  const TfToken diffuse;
  const TfToken glossy;
  const TfToken transmission;
  const TfToken diffuse_color;
  const TfToken glossy_color;
  const TfToken transmission_color;

  const TfToken primvarsCyclesObjectMblur;
  const TfToken primvarsCyclesObjectTransformSamples;
  const TfToken primvarsCyclesObjectDeformSamples;
  const TfToken primvarsCyclesObjectMblurVolumeVelScale;

  const TfToken primvarsCyclesObjectPassId;
  const TfToken primvarsCyclesObjectUseHoldout;
  const TfToken primvarsCyclesObjectIsShadowCatcher;

  const TfToken primvarsCyclesObjectVisibilityCamera;
  const TfToken primvarsCyclesObjectVisibilityDiffuse;
  const TfToken primvarsCyclesObjectVisibilityGlossy;
  const TfToken primvarsCyclesObjectVisibilityTransmission;
  const TfToken primvarsCyclesObjectVisibilityShadow;
  const TfToken primvarsCyclesObjectVisibilityScatter;

  const TfToken primvarsCyclesObjectAssetName;

  const TfToken primvarsCyclesMeshDicingRate;
  const TfToken primvarsCyclesMeshSubdivisionMaxLevel;

  const TfToken primvarsCyclesCurveShape;

  const TfToken cyclesDevice;
  const TfToken cyclesShadingSystem;
  const TfToken cyclesProgressive;
  const TfToken cyclesProgressiveRefine;
  const TfToken cyclesProgressiveUpdateTimeout;
  const TfToken cyclesAdaptiveSampling;
  const TfToken cyclesStartResolution;
  const TfToken cyclesPixelSize;
  const TfToken cyclesSamples;
  const TfToken cyclesThreads;
  const TfToken cyclesTileSize;
  const TfToken cyclesTileOrder;
  const TfToken cyclesExperimental;
  const TfToken cyclesUseSquareSamples;
  const TfToken cyclesDicingCamera;
  const TfToken cyclesUseProfiling;
  const TfToken cyclesUseBvhUnalignedNodes;
  const TfToken cyclesBvhType;
  const TfToken cyclesUseBvhSpatialSplit;
  const TfToken cyclesNumBvhTimeSteps;
  const TfToken cyclesDisplayBufferLinear;
  const TfToken cyclesCurveSubdivisions;

  const TfToken cyclesDenoiseUse;
  const TfToken cyclesDenoiseStorePasses;
  const TfToken cyclesDenoiseType;
  const TfToken cyclesDenoiseStartSample;
  const TfToken cyclesDenoiseInputPasses;

  const TfToken cyclesIntegratorMethod;
  const TfToken cyclesIntegratorSamplingMethod;
  const TfToken cyclesIntegratorSeed;
  const TfToken cyclesIntegratorMinBounce;
  const TfToken cyclesIntegratorMaxBounce;
  const TfToken cyclesIntegratorMaxDiffuseBounce;
  const TfToken cyclesIntegratorMaxGlossyBounce;
  const TfToken cyclesIntegratorMaxTransmissionBounce;
  const TfToken cyclesIntegratorMaxVolumeBounce;
  const TfToken cyclesIntegratorTransparentMinBounce;
  const TfToken cyclesIntegratorTransparentMaxBounce;
  const TfToken cyclesIntegratorAoBounces;
  const TfToken cyclesIntegratorVolumeMaxSteps;
  const TfToken cyclesIntegratorVolumeStepSize;
  const TfToken cyclesIntegratorAaSamples;
  const TfToken cyclesIntegratorDiffuseSamples;
  const TfToken cyclesIntegratorGlossySamples;
  const TfToken cyclesIntegratorTransmissionSamples;
  const TfToken cyclesIntegratorAoSamples;
  const TfToken cyclesIntegratorMeshLightSamples;
  const TfToken cyclesIntegratorSubsurfaceSamples;
  const TfToken cyclesIntegratorVolumeSamples;
  const TfToken cyclesIntegratorStartSample;
  const TfToken cyclesIntegratorCausticsReflective;
  const TfToken cyclesIntegratorCausticsRefractive;
  const TfToken cyclesIntegratorFilterGlossy;
  const TfToken cyclesIntegratorSampleClampDirect;
  const TfToken cyclesIntegratorSampleClampIndirect;
  const TfToken cyclesIntegratorMotionBlur;
  const TfToken cyclesIntegratorSampleAllLightsDirect;
  const TfToken cyclesIntegratorSampleAllLightsIndirect;
  const TfToken cyclesIntegratorLightSamplingThreshold;
  const TfToken cyclesIntegratorAdaptiveMinSamples;
  const TfToken cyclesIntegratorAdaptiveThreshold;

  const TfToken cyclesFilmExposure;
  const TfToken cyclesFilmPassAlphaThreshold;
  const TfToken cyclesFilmFilterType;
  const TfToken cyclesFilmFilterWidth;
  const TfToken cyclesFilmMistStart;
  const TfToken cyclesFilmMistDepth;
  const TfToken cyclesFilmMistFalloff;
  const TfToken cyclesFilmUseLightVisibility;
  const TfToken cyclesFilmUseAdaptiveSampling;
  const TfToken cyclesFilmDenoisingDataPass;
  const TfToken cyclesFilmDenoisingCleanPass;
  const TfToken cyclesFilmDenoisingPrefilteredPass;
  const TfToken cyclesFilmDenoisingFlags;
  const TfToken cyclesFilmCryptomatteDepth;

  const TfToken cyclesTextureUseCache;
  const TfToken cyclesTextureCacheSize;
  const TfToken cyclesTextureTileSize;
  const TfToken cyclesTextureDiffuseBlur;
  const TfToken cyclesTextureGlossyBlur;
  const TfToken cyclesTextureAutoConvert;
  const TfToken cyclesTextureAcceptUnmipped;
  const TfToken cyclesTextureAcceptUntiled;
  const TfToken cyclesTextureAutoTile;
  const TfToken cyclesTextureAutoMip;
  const TfToken cyclesTextureUseCustomPath;
  const TfToken cyclesTextureMaxSize;

  const TfToken cyclesLightUseMis;
  const TfToken cyclesLightUseDiffuse;
  const TfToken cyclesLightUseGlossy;
  const TfToken cyclesLightUseTransmission;
  const TfToken cyclesLightUseScatter;
  const TfToken cyclesLightIsPortal;
  const TfToken cyclesLightSamples;
  const TfToken cyclesLightMaxBounces;
  const TfToken cyclesLightMapResolution;

  const TfToken cyclesCameraMotionPosition;
  const TfToken cyclesCameraShutterTime;
  const TfToken cyclesCameraShutterCurve;
  const TfToken cyclesCameraRollingShutterType;
  const TfToken cyclesCameraRollingShutterDuration;
  const TfToken cyclesCameraStereoEye;
  const TfToken cyclesCameraUsePanoramic;
  const TfToken cyclesCameraPanoramaType;
  const TfToken cyclesCameraBlades;
  const TfToken cyclesCameraBladesRotation;
  const TfToken cyclesCameraOffscreenDicingScale;
  const TfToken cyclesCameraFisheyeFov;
  const TfToken cyclesCameraFisheyeLens;
  const TfToken cyclesCameraLatitudeMin;
  const TfToken cyclesCameraLatitudeMax;
  const TfToken cyclesCameraLongitudeMin;
  const TfToken cyclesCameraLongitudeMax;
  const TfToken cyclesCameraUseSphericalStereo;
  const TfToken cyclesCameraInterocularDistance;
  const TfToken cyclesCameraConvergenceDistance;
  const TfToken cyclesCameraUsePoleMerge;
  const TfToken cyclesCameraPoleMergeAngleFrom;
  const TfToken cyclesCameraPoleMergeAngleTo;

  const TfToken cyclesMaterialPassId;
  const TfToken cyclesMaterialDisplacementMethod;
  const TfToken cyclesMaterialUseMis;
  const TfToken cyclesMaterialUseTransparentShadow;
  const TfToken cyclesMaterialHeterogeneousVolume;
  const TfToken cyclesMaterialVolumeSamplingMethod;
  const TfToken cyclesMaterialVolumeInterpolationMethod;
  const TfToken cyclesMaterialVolumeStepRate;

  const TfToken cyclesBackgroundAoFactor;
  const TfToken cyclesBackgroundAoDistance;
  const TfToken cyclesBackgroundUseShader;
  const TfToken cyclesBackgroundUseAo;
  const TfToken cyclesBackgroundTransparent;
  const TfToken cyclesBackgroundTransparentGlass;
  const TfToken cyclesBackgroundTransparentRoughnessThreshold;
  const TfToken cyclesBackgroundVolumeStepSize;
  const TfToken cyclesBackgroundVisibilityCamera;
  const TfToken cyclesBackgroundVisibilityDiffuse;
  const TfToken cyclesBackgroundVisibilityGlossy;
  const TfToken cyclesBackgroundVisibilityTransmission;
  const TfToken cyclesBackgroundVisibilityScatter;

  const TfToken cyclesObjectPointStyle;
  const TfToken cyclesObjectPointResolution;

  const TfToken cyclesBakeEnable;
  const TfToken cyclesBakeObject;
  const TfToken cyclesBakeBakeType;
  const TfToken cyclesBakeDirect;
  const TfToken cyclesBakeIndirect;
  const TfToken cyclesBakeFilterColor;
  const TfToken cyclesBakeFilterDiffuse;
  const TfToken cyclesBakeFilterGlossy;
  const TfToken cyclesBakeFilterTransmission;
  const TfToken cyclesBakeFilterAmbientOcclusion;
  const TfToken cyclesBakeFilterEmission;
  const TfToken cyclesBakeMargin;

  /**
   *  A vector of all of the tokens listed above. */
  const std::vector<TfToken> allTokens;
};

extern HDCYCLES_API TfStaticData<HdCyclesTokensType> HdCyclesTokens;

WABI_NAMESPACE_END

#endif /* USDCYCLES_TOKENS_H */