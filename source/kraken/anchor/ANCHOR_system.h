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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"
#include "ANCHOR_display_manager.h"
#include "ANCHOR_event_manager.h"
#include "ANCHOR_window_manager.h"

class AnchorISystem
{
 public:

  /**
   * Creates the one and only system.
   * @return An indication of success. */
  static eAnchorStatus createSystem();

  /**
   * Destroys the one and only system.
   * @return An indication of success. */
  static eAnchorStatus destroySystem();

  /**
   * Returns a pointer to the one and only
   * system (nil if it hasn't been created).
   * @return A pointer to the system. */
  static AnchorISystem *getSystem();

 protected:

  /**
   * Constructor.
   * Protected default constructor to
   * force use of static createSystem
   * member. */
  AnchorISystem() {}

  /**
   * Destructor.
   * Protected default constructor to
   * force use of static dispose member. */
  virtual ~AnchorISystem() {}

 public:

  /**
   * Returns the system time.
   * Returns the number of milliseconds since the start of the system process.
   * Based on ANSI clock() routine.
   * @return The number of milliseconds. */
  virtual AnchorU64 getMilliSeconds() const = 0;

  /**
   * Returns the number of displays on this system.
   * @return The number of displays. */
  virtual AnchorU8 getNumDisplays() const = 0;

  /**
   * Returns the dimensions of the main display on this system.
   * @return The dimension of the main display. */
  virtual void getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const = 0;

  /**
   * Returns the combine dimensions of all monitors.
   * @return The dimension of the workspace. */
  virtual void getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const = 0;

  /**
   * Create a new window.
   * The new window is added to the list of windows managed.
   * Never explicitly delete the window, use disposeWindow() instead.
   * @param title: The name of the window
   * (displayed in the title bar of the window if the OS supports it).
   * @param left: The coordinate of the left edge of the window.
   * @param top: The coordinate of the top edge of the window.
   * @param width: The width the window.
   * @param height: The height the window.
   * @param state: The state of the window when opened.
   * @param type: The type of drawing context installed in this window.
   * @param glSettings: Misc OpenGL settings.
   * @param exclusive: Use to show the window on top and ignore others (used full-screen).
   * @param is_dialog: Stay on top of parent window, no icon in taskbar, can't be minimized.
   * @param parentWindow: Parent (embedder) window
   * @return The new window (or 0 if creation failed). */
  virtual AnchorISystemWindow *createWindow(const char *title,
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
                                            const AnchorISystemWindow *parentWindow = NULL) = 0;

