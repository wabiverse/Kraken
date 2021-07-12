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

#include "ANCHOR_api.h"
#include "ANCHOR_buttons.h"
#include "ANCHOR_modifier_keys.h"
#include "ANCHOR_system.h"

#include <wabi/base/tf/error.h>

WABI_NAMESPACE_USING

ANCHOR_System::ANCHOR_System()
  : m_nativePixel(false),
    m_windowFocus(true),
    m_displayManager(NULL),
    m_windowManager(NULL),
    m_eventManager(NULL),
    m_tabletAPI(ANCHOR_TabletAutomatic)
{}

ANCHOR_System::~ANCHOR_System()
{
  exit();
}

AnchorU64 ANCHOR_System::getMilliSeconds() const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

eAnchorStatus ANCHOR_System::init()
{
  m_windowManager = new ANCHOR_WindowManager();
  m_eventManager = new ANCHOR_EventManager();

  if (m_windowManager && m_eventManager)
  {
    return ANCHOR_SUCCESS;
  }
  else
  {
    return ANCHOR_ERROR;
  }
}

eAnchorStatus ANCHOR_System::exit()
{
  if (getFullScreen())
  {
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

eAnchorStatus ANCHOR_System::createFullScreenWindow(ANCHOR_SystemWindow **window,
                                                    const ANCHOR_DisplaySetting &settings,
                                                    const bool stereoVisual,
                                                    const bool alphaBackground)
{
  ANCHOR_ASSERT(m_displayManager);
  *window = (ANCHOR_SystemWindow *)createWindow("",
                                                "",
                                                0,
                                                0,
                                                settings.xPixels,
                                                settings.yPixels,
                                                ANCHOR_WindowStateNormal,
                                                ANCHOR_DrawingContextTypeVulkan,
                                                0,
                                                true /* exclusive */);
  return (*window == NULL) ? ANCHOR_ERROR : ANCHOR_SUCCESS;
}

eAnchorStatus ANCHOR_System::beginFullScreen(const ANCHOR_DisplaySetting &setting,
                                             ANCHOR_ISystemWindow **window,
                                             const bool stereoVisual,
                                             const bool alphaBackground)
{
  eAnchorStatus success = ANCHOR_ERROR;
  ANCHOR_ASSERT(m_windowManager);
  if (m_displayManager)
  {
    if (!m_windowManager->getFullScreen())
    {
      m_displayManager->getCurrentDisplaySetting(ANCHOR_DisplayManager::kMainDisplay,
                                                 m_preFullScreenSetting);

      success = m_displayManager->setCurrentDisplaySetting(ANCHOR_DisplayManager::kMainDisplay, setting);
      if (success == ANCHOR_ERROR)
      {
        success = createFullScreenWindow(
          (ANCHOR_SystemWindow **)window, setting, stereoVisual, alphaBackground);
        if (success == ANCHOR_ERROR)
        {
          m_windowManager->beginFullScreen(*window, stereoVisual);
        }
        else
        {
          m_displayManager->setCurrentDisplaySetting(ANCHOR_DisplayManager::kMainDisplay,
                                                     m_preFullScreenSetting);
        }
      }
    }
  }
  if (success == ANCHOR_ERROR)
  {
    TF_CODING_ERROR("ANCHOR_System::beginFullScreen(): could not enter full-screen mode\n");
  }
  return success;
}

eAnchorStatus ANCHOR_System::endFullScreen(void)
{
  eAnchorStatus success = ANCHOR_ERROR;
  ANCHOR_ASSERT(m_windowManager);
  if (m_windowManager->getFullScreen())
  {
    success = m_windowManager->endFullScreen();
    ANCHOR_ASSERT(m_displayManager);
    success = m_displayManager->setCurrentDisplaySetting(ANCHOR_DisplayManager::kMainDisplay,
                                                         m_preFullScreenSetting);
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

bool ANCHOR_System::getFullScreen(void)
{
  bool fullScreen;
  if (m_windowManager)
  {
    fullScreen = m_windowManager->getFullScreen();
  }
  else
  {
    fullScreen = false;
  }
  return fullScreen;
}

void ANCHOR_System::dispatchEvents()
{
  if (m_eventManager)
  {
    m_eventManager->dispatchEvents();
  }
}

bool ANCHOR_System::validWindow(ANCHOR_ISystemWindow *window)
{
  return m_windowManager->getWindowFound(window);
}

eAnchorStatus ANCHOR_System::addEventConsumer(ANCHOR_IEventConsumer *consumer)
{
  eAnchorStatus success;
  if (m_eventManager)
  {
    success = m_eventManager->addConsumer(consumer);
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

eAnchorStatus ANCHOR_System::removeEventConsumer(ANCHOR_IEventConsumer *consumer)
{
  eAnchorStatus success;
  if (m_eventManager)
  {
    success = m_eventManager->removeConsumer(consumer);
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

eAnchorStatus ANCHOR_System::pushEvent(ANCHOR_IEvent *event)
{
  eAnchorStatus success;
  if (m_eventManager)
  {
    success = m_eventManager->pushEvent(event);
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

eAnchorStatus ANCHOR_System::getModifierKeyState(eAnchorModifierKeyMask mask, bool &isDown) const
{
  ANCHOR_ModifierKeys keys;
  // Get the state of all modifier keys
  eAnchorStatus success = getModifierKeys(keys);
  if (success)
  {
    // Isolate the state of the key requested
    isDown = keys.get(mask);
  }
  return success;
}

eAnchorStatus ANCHOR_System::getButtonState(eAnchorButtonMask mask, bool &isDown) const
{
  ANCHOR_Buttons buttons;
  // Get the state of all mouse buttons
  eAnchorStatus success = getButtons(buttons);
  if (success)
  {
    // Isolate the state of the mouse button requested
    isDown = buttons.get(mask);
  }
  return success;
}

void ANCHOR_System::setTabletAPI(eAnchorTabletAPI api)
{
  m_tabletAPI = api;
}

eAnchorTabletAPI ANCHOR_System::getTabletAPI(void)
{
  return m_tabletAPI;
}

ANCHOR_SystemHandle ANCHOR_CreateSystem()
{
  ANCHOR_ISystem::createSystem();
  ANCHOR_ISystem *system = ANCHOR_ISystem::getSystem();

  return (ANCHOR_SystemHandle)system;
}

// void ANCHOR_DestroySystem(ANCHOR_SystemHandle *system)
// {
//   ANCHOR_clean_vulkan(system);
// }

bool ANCHOR_System::useNativePixel(void)
{
  m_nativePixel = true;
  return 1;
}

void ANCHOR_System::useWindowFocus(const bool use_focus)
{
  m_windowFocus = use_focus;
}