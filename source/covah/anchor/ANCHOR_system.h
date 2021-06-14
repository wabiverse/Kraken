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

class ANCHOR_ISystem {
 public:
  /**
   * Creates the one and only system.
   * @return An indication of success. */
  static eAnchorStatus createSystem();

  /**
   * Destroys the one and only system.
   * @return An indication of success. */
  static eAnchorStatus destroySystem();

  /**
   * Returns a pointer to the one and only
   * system (nil if it hasn't been created).
   * @return A pointer to the system. */
  static ANCHOR_ISystem *getSystem();

 protected:
  /**
   * Constructor.
   * Protected default constructor to
   * force use of static createSystem
   * member. */
  ANCHOR_ISystem()
  {}

  /**
   * Destructor.
   * Protected default constructor to
   * force use of static dispose member. */
  virtual ~ANCHOR_ISystem()
  {}
};

class ANCHOR_System : public ANCHOR_ISystem {
 protected:
  /**
   * Constructor.
   * Protected default constructor to force use of static createSystem member. */
  ANCHOR_System();

  /**
   * Destructor.
   * Protected default constructor to force use of static dispose member. */
  virtual ~ANCHOR_System();

 public:
  /**
   * Pushes an event on the stack.
   * To dispatch it, call dispatchEvent()
   * or dispatchEvents().
   * Do not delete the event!
   * @param event: The event to push on the stack. */
  eAnchorStatus pushEvent(ANCHOR_IEvent *event);

 protected:
  /**
   * The window manager. */
  ANCHOR_WindowManager *m_windowManager;
};

ANCHOR_API
ANCHOR_SystemHandle ANCHOR_CreateSystem(int backend);