  /**
   * Begins full screen mode.
   * @param setting: The new setting of the display.
   * @param window: Window displayed in full screen.
   * This window is invalid after full screen has been ended.
   * @return Indication of success. */
  virtual eAnchorStatus beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                        AnchorISystemWindow **window,
                                        const bool stereoVisual,
                                        const bool alphaBackground = 0) = 0;

  /**
   * Ends full screen mode.
   * @return Indication of success. */
  virtual eAnchorStatus endFullScreen(void) = 0;

  /**
   * Returns the current location of the cursor (location in screen coordinates)
   * @param x: The x-coordinate of the cursor.
   * @param y: The y-coordinate of the cursor.
   * @return Indication of success. */
  virtual eAnchorStatus getCursorPosition(AnchorS32 &x, AnchorS32 &y) const = 0;

  /**
   * Updates the location of the cursor (location in screen coordinates).
   * Not all operating systems allow the cursor to be moved (without the input device being moved).
   * @param x: The x-coordinate of the cursor.
   * @param y: The y-coordinate of the cursor.
   * @return Indication of success. */
  virtual eAnchorStatus setCursorPosition(AnchorS32 x, AnchorS32 y) = 0;

  /**
   * Returns current full screen mode status.
   * @return The current status. */
  virtual bool getFullScreen(void) = 0;

  /**
   * Native pixel size support (MacBook 'retina'). */
  virtual bool useNativePixel(void) = 0;

  /**
   * Focus window after opening, or put them in the background. */
  virtual void useWindowFocus(const bool use_focus) = 0;

  /**
   * Retrieves events from the system and stores them in the queue.
   * @param waitForEvent: Flag to wait for an event (or return immediately).
   * @return Indication of the presence of events. */
  virtual bool processEvents(bool waitForEvent) = 0;

  /**
   * Returns whether a window is valid.
   * @param window: Pointer to the window to be checked.
   * @return Indication of validity. */
  virtual bool validWindow(AnchorISystemWindow *window) = 0;

  /**
   * Retrieves events from the queue and send them to the event consumers. */
  virtual void dispatchEvents() = 0;

  /**
   * Adds the given event consumer to our list.
   * @param consumer: The event consumer to add.
   * @return Indication of success. */
  virtual eAnchorStatus addEventConsumer(AnchorIEventConsumer *consumer) = 0;

  /**
   * Removes the given event consumer to our list.
   * @param consumer: The event consumer to remove.
   * @return Indication of success. */
  virtual eAnchorStatus removeEventConsumer(AnchorIEventConsumer *consumer) = 0;

  /**
   * Returns the state of a modifier key (outside the message queue).
   * @param mask: The modifier key state to retrieve.
   * @param isDown: The state of a modifier key (true == pressed).
   * @return Indication of success. */
  virtual eAnchorStatus getModifierKeyState(eAnchorModifierKeyMask mask, bool &isDown) const = 0;

  /**
   * Returns the state of a mouse button (outside the message queue).
   * @param mask: The button state to retrieve.
   * @param isDown: Button state.
   * @return Indication of success. */
  virtual eAnchorStatus getButtonState(eAnchorButtonMask mask, bool &isDown) const = 0;

  /**
   * Set which tablet API to use. Only affects Windows, other platforms have a single API.
   * @param api: Enum indicating which API to use. */
  virtual void setTabletAPI(eAnchorTabletAPI api) = 0;

  /**
   * Toggles console
   * @param action:
   * - 0: Hides.
   * - 1: Shows
   * - 2: Toggles
   * - 3: Hides if it runs not from  command line
   * - *: Does nothing
   * @return current status (1 -visible, 0 - hidden) */
  virtual int toggleConsole(int action) = 0;

 protected:

  /**
   * Initialize the system.
   * @return Indication of success. */
  virtual eAnchorStatus init() = 0;

  /**
   * Shut the system down.
   * @return Indication of success. */
  virtual eAnchorStatus exit() = 0;

  /**
   * The one and only system */
  static AnchorISystem *m_system;
};

class AnchorSystem : public AnchorISystem
{
 protected:

  /**
   * Constructor.
   * Protected default constructor to force use of static createSystem member. */
  AnchorSystem();

  /**
   * Destructor.
   * Protected default constructor to force use of static dispose member. */
  virtual ~AnchorSystem();

 public:

  /**
   * Returns the system time.
   * Returns the number of milliseconds since the start of the system process.
   * Based on ANSI clock() routine.
   * @return The number of milliseconds. */
  virtual AnchorU64 getMilliSeconds() const;

