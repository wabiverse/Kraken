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

class ANCHOR_IEventConsumer
{
 public:
  /**
   * Destructor. */
  virtual ~ANCHOR_IEventConsumer()
  {}

  /**
   * This method is called by the system when it has events to dispatch.
   * @see ANCHOR_System#dispatchEvents
   * @param event: The event that can be handled or ignored.
   * @return Indication as to whether the event was handled. */
  virtual bool processEvent(ANCHOR_IEvent *event) = 0;
};

/**
 * Callback routine that threads the Kraken backend. */
typedef int (*ANCHOR_EventCallbackProcPtr)(ANCHOR_EventHandle event, ANCHOR_UserPtr userdata);

ANCHOR_EventConsumerHandle ANCHOR_CreateEventConsumer(ANCHOR_EventCallbackProcPtr eventCallback,
                                                      ANCHOR_UserPtr userdata);

class ANCHOR_CallbackEventConsumer : public ANCHOR_IEventConsumer
{
 public:
  /**
   * Constructor.
   * @param eventCallback: The call-back routine invoked.
   * @param userData: The data passed back through the call-back routine. */
  ANCHOR_CallbackEventConsumer(ANCHOR_EventCallbackProcPtr eventCallback, ANCHOR_UserPtr userData);

  /**
   * Destructor. */
  ~ANCHOR_CallbackEventConsumer(void)
  {}

  /**
   * This method is called by an event producer when an event is available.
   * @param event: The event that can be handled or ignored.
   * @return Indication as to whether the event was handled. */
  bool processEvent(ANCHOR_IEvent *event);

 protected:
  /** The call-back routine invoked. */
  ANCHOR_EventCallbackProcPtr m_eventCallback;
  /** The data passed back through the call-back routine. */
  ANCHOR_UserPtr m_userData;
};
