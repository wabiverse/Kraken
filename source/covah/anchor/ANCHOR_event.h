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

class ANCHOR_IEvent {
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
  virtual ANCHOR_Time getTime() = 0;

  /**
   * Returns the window this event was generated on,
   * or NULL if it is a 'system' event.
   * @return The generating window. */
  virtual ANCHOR_Window *getWindow() = 0;

  /**
   * Returns the event data.
   * @return The event data. */
  virtual ANCHOR_EventPtr getData() = 0;
};