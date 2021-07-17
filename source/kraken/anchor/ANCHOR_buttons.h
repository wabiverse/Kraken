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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"

/**
 * This struct stores the state of the mouse buttons.
 * Buttons can be set using button masks. */
struct AnchorButtons
{
  /**
   * Constructor. */
  AnchorButtons();

  ~AnchorButtons();

  /**
   * Returns the state of a single button.
   * @param mask: Key button to return.
   * @return The state of the button (pressed == true). */
  bool get(eAnchorButtonMask mask) const;

  /**
   * Updates the state of a single button.
   * @param mask: Button state to update.
   * @param down: The new state of the button. */
  void set(eAnchorButtonMask mask, bool down);

  /**
   * Sets the state of all buttons to up. */
  void clear();

  AnchorU8 m_ButtonLeft : 1;
  AnchorU8 m_ButtonMiddle : 1;
  AnchorU8 m_ButtonRight : 1;
};
