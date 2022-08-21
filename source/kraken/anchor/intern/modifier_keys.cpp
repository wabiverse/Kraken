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

#include "ANCHOR_modifier_keys.h"

AnchorModifierKeys::AnchorModifierKeys()
{
  clear();
}

AnchorModifierKeys::~AnchorModifierKeys() {}

eAnchorKey AnchorModifierKeys::getModifierKeyCode(eAnchorModifierKeyMask mask)
{
  eAnchorKey key;
  switch (mask) {
    case ANCHOR_ModifierKeyLeftShift:
      key = AnchorKeyLeftShift;
      break;
    case ANCHOR_ModifierKeyRightShift:
      key = AnchorKeyRightShift;
      break;
    case ANCHOR_ModifierKeyLeftAlt:
      key = AnchorKeyLeftAlt;
      break;
    case ANCHOR_ModifierKeyRightAlt:
      key = AnchorKeyRightAlt;
      break;
    case ANCHOR_ModifierKeyLeftControl:
      key = AnchorKeyLeftControl;
      break;
    case ANCHOR_ModifierKeyRightControl:
      key = AnchorKeyRightControl;
      break;
    case ANCHOR_ModifierKeyOS:
      key = AnchorKeyOS;
      break;
    default:
      /**
       * Should not happen. */
      key = AnchorKeyUnknown;
      break;
  }
  return key;
}

bool AnchorModifierKeys::get(eAnchorModifierKeyMask mask) const
{
  switch (mask) {
    case ANCHOR_ModifierKeyLeftShift:
      return m_LeftShift;
    case ANCHOR_ModifierKeyRightShift:
      return m_RightShift;
    case ANCHOR_ModifierKeyLeftAlt:
      return m_LeftAlt;
    case ANCHOR_ModifierKeyRightAlt:
      return m_RightAlt;
    case ANCHOR_ModifierKeyLeftControl:
      return m_LeftControl;
    case ANCHOR_ModifierKeyRightControl:
      return m_RightControl;
    case ANCHOR_ModifierKeyOS:
      return m_OS;
    default:
      return false;
  }
}

void AnchorModifierKeys::set(eAnchorModifierKeyMask mask, bool down)
{
  switch (mask) {
    case ANCHOR_ModifierKeyLeftShift:
      m_LeftShift = down;
      break;
    case ANCHOR_ModifierKeyRightShift:
      m_RightShift = down;
      break;
    case ANCHOR_ModifierKeyLeftAlt:
      m_LeftAlt = down;
      break;
    case ANCHOR_ModifierKeyRightAlt:
      m_RightAlt = down;
      break;
    case ANCHOR_ModifierKeyLeftControl:
      m_LeftControl = down;
      break;
    case ANCHOR_ModifierKeyRightControl:
      m_RightControl = down;
      break;
    case ANCHOR_ModifierKeyOS:
      m_OS = down;
      break;
    default:
      break;
  }
}

void AnchorModifierKeys::clear()
{
  m_LeftShift = false;
  m_RightShift = false;
  m_LeftAlt = false;
  m_RightAlt = false;
  m_LeftControl = false;
  m_RightControl = false;
  m_OS = false;
}

bool AnchorModifierKeys::equals(const AnchorModifierKeys &keys) const
{
  return (m_LeftShift == keys.m_LeftShift) && (m_RightShift == keys.m_RightShift) &&
         (m_LeftAlt == keys.m_LeftAlt) && (m_RightAlt == keys.m_RightAlt) &&
         (m_LeftControl == keys.m_LeftControl) && (m_RightControl == keys.m_RightControl) &&
         (m_OS == keys.m_OS);
}