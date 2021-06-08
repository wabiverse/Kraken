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
// context creation, etc.) (Requires: SDL 2.0. Prefer SDL 2.0.4+ for full feature support.)

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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2021-03-22: Rework global mouse pos availability check listing supported platforms explicitly,
//  effectively fixing mouse access on Raspberry Pi. (#2837, #3950) 2020-05-25: Misc: Report a zero
//  display-size when window is minimized, to be consistent with other backends. 2020-02-20:
//  Inputs: Fixed mapping for ANCHOR_Key_KeyPadEnter (using SDL_SCANCODE_KP_ENTER instead of
//  SDL_SCANCODE_RETURN2). 2019-12-17: Inputs: On Wayland, use SDL_GetMouseState (because there is
//  no global mouse state). 2019-12-05: Inputs: Added support for ANCHOR_MouseCursor_NotAllowed
//  mouse cursor. 2019-07-21: Inputs: Added mapping for ANCHOR_Key_KeyPadEnter. 2019-04-23: Inputs:
//  Added support for SDL_GameController (if ANCHORConfigFlags_NavEnableGamepad is set by user
//  application). 2019-03-12: Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-12-21: Inputs: Workaround for Android/iOS which don't seem to handle focus related calls.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-14: Changed the signature of ANCHOR_ImplSDL2_ProcessEvent() to take a 'const
//  SDL_Event*'. 2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus
//  related calls. 2018-06-29: Inputs: Added support for the ANCHOR_MouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted anchor_impl_sdl.cpp/.h away from the old combined
//  SDL2+OpenGL/Vulkan examples. 2018-06-08: Misc: ANCHOR_ImplSDL2_InitForOpenGL() now takes a
//  SDL_GLContext parameter. 2018-05-09: Misc: Fixed clipboard paste memory leak (we didn't call
//  SDL_FreeMemory on the data returned by SDL_GetClipboardText). 2018-03-20: Misc: Setup
//  io.BackendFlags ANCHORBackendFlags_HasMouseCursors flag + honor
//  ANCHORConfigFlags_NoMouseCursorChange flag. 2018-02-16: Inputs: Added support for mouse
//  cursors, honoring ANCHOR::GetMouseCursor() value. 2018-02-06: Misc: Removed call to
//  ANCHOR::Shutdown() which is not available from 1.60 WIP, user needs to call
//  CreateContext/DestroyContext themselves. 2018-02-06: Inputs: Added mapping for
//  ANCHOR_Key_Space. 2018-02-05: Misc: Using SDL_GetPerformanceCounter() instead of SDL_GetTicks()
//  to be able to handle very high framerate (1000+ FPS). 2018-02-05: Inputs: Keyboard mapping is
//  using scancodes everywhere instead of a confusing mixture of keycodes and scancodes.
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-19: Inputs: When available (SDL 2.0.4+) using SDL_CaptureMouse() to retrieve
//  coordinates outside of client area when dragging. Otherwise (SDL 2.0.3 and before) testing for
//  SDL_WINDOW_INPUT_FOCUS instead of SDL_WINDOW_MOUSE_FOCUS. 2018-01-18: Inputs: Added mapping for
//  ANCHOR_Key_Insert. 2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is
//  unavailable/missing (instead of -1,-1). 2016-10-15: Misc: Added a void* user_data parameter to
//  Clipboard function handlers.

#include "ANCHOR_impl_sdl.h"
#include "ANCHOR_api.h"

// SDL
#include <SDL.h>
#include <SDL_syswm.h>
#if defined(__APPLE__)
#  include "TargetConditionals.h"
#endif

#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE SDL_VERSION_ATLEAST(2, 0, 4)
#define SDL_HAS_VULKAN SDL_VERSION_ATLEAST(2, 0, 6)

WABI_NAMESPACE_USING