  /**
   * Begins full screen mode.
   * @param setting: The new setting of the display.
   * @param window: Window displayed in full screen.
   * @param stereoVisual: Stereo visual for quad buffered stereo.
   * This window is invalid after full screen has been ended.
   * @return Indication of success. */
  eAnchorStatus beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                AnchorISystemWindow **window,
                                const bool stereoVisual,
                                const bool alphaBackground);

  /**
   * Ends full screen mode.
   * @return Indication of success. */
  eAnchorStatus endFullScreen(void);

  /**
   * Returns current full screen mode status.
   * @return The current status. */
  bool getFullScreen(void);

  /**
   * Native pixel size support (MacBook 'retina').
   * @return The pixel size in float. */
  bool useNativePixel(void);
  bool m_nativePixel;

  /**
   * Focus window after opening, or put them in the background. */
  void useWindowFocus(const bool use_focus);
  bool m_windowFocus;

  /**
   * Returns whether a window is valid.
   * @param window: Pointer to the window to be checked.
   * @return Indication of validity. */
  bool validWindow(AnchorISystemWindow *window);

  /**
   * Dispatches all the events on the stack.
   * The event stack will be empty afterwards. */
  void dispatchEvents();

  /**
   * Adds the given event consumer to our list.
   * @param consumer: The event consumer to add.
   * @return Indication of success. */
  eAnchorStatus addEventConsumer(AnchorIEventConsumer *consumer);

  /**
   * Remove the given event consumer to our list.
   * @param consumer: The event consumer to remove.
   * @return Indication of success. */
  eAnchorStatus removeEventConsumer(AnchorIEventConsumer *consumer);

  /**
   * Returns the state of a modifier key (outside the message queue).
   * @param mask: The modifier key state to retrieve.
   * @param isDown: The state of a modifier key (true == pressed).
   * @return Indication of success. */
  eAnchorStatus getModifierKeyState(eAnchorModifierKeyMask mask, bool &isDown) const;

  /**
   * Returns the state of a mouse button (outside the message queue).
   * @param mask: The button state to retrieve.
   * @param isDown: Button state.
   * @return Indication of success. */
  eAnchorStatus getButtonState(eAnchorButtonMask mask, bool &isDown) const;

  /**
   * Set which tablet API to use. Only affects Windows,
   * other platforms have a single API.
   * @param api: Enum indicating which API to use. */
  void setTabletAPI(eAnchorTabletAPI api);
  eAnchorTabletAPI getTabletAPI(void);

  /**
   * Pushes an event on the stack.
   * To dispatch it, call dispatchEvent()
   * or dispatchEvents().
   * Do not delete the event!
   * @param event: The event to push on the stack. */
  eAnchorStatus pushEvent(AnchorIEvent *event);

  /**
   * @return A pointer to our event manager. */
  inline AnchorEventManager *getEventManager() const;

  /**
   * @return A pointer to our window manager. */
  inline AnchorWindowManager *getWindowManager() const;

  /**
   * Returns the state of all modifier keys.
   * @param keys: The state of all modifier keys (true == pressed).
   * \return Indication of success. */
  virtual eAnchorStatus getModifierKeys(AnchorModifierKeys &keys) const = 0;

  /**
   * Returns the state of the mouse buttons (outside the message queue).
   * @param buttons: The state of the buttons.
   * \return Indication of success. */
  virtual eAnchorStatus getButtons(AnchorButtons &buttons) const = 0;

 protected:

  /**
   * Initialize the system.
   * @return Indication of success. */
  virtual eAnchorStatus init();

  /**
   * Shut the system down.
   * @return Indication of success. */
  virtual eAnchorStatus exit();

  /**
   * Creates a fullscreen window.
   * @param window: The window created.
   * @return Indication of success. */
  eAnchorStatus createFullScreenWindow(AnchorSystemWindow **window,
                                       const ANCHOR_DisplaySetting &settings,
                                       const bool stereoVisual,
                                       const bool alphaBackground = 0);

  /**
   * The display manager (platform dependent). */
  AnchorDisplayManager *m_displayManager;

  /**
   * The window manager. */
  AnchorWindowManager *m_windowManager;

  /**
   * The event manager. */
  AnchorEventManager *m_eventManager;

  /**
   * Settings of the display before the display went fullscreen. */
  ANCHOR_DisplaySetting m_preFullScreenSetting;

  /**
   * Which tablet API to use. */
  eAnchorTabletAPI m_tabletAPI;
};

inline AnchorEventManager *AnchorSystem::getEventManager() const
{
  return m_eventManager;
}

inline AnchorWindowManager *AnchorSystem::getWindowManager() const
{
  return m_windowManager;
}

ANCHOR_API
AnchorSystemHandle ANCHOR_CreateSystem();
