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
#ifndef WABI_IMAGING_HD_ST_GL_CONVERSIONS_H
#define WABI_IMAGING_HD_ST_GL_CONVERSIONS_H

#include "wabi/wabi.h"
#include "wabi/imaging/hdSt/api.h"
#include "wabi/imaging/hd/enums.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hio/types.h"
#include "wabi/base/tf/token.h"

WABI_NAMESPACE_BEGIN


class HdSt_GeometricShader;

class HdStGLConversions {
public:
    HDST_API
    static GLenum GetGlDepthFunc(HdCompareFunction func);

    HDST_API
    static GLenum GetGlStencilFunc(HdCompareFunction func);

    HDST_API
    static GLenum GetGlStencilOp(HdStencilOp op);

    HDST_API
    static GLenum GetGlBlendOp(HdBlendOp op);

    HDST_API
    static GLenum GetGlBlendFactor(HdBlendFactor factor);

    HDST_API
    static GLenum GetGLAttribType(HdType type);

    HDST_API
    static GLenum GetPrimitiveMode(HdSt_GeometricShader const *geometricShader);

    /// Return the name of the given type as represented in GLSL.
    HDST_API
    static TfToken GetGLSLTypename(HdType type);

    /// Return a GLSL-safe, mangled name identifier.
    HDST_API
    static TfToken GetGLSLIdentifier(TfToken const& identifier);
};


WABI_NAMESPACE_END

#endif // WABI_IMAGING_HD_ST_GL_CONVERSIONS_H
