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
 * ⚓︎ Anchor.
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
class AnchorWindowSDL;

class AnchorSystemSDL : public AnchorSystem
{
 public:
  AnchorSystemSDL();
  ~AnchorSystemSDL();

  bool processEvents(bool waitForEvent);

  eAnchorStatus getModifierKeys(AnchorModifierKeys &keys) const;

  eAnchorStatus getButtons(AnchorButtons &buttons) const;

  void getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  void getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  static bool ANCHOR_ImplSDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);
  static bool ANCHOR_ImplSDL2_InitForVulkan(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_InitForD3D(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_InitForMetal(SDL_Window *window);
  static void ANCHOR_ImplSDL2_Shutdown();
  static void ANCHOR_ImplSDL2_NewFrame(SDL_Window *window);
  static bool ANCHOR_ImplSDL2_ProcessEvent(const SDL_Event *event);

 private:
  eAnchorStatus init();

  AnchorISystemWindow *createWindow(const char *title,
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
                                    const AnchorISystemWindow *parentWindow = NULL);

  AnchorWindowSDL *findAnchorWindow(SDL_Window *sdl_win);

  bool generateWindowExposeEvents();

  void processEvent(SDL_Event *sdl_event);

  eAnchorStatus getCursorPosition(AnchorS32 &x, AnchorS32 &y) const;

  /** The vector of windows that need to be updated. */
  // TODO std::vector<AnchorWindowSDL *> m_dirty_windows;
  AnchorWindowSDL *m_sdl_window;
};

class AnchorDisplayManagerSDL : public AnchorDisplayManager
{
 public:
  AnchorDisplayManagerSDL(AnchorSystemSDL *system);

  eAnchorStatus getNumDisplays(AnchorU8 &numDisplays) const;

  eAnchorStatus getNumDisplaySettings(AnchorU8 display, AnchorS32 &numSettings) const;

  eAnchorStatus getDisplaySetting(AnchorU8 display, AnchorS32 index, ANCHOR_DisplaySetting &setting) const;

  eAnchorStatus getCurrentDisplaySetting(AnchorU8 display, ANCHOR_DisplaySetting &setting) const;

  eAnchorStatus getCurrentDisplayModeSDL(SDL_DisplayMode &mode) const;

  eAnchorStatus setCurrentDisplaySetting(AnchorU8 display, const ANCHOR_DisplaySetting &setting);

 private:
  AnchorSystemSDL *m_system;
  SDL_DisplayMode m_mode;
};

class AnchorWindowSDL : public AnchorSystemWindow
{
 private:
  AnchorSystemSDL *m_system;
  bool m_valid_setup;
  bool m_invalid_window;

  SDL_Window *m_sdl_win;
  SDL_Cursor *m_sdl_custom_cursor;

  ANCHOR_VulkanGPU_Surface *m_vulkan_context;

 public:
  AnchorWindowSDL(AnchorSystemSDL *system,
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
                  const AnchorISystemWindow *parentWindow = NULL);

  ~AnchorWindowSDL();

  std::string getTitle() const;

  /* SDL specific */
  SDL_Window *getSDLWindow()
  {
    return m_sdl_win;
  }

  ANCHOR_VulkanGPU_Surface *getVulkanSurface()
  {
    return m_vulkan_context;
  }

  ANCHOR_VulkanGPU_Surface *updateVulkanSurface(ANCHOR_VulkanGPU_Surface *data)
  {
    m_vulkan_context = data;
  }

  bool getValid() const;

  void getClientBounds(AnchorRect &bounds) const;

 protected:
  /**
   * @param type: The type of rendering context create.
   * @return Indication of success. */
  void newDrawingContext(eAnchorDrawingContextType type);

  /**
   * Swaps front and back buffers of a window.
   * @return A boolean success indicator. */
  eAnchorStatus swapBuffers();

  void screenToClient(AnchorS32 inX,
                      AnchorS32 inY,
                      AnchorS32 &outX,
                      AnchorS32 &outY) const;

  void setTitle(const char *title);
  void setIcon(const char *icon);

  void clientToScreen(AnchorS32 inX,
                      AnchorS32 inY,
                      AnchorS32 &outX,
                      AnchorS32 &outY) const;

  eAnchorStatus setClientSize(AnchorU32 width, AnchorU32 height);

  eAnchorStatus setState(eAnchorWindowState state);

  eAnchorWindowState getState() const;

  eAnchorStatus setOrder(eAnchorWindowOrder order)
  {
    // TODO
    return ANCHOR_SUCCESS;
  }

  eAnchorStatus beginFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

  eAnchorStatus endFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

  AnchorU16 getDPIHint();
};