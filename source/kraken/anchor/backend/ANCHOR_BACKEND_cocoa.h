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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
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

class AnchorAppleMetal;

class AnchorSystemCocoa : public AnchorSystem
{
 public:

  /**
   * This method converts performance counter measurements into milliseconds since the start of the
   * system process.
   * @return The number of milliseconds since the start of the system process. */
  AnchorU64 performanceCounterToMillis(int perf_ticks) const;

  /**
   * This method converts system ticks into milliseconds since the start of the
   * system process.
   * @return The number of milliseconds since the start of the system process. */
  AnchorU64 tickCountToMillis(int ticks) const;

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
  static AnchorEvent *processWindowEvent(eAnchorEventType type, AnchorISystemWindow *window);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static void processPointerEvent(int type,
                                  AnchorISystemWindow *window,
                                  AnchorS32 wParam,
                                  AnchorS32 lParam,
                                  bool &eventhandled);

  /**
   * Creates tablet events from pointer events.
   * @param type: The type of pointer event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param eventhandled: True if the method handled the event. */
  static AnchorEventCursor *processCursorEvent(AnchorISystemWindow *window);

  /**
   * Handles a mouse wheel event.
   * @param window: The window receiving the event (the active window).
   * @param wParam: The wParam from the wndproc.
   * @param lParam: The lParam from the wndproc.
   * @param isHorizontal: Whether the wheel event is horizontal or (false) for vertical. */
  static void processWheelEvent(AnchorISystemWindow *window,
                                AnchorS32 wParam,
                                AnchorS32 lParam,
                                bool isHorizontal);

  /**
   * Handles minimum window size.
   * @param minmax: The MINMAXINFO structure. */
  static void processMinMaxInfo(void *minmax);

  /**
   * Returns the local state of the modifier keys (from the message queue).
   * @param keys: The state of the keys. */
  inline void retrieveModifierKeys(AnchorModifierKeys &keys) const {}

  /**
   * Stores the state of the modifier keys locally.
   * For internal use only!
   * param keys The new state of the modifier keys. */
  inline void storeModifierKeys(const AnchorModifierKeys &keys) {}

  /**
   * Check current key layout for AltGr. */
  inline void handleKeyboardChange(void) {}

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

  static AnchorEventKey *processKeyEvent(AnchorISystemWindow *window, AnchorS32 const &raw);

  static AnchorEvent *processWindowSizeEvent(AnchorISystemWindow *window);

  /**
   * Creates mouse button event.
   * @param type: The type of event to create.
   * @param window: The window receiving the event (the active window).
   * @param mask: The button mask of this event.
   * @return The event created. */
  static AnchorEventButton *processButtonEvent(eAnchorEventType type,
                                               AnchorISystemWindow *window,
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
  eAnchorKey hardKey(AnchorS32 const &raw, bool *r_keyDown, bool *r_is_repeated_modifier);

  eAnchorKey processSpecialKey(short vKey, short scanCode) const;
};
