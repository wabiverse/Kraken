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

#include "ANCHOR_window.h"
#include "ANCHOR_window_manager.h"
#include <algorithm>

ANCHOR_WindowManager::ANCHOR_WindowManager()
  : m_fullScreenWindow(0),
    m_activeWindow(0),
    m_activeWindowBeforeFullScreen(0)
{}

ANCHOR_WindowManager::~ANCHOR_WindowManager()
{
  /* m_windows is freed by ANCHOR_System::disposeWindow */
}

eAnchorStatus ANCHOR_WindowManager::addWindow(ANCHOR_ISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_ERROR;
  if (window) {
    if (!getWindowFound(window)) {
      // Store the pointer to the window
      m_windows.push_back(window);
      success = ANCHOR_SUCCESS;
    }
  }
  return success;
}

eAnchorStatus ANCHOR_WindowManager::removeWindow(const ANCHOR_ISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_ERROR;
  if (window) {
    if (window == m_fullScreenWindow) {
      endFullScreen();
    }
    else {
      std::vector<ANCHOR_ISystemWindow *>::iterator result = find(
        m_windows.begin(), m_windows.end(), window);
      if (result != m_windows.end()) {
        setWindowInactive(window);
        m_windows.erase(result);
        success = ANCHOR_SUCCESS;
      }
    }
  }
  return success;
}

bool ANCHOR_WindowManager::getWindowFound(const ANCHOR_ISystemWindow *window) const
{
  bool found = false;
  if (window) {
    if (getFullScreen() && (window == m_fullScreenWindow)) {
      found = true;
    }
    else {
      std::vector<ANCHOR_ISystemWindow *>::const_iterator result = find(
        m_windows.begin(), m_windows.end(), window);
      if (result != m_windows.end()) {
        found = true;
      }
    }
  }
  return found;
}

bool ANCHOR_WindowManager::getFullScreen(void) const
{
  return (m_fullScreenWindow != NULL);
}

ANCHOR_ISystemWindow *ANCHOR_WindowManager::getFullScreenWindow(void) const
{
  return m_fullScreenWindow;
}

eAnchorStatus ANCHOR_WindowManager::beginFullScreen(ANCHOR_ISystemWindow *window, bool /*stereoVisual*/)
{
  eAnchorStatus success = ANCHOR_ERROR;
  ANCHOR_ASSERT(window);
  ANCHOR_ASSERT(window->getValid());
  if (!getFullScreen()) {
    m_fullScreenWindow = window;
    m_activeWindowBeforeFullScreen = getActiveWindow();
    setActiveWindow(m_fullScreenWindow);
    m_fullScreenWindow->beginFullScreen();
    success = ANCHOR_SUCCESS;
  }
  return success;
}

eAnchorStatus ANCHOR_WindowManager::endFullScreen(void)
{
  eAnchorStatus success = ANCHOR_ERROR;
  if (getFullScreen()) {
    if (m_fullScreenWindow != NULL) {
      setWindowInactive(m_fullScreenWindow);
      m_fullScreenWindow->endFullScreen();
      delete m_fullScreenWindow;
      m_fullScreenWindow = NULL;
      if (m_activeWindowBeforeFullScreen) {
        setActiveWindow(m_activeWindowBeforeFullScreen);
      }
    }
    success = ANCHOR_SUCCESS;
  }
  return success;
}

eAnchorStatus ANCHOR_WindowManager::setActiveWindow(ANCHOR_ISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  if (window != m_activeWindow) {
    if (getWindowFound(window)) {
      m_activeWindow = window;
    }
    else {
      success = ANCHOR_ERROR;
    }
  }
  return success;
}

ANCHOR_ISystemWindow *ANCHOR_WindowManager::getActiveWindow(void) const
{
  return m_activeWindow;
}

void ANCHOR_WindowManager::setWindowInactive(const ANCHOR_ISystemWindow *window)
{
  if (window == m_activeWindow) {
    m_activeWindow = NULL;
  }
}

const std::vector<ANCHOR_ISystemWindow *> &ANCHOR_WindowManager::getWindows() const
{
  return m_windows;
}

ANCHOR_ISystemWindow *ANCHOR_WindowManager::getWindowAssociatedWithOSWindow(void *osWindow)
{
  std::vector<ANCHOR_ISystemWindow *>::iterator iter;

  for (iter = m_windows.begin(); iter != m_windows.end(); ++iter) {
    if ((*iter)->getOSWindow() == osWindow)
      return *iter;
  }

  return NULL;
}

bool ANCHOR_WindowManager::getAnyModifiedState()
{
  bool isAnyModified = false;
  std::vector<ANCHOR_ISystemWindow *>::iterator iter;

  for (iter = m_windows.begin(); iter != m_windows.end(); ++iter) {
    if ((*iter)->getModifiedState())
      isAnyModified = true;
  }

  return isAnyModified;
}