// Data
static SDL_Window *g_Window                                 = NULL;
static Uint64 g_Time                                        = 0;
static bool g_MousePressed[3]                               = {false, false, false};
static SDL_Cursor *g_MouseCursors[ANCHOR_MouseCursor_COUNT] = {};
static char *g_ClipboardTextData                            = NULL;
static bool g_MouseCanUseGlobalState                        = true;

static const char *ANCHOR_ImplSDL2_GetClipboardText(void *)
{
  if (g_ClipboardTextData)
    SDL_free(g_ClipboardTextData);
  g_ClipboardTextData = SDL_GetClipboardText();
  return g_ClipboardTextData;
}

static void ANCHOR_ImplSDL2_SetClipboardText(void *, const char *text)
{
  SDL_SetClipboardText(text);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if ANCHOR wants to
// use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
// application. Generally you may always pass all inputs to ANCHOR, and hide them from your
// application based on those two flags. If you have multiple SDL events and some of them are not
// meant to be used by ANCHOR, you may need to filter events based on their windowID field.
bool ANCHOR_ImplSDL2_ProcessEvent(const SDL_Event *event)
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  switch (event->type) {
    case SDL_MOUSEWHEEL: {
      if (event->wheel.x > 0)
        io.MouseWheelH += 1;
      if (event->wheel.x < 0)
        io.MouseWheelH -= 1;
      if (event->wheel.y > 0)
        io.MouseWheel += 1;
      if (event->wheel.y < 0)
        io.MouseWheel -= 1;
      return true;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (event->button.button == SDL_BUTTON_LEFT)
        g_MousePressed[0] = true;
      if (event->button.button == SDL_BUTTON_RIGHT)
        g_MousePressed[1] = true;
      if (event->button.button == SDL_BUTTON_MIDDLE)
        g_MousePressed[2] = true;
      return true;
    }
    case SDL_TEXTINPUT: {
      io.AddInputCharactersUTF8(event->text.text);
      return true;
    }
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      int key = event->key.keysym.scancode;
      ANCHOR_ASSERT(key >= 0 && key < ANCHOR_ARRAYSIZE(io.KeysDown));
      io.KeysDown[key] = (event->type == SDL_KEYDOWN);
      io.KeyShift      = ((SDL_GetModState() & KMOD_SHIFT) != 0);
      io.KeyCtrl       = ((SDL_GetModState() & KMOD_CTRL) != 0);
      io.KeyAlt        = ((SDL_GetModState() & KMOD_ALT) != 0);
#ifdef _WIN32
      io.KeySuper = false;
#else
      io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
#endif
      return true;
    }
  }
  return false;
}

