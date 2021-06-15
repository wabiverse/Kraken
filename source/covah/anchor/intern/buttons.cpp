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

#include "ANCHOR_buttons.h"

ANCHOR_Buttons::ANCHOR_Buttons()
{
  clear();
}

bool ANCHOR_Buttons::get(eAnchorButtonMask mask) const
{
  switch (mask) {
    case ANCHOR_ButtonMaskLeft:
      return m_ButtonLeft;
    case ANCHOR_ButtonMaskMiddle:
      return m_ButtonMiddle;
    case ANCHOR_ButtonMaskRight:
      return m_ButtonRight;
    default:
      return false;
  }
}

void ANCHOR_Buttons::set(eAnchorButtonMask mask, bool down)
{
  switch (mask) {
    case ANCHOR_ButtonMaskLeft:
      m_ButtonLeft = down;
      break;
    case ANCHOR_ButtonMaskMiddle:
      m_ButtonMiddle = down;
      break;
    case ANCHOR_ButtonMaskRight:
      m_ButtonRight = down;
      break;
    default:
      break;
  }
}

void ANCHOR_Buttons::clear()
{
  m_ButtonLeft = false;
  m_ButtonMiddle = false;
  m_ButtonRight = false;
}

ANCHOR_Buttons::~ANCHOR_Buttons()
{}
