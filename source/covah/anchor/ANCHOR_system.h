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
#include "ANCHOR_event_manager.h"
#include "ANCHOR_window_manager.h"

class ANCHOR_ISystem {
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
  static ANCHOR_ISystem *getSystem();

 protected:
  /**
   * Constructor.
   * Protected default constructor to
   * force use of static createSystem
   * member. */
  ANCHOR_ISystem()
  {}

  /**
   * Destructor.
   * Protected default constructor to
   * force use of static dispose member. */
  virtual ~ANCHOR_ISystem()
  {}

 public:
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
  virtual ANCHOR_ISystemWindow *createWindow(const char *title,
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
                                             const ANCHOR_ISystemWindow *parentWindow = NULL) = 0;

  /**
   * Begins full screen mode.
   * @param setting: The new setting of the display.
   * @param window: Window displayed in full screen.
   * This window is invalid after full screen has been ended.
   * @return Indication of success. */
  virtual eAnchorStatus beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                        ANCHOR_ISystemWindow **window,
                                        const bool stereoVisual,
                                        const bool alphaBackground = 0) = 0;

  /**
   * Ends full screen mode.
   * @return Indication of success. */
  virtual eAnchorStatus endFullScreen(void) = 0;

  /**
   * Returns current full screen mode status.
   * @return The current status. */
  virtual bool getFullScreen(void) = 0;

  /**
   * Retrieves events from the system and stores them in the queue.
   * @param waitForEvent: Flag to wait for an event (or return immediately).
   * @return Indication of the presence of events. */
  virtual bool processEvents(bool waitForEvent) = 0;

  /**
   * Returns whether a window is valid.
   * @param window: Pointer to the window to be checked.
   * @return Indication of validity. */
  virtual bool validWindow(ANCHOR_ISystemWindow *window) = 0;

  /**
   * Retrieves events from the queue and send them to the event consumers. */
  virtual void dispatchEvents() = 0;

  /**
   * Adds the given event consumer to our list.
   * @param consumer: The event consumer to add.
   * @return Indication of success. */
  virtual eAnchorStatus addEventConsumer(ANCHOR_IEventConsumer *consumer) = 0;

  /**
   * Removes the given event consumer to our list.
   * @param consumer: The event consumer to remove.
   * @return Indication of success. */
  virtual eAnchorStatus removeEventConsumer(ANCHOR_IEventConsumer *consumer) = 0;

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
  static ANCHOR_ISystem *m_system;
};

class ANCHOR_System : public ANCHOR_ISystem {
 protected:
  /**
   * Constructor.
   * Protected default constructor to force use of static createSystem member. */
  ANCHOR_System();

  /**
   * Destructor.
   * Protected default constructor to force use of static dispose member. */
  virtual ~ANCHOR_System();

 public:
  /**
   * Begins full screen mode.
   * @param setting: The new setting of the display.
   * @param window: Window displayed in full screen.
   * @param stereoVisual: Stereo visual for quad buffered stereo.
   * This window is invalid after full screen has been ended.
   * @return Indication of success. */
  eAnchorStatus beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                ANCHOR_ISystemWindow **window,
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
   * Returns whether a window is valid.
   * @param window: Pointer to the window to be checked.
   * @return Indication of validity. */
  bool validWindow(ANCHOR_ISystemWindow *window);

  /**
   * Dispatches all the events on the stack.
   * The event stack will be empty afterwards. */
  void dispatchEvents();

  /**
   * Adds the given event consumer to our list.
   * @param consumer: The event consumer to add.
   * @return Indication of success. */
  eAnchorStatus addEventConsumer(ANCHOR_IEventConsumer *consumer);

  /**
   * Remove the given event consumer to our list.
   * @param consumer: The event consumer to remove.
   * @return Indication of success. */
  eAnchorStatus removeEventConsumer(ANCHOR_IEventConsumer *consumer);

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
  eAnchorStatus pushEvent(ANCHOR_IEvent *event);

  /**
   * @return A pointer to our event manager. */
  inline ANCHOR_EventManager *getEventManager() const;

  /**
   * @return A pointer to our window manager. */
  inline ANCHOR_WindowManager *getWindowManager() const;

  /**
   * Returns the state of all modifier keys.
   * \param keys: The state of all modifier keys (true == pressed).
   * \return Indication of success. */
  virtual eAnchorStatus getModifierKeys(ANCHOR_ModifierKeys &keys) const = 0;

  /**
   * Returns the state of the mouse buttons (outside the message queue).
   * \param buttons: The state of the buttons.
   * \return Indication of success. */
  virtual eAnchorStatus getButtons(ANCHOR_Buttons &buttons) const = 0;

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
  eAnchorStatus createFullScreenWindow(ANCHOR_SystemWindow **window,
                                       const ANCHOR_DisplaySetting &settings,
                                       const bool stereoVisual,
                                       const bool alphaBackground = 0);

  /**
   * The display manager (platform dependent). */
  ANCHOR_DisplayManager *m_displayManager;

  /**
   * The window manager. */
  ANCHOR_WindowManager *m_windowManager;

  /**
   * The event manager. */
  ANCHOR_EventManager *m_eventManager;

  /**
   * Settings of the display before the display went fullscreen. */
  ANCHOR_DisplaySetting m_preFullScreenSetting;

  /**
   * Which tablet API to use. */
  eAnchorTabletAPI m_tabletAPI;
};

inline ANCHOR_EventManager *ANCHOR_System::getEventManager() const
{
  return m_eventManager;
}

inline ANCHOR_WindowManager *ANCHOR_System::getWindowManager() const
{
  return m_windowManager;
}

ANCHOR_API
ANCHOR_SystemHandle ANCHOR_CreateSystem();
