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
#include "ANCHOR_event.h"
#include "ANCHOR_event_manager.h"

class ANCHOR_EventManager {
 public:
  /**
   * Constructor. */
  ANCHOR_EventManager();

  /**
   * Destructor. */
  ~ANCHOR_EventManager();

  /**
   * Adds a consumer to the list of event consumers.
   * @param consumer: The consumer added to the list.
   * @return Indication as to whether addition has succeeded. */
  eAnchorStatus addConsumer(ANCHOR_IEventConsumer *consumer);

  /**
   * Removes a consumer from the list of event consumers.
   * @param consumer: The consumer removed from the list.
   * @return Indication as to whether removal has succeeded. */
  eAnchorStatus removeConsumer(ANCHOR_IEventConsumer *consumer);

 protected:
  /** A stack with events. */
  typedef std::deque<ANCHOR_IEvent *> EventStack;

  /** The event stack. */
  std::deque<ANCHOR_IEvent *> m_events;
  std::deque<ANCHOR_IEvent *> m_handled_events;

  /** A vector with event consumers. */
  typedef std::vector<ANCHOR_IEventConsumer *> ConsumerVector;

  /** The list with event consumers. */
  ConsumerVector m_consumers;

 protected:
  /**
   * Removes all events from the stack. */
  void destroyEvents();
};