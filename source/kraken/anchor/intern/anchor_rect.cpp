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

#include "ANCHOR_rect.h"

void AnchorRect::inset(AnchorS32 i)
{
  if (i > 0) {
    // Grow the rectangle
    m_l -= i;
    m_r += i;
    m_t -= i;
    m_b += i;
  } else if (i < 0) {
    // Shrink the rectangle, check for insets larger than half the size
    AnchorS32 i2 = i * 2;
    if (getWidth() > i2) {
      m_l += i;
      m_r -= i;
    } else {
      m_l = m_l + ((m_r - m_l) / 2);
      m_r = m_l;
    }
    if (getHeight() > i2) {
      m_t += i;
      m_b -= i;
    } else {
      m_t = m_t + ((m_b - m_t) / 2);
      m_b = m_t;
    }
  }
}

eAnchorVisibility AnchorRect::getVisibility(AnchorRect &r) const
{
  bool lt = isInside(r.m_l, r.m_t);
  bool rt = isInside(r.m_r, r.m_t);
  bool lb = isInside(r.m_l, r.m_b);
  bool rb = isInside(r.m_r, r.m_b);
  eAnchorVisibility v;
  if (lt && rt && lb && rb) {
    // All points inside, rectangle is inside this
    v = ANCHOR_FullyVisible;
  } else if (!(lt || rt || lb || rb)) {
    // None of the points inside
    // Check to see whether the rectangle is larger than this one
    if ((r.m_l < m_l) && (r.m_t < m_t) && (r.m_r > m_r) && (r.m_b > m_b)) {
      v = ANCHOR_PartiallyVisible;
    } else {
      v = ANCHOR_NotVisible;
    }
  } else {
    // Some of the points inside, rectangle is partially inside
    v = ANCHOR_PartiallyVisible;
  }
  return v;
}

void AnchorRect::setCenter(AnchorS32 cx, AnchorS32 cy)
{
  AnchorS32 offset = cx - (m_l + (m_r - m_l) / 2);
  m_l += offset;
  m_r += offset;
  offset = cy - (m_t + (m_b - m_t) / 2);
  m_t += offset;
  m_b += offset;
}

void AnchorRect::setCenter(AnchorS32 cx, AnchorS32 cy, AnchorS32 w, AnchorS32 h)
{
  long w_2, h_2;

  w_2 = w >> 1;
  h_2 = h >> 1;
  m_l = cx - w_2;
  m_t = cy - h_2;
  m_r = m_l + w;
  m_b = m_t + h;
}

bool AnchorRect::clip(AnchorRect &r) const
{
  bool clipped = false;
  if (r.m_l < m_l) {
    r.m_l = m_l;
    clipped = true;
  }
  if (r.m_t < m_t) {
    r.m_t = m_t;
    clipped = true;
  }
  if (r.m_r > m_r) {
    r.m_r = m_r;
    clipped = true;
  }
  if (r.m_b > m_b) {
    r.m_b = m_b;
    clipped = true;
  }
  return clipped;
}
