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

#include "wabi/imaging/garch/glDebugWindow.h"
#include "wabi/base/arch/defines.h"
#include "wabi/imaging/garch/glPlatformDebugContext.h"

#if defined(ARCH_OS_LINUX)
#  include "wabi/imaging/garch/glPlatformDebugWindowGLX.h"
#elif defined(ARCH_OS_DARWIN)
#  include "wabi/imaging/garch/glPlatformDebugWindowDarwin.h"
#elif defined(ARCH_OS_WINDOWS)
#  include "wabi/imaging/garch/glPlatformDebugWindowWindows.h"
#endif

WABI_NAMESPACE_BEGIN

GarchGLDebugWindow::GarchGLDebugWindow(const char *title, int width, int height)
  : _title(title),
    _width(width),
    _height(height)
{
  _private = new Garch_GLPlatformDebugWindow(this);
}

GarchGLDebugWindow::~GarchGLDebugWindow()
{
  delete _private;
}

void GarchGLDebugWindow::Init()
{
  _private->Init(_title.c_str(), _width, _height);
}

void GarchGLDebugWindow::Run()
{
  _private->Run();
}

void GarchGLDebugWindow::ExitApp()
{
  _private->ExitApp();
}

/* virtual */
void GarchGLDebugWindow::OnInitializeGL()
{}

/* virtual */
void GarchGLDebugWindow::OnUninitializeGL()
{}

/* virtual */
void GarchGLDebugWindow::OnResize(int w, int h)
{
  _width = w;
  _height = h;
}

/* virtual */
void GarchGLDebugWindow::OnIdle()
{}

/* virtual */
void GarchGLDebugWindow::OnPaintGL()
{}

/* virtual */
void GarchGLDebugWindow::OnKeyRelease(int key)
{}

/* virtual */
void GarchGLDebugWindow::OnMousePress(int button, int x, int y, int modKeys)
{}

/* virtual */
void GarchGLDebugWindow::OnMouseRelease(int button, int x, int y, int modKeys)
{}

/* virtual */
void GarchGLDebugWindow::OnMouseMove(int x, int y, int modKeys)
{}

WABI_NAMESPACE_END
