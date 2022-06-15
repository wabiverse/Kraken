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

class AnchorRect
{
 public:

  /**
   * Constructs a rectangle with the given values.
   * @param l: requested left coordinate of the rectangle.
   * @param t: requested top coordinate of the rectangle.
   * @param r: requested right coordinate of the rectangle.
   * @param b: requested bottom coordinate of the rectangle. */
  AnchorRect(AnchorS32 l = 0, AnchorS32 t = 0, AnchorS32 r = 0, AnchorS32 b = 0)
    : m_l(l),
      m_t(t),
      m_r(r),
      m_b(b)
  {}

  /**
   * Destructor.
   */
  virtual ~AnchorRect() {}

  /**
   * Access to rectangle width.
   * @return width of the rectangle. */
  virtual inline AnchorS32 getWidth() const;

  /**
   * Access to rectangle height.
   * @return height of the rectangle. */
  virtual inline AnchorS32 getHeight() const;

  /**
   * Sets all members of the rectangle.
   * @param l: requested left coordinate of the rectangle.
   * @param t: requested top coordinate of the rectangle.
   * @param r: requested right coordinate of the rectangle.
   * @param b: requested bottom coordinate of the rectangle. */
  virtual inline void set(AnchorS32 l, AnchorS32 t, AnchorS32 r, AnchorS32 b);

  /**
   * Returns whether this rectangle is empty.
   * Empty rectangles are rectangles that have width==0 and/or height==0.
   * @return boolean value (true==empty rectangle) */
  virtual inline bool isEmpty() const;

  /**
   * Returns whether this rectangle is valid.
   * Valid rectangles are rectangles that have m_l <= m_r and m_t <= m_b.
   * Thus, empty rectangles are valid.
   * @return boolean value (true==valid rectangle) */
  virtual inline bool isValid() const;

  /**
   * Grows (or shrinks the rectangle).
   * The method avoids negative insets making the rectangle invalid
   * @param i: The amount of offset given to each extreme (negative values shrink the rectangle).
   */
  virtual void inset(AnchorS32 i);

  /**
   * Does a union of the rectangle given and this rectangle.
   * The result is stored in this rectangle.
   * @param r: The rectangle that is input for the union operation. */
  virtual inline void unionRect(const AnchorRect &r);

  /**
   * Grows the rectangle to included a point.
   * @param x: The x-coordinate of the point.
   * @param y: The y-coordinate of the point. */
  virtual inline void unionPoint(AnchorS32 x, AnchorS32 y);

  /**
   * Grows the rectangle to included a point.
   * @param x: The x-coordinate of the point.
   * @param y: The y-coordinate of the point. */
  virtual inline void wrapPoint(AnchorS32 &x, AnchorS32 &y, AnchorS32 ofs, eAnchorAxisFlag axis);

  /**
   * Returns whether the point is inside this rectangle.
   * Point on the boundary is considered inside.
   * @param x: x-coordinate of point to test.
   * @param y: y-coordinate of point to test.
   * @return boolean value (true if point is inside). */
  virtual inline bool isInside(AnchorS32 x, AnchorS32 y) const;

  /**
   * Returns whether the rectangle is inside this rectangle.
   * @param r: rectangle to test.
   * @return visibility (not, partially or fully visible). */
  virtual eAnchorVisibility getVisibility(AnchorRect &r) const;

  /**
   * Sets rectangle members.
   * Sets rectangle members such that it is centered at the given location.
   * @param cx: requested center x-coordinate of the rectangle.
   * @param cy: requested center y-coordinate of the rectangle. */
  virtual void setCenter(AnchorS32 cx, AnchorS32 cy);

  /**
   * Sets rectangle members.
   * Sets rectangle members such that it is centered at the given location,
   * with the width requested.
   * @param cx: requested center x-coordinate of the rectangle.
   * @param cy: requested center y-coordinate of the rectangle.
   * @param w: requested width of the rectangle.
   * @param h: requested height of the rectangle. */
  virtual void setCenter(AnchorS32 cx, AnchorS32 cy, AnchorS32 w, AnchorS32 h);

  /**
   * Clips a rectangle.
   * Updates the rectangle given such that it will fit within this one.
   * This can result in an empty rectangle.
   * @param r: the rectangle to clip.
   * @return whether clipping has occurred */
  virtual bool clip(AnchorRect &r) const;

  /** Left coordinate of the rectangle */
  AnchorS32 m_l;
  /** Top coordinate of the rectangle */
  AnchorS32 m_t;
  /** Right coordinate of the rectangle */
  AnchorS32 m_r;
  /** Bottom coordinate of the rectangle */
  AnchorS32 m_b;
};

inline AnchorS32 AnchorRect::getWidth() const
{
  return m_r - m_l;
}

inline AnchorS32 AnchorRect::getHeight() const
{
  return m_b - m_t;
}

inline void AnchorRect::set(AnchorS32 l, AnchorS32 t, AnchorS32 r, AnchorS32 b)
{
  m_l = l;
  m_t = t;
  m_r = r;
  m_b = b;
}

inline bool AnchorRect::isEmpty() const
{
  return (getWidth() == 0) || (getHeight() == 0);
}

inline bool AnchorRect::isValid() const
{
  return (m_l <= m_r) && (m_t <= m_b);
}

inline void AnchorRect::unionRect(const AnchorRect &r)
{
  if (r.m_l < m_l)
    m_l = r.m_l;
  if (r.m_r > m_r)
    m_r = r.m_r;
  if (r.m_t < m_t)
    m_t = r.m_t;
  if (r.m_b > m_b)
    m_b = r.m_b;
}

inline void AnchorRect::unionPoint(AnchorS32 x, AnchorS32 y)
{
  if (x < m_l)
    m_l = x;
  if (x > m_r)
    m_r = x;
  if (y < m_t)
    m_t = y;
  if (y > m_b)
    m_b = y;
}

inline void AnchorRect::wrapPoint(AnchorS32 &x, AnchorS32 &y, AnchorS32 ofs, eAnchorAxisFlag axis)
{
  AnchorS32 w = getWidth();
  AnchorS32 h = getHeight();

  /* highly unlikely but avoid eternal loop */
  if (w - ofs * 2 <= 0 || h - ofs * 2 <= 0) {
    return;
  }

  if (axis & ANCHOR_GrabAxisX) {
    while (x - ofs < m_l)
      x += w - (ofs * 2);
    while (x + ofs > m_r)
      x -= w - (ofs * 2);
  }
  if (axis & ANCHOR_GrabAxisY) {
    while (y - ofs < m_t)
      y += h - (ofs * 2);
    while (y + ofs > m_b)
      y -= h - (ofs * 2);
  }
}

inline bool AnchorRect::isInside(AnchorS32 x, AnchorS32 y) const
{
  return (x >= m_l) && (x <= m_r) && (y >= m_t) && (y <= m_b);
}