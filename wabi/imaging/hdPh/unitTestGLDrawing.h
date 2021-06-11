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

#include "wabi/base/gf/frustum.h"
#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3f.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/wabi.h"

#include <string>

WABI_NAMESPACE_BEGIN

class HdPh_UnitTestWindow;

/// \class HdPh_UnitTestGLDrawing
///
/// A helper class for unit tests which need to perform GL drawing.
///
class HdPh_UnitTestGLDrawing {
 public:
  HDPH_API
  HdPh_UnitTestGLDrawing();
  HDPH_API
  virtual ~HdPh_UnitTestGLDrawing();

  HDPH_API
  int GetWidth() const;
  HDPH_API
  int GetHeight() const;
  HDPH_API
  void RunTest(int argc, char *argv[]);
  HDPH_API
  void RunOffscreenTest();

  virtual void InitTest() = 0;
  HDPH_API virtual void UninitTest();
  virtual void DrawTest()      = 0;  // interactive mode
  virtual void OffscreenTest() = 0;  // offscreen mode (automated test)

  HDPH_API
  virtual void MousePress(int button, int x, int y, int modKeys);
  HDPH_API
  virtual void MouseRelease(int button, int x, int y, int modKeys);
  HDPH_API
  virtual void MouseMove(int x, int y, int modKeys);
  HDPH_API
  virtual void KeyRelease(int key);

  HDPH_API
  virtual void Idle();

  HDPH_API
  bool WriteToFile(std::string const &attachment, std::string const &filename) const;

 protected:
  HDPH_API
  virtual void ParseArgs(int argc, char *argv[]);

  void SetCameraRotate(float rx, float ry)
  {
    _rotate[0] = rx;
    _rotate[1] = ry;
  }
  void SetCameraTranslate(GfVec3f t)
  {
    _translate = t;
  }
  GfVec3f GetCameraTranslate() const
  {
    return _translate;
  }
  HDPH_API
  GfMatrix4d GetViewMatrix() const;
  HDPH_API
  GfMatrix4d GetProjectionMatrix() const;
  HDPH_API
  GfFrustum GetFrustum() const;

  GfVec2i GetMousePos() const
  {
    return GfVec2i(_mousePos[0], _mousePos[1]);
  }

 private:
  HdPh_UnitTestWindow *_widget;
  float _rotate[2];
  GfVec3f _translate;

  int _mousePos[2];
  bool _mouseButton[3];
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_ST_UNIT_TEST_GLDRAWING_H
