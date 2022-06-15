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

#include <wabi/base/tf/refPtr.h>

class AnchorIEvent
{
 public:

  /**
   * Destructor. */
  virtual ~AnchorIEvent() {}

  /**
   * Returns the event type.
   * @return The event type. */
  virtual eAnchorEventType getType() = 0;

  /**
   * Returns the time this event was generated.
   * @return The event generation time. */
  virtual AnchorU64 getTime() = 0;

  /**
   * Returns the window this event was generated on,
   * or NULL if it is a 'system' event.
   * @return The generating window. */
  virtual AnchorISystemWindow *getWindow() = 0;

  /**
   * Returns the event data.
   * @return The event data. */
  virtual AnchorEventDataPtr getData() = 0;
};

/**
 * Base class for events received the operating system. */
class AnchorEvent : public AnchorIEvent
{
 public:

  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of this event.
   * @param window: The generating window (or NULL if system event). */
  AnchorEvent(AnchorU64 msec, eAnchorEventType type, AnchorISystemWindow *window)
    : m_type(type),
      m_time(msec),
      m_window(window),
      m_data(NULL)
  {}

  /**
   * Returns the event type.
   * @return The event type. */
  eAnchorEventType getType()
  {
    return m_type;
  }

  /**
   * Returns the time this event was generated.
   * @return The event generation time. */
  AnchorU64 getTime()
  {
    return m_time;
  }

  /**
   * Returns the window this event was generated on,
   * or NULL if it is a 'system' event.
   * @return The generating window. */
  AnchorISystemWindow *getWindow()
  {
    return m_window;
  }

  /**
   * Returns the event data.
   * @return The event data. */
  AnchorEventDataPtr getData()
  {
    return m_data;
  }

 protected:

  /**
   * Type of this event. */
  eAnchorEventType m_type;
  /**
   * The time this event was generated. */
  AnchorU64 m_time;
  /**
   * Pointer to the generating window. */
  AnchorISystemWindow *m_window;
  /**
   * Pointer to the event data. */
  AnchorEventDataPtr m_data;
};

/**
 * Cursor event. */
class AnchorEventCursor : public AnchorEvent
{
 public:

  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of this event.
   * @param x: The x-coordinate of the location the cursor was at the time of the event.
   * @param y: The y-coordinate of the location the cursor was at the time of the event.
   * @param tablet: The tablet data associated with this event. */
  AnchorEventCursor(AnchorU64 msec,
                    eAnchorEventType type,
                    AnchorISystemWindow *window,
                    AnchorS32 x,
                    AnchorS32 y,
                    const AnchorTabletData &tablet)
    : AnchorEvent(msec, type, window),
      m_cursorEventData({x, y, tablet})
  {
    m_data = &m_cursorEventData;
  }

 protected:

  /** The x,y-coordinates of the cursor position. */
  AnchorEventCursorData m_cursorEventData;
};

/**
 * Mouse button event. */
class AnchorEventButton : public AnchorEvent
{
 public:

  /**
   * Constructor.
   * @param time: The time this event was generated.
   * @param type: The type of this event.
   * @param window: The window of this event.
   * @param button: The state of the buttons were at the time of the event.
   * @param tablet: The tablet data associated with this event. */
  AnchorEventButton(AnchorU64 time,
                    eAnchorEventType type,
                    AnchorISystemWindow *window,
                    eAnchorButtonMask button,
                    const AnchorTabletData &tablet)
    : AnchorEvent(time, type, window),
      m_buttonEventData({button, tablet})
  {
    m_data = &m_buttonEventData;
  }

 protected:

  /** The button event data. */
  AnchorEventButtonData m_buttonEventData;
};

/**
 * Mouse wheel event.
 * The displacement of the mouse wheel is counted in ticks.
 * A positive value means the wheel is turned away from the user. */
class AnchorEventWheel : public AnchorEvent
{
 public:

  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param window: The window of this event.
   * @param z: The displacement of the mouse wheel. */
  AnchorEventWheel(AnchorU64 msec, AnchorISystemWindow *window, AnchorS32 z)
    : AnchorEvent(msec, AnchorEventTypeWheel, window)
  {
    m_wheelEventData.z = z;
    m_data = &m_wheelEventData;
  }

 protected:

  /** The z-displacement of the mouse wheel. */
  AnchorEventWheelData m_wheelEventData;
};

/**
 * Key event. */
class AnchorEventKey : public AnchorEvent
{
 public:

  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of key event.
   * @param key: The key code of the key. */
  AnchorEventKey(AnchorU64 msec,
                 eAnchorEventType type,
                 AnchorISystemWindow *window,
                 eAnchorKey key,
                 bool is_repeat)
    : AnchorEvent(msec, type, window)
  {
    m_keyEventData.key = key;
    m_keyEventData.ascii = '\0';
    m_keyEventData.utf8_buf[0] = '\0';
    m_keyEventData.is_repeat = is_repeat;
    m_data = &m_keyEventData;
  }

  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of key event.
   * @param key: The key code of the key.
   * @param ascii: The ascii code for the key event. */
  AnchorEventKey(AnchorU64 msec,
                 eAnchorEventType type,
                 AnchorISystemWindow *window,
                 eAnchorKey key,
                 char ascii,
                 const char utf8_buf[6],
                 bool is_repeat)
    : AnchorEvent(msec, type, window)
  {
    m_keyEventData.key = key;
    m_keyEventData.ascii = ascii;
    if (utf8_buf)
      memcpy(m_keyEventData.utf8_buf, utf8_buf, sizeof(m_keyEventData.utf8_buf));
    else
      m_keyEventData.utf8_buf[0] = '\0';
    m_keyEventData.is_repeat = is_repeat;
    m_data = &m_keyEventData;
  }

 protected:

  /** The key event data. */
  AnchorEventKeyData m_keyEventData;
};
