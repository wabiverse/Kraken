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

#include <vector>

#include "ANCHOR_window.h"

/**
 * Manages system windows (platform independent implementation). */
class ANCHOR_WindowManager
{
 public:
  /**
   * Constructor.
   */
  ANCHOR_WindowManager();

  /**
   * Destructor.
   */
  ~ANCHOR_WindowManager();

  /**
   * Add a window to our list.
   * It is only added if it is not already in the list.
   * \param window: Pointer to the window to be added.
   * \return Indication of success.
   */
  eAnchorStatus addWindow(ANCHOR_ISystemWindow *window);

  /**
   * Remove a window from our list.
   * \param window: Pointer to the window to be removed.
   * \return Indication of success.
   */
  eAnchorStatus removeWindow(const ANCHOR_ISystemWindow *window);

  /**
   * Returns whether the window is in our list.
   * \param window: Pointer to the window to query.
   * \return A boolean indicator.
   */
  bool getWindowFound(const ANCHOR_ISystemWindow *window) const;

  /**
   * Returns whether one of the windows is full-screen.
   * \return A boolean indicator.
   */
  bool getFullScreen(void) const;

  /**
   * Returns pointer to the full-screen window.
   * \return The full-screen window (NULL if not in full-screen).
   */
  ANCHOR_ISystemWindow *getFullScreenWindow(void) const;

  /**
   * Activates full-screen mode for a window.
   * \param window: The window displayed full-screen.
   * \return Indication of success.
   */
  eAnchorStatus beginFullScreen(ANCHOR_ISystemWindow *window, const bool stereoVisual);

  /**
   * Closes full-screen mode down.
   * \return Indication of success.
   */
  eAnchorStatus endFullScreen(void);

  /**
   * Sets new window as active window (the window receiving events).
   * There can be only one window active which should be in the current window list.
   * \param window: The new active window.
   */
  eAnchorStatus setActiveWindow(ANCHOR_ISystemWindow *window);

  /**
   * Returns the active window (the window receiving events).
   * There can be only one window active which should be in the current window list.
   * \return window The active window (or NULL if there is none).
   */
  ANCHOR_ISystemWindow *getActiveWindow(void) const;

  /**
   * Set this window to be inactive (not receiving events).
   * \param window: The window to deactivate.
   */
  void setWindowInactive(const ANCHOR_ISystemWindow *window);

  /**
   * Return a vector of the windows currently managed by this
   * class.
   * \return Constant reference to the vector of windows managed
   */
  const std::vector<ANCHOR_ISystemWindow *> &getWindows() const;

  /**
   * Finds the window associated with an OS window object/handle.
   * \param osWindow: The OS window object/handle.
   * \return The associated window, null if none corresponds.
   */
  ANCHOR_ISystemWindow *getWindowAssociatedWithOSWindow(void *osWindow);

  /**
   * Return true if any windows has a modified status
   * \return True if any window has unsaved changes
   */
  bool getAnyModifiedState();

 protected:
  /** The list of windows managed */
  std::vector<ANCHOR_ISystemWindow *> m_windows;

  /** Window in fullscreen state. There can be only one of this which is not in or window list. */
  ANCHOR_ISystemWindow *m_fullScreenWindow;

  /** The active window. */
  ANCHOR_ISystemWindow *m_activeWindow;

  /** Window that was active before entering fullscreen state. */
  ANCHOR_ISystemWindow *m_activeWindowBeforeFullScreen;
};
