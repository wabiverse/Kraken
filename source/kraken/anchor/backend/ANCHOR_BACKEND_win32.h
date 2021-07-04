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

#pragma once

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

#include "ANCHOR_BACKEND_vulkan.h"
#include "ANCHOR_api.h"
#include "ANCHOR_modifier_keys.h"
#include "ANCHOR_system.h"

#ifdef _WIN32

#  define WIN32_LEAN_AND_MEAN
#  include <ole2.h>  // for drag-n-drop
#  include <windows.h>

class ANCHOR_WindowWin32;

enum eAnchorMouseCaptureEventWin32 {
  MousePressed,
  MouseReleased,
  OperatorGrab,
  OperatorUngrab
};

struct ANCHOR_PointerInfoWin32 {
  AnchorS32 pointerId;
  AnchorS32 isPrimary;
  eAnchorButtonMask buttonMask;
  POINT pixelLocation;
  AnchorU64 time;
  ANCHOR_TabletData tabletData;
};

ANCHOR_BACKEND_API bool ANCHOR_ImplWin32_Init(void *hwnd);
ANCHOR_BACKEND_API void ANCHOR_ImplWin32_Shutdown();
ANCHOR_BACKEND_API void ANCHOR_ImplWin32_NewFrame();

// DPI-related helpers (optional)
// - Use to enable DPI awareness without having to create an application manifest.
ANCHOR_BACKEND_API void ANCHOR_ImplWin32_EnableDpiAwareness();
ANCHOR_BACKEND_API float ANCHOR_ImplWin32_GetDpiScaleForHwnd(void *hwnd);        // HWND hwnd
ANCHOR_BACKEND_API float ANCHOR_ImplWin32_GetDpiScaleForMonitor(void *monitor);  // HMONITOR monitor

// Transparency related helpers (optional) [experimental]
// - Use to enable alpha compositing transparency with the desktop.
// - Use together with e.g. clearing your framebuffer with zero-alpha.
ANCHOR_BACKEND_API void ANCHOR_ImplWin32_EnableAlphaCompositing(void *hwnd);  // HWND hwnd


class ANCHOR_SystemWin32 : public ANCHOR_System
{
 public:
  ANCHOR_SystemWin32();
  ~ANCHOR_SystemWin32();

  bool processEvents(bool waitForEvent);

  AnchorU64 performanceCounterToMillis(__int64 perf_ticks) const;

  /**
   * Updates the location of the cursor (location in screen coordinates).
   * @param x: The x-coordinate of the cursor.
   * @param y: The y-coordinate of the cursor.
   * @return Indication of success. */
  eAnchorStatus setCursorPosition(AnchorS32 x, AnchorS32 y);

  eAnchorStatus getModifierKeys(ANCHOR_ModifierKeys &keys) const;

  eAnchorStatus getButtons(ANCHOR_Buttons &buttons) const;

  void getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  void getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  /**
   * Creates a window event.
   * @param type: The type of event to create.
   * @param window: The window receiving the event (the active window).
   * @return The event created. */
  static ANCHOR_Event *processWindowEvent(eAnchorEventType type, ANCHOR_WindowWin32 *window);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static void processPointerEvent(UINT type, ANCHOR_WindowWin32 *window, WPARAM wParam, LPARAM lParam, bool &eventhandled);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static ANCHOR_EventCursor *processCursorEvent(ANCHOR_WindowWin32 *window);

  /**
   * Handles a mouse wheel event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc. */
  static void processWheelEvent(ANCHOR_WindowWin32 *window, WPARAM wParam, LPARAM lParam);

  /**
   * Handles minimum window size.
   * @param minmax: The MINMAXINFO structure. */
  static void processMinMaxInfo(MINMAXINFO *minmax);

  /**
   * Windows call back routine for our window class. */
  static LRESULT WINAPI s_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  /**
   * Returns the local state of the modifier keys (from the message queue).
   * @param keys: The state of the keys. */
  inline void retrieveModifierKeys(ANCHOR_ModifierKeys &keys) const;

  /**
   * Stores the state of the modifier keys locally.
   * For internal use only!
   * param keys The new state of the modifier keys. */
  inline void storeModifierKeys(const ANCHOR_ModifierKeys &keys);

  /**
   * Check current key layout for AltGr. */
  inline void handleKeyboardChange(void);

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

  bool generateWindowExposeEvents();

  eAnchorStatus getCursorPosition(AnchorS32 &x, AnchorS32 &y) const;

