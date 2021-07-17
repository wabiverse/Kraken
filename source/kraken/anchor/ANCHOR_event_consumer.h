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

class AnchorIEventConsumer
{
 public:
  /**
   * Destructor. */
  virtual ~AnchorIEventConsumer()
  {}

  /**
   * This method is called by the system when it has events to dispatch.
   * @see AnchorSystem#dispatchEvents
   * @param event: The event that can be handled or ignored.
   * @return Indication as to whether the event was handled. */
  virtual bool processEvent(AnchorIEvent *event) = 0;
};

/**
 * Callback routine that threads the Kraken backend. */
typedef int (*AnchorEventCallbackProcPtr)(AnchorEventHandle event, ANCHOR_UserPtr userdata);

AnchorEventConsumerHandle ANCHOR_CreateEventConsumer(AnchorEventCallbackProcPtr eventCallback,
                                                     ANCHOR_UserPtr userdata);

class ANCHOR_CallbackEventConsumer : public AnchorIEventConsumer
{
 public:
  /**
   * Constructor.
   * @param eventCallback: The call-back routine invoked.
   * @param userData: The data passed back through the call-back routine. */
  ANCHOR_CallbackEventConsumer(AnchorEventCallbackProcPtr eventCallback, ANCHOR_UserPtr userData);

  /**
   * Destructor. */
  ~ANCHOR_CallbackEventConsumer(void)
  {}

  /**
   * This method is called by an event producer when an event is available.
   * @param event: The event that can be handled or ignored.
   * @return Indication as to whether the event was handled. */
  bool processEvent(AnchorIEvent *event);

 protected:
  /** The call-back routine invoked. */
  AnchorEventCallbackProcPtr m_eventCallback;
  /** The data passed back through the call-back routine. */
  ANCHOR_UserPtr m_userData;
};