static bool ANCHOR_ImplSDL2_Init(SDL_Window *window)
{
  g_Window = window;

  // Setup backend capabilities flags
  ANCHOR_IO &io = ANCHOR::GetIO();
  io.BackendFlags |=
      ANCHORBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
  io.BackendFlags |= ANCHORBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos
                                                         // requests (optional, rarely used)
  io.BackendPlatformName = "anchor_impl_sdl";

  // Keyboard mapping. ANCHOR will use those indices to peek into the io.KeysDown[] array.
  io.KeyMap[ANCHOR_Key_Tab]         = SDL_SCANCODE_TAB;
  io.KeyMap[ANCHOR_Key_LeftArrow]   = SDL_SCANCODE_LEFT;
  io.KeyMap[ANCHOR_Key_RightArrow]  = SDL_SCANCODE_RIGHT;
  io.KeyMap[ANCHOR_Key_UpArrow]     = SDL_SCANCODE_UP;
  io.KeyMap[ANCHOR_Key_DownArrow]   = SDL_SCANCODE_DOWN;
  io.KeyMap[ANCHOR_Key_PageUp]      = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ANCHOR_Key_PageDown]    = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ANCHOR_Key_Home]        = SDL_SCANCODE_HOME;
  io.KeyMap[ANCHOR_Key_End]         = SDL_SCANCODE_END;
  io.KeyMap[ANCHOR_Key_Insert]      = SDL_SCANCODE_INSERT;
  io.KeyMap[ANCHOR_Key_Delete]      = SDL_SCANCODE_DELETE;
  io.KeyMap[ANCHOR_Key_Backspace]   = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ANCHOR_Key_Space]       = SDL_SCANCODE_SPACE;
  io.KeyMap[ANCHOR_Key_Enter]       = SDL_SCANCODE_RETURN;
  io.KeyMap[ANCHOR_Key_Escape]      = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ANCHOR_Key_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
  io.KeyMap[ANCHOR_Key_A]           = SDL_SCANCODE_A;
  io.KeyMap[ANCHOR_Key_C]           = SDL_SCANCODE_C;
  io.KeyMap[ANCHOR_Key_V]           = SDL_SCANCODE_V;
  io.KeyMap[ANCHOR_Key_X]           = SDL_SCANCODE_X;
  io.KeyMap[ANCHOR_Key_Y]           = SDL_SCANCODE_Y;
  io.KeyMap[ANCHOR_Key_Z]           = SDL_SCANCODE_Z;

  io.SetClipboardTextFn = ANCHOR_ImplSDL2_SetClipboardText;
  io.GetClipboardTextFn = ANCHOR_ImplSDL2_GetClipboardText;
  io.ClipboardUserData  = NULL;

  // Load mouse cursors
  g_MouseCursors[ANCHOR_MouseCursor_Arrow]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  g_MouseCursors[ANCHOR_MouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  g_MouseCursors[ANCHOR_MouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  g_MouseCursors[ANCHOR_MouseCursor_ResizeNS]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  g_MouseCursors[ANCHOR_MouseCursor_ResizeEW]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  g_MouseCursors[ANCHOR_MouseCursor_ResizeNESW] = SDL_CreateSystemCursor(
      SDL_SYSTEM_CURSOR_SIZENESW);
  g_MouseCursors[ANCHOR_MouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(
      SDL_SYSTEM_CURSOR_SIZENWSE);
  g_MouseCursors[ANCHOR_MouseCursor_Hand]       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  g_MouseCursors[ANCHOR_MouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

  // Check and store if we are on a SDL backend that supports global mouse position
  // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a
  // black-list)
  const char *sdl_backend              = SDL_GetCurrentVideoDriver();
  const char *global_mouse_whitelist[] = {"windows", "cocoa", "x11", "DIVE", "VMAN"};
  g_MouseCanUseGlobalState             = false;
  for (int n = 0; n < ANCHOR_ARRAYSIZE(global_mouse_whitelist); n++)
    if (strncmp(sdl_backend, global_mouse_whitelist[n], strlen(global_mouse_whitelist[n])) == 0)
      g_MouseCanUseGlobalState = true;

#ifdef _WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  io.ImeWindowHandle = wmInfo.info.win.window;
#else
  (void)window;
#endif

  return true;
}

bool ANCHOR_ImplSDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context)
{
  (void)sdl_gl_context;  // Viewport branch will need this.
  return ANCHOR_ImplSDL2_Init(window);
}

bool ANCHOR_ImplSDL2_InitForVulkan(SDL_Window *window)
{
#if !SDL_HAS_VULKAN
  ANCHOR_ASSERT(0 && "Unsupported");
#endif
  return ANCHOR_ImplSDL2_Init(window);
}

bool ANCHOR_ImplSDL2_InitForD3D(SDL_Window *window)
{
#if !defined(_WIN32)
  ANCHOR_ASSERT(0 && "Unsupported");
#endif
  return ANCHOR_ImplSDL2_Init(window);
}

bool ANCHOR_ImplSDL2_InitForMetal(SDL_Window *window)
{
  return ANCHOR_ImplSDL2_Init(window);
}

void ANCHOR_ImplSDL2_Shutdown()
{
  g_Window = NULL;

  // Destroy last known clipboard data
  if (g_ClipboardTextData)
    SDL_free(g_ClipboardTextData);
  g_ClipboardTextData = NULL;

  // Destroy SDL mouse cursors
  for (ANCHOR_MouseCursor cursor_n = 0; cursor_n < ANCHOR_MouseCursor_COUNT; cursor_n++)
    SDL_FreeCursor(g_MouseCursors[cursor_n]);
  memset(g_MouseCursors, 0, sizeof(g_MouseCursors));
}

static void ANCHOR_ImplSDL2_UpdateMousePosAndButtons()
{
  ANCHOR_IO &io = ANCHOR::GetIO();

  // Set OS mouse position if requested (rarely used, only when
  // ANCHORConfigFlags_NavEnableSetMousePos is enabled by user)
  if (io.WantSetMousePos)
    SDL_WarpMouseInWindow(g_Window, (int)io.MousePos[0], (int)io.MousePos[1]);
  else
    io.MousePos = GfVec2f(-FLT_MAX, -FLT_MAX);

  int mx, my;
  Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
  io.MouseDown[0] =
      g_MousePressed[0] ||
      (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) !=
          0;  // If a mouse press event came, always pass it as "mouse held this frame", so we
              // don't miss click-release events that are shorter than 1 frame.
  io.MouseDown[1]   = g_MousePressed[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  io.MouseDown[2]   = g_MousePressed[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && \
    !(defined(__APPLE__) && TARGET_OS_IOS)
  SDL_Window *focused_window = SDL_GetKeyboardFocus();
  if (g_Window == focused_window) {
    if (g_MouseCanUseGlobalState) {
      // SDL_GetMouseState() gives mouse position seemingly based on the last window
      // entered/focused(?) The creation of a new windows at runtime and SDL_CaptureMouse both
      // seems to severely mess up with that, so we retrieve that position globally. Won't use this
      // workaround on SDL backends that have no global mouse position, like Wayland or RPI
      int wx, wy;
      SDL_GetWindowPosition(focused_window, &wx, &wy);
      SDL_GetGlobalMouseState(&mx, &my);
      mx -= wx;
      my -= wy;
    }
    io.MousePos = GfVec2f((float)mx, (float)my);
  }

  // SDL_CaptureMouse() let the OS know e.g. that our anchor drag outside the SDL window boundaries
  // shouldn't e.g. trigger the OS window resize cursor. The function is only supported from
  // SDL 2.0.4 (released Jan 2016)
  bool any_mouse_button_down = ANCHOR::IsAnyMouseDown();
  SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else
  if (SDL_GetWindowFlags(g_Window) & SDL_WINDOW_INPUT_FOCUS)
    io.MousePos = GfVec2f((float)mx, (float)my);
#endif
}

static void ANCHOR_ImplSDL2_UpdateMouseCursor()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  if (io.ConfigFlags & ANCHORConfigFlags_NoMouseCursorChange)
    return;

  ANCHOR_MouseCursor anchor_cursor = ANCHOR::GetMouseCursor();
  if (io.MouseDrawCursor || anchor_cursor == ANCHOR_MouseCursor_None) {
    // Hide OS mouse cursor if anchor is drawing it or if it wants no cursor
    SDL_ShowCursor(SDL_FALSE);
  }
  else {
    // Show OS mouse cursor
    SDL_SetCursor(g_MouseCursors[anchor_cursor] ? g_MouseCursors[anchor_cursor] :
                                                  g_MouseCursors[ANCHOR_MouseCursor_Arrow]);
    SDL_ShowCursor(SDL_TRUE);
  }
}

static void ANCHOR_ImplSDL2_UpdateGamepads()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  memset(io.NavInputs, 0, sizeof(io.NavInputs));
  if ((io.ConfigFlags & ANCHORConfigFlags_NavEnableGamepad) == 0)
    return;

  // Get gamepad
  SDL_GameController *game_controller = SDL_GameControllerOpen(0);
  if (!game_controller) {
    io.BackendFlags &= ~ANCHORBackendFlags_HasGamepad;
    return;
  }

// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO) \
  { \
    io.NavInputs[NAV_NO] = (SDL_GameControllerGetButton(game_controller, BUTTON_NO) != 0) ? \
                               1.0f : \
                               0.0f; \
  }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) \
  { \
    float vn = (float)(SDL_GameControllerGetAxis(game_controller, AXIS_NO) - V0) / \
               (float)(V1 - V0); \
    if (vn > 1.0f) \
      vn = 1.0f; \
    if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) \
      io.NavInputs[NAV_NO] = vn; \
  }
  const int thumb_dead_zone = 8000;  // SDL_gamecontroller.h suggests using this value.
  MAP_BUTTON(ANCHOR_NavInput_Activate, SDL_CONTROLLER_BUTTON_A);               // Cross / A
  MAP_BUTTON(ANCHOR_NavInput_Cancel, SDL_CONTROLLER_BUTTON_B);                 // Circle / B
  MAP_BUTTON(ANCHOR_NavInput_Menu, SDL_CONTROLLER_BUTTON_X);                   // Square / X
  MAP_BUTTON(ANCHOR_NavInput_Input, SDL_CONTROLLER_BUTTON_Y);                  // Triangle / Y
  MAP_BUTTON(ANCHOR_NavInput_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);       // D-Pad Left
  MAP_BUTTON(ANCHOR_NavInput_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);     // D-Pad Right
  MAP_BUTTON(ANCHOR_NavInput_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);           // D-Pad Up
  MAP_BUTTON(ANCHOR_NavInput_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);       // D-Pad Down
  MAP_BUTTON(ANCHOR_NavInput_FocusPrev, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);   // L1 / LB
  MAP_BUTTON(ANCHOR_NavInput_FocusNext, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);  // R1 / RB
  MAP_BUTTON(ANCHOR_NavInput_TweakSlow, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);   // L1 / LB
  MAP_BUTTON(ANCHOR_NavInput_TweakFast, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);  // R1 / RB
  MAP_ANALOG(ANCHOR_NavInput_LStickLeft, SDL_CONTROLLER_AXIS_LEFTX, -thumb_dead_zone, -32768);
  MAP_ANALOG(ANCHOR_NavInput_LStickRight, SDL_CONTROLLER_AXIS_LEFTX, +thumb_dead_zone, +32767);
  MAP_ANALOG(ANCHOR_NavInput_LStickUp, SDL_CONTROLLER_AXIS_LEFTY, -thumb_dead_zone, -32767);
  MAP_ANALOG(ANCHOR_NavInput_LStickDown, SDL_CONTROLLER_AXIS_LEFTY, +thumb_dead_zone, +32767);

  io.BackendFlags |= ANCHORBackendFlags_HasGamepad;
#undef MAP_BUTTON
#undef MAP_ANALOG
}

void ANCHOR_ImplSDL2_NewFrame(SDL_Window *window)
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ASSERT(io.Fonts->IsBuilt() &&
                "Font atlas not built! It is generally built by the renderer backend. Missing "
                "call to renderer _NewFrame() function? e.g. ANCHOR_ImplOpenGL3_NewFrame().");

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  SDL_GetWindowSize(window, &w, &h);
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
    w = h = 0;
  SDL_GL_GetDrawableSize(window, &display_w, &display_h);
  io.DisplaySize = GfVec2f((float)w, (float)h);
  if (w > 0 && h > 0)
    io.DisplayFramebufferScale = GfVec2f((float)display_w / w, (float)display_h / h);

  // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
  static Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 current_time     = SDL_GetPerformanceCounter();
  io.DeltaTime            = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) :
                                         (float)(1.0f / 60.0f);
  g_Time                  = current_time;

  ANCHOR_ImplSDL2_UpdateMousePosAndButtons();
  ANCHOR_ImplSDL2_UpdateMouseCursor();

  // Update game controllers (if enabled and available)
  ANCHOR_ImplSDL2_UpdateGamepads();
}
