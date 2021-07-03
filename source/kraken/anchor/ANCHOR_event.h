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

#include <wabi/base/tf/refPtr.h>

class ANCHOR_IEvent
{
 public:
  /**
   * Destructor. */
  virtual ~ANCHOR_IEvent()
  {}

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
  virtual ANCHOR_ISystemWindow *getWindow() = 0;

  /**
   * Returns the event data.
   * @return The event data. */
  virtual ANCHOR_EventDataPtr getData() = 0;
};

/**
 * Base class for events received the operating system. */
class ANCHOR_Event : public ANCHOR_IEvent
{
 public:
  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of this event.
   * @param window: The generating window (or NULL if system event). */
  ANCHOR_Event(AnchorU64 msec, eAnchorEventType type, ANCHOR_ISystemWindow *window)
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
  ANCHOR_ISystemWindow *getWindow()
  {
    return m_window;
  }

  /**
   * Returns the event data.
   * @return The event data. */
  ANCHOR_EventDataPtr getData()
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
  ANCHOR_ISystemWindow *m_window;
  /**
   * Pointer to the event data. */
  ANCHOR_EventDataPtr m_data;
};

/**
 * Cursor event. */
class ANCHOR_EventCursor : public ANCHOR_Event
{
 public:
  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of this event.
   * @param x: The x-coordinate of the location the cursor was at the time of the event.
   * @param y: The y-coordinate of the location the cursor was at the time of the event.
   * @param tablet: The tablet data associated with this event. */
  ANCHOR_EventCursor(AnchorU64 msec,
                     eAnchorEventType type,
                     ANCHOR_ISystemWindow *window,
                     AnchorS32 x,
                     AnchorS32 y,
                     const ANCHOR_TabletData &tablet)
    : ANCHOR_Event(msec, type, window),
      m_cursorEventData({x, y, tablet})
  {
    m_data = &m_cursorEventData;
  }

 protected:
  /** The x,y-coordinates of the cursor position. */
  ANCHOR_EventCursorData m_cursorEventData;
};

/**
 * Mouse button event. */
class ANCHOR_EventButton : public ANCHOR_Event
{
 public:
  /**
   * Constructor.
   * @param time: The time this event was generated.
   * @param type: The type of this event.
   * @param window: The window of this event.
   * @param button: The state of the buttons were at the time of the event.
   * @param tablet: The tablet data associated with this event. */
  ANCHOR_EventButton(AnchorU64 time,
                     eAnchorEventType type,
                     ANCHOR_ISystemWindow *window,
                     eAnchorButtonMask button,
                     const ANCHOR_TabletData &tablet)
    : ANCHOR_Event(time, type, window),
      m_buttonEventData({button, tablet})
  {
    m_data = &m_buttonEventData;
  }

 protected:
  /** The button event data. */
  ANCHOR_EventButtonData m_buttonEventData;
};

/**
 * Mouse wheel event.
 * The displacement of the mouse wheel is counted in ticks.
 * A positive value means the wheel is turned away from the user. */
class ANCHOR_EventWheel : public ANCHOR_Event
{
 public:
  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param window: The window of this event.
   * @param z: The displacement of the mouse wheel. */
  ANCHOR_EventWheel(AnchorU64 msec, ANCHOR_ISystemWindow *window, AnchorS32 z)
    : ANCHOR_Event(msec, ANCHOR_EventTypeWheel, window)
  {
    m_wheelEventData.z = z;
    m_data = &m_wheelEventData;
  }

 protected:
  /** The z-displacement of the mouse wheel. */
  ANCHOR_EventWheelData m_wheelEventData;
};

/**
 * Key event. */
class ANCHOR_EventKey : public ANCHOR_Event
{
 public:
  /**
   * Constructor.
   * @param msec: The time this event was generated.
   * @param type: The type of key event.
   * @param key: The key code of the key. */
  ANCHOR_EventKey(AnchorU64 msec,
                  eAnchorEventType type,
                  ANCHOR_ISystemWindow *window,
                  eAnchorKey key,
                  bool is_repeat)
    : ANCHOR_Event(msec, type, window)
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
  ANCHOR_EventKey(AnchorU64 msec,
                  eAnchorEventType type,
                  ANCHOR_ISystemWindow *window,
                  eAnchorKey key,
                  char ascii,
                  const char utf8_buf[6],
                  bool is_repeat)
    : ANCHOR_Event(msec, type, window)
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
  ANCHOR_EventKeyData m_keyEventData;
};
