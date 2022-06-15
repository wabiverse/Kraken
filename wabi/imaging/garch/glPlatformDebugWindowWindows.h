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
#ifndef WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_WINDOW_WINDOWS_H
#define WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_WINDOW_WINDOWS_H

#include "wabi/wabi.h"
#include <Windows.h>

WABI_NAMESPACE_BEGIN

class GarchGLDebugWindow;

#ifndef WINAPI_PARTITION_DESKTOP

/// \class Garch_GLPlatformDebugWindow
///
class Garch_GLPlatformDebugWindow
{
 public:

  Garch_GLPlatformDebugWindow(GarchGLDebugWindow *w);

  void Init(const char *title, int width, int height, int nSamples = 1);
  void Run();
  void ExitApp();

 private:

  static Garch_GLPlatformDebugWindow *_GetWindowByHandle(HWND);
  static LRESULT WINAPI _MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  bool _running;
  GarchGLDebugWindow *_callback;
  HWND _hWND;
  HDC _hDC;
  HGLRC _hGLRC;
  static LPCTSTR _className;
};

#else /* WINAPI_PARTITION_DESKTOP */

class Garch_GLPlatformDebugWindow
{
 public:

  Garch_GLPlatformDebugWindow(GarchGLDebugWindow *w) {}

  void Init(const char *title, int width, int height, int nSamples = 1) {}

  void Run() {}

  void ExitApp() {}

 private:

  static Garch_GLPlatformDebugWindow *_GetWindowByHandle(HWND)
  {
    return nullptr;
  }

  static LRESULT WINAPI _MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return NULL;
  }

  bool _running;
  GarchGLDebugWindow *_callback;
  HWND _hWND;
  HDC _hDC;
  HGLRC _hGLRC;
  static LPCTSTR _className;
};

#endif /* WINAPI_PARTITION_DESKTOP */

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_WINDOW_WINDOWS_H
