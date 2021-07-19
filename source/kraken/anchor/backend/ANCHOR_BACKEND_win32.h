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
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_BACKEND_vulkan.h"
#include "ANCHOR_api.h"
#include "ANCHOR_modifier_keys.h"
#include "ANCHOR_system.h"

#ifdef _WIN32

#  define VK_USE_PLATFORM_WIN32_KHR
#  include <vulkan/vulkan.h>

#  include <wabi/imaging/hgiVulkan/vulkan.h>
#  include <wabi/imaging/hgiVulkan/capabilities.h>

#  define WIN32_LEAN_AND_MEAN
#  include <ole2.h>  // for drag-n-drop
#  include <shlobj.h>
#  include <windows.h>

class AnchorWindowWin32;

enum eAnchorMouseCaptureEventWin32
{
  MousePressed,
  MouseReleased,
  OperatorGrab,
  OperatorUngrab
};

typedef UINT(WINAPI *AnchorGetDpiForWindowCallback)(HWND);
typedef BOOL(WINAPI *AnchorAdjustWindowRectExForDpiCallback)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

struct AnchorBackendWin32PointerInfo
{
  AnchorS32 pointerId;
  AnchorS32 isPrimary;
  eAnchorButtonMask buttonMask;
  POINT pixelLocation;
  AnchorU64 time;
  AnchorTabletData tabletData;
};

class AnchorSystemWin32 : public AnchorSystem
{
 public:
  AnchorSystemWin32();
  ~AnchorSystemWin32();

  /**
   * This method converts performance counter measurements into milliseconds since the start of the
   * system process.
   * @return The number of milliseconds since the start of the system process. */
  AnchorU64 performanceCounterToMillis(__int64 perf_ticks) const;

  /**
   * This method converts system ticks into milliseconds since the start of the
   * system process.
   * @return The number of milliseconds since the start of the system process. */
  AnchorU64 tickCountToMillis(__int64 ticks) const;

  /**
   * Returns the system time.
   * Returns the number of milliseconds since the start of the system process.
   * This overloaded method uses the high frequency timer if available.
   * @return The number of milliseconds. */
  AnchorU64 getMilliSeconds() const;

  bool processEvents(bool waitForEvent);

  eAnchorStatus getModifierKeys(AnchorModifierKeys &keys) const;

  eAnchorStatus getButtons(AnchorButtons &buttons) const;

  AnchorU8 getNumDisplays() const;

  void getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  void getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const;

  /**
   * Creates a window event.
   * @param type: The type of event to create.
   * @param window: The window receiving the event (the active window).
   * @return The event created. */
  static AnchorEvent *processWindowEvent(eAnchorEventType type, AnchorWindowWin32 *window);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static void processPointerEvent(UINT type, AnchorWindowWin32 *window, WPARAM wParam, LPARAM lParam, bool &eventhandled);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static AnchorEventCursor *processCursorEvent(AnchorWindowWin32 *window);

  /**
   * Handles a mouse wheel event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc. */
  static void processWheelEvent(AnchorWindowWin32 *window, WPARAM wParam, LPARAM lParam);

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
  inline void retrieveModifierKeys(AnchorModifierKeys &keys) const;

  /**
   * Stores the state of the modifier keys locally.
   * For internal use only!
   * param keys The new state of the modifier keys. */
  inline void storeModifierKeys(const AnchorModifierKeys &keys);

  /**
   * Check current key layout for AltGr. */
  inline void handleKeyboardChange(void);

 private:
  eAnchorStatus init();
  eAnchorStatus exit();

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

  /**
   * Returns the current location of the cursor (location in screen coordinates)
   * @param x: The x-coordinate of the cursor.
   * @param y: The y-coordinate of the cursor.
   * @return Indication of success. */
  eAnchorStatus getCursorPosition(AnchorS32 &x, AnchorS32 &y) const;

  /**
   * Updates the location of the cursor (location in screen coordinates).
   * @param x: The x-coordinate of the cursor.
   * @param y: The y-coordinate of the cursor.
   * @return Indication of success. */
  eAnchorStatus setCursorPosition(AnchorS32 x, AnchorS32 y);

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

  static AnchorEventKey *processKeyEvent(AnchorWindowWin32 *window, RAWINPUT const &raw);

  static AnchorEvent *processWindowSizeEvent(AnchorWindowWin32 *window);

