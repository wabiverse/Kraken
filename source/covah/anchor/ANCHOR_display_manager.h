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

class ANCHOR_DisplayManager {
 public:
  enum { kMainDisplay = 0 };
  /**
   * Constructor. */
  ANCHOR_DisplayManager(void);

  /**
   * Destructor. */
  virtual ~ANCHOR_DisplayManager(void);

  /**
   * Initializes the list with devices and settings.
   * @return Indication of success. */
  virtual eAnchorStatus initialize(void);

  /**
   * Returns the number of display devices on this system.
   * @param numDisplays: The number of displays on this system.
   * @return Indication of success. */
  virtual eAnchorStatus getNumDisplays(AnchorU8 &numDisplays) const;

  /**
   * Returns the number of display settings for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param numSettings: The number of settings of the display device with this index.
   * @return Indication of success. */
  virtual eAnchorStatus getNumDisplaySettings(AnchorU8 display, AnchorS32 &numSettings) const;

  /**
   * Returns the current setting for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param index: The setting index to be returned.
   * @param setting: The setting of the display device with this index.
   * @return Indication of success. */
  virtual eAnchorStatus getDisplaySetting(AnchorU8 display,
                                          AnchorS32 index,
                                          ANCHOR_DisplaySetting &setting) const;

  /**
   * Returns the current setting for this display device.
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param setting: The current setting of the display device with this index.
   * @return Indication of success. */
  virtual eAnchorStatus getCurrentDisplaySetting(AnchorU8 display,
                                                 ANCHOR_DisplaySetting &setting) const;

  /**
   * Changes the current setting for this display device.
   * The setting given to this method is matched against the available display settings.
   * The best match is activated (@see findMatch()).
   * @param display: The index of the display to query with 0 <= display < getNumDisplays().
   * @param setting: The setting of the display device to be matched and activated.
   * @return Indication of success. */
  virtual eAnchorStatus setCurrentDisplaySetting(AnchorU8 display,
                                                 const ANCHOR_DisplaySetting &setting);

 protected:
  typedef std::vector<ANCHOR_DisplaySetting> ANCHOR_DisplaySettings;

  /**
   * Finds the best display settings match.
   * @param display: The index of the display device.
   * @param setting: The setting to match.
   * @param match: The optimal display setting.
   * @return Indication of success. */
  eAnchorStatus findMatch(AnchorU8 display,
                          const ANCHOR_DisplaySetting &setting,
                          ANCHOR_DisplaySetting &match) const;

  /**
   * Retrieves settings for each display device and stores them.
   * @return Indication of success. */
  eAnchorStatus initializeSettings(void);

  /** Tells whether the list of display modes has been stored already. */
  bool m_settingsInitialized;

  /** The list with display settings for the main display. */
  std::vector<ANCHOR_DisplaySettings> m_settings;
};