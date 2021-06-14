/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

// ANCHOR: Platform Backend for SDL2
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)
// (Info: SDL2 is a cross-platform general purpose library for handling windows, inputs, graphics
// context creation, etc.)

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |=
//  ANCHORConfigFlags_NoMouseCursorChange'. [X] Platform: Clipboard support. [X] Platform: Keyboard
//  arrays indexed using SDL_SCANCODE_* codes, e.g. ANCHOR::IsKeyPressed(SDL_SCANCODE_SPACE). [X]
//  Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ANCHORConfigFlags_NavEnableGamepad'.
// Missing features:
//  [ ] Platform: SDL2 handling of IME under Windows appears to be broken and it explicitly disable
//  the regular Windows IME. You can restore Windows IME by compiling SDL with
//  SDL_DISABLE_WINDOWS_IME.

// You can use unmodified anchor_impl_* files in your project. See examples/ folder for examples of
// using this. Prefer including the entire anchor/ repository into your project (either as a copy
// or as a submodule), and only build the backends you need. If you are new to ANCHOR, read
// documentation from the docs/ folder + read the top of anchor.cpp. Read online:
// https://github.com/ocornut/anchor/tree/master/docs

#pragma once

#include "ANCHOR_api.h"  // ANCHOR_IMPL_API
#include "ANCHOR_system.h"
#include "ANCHOR_window.h"

struct SDL_Window;
typedef union SDL_Event SDL_Event;

class ANCHOR_SystemSDL : public ANCHOR_System {

  ANCHOR_SystemSDL();
  ~ANCHOR_SystemSDL();

  ANCHOR_IMPL_API
  static bool ANCHOR_ImplSDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);

  ANCHOR_IMPL_API
  static bool ANCHOR_ImplSDL2_InitForVulkan(SDL_Window *window);

  ANCHOR_IMPL_API
  static bool ANCHOR_ImplSDL2_InitForD3D(SDL_Window *window);

  ANCHOR_IMPL_API
  static bool ANCHOR_ImplSDL2_InitForMetal(SDL_Window *window);

  ANCHOR_IMPL_API
  static void ANCHOR_ImplSDL2_Shutdown();

  ANCHOR_IMPL_API
  static void ANCHOR_ImplSDL2_NewFrame(SDL_Window *window);

  ANCHOR_IMPL_API
  static bool ANCHOR_ImplSDL2_ProcessEvent(const SDL_Event *event);

 private:
  ANCHOR_ISystemWindow *createWindow(const char *title,
                                     AnchorS32 left,
                                     AnchorS32 top,
                                     AnchorU32 width,
                                     AnchorU32 height,
                                     eAnchorWindowState state,
                                     eAnchorDrawingContextType type,
                                     int vkSettings,
                                     const bool exclusive                     = false,
                                     const bool is_dialog                     = false,
                                     const ANCHOR_ISystemWindow *parentWindow = NULL);
};

class ANCHOR_WindowSDL : public ANCHOR_SystemWindow {
 private:
  ANCHOR_SystemSDL *m_system;
  SDL_Window *m_sdl_win;
  bool m_valid_setup;
  bool m_invalid_window;

  SDL_Window *m_sdl_win;
  SDL_Cursor *m_sdl_custom_cursor;

 public:
  ANCHOR_WindowSDL(ANCHOR_SystemSDL *system,
                   const char *title,
                   const char *icon,
                   AnchorS32 left,
                   AnchorS32 top,
                   AnchorU32 width,
                   AnchorU32 height,
                   eAnchorWindowState state,
                   eAnchorDrawingContextType type           = 0,
                   const bool stereoVisual                  = false,
                   const bool exclusive                     = false,
                   const ANCHOR_ISystemWindow *parentWindow = NULL);

  ~ANCHOR_WindowSDL();

  /* SDL specific */
  SDL_Window *getSDLWindow()
  {
    return m_sdl_win;
  }

 protected:
  void setTitle(const char *title);
  void setIcon(const char *icon);
};