  /**
   * Creates mouse button event.
   * @param type: The type of event to create.
   * @param window: The window receiving the event (the active window).
   * @param mask: The button mask of this event.
   * @return The event created. */
  static AnchorEventButton *processButtonEvent(eAnchorEventType type,
                                               AnchorWindowWin32 *window,
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
  AnchorModifierKeys m_modifierKeys;
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


inline void AnchorSystemWin32::retrieveModifierKeys(AnchorModifierKeys &keys) const
{
  keys = m_modifierKeys;
}

inline void AnchorSystemWin32::storeModifierKeys(const AnchorModifierKeys &keys)
{
  m_modifierKeys = keys;
}

inline void AnchorSystemWin32::handleKeyboardChange(void)
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
class AnchorDisplayManagerWin32 : public AnchorDisplayManager
{
 public:
  /**
   * Constructor.*/
  AnchorDisplayManagerWin32(void);

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


class AnchorWindowWin32 : public AnchorSystemWindow
{
 private:
  AnchorSystemWin32 *m_system;

  ANCHOR_VulkanGPU_Surface *m_vulkan_context;

  bool m_hasGrabMouse;
  int m_nPressedButtons;
  bool m_hasMouseCaptured;

  /** 
   * Most recent tablet data. */
  AnchorTabletData m_lastPointerTabletData;

  /** 
   * HCURSOR structure of the custom cursor. */
  HCURSOR m_customCursor;

  /** 
   * Request Vulkan with alpha channel. */
  bool m_wantAlphaBackground;

  /** ITaskbarList3 structure for progress bar. */
  ITaskbarList3 *m_Bar;

  static const wchar_t *s_windowClassName;
  static const int s_maxTitleLength;

  /** Window handle. */
  HWND m_hWnd;

  /** Device context handle. */
  HDC m_hDC;

  bool m_isDialog;

  eAnchorWindowState m_normal_state;

  /** `user32.dll` handle */
  HMODULE m_user32;

  HWND m_parentWindowHwnd;

  /**
   * Vulkan device objects. */
  VkInstance m_instance;
  VkImage m_fontImage;
  VkDeviceMemory m_fontMemory;
  VkImageView m_fontView;
  VkSampler m_fontSampler;
  VkBuffer m_fontUploadBuffer;
  VkDeviceMemory m_fontUploadBufferMemory;
  VkDeviceSize m_bufferMemoryAlignment;
  wabi::HgiVulkan *m_hgi;
  wabi::HgiVulkanInstance *m_vkinstance;
  wabi::HgiVulkanDevice *m_device;
  wabi::HgiVulkanCommandQueue *m_commandQueue;
  wabi::HgiVulkanPipelineCache *m_pipelineCache;

 public:
  AnchorWindowWin32(AnchorSystemWin32 *system,
                    const char *title,
                    const char *icon,
                    AnchorS32 left,
                    AnchorS32 top,
                    AnchorU32 width,
                    AnchorU32 height,
                    eAnchorWindowState state,
                    eAnchorDrawingContextType type = ANCHOR_DrawingContextTypeNone,
                    bool alphaBackground = false,
                    AnchorWindowWin32 *parentWindow = 0,
                    bool dialog = false);

  ~AnchorWindowWin32();

  void adjustWindowRectForClosestMonitor(LPRECT win_rect,
                                         DWORD dwStyle,
                                         DWORD dwExStyle);

  void setTitle(const char *title);
  std::string getTitle() const;

  void setIcon(const char *icon);

  /**
   * Returns the window rectangle dimensions.
   * The dimensions are given in screen coordinates that are
   * relative to the upper-left corner of the screen.
   * @param bounds: The bounding rectangle of the window. */
  void getWindowBounds(AnchorRect &bounds) const;

  void SetupVulkan();
  eAnchorStatus DestroyVulkan();
  uint32_t GetVulkanMemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits);
  void CreateVulkanDescriptorSetLayout();
  void CreateVulkanFontTexture(VkCommandBuffer command_buffer);
  void DestroyVulkanFontTexture();

  wabi::HgiVulkanDevice *getHydraDevice()
  {
    return m_device;
  }

  wabi::HgiVulkanInstance *getHydraInstance()
  {
    return m_vkinstance;
  }

  ANCHOR_VulkanGPU_Surface *getVulkanSurface()
  {
    return m_vulkan_context;
  }

  int getMinImageCount();

  ANCHOR_VulkanGPU_Surface *updateVulkanSurface(ANCHOR_VulkanGPU_Surface *data)
  {
    m_vulkan_context = data;
    return m_vulkan_context;
  }

  /**
   * Returns indication as to whether the window is valid.
   * @return The validity of the window. */
  bool getValid() const;

  /**
   * Access to the handle of the window.
   * @return The handle of the window. */
  HWND getHWND() const;

  void getClientBounds(AnchorRect &bounds) const;

  /**
   * @param type: The type of rendering context create.
   * @return Indication of success. */
  void newDrawingContext(eAnchorDrawingContextType type);

  /**
   * Activates the drawing context of this window.
   * @return A boolean success indicator. */
  eAnchorStatus activateDrawingContext();

  bool isDialog() const;

  /**
   * Removes references to native handles from this context and then returns
   * @return ANCHOR_SUCCESS if it is OK for the parent to release the handles
   * and ANCHOR_FAILURE if releasing the handles will interfere with sharing */
  eAnchorStatus releaseNativeHandles();

  void FrameRender(AnchorDrawData *draw_data);
  void FramePresent(void);

  /**
   * Swaps front and back buffers of a window.
   * @return A boolean success indicator. */
  eAnchorStatus swapBuffers();

  void screenToClient(AnchorS32 inX,
                      AnchorS32 inY,
                      AnchorS32 &outX,
                      AnchorS32 &outY) const;

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
  AnchorTabletData getTabletData(void);

  void lostMouseCapture(void);

  eAnchorStatus setWindowCursorShape(eAnchorStandardCursor cursorShape);

  /**
   * Loads the windows equivalent of a standard ANCHOR cursor.
   * @param visible: Flag for cursor visibility.
   * @param cursorShape: The cursor shape. */
  HCURSOR getStandardCursor(eAnchorStandardCursor shape) const;
  void loadCursor(bool visible, eAnchorStandardCursor cursorShape) const;

  eAnchorStatus getPointerInfo(std::vector<AnchorBackendWin32PointerInfo> &outPointerInfo,
                               WPARAM wParam,
                               LPARAM lParam);

  eAnchorStatus setOrder(eAnchorWindowOrder order);

  eAnchorStatus beginFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

  eAnchorStatus endFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

  eAnchorStatus setProgressBar(float progress);
  eAnchorStatus endProgressBar();

  AnchorU16 getDPIHint();

 public:
  /** 
   * True if the window currently resizing. */
  bool m_inLiveResize;

  /** 
   * True if the mouse is either over or captured by the window. */
  bool m_mousePresent;
};


class VulkanSwapchain
{

 public:
  void initSwapchain();
};


#elif

/* This file is ignored on all platforms outside Windows. */

#endif /* _WIN32 */