  /** The vector of windows that need to be updated. */
  // TODO std::vector<ANCHOR_WindowWin32 *> m_dirty_windows;
  ANCHOR_WindowWin32 *m_win32_window;

 protected:
  /**
   * Toggles console
   * @param action:
   * - 0 - Hides
   * - 1 - Shows
   * - 2 - Toggles
   * - 3 - Hides if it runs not from  command line
   * - * - Does nothing
   * @return current status (1 -visible, 0 - hidden) */
  int toggleConsole(int action);

  static ANCHOR_EventKey *processKeyEvent(ANCHOR_WindowWin32 *window, RAWINPUT const &raw);

  static ANCHOR_Event *processWindowSizeEvent(ANCHOR_WindowWin32 *window);

  /**
   * Creates mouse button event.
   * @param type: The type of event to create.
   * @param window: The window receiving the event (the active window).
   * @param mask: The button mask of this event.
   * @return The event created. */
  static ANCHOR_EventButton *processButtonEvent(eAnchorEventType type,
                                                ANCHOR_WindowWin32 *window,
                                                eAnchorButtonMask mask);

  /**
   * Converts raw WIN32 key codes from the wndproc to GHOST keys.
   * @param vKey: The virtual key from #hardKey.
   * @param ScanCode: The ScanCode of pressed key (similar to PS/2 Set 1).
   * @param extend: Flag if key is not primly (left or right).
   * @return The GHOST key (GHOST_kKeyUnknown if no match). */
  eAnchorKey convertKey(short vKey, short ScanCode, short extend) const;

  /**
   * Catches raw WIN32 key codes from WM_INPUT in the wndproc.
   * @param raw: RawInput structure with detailed info about the key event.
   * @param keyDown: Pointer flag that specify if a key is down.
   * @param vk: Pointer to virtual key.
   * @return The GHOST key (GHOST_kKeyUnknown if no match). */
  eAnchorKey hardKey(RAWINPUT const &raw, bool *r_keyDown, bool *r_is_repeated_modifier);

  eAnchorKey processSpecialKey(short vKey, short scanCode) const;

  /** The current state of the modifier keys. */
  ANCHOR_ModifierKeys m_modifierKeys;
  /** The virtual-key code (VKey) of the last press event. Used to detect repeat events. */
  unsigned short m_keycode_last_repeat_key;
  /** State variable set at initialization. */
  bool m_hasPerformanceCounter;
  /** High frequency timer variable. */
  __int64 m_freq;
  /** High frequency timer variable. */
  __int64 m_start;
  /** Low frequency timer variable. */
  __int64 m_lfstart;
  /** AltGr on current keyboard layout. */
  bool m_hasAltGr;
  /** Language identifier. */
  WORD m_langId;
  /** Stores keyboard layout. */
  HKL m_keylayout;

  /** Console status. */
  int m_consoleStatus;

  /** Wheel delta accumulator. */
  int m_wheelDeltaAccum;
};


inline void ANCHOR_SystemWin32::retrieveModifierKeys(ANCHOR_ModifierKeys &keys) const
{
  keys = m_modifierKeys;
}

inline void ANCHOR_SystemWin32::storeModifierKeys(const ANCHOR_ModifierKeys &keys)
{
  m_modifierKeys = keys;
}

inline void ANCHOR_SystemWin32::handleKeyboardChange(void)
{
  m_keylayout = GetKeyboardLayout(0);  // get keylayout for current thread
  int i;
  SHORT s;

  // save the language identifier.
  m_langId = LOWORD(m_keylayout);

  for (m_hasAltGr = false, i = 32; i < 256; ++i)
  {
    s = VkKeyScanEx((char)i, m_keylayout);
    // s == -1 means no key that translates passed char code
    // high byte contains shift state. bit 2 ctrl pressed, bit 4 alt pressed
    // if both are pressed, we have AltGr keycombo on keylayout
    if (s != -1 && (s & 0x600) == 0x600)
    {
      m_hasAltGr = true;
      break;
    }
  }
}


/**
 * Manages system displays  (WIN32 implementation). */
class ANCHOR_DisplayManagerWin32 : public ANCHOR_DisplayManager
{
 public:
  /**
   * Constructor.*/
  ANCHOR_DisplayManagerWin32(void);

