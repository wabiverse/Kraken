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

#include "ANCHOR_buttons.h"

AnchorButtons::AnchorButtons()
{
  clear();
}

bool AnchorButtons::get(eAnchorButtonMask mask) const
{
  switch (mask) {
    case ANCHOR_BUTTON_MASK_LEFT:
      return m_ButtonLeft;
    case ANCHOR_BUTTON_MASK_MIDDLE:
      return m_ButtonMiddle;
    case ANCHOR_BUTTON_MASK_RIGHT:
      return m_ButtonRight;
    default:
      return false;
  }
}

void AnchorButtons::set(eAnchorButtonMask mask, bool down)
{
  switch (mask) {
    case ANCHOR_BUTTON_MASK_LEFT:
      m_ButtonLeft = down;
      break;
    case ANCHOR_BUTTON_MASK_MIDDLE:
      m_ButtonMiddle = down;
      break;
    case ANCHOR_BUTTON_MASK_RIGHT:
      m_ButtonRight = down;
      break;
    default:
      break;
  }
}

void AnchorButtons::clear()
{
  m_ButtonLeft = false;
  m_ButtonMiddle = false;
  m_ButtonRight = false;
}

AnchorButtons::~AnchorButtons() {}
