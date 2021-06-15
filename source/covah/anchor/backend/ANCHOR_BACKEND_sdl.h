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

#pragma once

#include "ANCHOR_api.h"
#include "ANCHOR_display_manager.h"
#include "ANCHOR_event.h"
#include "ANCHOR_system.h"
#include "ANCHOR_window.h"

#include <SDL.h>

/** Vulkan Surface Forward -> */
struct ANCHOR_VulkanGPU_Surface;

/** SDL Forwards -> */
struct SDL_Cursor;
struct SDL_Window;

typedef union SDL_Event SDL_Event;

/** SDL Window Forward -> */
class ANCHOR_WindowSDL;

class ANCHOR_SystemSDL : public ANCHOR_System {
 public:
  ANCHOR_SystemSDL();
  ~ANCHOR_SystemSDL();

  bool processEvents(bool waitForEvent);

  eAnchorStatus getModifierKeys(ANCHOR_ModifierKeys &keys) const;

  eAnchorStatus getButtons(ANCHOR_Buttons &buttons) const;

  static bool ANCHOR_ImplSDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);
  static bool ANCHOR_ImplSDL2_InitForVulkan(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_InitForD3D(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_InitForMetal(SDL_Window *window);
  static void ANCHOR_ImplSDL2_Shutdown();
  static void ANCHOR_ImplSDL2_NewFrame(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_ProcessEvent(const SDL_Event *event);

 private:
  eAnchorStatus init();

  ANCHOR_ISystemWindow *createWindow(const char *title,
                                     const char *icon,
                                     AnchorS32 left,
                                     AnchorS32 top,
                                     AnchorU32 width,
                                     AnchorU32 height,
                                     eAnchorWindowState state,
                                     eAnchorDrawingContextType type,
                                     int vkSettings,
                                     const bool exclusive = false,
                                     const bool is_dialog = false,
                                     const ANCHOR_ISystemWindow *parentWindow = NULL);

  ANCHOR_WindowSDL *findAnchorWindow(SDL_Window *sdl_win);

  bool generateWindowExposeEvents();

  void processEvent(SDL_Event *sdl_event);

  /** The vector of windows that need to be updated. */
  // TODO std::vector<ANCHOR_WindowSDL *> m_dirty_windows;
  ANCHOR_WindowSDL *m_sdl_window;
};

class ANCHOR_DisplayManagerSDL : public ANCHOR_DisplayManager {
 public:
  ANCHOR_DisplayManagerSDL(ANCHOR_SystemSDL *system);

  eAnchorStatus getNumDisplays(AnchorU8 &numDisplays) const;

  eAnchorStatus getNumDisplaySettings(AnchorU8 display, AnchorS32 &numSettings) const;

  eAnchorStatus getDisplaySetting(AnchorU8 display, AnchorS32 index, ANCHOR_DisplaySetting &setting) const;

  eAnchorStatus getCurrentDisplaySetting(AnchorU8 display, ANCHOR_DisplaySetting &setting) const;

  eAnchorStatus getCurrentDisplayModeSDL(SDL_DisplayMode &mode) const;

  eAnchorStatus setCurrentDisplaySetting(AnchorU8 display, const ANCHOR_DisplaySetting &setting);

 private:
  ANCHOR_SystemSDL *m_system;
  SDL_DisplayMode m_mode;
};

class ANCHOR_WindowSDL : public ANCHOR_SystemWindow {
 private:
  ANCHOR_SystemSDL *m_system;
  bool m_valid_setup;
  bool m_invalid_window;

  SDL_Window *m_sdl_win;
  SDL_Cursor *m_sdl_custom_cursor;

  ANCHOR_VulkanGPU_Surface *m_vulkan_context;

 public:
  ANCHOR_WindowSDL(ANCHOR_SystemSDL *system,
                   const char *title,
                   const char *icon,
                   AnchorS32 left,
                   AnchorS32 top,
                   AnchorU32 width,
                   AnchorU32 height,
                   eAnchorWindowState state,
                   eAnchorDrawingContextType type = ANCHOR_DrawingContextTypeNone,
                   const bool stereoVisual = false,
                   const bool exclusive = false,
                   const ANCHOR_ISystemWindow *parentWindow = NULL);

  ~ANCHOR_WindowSDL();

  void frameUpdate();

  /* SDL specific */
  SDL_Window *getSDLWindow()
  {
    return m_sdl_win;
  }

 protected:
  /**
   * @param type: The type of rendering context create.
   * @return Indication of success. */
  ANCHOR_Context *newDrawingContext(eAnchorDrawingContextType type);

  /**
   * Swaps front and back buffers of a window.
   * @return A boolean success indicator. */
  eAnchorStatus swapBuffers();

  void setTitle(const char *title);
  void setIcon(const char *icon);

  eAnchorStatus beginFullScreen() const
  {
    return ANCHOR_ERROR;
  }

  eAnchorStatus endFullScreen() const
  {
    return ANCHOR_ERROR;
  }
};