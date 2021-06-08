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
#include "wabi/imaging/hdPh/cullingShaderKey.h"
#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    ((baseGLSLFX, "frustumCull.glslfx"))((instancing,
                                          "Instancing.Transform"))((counting,
                                                                    "ViewFrustumCull.Counting"))((
        noCounting,
        "ViewFrustumCull.NoCounting"))((tinyCull, "ViewFrustumCull.TinyCull"))((
        noTinyCull,
        "ViewFrustumCull.NoTinyCull"))((isVisible, "ViewFrustumCull.IsVisible"))((
        mainInstancingVS,
        "ViewFrustumCull.VertexInstancing"))((mainVS, "ViewFrustumCull.Vertex")));

HdPh_CullingShaderKey::HdPh_CullingShaderKey(bool instancing, bool tinyCull, bool counting)
    : glslfx(_tokens->baseGLSLFX)
{

  VS[0] = _tokens->instancing;
  VS[1] = counting ? _tokens->counting : _tokens->noCounting;
  VS[2] = tinyCull ? _tokens->tinyCull : _tokens->noTinyCull;
  VS[3] = _tokens->isVisible;
  VS[4] = instancing ? _tokens->mainInstancingVS : _tokens->mainVS;
  VS[5] = TfToken();
}

HdPh_CullingShaderKey::~HdPh_CullingShaderKey()
{}

WABI_NAMESPACE_END
