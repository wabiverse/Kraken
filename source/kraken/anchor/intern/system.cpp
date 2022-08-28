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

#include "ANCHOR_api.h"
#include "ANCHOR_buttons.h"
#include "ANCHOR_modifier_keys.h"
#include "ANCHOR_system.h"

#include <wabi/base/tf/error.h>

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_USING

AnchorSystem::AnchorSystem()
  : m_nativePixel(false),
    m_windowFocus(true),
    m_displayManager(NULL),
    m_windowManager(NULL),
    m_eventManager(NULL),
    m_tabletAPI(AnchorTabletAutomatic)
{}

AnchorSystem::~AnchorSystem()
{
  exit();
}

AnchorU64 AnchorSystem::getMilliSeconds() const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now().time_since_epoch())
    .count();
}

eAnchorStatus AnchorSystem::init()
{
  m_windowManager = new AnchorWindowManager();
  m_eventManager = new AnchorEventManager();

  if (m_windowManager && m_eventManager) {
    return ANCHOR_SUCCESS;
  } else {
    return ANCHOR_FAILURE;
  }
}

eAnchorStatus AnchorSystem::exit()
{
  if (getFullScreen()) {
    endFullScreen();
  }

  delete m_displayManager;
  m_displayManager = NULL;

  delete m_windowManager;
  m_windowManager = NULL;

  delete m_eventManager;
  m_eventManager = NULL;

  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystem::createFullScreenWindow(AnchorSystemWindow **window,
                                                   const ANCHOR_DisplaySetting &settings,
                                                   const bool stereoVisual,
                                                   const bool alphaBackground)
{
  ANCHOR_ASSERT(m_displayManager);
  *window = (AnchorSystemWindow *)createWindow("",
                                               "",
                                               0,
                                               0,
                                               settings.xPixels,
                                               settings.yPixels,
                                               AnchorWindowStateNormal,
#if defined(ARCH_OS_WINDOWS)
                                               ANCHOR_DrawingContextTypeVulkan,
#elif defined(ARCH_OS_DARWIN)
                                               ANCHOR_DrawingContextTypeMetal,
#else /* ARCH_OS_LINUX */
                                               ANCHOR_DrawingContextTypeOpenGL
#endif /* ARCH_OS_WINDOWS */
                                               0,
                                               true /* exclusive */);
  return (*window == NULL) ? ANCHOR_FAILURE : ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystem::beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                            AnchorISystemWindow **window,
                                            const bool stereoVisual,
                                            const bool alphaBackground)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  ANCHOR_ASSERT(m_windowManager);
  if (m_displayManager) {
    if (!m_windowManager->getFullScreen()) {
      m_displayManager->getCurrentDisplaySetting(AnchorDisplayManager::kMainDisplay,
                                                 m_preFullScreenSetting);

      success = m_displayManager->setCurrentDisplaySetting(AnchorDisplayManager::kMainDisplay,
                                                           setting);
      if (success == ANCHOR_FAILURE) {
        success = createFullScreenWindow((AnchorSystemWindow **)window,
                                         setting,
                                         stereoVisual,
                                         alphaBackground);
        if (success == ANCHOR_FAILURE) {
          m_windowManager->beginFullScreen(*window, stereoVisual);
        } else {
          m_displayManager->setCurrentDisplaySetting(AnchorDisplayManager::kMainDisplay,
                                                     m_preFullScreenSetting);
        }
      }
    }
  }
  if (success == ANCHOR_FAILURE) {
    TF_CODING_ERROR("AnchorSystem::beginFullScreen(): could not enter full-screen mode\n");
  }
  return success;
}

eAnchorStatus AnchorSystem::endFullScreen(void)
{
  eAnchorStatus success = ANCHOR_FAILURE;
  ANCHOR_ASSERT(m_windowManager);
  if (m_windowManager->getFullScreen()) {
    success = m_windowManager->endFullScreen();
    ANCHOR_ASSERT(m_displayManager);
    success = m_displayManager->setCurrentDisplaySetting(AnchorDisplayManager::kMainDisplay,
                                                         m_preFullScreenSetting);
  } else {
    success = ANCHOR_FAILURE;
  }
  return success;
}

bool AnchorSystem::getFullScreen(void)
{
  bool fullScreen;
  if (m_windowManager) {
    fullScreen = m_windowManager->getFullScreen();
  } else {
    fullScreen = false;
  }
  return fullScreen;
}

void AnchorSystem::dispatchEvents()
{
  if (m_eventManager) {
    m_eventManager->dispatchEvents();
  }
}

bool AnchorSystem::validWindow(AnchorISystemWindow *window)
{
  return m_windowManager->getWindowFound(window);
}

eAnchorStatus AnchorSystem::addEventConsumer(AnchorIEventConsumer *consumer)
{
  eAnchorStatus success;
  if (m_eventManager) {
    success = m_eventManager->addConsumer(consumer);
  } else {
    success = ANCHOR_FAILURE;
  }
  return success;
}

eAnchorStatus AnchorSystem::removeEventConsumer(AnchorIEventConsumer *consumer)
{
  eAnchorStatus success;
  if (m_eventManager) {
    success = m_eventManager->removeConsumer(consumer);
  } else {
    success = ANCHOR_FAILURE;
  }
  return success;
}

eAnchorStatus AnchorSystem::pushEvent(AnchorIEvent *event)
{
  eAnchorStatus success;
  if (m_eventManager) {
    success = m_eventManager->pushEvent(event);
  } else {
    success = ANCHOR_FAILURE;
  }
  return success;
}

eAnchorStatus AnchorSystem::getModifierKeyState(eAnchorModifierKeyMask mask, bool &isDown) const
{
  AnchorModifierKeys keys;
  // Get the state of all modifier keys
  eAnchorStatus success = getModifierKeys(keys);
  if (success) {
    // Isolate the state of the key requested
    isDown = keys.get(mask);
  }
  return success;
}

eAnchorStatus AnchorSystem::getButtonState(eAnchorButtonMask mask, bool &isDown) const
{
  AnchorButtons buttons;
  // Get the state of all mouse buttons
  eAnchorStatus success = getButtons(buttons);
  if (success) {
    // Isolate the state of the mouse button requested
    isDown = buttons.get(mask);
  }
  return success;
}

void AnchorSystem::setTabletAPI(eAnchorTabletAPI api)
{
  m_tabletAPI = api;
}

eAnchorTabletAPI AnchorSystem::getTabletAPI(void)
{
  return m_tabletAPI;
}

AnchorSystemHandle ANCHOR_CreateSystem()
{
  AnchorISystem::createSystem();
  AnchorISystem *system = AnchorISystem::getSystem();

  return (AnchorSystemHandle)system;
}

bool AnchorSystem::useNativePixel(void)
{
  m_nativePixel = true;
  return 1;
}

void AnchorSystem::useWindowFocus(const bool use_focus)
{
  m_windowFocus = use_focus;
}