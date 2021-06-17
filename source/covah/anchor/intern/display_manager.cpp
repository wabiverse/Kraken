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

#include <wabi/wabi.h>

#include "ANCHOR_debug_codes.h"
#include "ANCHOR_display_manager.h"

WABI_NAMESPACE_USING

ANCHOR_DisplayManager::ANCHOR_DisplayManager(void) : m_settingsInitialized(false)
{}

ANCHOR_DisplayManager::~ANCHOR_DisplayManager(void)
{}

eAnchorStatus ANCHOR_DisplayManager::initialize(void)
{
  eAnchorStatus success;
  if (!m_settingsInitialized) {
    success = initializeSettings();
    m_settingsInitialized = true;
  }
  else {
    success = ANCHOR_SUCCESS;
  }
  return success;
}

eAnchorStatus ANCHOR_DisplayManager::getNumDisplays(AnchorU8 & /*numDisplays*/) const
{
  /**
   * Don't know if we have a display... */
  return ANCHOR_ERROR;
}

eAnchorStatus ANCHOR_DisplayManager::getNumDisplaySettings(AnchorU8 display, AnchorS32 &numSettings) const
{
  eAnchorStatus success;

  ANCHOR_ASSERT(m_settingsInitialized);
  AnchorU8 numDisplays;
  success = getNumDisplays(numDisplays);
  if (success == ANCHOR_SUCCESS) {
    if (display < numDisplays) {
      numSettings = m_settings[display].size();
    }
    else {
      success = ANCHOR_ERROR;
    }
  }
  return success;
}

eAnchorStatus ANCHOR_DisplayManager::getDisplaySetting(AnchorU8 display,
                                                       AnchorS32 index,
                                                       ANCHOR_DisplaySetting &setting) const
{
  eAnchorStatus success;

  ANCHOR_ASSERT(m_settingsInitialized);
  AnchorU8 numDisplays;
  success = getNumDisplays(numDisplays);
  if (success == ANCHOR_SUCCESS) {
    if (display < numDisplays && ((AnchorU8)index < m_settings[display].size())) {
      setting = m_settings[display][index];
    }
    else {
      success = ANCHOR_ERROR;
    }
  }
  return success;
}

eAnchorStatus ANCHOR_DisplayManager::getCurrentDisplaySetting(AnchorU8 /*display*/,
                                                              ANCHOR_DisplaySetting & /*setting*/) const
{
  return ANCHOR_ERROR;
}

eAnchorStatus ANCHOR_DisplayManager::setCurrentDisplaySetting(AnchorU8 /*display*/,
                                                              const ANCHOR_DisplaySetting & /*setting*/)
{
  return ANCHOR_ERROR;
}

eAnchorStatus ANCHOR_DisplayManager::findMatch(AnchorU8 display,
                                               const ANCHOR_DisplaySetting &setting,
                                               ANCHOR_DisplaySetting &match) const
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  ANCHOR_ASSERT(m_settingsInitialized);

  int criteria[4] = {(int)setting.xPixels, (int)setting.yPixels, (int)setting.bpp, (int)setting.frequency};
  int capabilities[4];
  double field, score;
  double best = 1e12; /** A big number. */
  int found = 0;

  /**
   * Look at all the display modes. */
  for (int i = 0; (i < (int)m_settings[display].size()); i++) {
    // Store the capabilities of the display device
    capabilities[0] = m_settings[display][i].xPixels;
    capabilities[1] = m_settings[display][i].yPixels;
    capabilities[2] = m_settings[display][i].bpp;
    capabilities[3] = m_settings[display][i].frequency;

    /**
     * Match against all the fields of the display settings. */
    score = 0;
    for (int j = 0; j < 4; j++) {
      field = capabilities[j] - criteria[j];
      score += field * field;
    }

    if (score < best) {
      found = i;
      best = score;
    }
  }

  match = m_settings[display][found];

  TF_DEBUG(ANCHOR_DISPLAY_MANAGER).Msg("ANCHOR_DisplayManager::findMatch(): settings of match:\n");
  TF_DEBUG(ANCHOR_DISPLAY_MANAGER).Msg("  setting.xPixels=%d\n", match.xPixels);
  TF_DEBUG(ANCHOR_DISPLAY_MANAGER).Msg("  setting.yPixels=%d\n", match.yPixels);
  TF_DEBUG(ANCHOR_DISPLAY_MANAGER).Msg("  setting.bpp=%d\n", match.bpp);
  TF_DEBUG(ANCHOR_DISPLAY_MANAGER).Msg("  setting.frequency=%d\n", match.frequency);

  return success;
}

eAnchorStatus ANCHOR_DisplayManager::initializeSettings(void)
{
  AnchorU8 numDisplays;
  eAnchorStatus success = getNumDisplays(numDisplays);
  if (success == ANCHOR_SUCCESS) {
    for (AnchorU8 display = 0; (display < numDisplays) && (success == ANCHOR_SUCCESS); display++) {
      ANCHOR_DisplaySettings displaySettings;
      m_settings.push_back(displaySettings);
      AnchorS32 numSettings;
      success = getNumDisplaySettings(display, numSettings);
      if (success == ANCHOR_SUCCESS) {
        AnchorS32 index;
        ANCHOR_DisplaySetting setting;
        for (index = 0; (index < numSettings) && (success == ANCHOR_SUCCESS); index++) {
          success = getDisplaySetting(display, index, setting);
          m_settings[display].push_back(setting);
        }
      }
      else {
        break;
      }
    }
  }
  return success;
}