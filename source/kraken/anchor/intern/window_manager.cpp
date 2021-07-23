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

#include "ANCHOR_window.h"
#include "ANCHOR_window_manager.h"
#include <algorithm>

AnchorWindowManager::AnchorWindowManager()
  : m_fullScreenWindow(nullptr),
    m_activeWindow(nullptr),
    m_activeWindowBeforeFullScreen(nullptr)
{}

AnchorWindowManager::~AnchorWindowManager()
{
  /* m_windows is freed by AnchorSystem::disposeWindow */
}

eAnchorStatus AnchorWindowManager::addWindow(AnchorISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  if (window)
  {
    if (!getWindowFound(window))
    {
      // Store the pointer to the window
      m_windows.push_back(window);
      success = ANCHOR_SUCCESS;
    }
  }
  return success;
}

eAnchorStatus AnchorWindowManager::removeWindow(const AnchorISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  if (window)
  {
    if (window == m_fullScreenWindow)
    {
      endFullScreen();
    } else
    {
      std::vector<AnchorISystemWindow *>::iterator result = find(m_windows.begin(), m_windows.end(), window);
      if (result != m_windows.end())
      {
        setWindowInactive(window);
        m_windows.erase(result);
        success = ANCHOR_SUCCESS;
      }
    }
  }
  return success;
}

bool AnchorWindowManager::getWindowFound(const AnchorISystemWindow *window) const
{
  bool found = false;
  if (window)
  {
    if (getFullScreen() && (window == m_fullScreenWindow))
    {
      found = true;
    } else
    {
      std::vector<AnchorISystemWindow *>::const_iterator result = find(m_windows.begin(),
                                                                       m_windows.end(),
                                                                       window);
      if (result != m_windows.end())
      {
        found = true;
      }
    }
  }
  return found;
}

bool AnchorWindowManager::getFullScreen(void) const
{
  return (m_fullScreenWindow != NULL);
}

AnchorISystemWindow *AnchorWindowManager::getFullScreenWindow(void) const
{
  return m_fullScreenWindow;
}

eAnchorStatus AnchorWindowManager::beginFullScreen(AnchorISystemWindow *window, bool /*stereoVisual*/)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  ANCHOR_ASSERT(window);
  ANCHOR_ASSERT(window->getValid());
  if (!getFullScreen())
  {
    m_fullScreenWindow = window;
    m_activeWindowBeforeFullScreen = getActiveWindow();
    setActiveWindow(m_fullScreenWindow);
    m_fullScreenWindow->beginFullScreen();
    success = ANCHOR_SUCCESS;
  }
  return success;
}

eAnchorStatus AnchorWindowManager::endFullScreen(void)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  if (getFullScreen())
  {
    if (m_fullScreenWindow != NULL)
    {
      setWindowInactive(m_fullScreenWindow);
      m_fullScreenWindow->endFullScreen();
      delete m_fullScreenWindow;
      m_fullScreenWindow = NULL;
      if (m_activeWindowBeforeFullScreen)
      {
        setActiveWindow(m_activeWindowBeforeFullScreen);
      }
    }
    success = ANCHOR_SUCCESS;
  }
  return success;
}

eAnchorStatus AnchorWindowManager::setActiveWindow(AnchorISystemWindow *window)
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  if (window != m_activeWindow)
  {
    if (getWindowFound(window))
    {
      m_activeWindow = window;
    } else
    {
      success = ANCHOR_FAILURE;
    }
  }
  return success;
}

AnchorISystemWindow *AnchorWindowManager::getActiveWindow(void) const
{
  return m_activeWindow;
}

void AnchorWindowManager::setWindowInactive(const AnchorISystemWindow *window)
{
  if (window == m_activeWindow)
  {
    m_activeWindow = NULL;
  }
}

const std::vector<AnchorISystemWindow *> &AnchorWindowManager::getWindows() const
{
  return m_windows;
}

AnchorISystemWindow *AnchorWindowManager::getWindowAssociatedWithOSWindow(void *osWindow)
{
  std::vector<AnchorISystemWindow *>::iterator iter;

  for (iter = m_windows.begin(); iter != m_windows.end(); ++iter)
  {
    if ((*iter)->getOSWindow() == osWindow)
      return *iter;
  }

  return NULL;
}

bool AnchorWindowManager::getAnyModifiedState()
{
  bool isAnyModified = false;
  std::vector<AnchorISystemWindow *>::iterator iter;

  for (iter = m_windows.begin(); iter != m_windows.end(); ++iter)
  {
    if ((*iter)->getModifiedState())
      isAnyModified = true;
  }

  return isAnyModified;
}
