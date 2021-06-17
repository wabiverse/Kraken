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

#include "ANCHOR_window.h"

#include <assert.h>

ANCHOR_SystemWindow::ANCHOR_SystemWindow(AnchorU32 width,
                                         AnchorU32 height,
                                         eAnchorWindowState state,
                                         const bool wantStereoVisual,
                                         const bool /*exclusive*/)
  : m_drawingContextType(ANCHOR_DrawingContextTypeNone),
    m_cursorVisible(true),
    m_cursorGrab(ANCHOR_GrabDisable),
    m_cursorShape(ANCHOR_StandardCursorDefault),
    m_wantStereoVisual(wantStereoVisual)
{
  m_isUnsavedChanges = false;
  m_canAcceptDragOperation = false;

  m_progressBarVisible = false;

  m_cursorGrabAccumPos[0] = 0;
  m_cursorGrabAccumPos[1] = 0;

  m_nativePixelSize = 1.0f;

  m_fullScreen = state == ANCHOR_WindowStateFullScreen;
  if (m_fullScreen) {
    m_fullScreenWidth = width;
    m_fullScreenHeight = height;
  }
}

ANCHOR_SystemWindow::~ANCHOR_SystemWindow()
{
  ANCHOR::SetCurrentContext(NULL);
}

void *ANCHOR_SystemWindow::getOSWindow() const
{
  return NULL;
}

eAnchorStatus ANCHOR_SystemWindow::setDrawingContextType(eAnchorDrawingContextType type)
{
  if (type != m_drawingContextType) {
    ANCHOR::SetCurrentContext(NULL);

    if (type != ANCHOR_DrawingContextTypeNone)
      newDrawingContext(type);

    if (ANCHOR::GetCurrentContext() != NULL) {
      m_drawingContextType = type;
    }
    else {
      ANCHOR::CreateContext();
      m_drawingContextType = ANCHOR_DrawingContextTypeNone;
    }

    return (type == m_drawingContextType) ? ANCHOR_SUCCESS : ANCHOR_ERROR;
  }
  else {
    return ANCHOR_SUCCESS;
  }
}

eAnchorStatus ANCHOR_SystemWindow::swapBuffers()
{
  ANCHOR::SwapChain((ANCHOR_SystemWindowHandle)this);
  return ANCHOR_SUCCESS;
}

eAnchorStatus ANCHOR_SystemWindow::setModifiedState(bool isUnsavedChanges)
{
  m_isUnsavedChanges = isUnsavedChanges;

  return ANCHOR_SUCCESS;
}

bool ANCHOR_SystemWindow::getModifiedState()
{
  return m_isUnsavedChanges;
}