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

#include "ANCHOR_api.h"
#include "ANCHOR_event_consumer.h"

ANCHOR_CallbackEventConsumer::ANCHOR_CallbackEventConsumer(AnchorEventCallbackProcPtr eventCallback,
                                                           ANCHOR_UserPtr userData)
{
  m_eventCallback = eventCallback;
  m_userData = userData;
}

bool ANCHOR_CallbackEventConsumer::processEvent(AnchorIEvent *event)
{
  return m_eventCallback((AnchorEventHandle)event, m_userData) != 0;
}

AnchorEventConsumerHandle ANCHOR_CreateEventConsumer(AnchorEventCallbackProcPtr eventCallback,
                                                     ANCHOR_UserPtr userdata)
{
  return (AnchorEventConsumerHandle) new ANCHOR_CallbackEventConsumer(eventCallback, userdata);
}
