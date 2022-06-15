//
// Copyright 2019 Pixar
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
#ifndef WABI_IMAGING_HD_ST_VOLUME_SHADER_KEY_H
#define WABI_IMAGING_HD_ST_VOLUME_SHADER_KEY_H

#include "wabi/wabi.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hdSt/geometricShader.h"
#include "wabi/imaging/hdSt/shaderKey.h"
#include "wabi/base/tf/token.h"

WABI_NAMESPACE_BEGIN

struct HdSt_VolumeShaderKey : public HdSt_ShaderKey
{
    HdSt_VolumeShaderKey();
    ~HdSt_VolumeShaderKey();

    TfToken const &GetGlslfxFilename() const override { return glslfx; }
    TfToken const *GetVS() const override { return VS; }
    TfToken const *GetFS() const override { return FS; }

    HdSt_GeometricShader::PrimitiveType GetPrimitiveType() const override { 
        return HdSt_GeometricShader::PrimitiveType::PRIM_VOLUME;
    }

    TfToken glslfx;
    TfToken VS[3];
    TfToken FS[4];
};


WABI_NAMESPACE_END

#endif  // HDST_VOLUME_SHADER_KEY
