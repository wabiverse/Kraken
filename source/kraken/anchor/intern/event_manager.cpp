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

#include "ANCHOR_event.h"
#include "ANCHOR_event_consumer.h"
#include "ANCHOR_event_manager.h"

#include <algorithm>
#include <stdio.h>

AnchorEventManager::AnchorEventManager()
{}

AnchorEventManager::~AnchorEventManager()
{
  destroyEvents();
  ConsumerVector::iterator iter = m_consumers.begin();
  while (iter != m_consumers.end())
  {
    AnchorIEventConsumer *consumer = *iter;
    delete consumer;
    iter = m_consumers.erase(iter);
  }
}

AnchorU32 AnchorEventManager::getNumEvents()
{
  return (AnchorU32)m_events.size();
}

AnchorU32 AnchorEventManager::getNumEvents(eAnchorEventType type)
{
  AnchorU32 numEvents = 0;
  EventStack::iterator p;
  for (p = m_events.begin(); p != m_events.end(); ++p)
  {
    if ((*p)->getType() == type)
    {
      numEvents++;
    }
  }
  return numEvents;
}

eAnchorStatus AnchorEventManager::pushEvent(AnchorIEvent *event)
{
  eAnchorStatus success;
  ANCHOR_ASSERT(event);
  if (m_events.size() < m_events.max_size())
  {
    m_events.push_front(event);
    success = ANCHOR_SUCCESS;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

void AnchorEventManager::dispatchEvent(AnchorIEvent *event)
{
  ConsumerVector::iterator iter;

  for (iter = m_consumers.begin(); iter != m_consumers.end(); ++iter)
  {
    (*iter)->processEvent(event);
  }
}

void AnchorEventManager::dispatchEvent()
{
  AnchorIEvent *event = m_events.back();
  m_events.pop_back();
  m_handled_events.push_back(event);

  dispatchEvent(event);
}

void AnchorEventManager::dispatchEvents()
{
  while (!m_events.empty())
  {
    dispatchEvent();
  }

  destroyEvents();
}

eAnchorStatus AnchorEventManager::addConsumer(AnchorIEventConsumer *consumer)
{
  eAnchorStatus success;
  ANCHOR_ASSERT(consumer);

  /**
   * Check to see whether the consumer is already in our list. */
  ConsumerVector::const_iterator iter = std::find(m_consumers.begin(), m_consumers.end(), consumer);

  if (iter == m_consumers.end())
  {
    /**
     * Add the consumer. */
    m_consumers.push_back(consumer);
    success = ANCHOR_SUCCESS;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

eAnchorStatus AnchorEventManager::removeConsumer(AnchorIEventConsumer *consumer)
{
  eAnchorStatus success;
  ANCHOR_ASSERT(consumer);

  /**
   * Check to see whether the consumer is in our list. */
  ConsumerVector::iterator iter = std::find(m_consumers.begin(), m_consumers.end(), consumer);

  if (iter != m_consumers.end())
  {
    /**
     * Remove the consumer. */
    m_consumers.erase(iter);
    success = ANCHOR_SUCCESS;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

void AnchorEventManager::destroyEvents()
{
  while (m_handled_events.empty() == false)
  {
    ANCHOR_ASSERT(m_handled_events[0]);
    delete m_handled_events[0];
    m_handled_events.pop_front();
  }

  while (m_events.empty() == false)
  {
    ANCHOR_ASSERT(m_events[0]);
    delete m_events[0];
    m_events.pop_front();
  }
}