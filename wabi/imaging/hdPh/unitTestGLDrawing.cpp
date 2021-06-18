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

#include "wabi/imaging/garch/glDebugWindow.h"
#include "wabi/imaging/glf/contextCaps.h"
#include "wabi/imaging/glf/diagnostic.h"
#include "wabi/imaging/glf/drawTarget.h"
#include "wabi/imaging/hdPh/unitTestGLDrawing.h"

#include "wabi/base/gf/frustum.h"
#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/rotation.h"
#include "wabi/base/gf/vec2i.h"
#include "wabi/base/gf/vec4d.h"

#include <cstdlib>
#include <iostream>

WABI_NAMESPACE_BEGIN

class HdPh_UnitTestWindow : public GarchGLDebugWindow
{
 public:
  typedef HdPh_UnitTestWindow This;

 public:
  HdPh_UnitTestWindow(HdPh_UnitTestGLDrawing *unitTest, int width, int height);
  virtual ~HdPh_UnitTestWindow();

  void OffscreenTest();

  bool WriteToFile(std::string const &attachment, std::string const &filename);

  void StartTimer();

  // GarchGLDebugWindow overrides
  virtual void OnInitializeGL();
  virtual void OnUninitializeGL();
  virtual void OnIdle();
  virtual void OnPaintGL();
  virtual void OnKeyRelease(int key);
  virtual void OnMousePress(int button, int x, int y, int modKeys);
  virtual void OnMouseRelease(int button, int x, int y, int modKeys);
  virtual void OnMouseMove(int x, int y, int modKeys);

 private:
  HdPh_UnitTestGLDrawing *_unitTest;
  GlfDrawTargetRefPtr _drawTarget;
  bool _animate;
};

HdPh_UnitTestWindow::HdPh_UnitTestWindow(HdPh_UnitTestGLDrawing *unitTest, int w, int h)
  : GarchGLDebugWindow("Hd Test", w, h),
    _unitTest(unitTest),
    _animate(false)
{}

HdPh_UnitTestWindow::~HdPh_UnitTestWindow()
{}

/* virtual */
void HdPh_UnitTestWindow::OnInitializeGL()
{
  GarchGLApiLoad();
  GlfRegisterDefaultDebugOutputMessageCallback();
  GlfContextCaps::InitInstance();

  std::cout << glGetString(GL_VENDOR) << "\n";
  std::cout << glGetString(GL_RENDERER) << "\n";
  std::cout << glGetString(GL_VERSION) << "\n";

  //
  // Create an offscreen draw target which is the same size as this
  // widget and initialize the unit test with the draw target bound.
  //
  _drawTarget = GlfDrawTarget::New(GfVec2i(GetWidth(), GetHeight()));
  _drawTarget->Bind();
  _drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);
  _drawTarget->AddAttachment("depth", GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_DEPTH24_STENCIL8);
  _unitTest->InitTest();

  _drawTarget->Unbind();
}

/* virtual */
void HdPh_UnitTestWindow::OnUninitializeGL()
{
  _drawTarget = GlfDrawTargetRefPtr();
  _unitTest->UninitTest();
}