  /**
   * Returns the number of display devices on this system.
   * @param numDisplays: The number of displays on this system.
   * @return Indication of success. */
  eAnchorStatus getNumDisplays(AnchorU8 &numDisplays) const;

  /**
   * Returns the number of display settings for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param numSetting: The number of settings of the display device with this index.
   * @return Indication of success. */
  eAnchorStatus getNumDisplaySettings(AnchorU8 display, AnchorS32 &numSettings) const;

  /**
   * Returns the current setting for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param index: The setting index to be returned.
   * @param setting: The setting of the display device with this index.
   * @return Indication of success. */
  eAnchorStatus getDisplaySetting(AnchorU8 display,
                                  AnchorS32 index,
                                  ANCHOR_DisplaySetting &setting) const;

  /**
   * Returns the current setting for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param setting: The current setting of the display device with this index.
   * @return Indication of success. */
  eAnchorStatus getCurrentDisplaySetting(AnchorU8 display,
                                         ANCHOR_DisplaySetting &setting) const;

  /**
   * Changes the current setting for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param setting: The current setting of the display device with this index.
   * @return Indication of success. */
  eAnchorStatus setCurrentDisplaySetting(AnchorU8 display,
                                         const ANCHOR_DisplaySetting &setting);
};


class ANCHOR_WindowWin32 : public ANCHOR_SystemWindow
{
 private:
  ANCHOR_SystemWin32 *m_system;
  bool m_valid_setup;
  bool m_invalid_window;

  ANCHOR_VulkanGPU_Surface *m_vulkan_context;

  /** Most recent tablet data. */
  ANCHOR_TabletData m_lastPointerTabletData;

  /** Window handle. */
  HWND m_hWnd;

 public:
  ANCHOR_WindowWin32(ANCHOR_SystemWin32 *system,
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

  ~ANCHOR_WindowWin32();

  std::string getTitle() const;

  /* Windows specific */
  //   HWND *getHWND();

  ANCHOR_VulkanGPU_Surface *getVulkanSurface()
  {
    return m_vulkan_context;
  }

  ANCHOR_VulkanGPU_Surface *updateVulkanSurface(ANCHOR_VulkanGPU_Surface *data)
  {
    m_vulkan_context = data;
  }

  bool getValid() const;

  void getClientBounds(ANCHOR_Rect &bounds) const;

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

  /**
   * Register a mouse capture state (should be called
   * for any real button press, controls mouse
   * capturing).
   *
   * @param event: Whether mouse was pressed and released,
   * or an operator grabbed or ungrabbed the mouse. */
  void updateMouseCapture(eAnchorMouseCaptureEventWin32 event);

  /**
   * Resets pointer pen tablet state. */
  void resetPointerPenInfo();

  /**
   * Get the most recent Windows Pointer tablet data.
   * @return Most recent pointer tablet data. */
  ANCHOR_TabletData getTabletData(void);

  void lostMouseCapture(void);

  eAnchorStatus setWindowCursorShape(eAnchorStandardCursor cursorShape);

  /**
   * Loads the windows equivalent of a standard ANCHOR cursor.
   * @param visible: Flag for cursor visibility.
   * @param cursorShape: The cursor shape. */
  HCURSOR getStandardCursor(eAnchorStandardCursor shape) const;
  void loadCursor(bool visible, eAnchorStandardCursor cursorShape) const;

  eAnchorStatus getPointerInfo(std::vector<ANCHOR_PointerInfoWin32> &outPointerInfo,
                               WPARAM wParam,
                               LPARAM lParam);

  eAnchorStatus setOrder(eAnchorWindowOrder order)
  {
    // TODO
    return ANCHOR_SUCCESS;
  }

  eAnchorStatus beginFullScreen() const
  {
    return ANCHOR_ERROR;
  }

  eAnchorStatus endFullScreen() const
  {
    return ANCHOR_ERROR;
  }

  AnchorU16 getDPIHint();

 private:
  /** 
   * True if the window currently resizing. */
  bool m_inLiveResize;

  /** 
   * True if the mouse is either over or captured by the window. */
  bool m_mousePresent;

  bool m_hasGrabMouse;
  int m_nPressedButtons;
  bool m_hasMouseCaptured;

  /** 
   * Most recent tablet data. */
  ANCHOR_TabletData m_lastPointerTabletData;

  /** 
   * HCURSOR structure of the custom cursor. */
  HCURSOR m_customCursor;
};



#elif

/* This file is ignored on all platforms outside Windows. */

#endif /* _WIN32 */