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
#ifndef WABI_IMAGING_HD_ST_UNIT_TEST_GLDRAWING_H
#define WABI_IMAGING_HD_ST_UNIT_TEST_GLDRAWING_H

#include "wabi/wabi.h"
#include "wabi/imaging/hdSt/api.h"
#include "wabi/base/gf/frustum.h"
#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3f.h"

#include <string>

WABI_NAMESPACE_BEGIN


class HdSt_UnitTestWindow;

/// \class HdSt_UnitTestGLDrawing
///
/// A helper class for unit tests which need to perform GL drawing.
///
class HdSt_UnitTestGLDrawing {
public:
    HDST_API
    HdSt_UnitTestGLDrawing();
    HDST_API
    virtual ~HdSt_UnitTestGLDrawing();

    HDST_API
    int GetWidth() const;
    HDST_API
    int GetHeight() const;
    HDST_API
    void RunTest(int argc, char *argv[]);
    HDST_API
    void RunOffscreenTest();

    virtual void InitTest() = 0;
    HDST_API virtual void UninitTest();
    virtual void DrawTest() = 0;        // interactive mode
    virtual void OffscreenTest() = 0;   // offscreen mode (automated test)

    HDST_API
    virtual void MousePress(int button, int x, int y, int modKeys);
    HDST_API
    virtual void MouseRelease(int button, int x, int y, int modKeys);
    HDST_API
    virtual void MouseMove(int x, int y, int modKeys);
    HDST_API
    virtual void KeyRelease(int key);

    HDST_API
    virtual void Idle();

    HDST_API
    virtual void Present(uint32_t framebuffer) {
        // do nothing
    }

protected:
    HDST_API
    virtual void ParseArgs(int argc, char *argv[]);

    void SetCameraRotate(float rx, float ry) {
        _rotate[0] = rx; _rotate[1] = ry;
    }
    void SetCameraTranslate(GfVec3f t) {
        _translate = t;
    }
    GfVec3f GetCameraTranslate() const {
        return _translate;
    }
    HDST_API
    GfMatrix4d GetViewMatrix() const;
    HDST_API
    GfMatrix4d GetProjectionMatrix() const;
    HDST_API
    GfFrustum GetFrustum() const;

    GfVec2i GetMousePos() const { return GfVec2i(_mousePos[0], _mousePos[1]); }

private:
    HdSt_UnitTestWindow *_widget;
    float _rotate[2];
    GfVec3f _translate;

    int _mousePos[2];
    bool _mouseButton[3];
};


WABI_NAMESPACE_END

#endif // WABI_IMAGING_HD_ST_UNIT_TEST_GLDRAWING_H