/* virtual */
void HdPh_UnitTestWindow::OnPaintGL()
{
  //
  // Update the draw target's size and execute the unit test with
  // the draw target bound.
  //
  _drawTarget->Bind();
  _drawTarget->SetSize(GfVec2i(GetWidth(), GetHeight()));

  _unitTest->DrawTest();

  _drawTarget->Unbind();

  //
  // Blit the resulting color buffer to the window (this is a noop
  // if we're drawing offscreen).
  //
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, _drawTarget->GetFramebufferId());

  glBlitFramebuffer(
    0, 0, GetWidth(), GetHeight(), 0, 0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void HdPh_UnitTestWindow::OffscreenTest()
{
  _drawTarget->Bind();
  _drawTarget->SetSize(GfVec2i(GetWidth(), GetHeight()));

  _unitTest->OffscreenTest();

  _drawTarget->Unbind();
}

void HdPh_UnitTestWindow::StartTimer()
{
  _animate = true;
}

/* virtual */
void HdPh_UnitTestWindow::OnIdle()
{
  if (_animate)
  {
    _unitTest->Idle();
  }
}

bool HdPh_UnitTestWindow::WriteToFile(std::string const &attachment, std::string const &filename)
{
  _drawTarget->Unbind();
  bool ret = _drawTarget->WriteToFile(attachment, filename);
  _drawTarget->Bind();
  return ret;
}

/* virtual */
void HdPh_UnitTestWindow::OnKeyRelease(int key)
{
  switch (key)
  {
    case 'q':
      ExitApp();
      return;
  }
  _unitTest->KeyRelease(key);
}

/* virtual */
void HdPh_UnitTestWindow::OnMousePress(int button, int x, int y, int modKeys)
{
  _unitTest->MousePress(button, x, y, modKeys);
}

/* virtual */
void HdPh_UnitTestWindow::OnMouseRelease(int button, int x, int y, int modKeys)
{
  _unitTest->MouseRelease(button, x, y, modKeys);
}

/* virtual */
void HdPh_UnitTestWindow::OnMouseMove(int x, int y, int modKeys)
{
  _unitTest->MouseMove(x, y, modKeys);
}

////////////////////////////////////////////////////////////

HdPh_UnitTestGLDrawing::HdPh_UnitTestGLDrawing()
  : _widget(NULL)
{
  _rotate[0] = _rotate[1] = 0;
  _translate[0] = _translate[1] = _translate[2] = 0;

  _mousePos[0] = _mousePos[1] = 0;
  _mouseButton[0] = _mouseButton[1] = _mouseButton[2] = false;
}

HdPh_UnitTestGLDrawing::~HdPh_UnitTestGLDrawing()
{}

int HdPh_UnitTestGLDrawing::GetWidth() const
{
  return _widget->GetWidth();
}

int HdPh_UnitTestGLDrawing::GetHeight() const
{
  return _widget->GetHeight();
}

bool HdPh_UnitTestGLDrawing::WriteToFile(std::string const &attachment, std::string const &filename) const
{
  return _widget->WriteToFile(attachment, filename);
}

void HdPh_UnitTestGLDrawing::RunTest(int argc, char *argv[])
{
  bool offscreen = false;
  bool animate = false;
  for (int i = 0; i < argc; ++i)
  {
    if (std::string(argv[i]) == "--offscreen")
    {
      offscreen = true;
    }
    else if (std::string(argv[i]) == "--animate")
    {
      animate = true;
    }
  }

  this->ParseArgs(argc, argv);

  _widget = new HdPh_UnitTestWindow(this, 640, 480);
  _widget->Init();

  if (offscreen)
  {
    // no GUI mode (automated test)
    RunOffscreenTest();
  }
  else
  {
    // Interactive mode
    if (animate)
      _widget->StartTimer();
    _widget->Run();
  }
}

void HdPh_UnitTestGLDrawing::RunOffscreenTest()
{
  _widget->OffscreenTest();
}

/* virtual */
void HdPh_UnitTestGLDrawing::Idle()
{}

/* virtual */
void HdPh_UnitTestGLDrawing::ParseArgs(int argc, char *argv[])
{}

/* virtual */
void HdPh_UnitTestGLDrawing::UninitTest()
{}

/* virtual */
void HdPh_UnitTestGLDrawing::MousePress(int button, int x, int y, int modKeys)
{
  _mouseButton[button] = true;
  _mousePos[0] = x;
  _mousePos[1] = y;
}

/* virtual */
void HdPh_UnitTestGLDrawing::MouseRelease(int button, int x, int y, int modKeys)
{
  _mouseButton[button] = false;
}

/* virtual */
void HdPh_UnitTestGLDrawing::MouseMove(int x, int y, int modKeys)
{
  int dx = x - _mousePos[0];
  int dy = y - _mousePos[1];

  if (modKeys & GarchGLDebugWindow::Alt)
  {
    if (_mouseButton[0])
    {
      _rotate[1] += dx;
      _rotate[0] += dy;
    }
    else if (_mouseButton[1])
    {
      _translate[0] += 0.1 * dx;
      _translate[1] -= 0.1 * dy;
    }
    else if (_mouseButton[2])
    {
      _translate[2] += 0.1 * dx;
    }
  }

  _mousePos[0] = x;
  _mousePos[1] = y;
}

/* virtual */
void HdPh_UnitTestGLDrawing::KeyRelease(int key)
{}

GfMatrix4d HdPh_UnitTestGLDrawing::GetViewMatrix() const
{
  GfMatrix4d viewMatrix;
  viewMatrix.SetIdentity();
  // rotate from z-up to y-up
  viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(1.0, 0.0, 0.0), -90.0));
  viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(0, 1, 0), _rotate[1]));
  viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(1, 0, 0), _rotate[0]));
  viewMatrix *= GfMatrix4d().SetTranslate(GfVec3d(_translate[0], _translate[1], _translate[2]));

  return viewMatrix;
}

GfMatrix4d HdPh_UnitTestGLDrawing::GetProjectionMatrix() const
{
  return GetFrustum().ComputeProjectionMatrix();
}

GfFrustum HdPh_UnitTestGLDrawing::GetFrustum() const
{
  int width = GetWidth();
  int height = GetHeight();
  double aspectRatio = double(width) / height;

  GfFrustum frustum;
  frustum.SetPerspective(45.0, aspectRatio, 1, 100000.0);
  return frustum;
}

WABI_NAMESPACE_END
