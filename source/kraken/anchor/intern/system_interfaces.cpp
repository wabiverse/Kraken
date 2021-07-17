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

#include "ANCHOR_system.h"

#if defined(__linux__)
#  include "ANCHOR_BACKEND_sdl.h"
#elif defined(WIN32)
#  include "ANCHOR_BACKEND_win32.h"
#elif defined(__APPLE__)
#  include "ANCHOR_BACKEND_cocoa.h"
#endif

AnchorISystem *AnchorISystem::m_system = NULL;

eAnchorStatus AnchorISystem::createSystem()
{
  eAnchorStatus success;
  if (!m_system)
  {
#if defined(__linux__)
    m_system = new AnchorSystemSDL();
#elif defined(WIN32)
    m_system = new AnchorSystemWin32();
#elif defined(__APPLE__)
    m_system = new AnchorSystemCocoa();
#endif
    success = (m_system != NULL) ? ANCHOR_SUCCESS : ANCHOR_FAILURE;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }

  if (success == ANCHOR_SUCCESS)
  {
    success = m_system->init();
  }
  return success;
}

eAnchorStatus AnchorISystem::destroySystem()
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  if (m_system)
  {
    delete m_system;
    m_system = NULL;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

AnchorISystem *AnchorISystem::getSystem()
{
  return m_system;
}