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

class ANCHOR_ISystemWindow {
 public:
  /**
   * Destructor. */
  virtual ~ANCHOR_ISystemWindow()
  {}

  /**
   * Returns indication as to whether the window is valid.
   * @return The validity of the window. */
  virtual bool getValid() const = 0;

  /**
   * Tries to install a rendering context in this window.
   * @param type: The type of rendering context installed.
   * @return Indication as to whether installation has succeeded. */
  virtual eAnchorStatus setDrawingContextType(eAnchorDrawingContextType type) = 0;

  /**
   * Sets the title displayed in the title bar.
   * @param title: The title to display in the
   * title bar. */
  virtual void setTitle(const char *title) = 0;

  /**
   * Sets the icon displayed in the title bar.
   * @param icon: The icon path to display in
   * the title bar. */
  virtual void setIcon(const char *icon) = 0;
};

class ANCHOR_SystemWindow : public ANCHOR_ISystemWindow {
 public:
  /**
   * Constructor.
   * Creates a new window and opens it.
   * To check if the window was created properly, use the getValid() method.
   * @param width: The width the window.
   * @param heigh: The height the window.
   * @param state: The state the window is initially opened with.
   * @param type: The type of drawing context installed in this window.
   * @param stereoVisual: Stereo visual for quad buffered stereo.
   * @param exclusive: Use to show the window ontop and ignore others (used full-screen). */
  ANCHOR_SystemWindow(AnchorU32 width,
                      AnchorU32 height,
                      eAnchorWindowState state,
                      const bool wantStereoVisual = false,
                      const bool exclusive        = false);

  virtual ~ANCHOR_SystemWindow();

  /**
   * Returns indication as to whether the window is valid.
   * @return The validity of the window. */
  virtual bool getValid() const
  {
    return (ANCHOR::GetCurrentContext() != NULL);
  }

  /**
   * Tries to install a rendering context in this window.
   * Child classes do not need to overload this method,
   * They should overload #newDrawingContext instead.
   * @param type: The type of rendering context installed.
   * @return Indication as to whether installation has succeeded. */
  eAnchorStatus setDrawingContextType(eAnchorDrawingContextType type);

 protected:
  /**
   * Tries to install a rendering context in this window.
   * @param type: The type of rendering context installed.
   * @return Indication as to whether installation has succeeded. */
  virtual ANCHOR_Context *newDrawingContext(eAnchorDrawingContextType type) = 0;

 protected:
  /** The drawing context installed in this window. */
  eAnchorDrawingContextType m_drawingContextType;

  /** The window user data */
  ANCHOR_UserPtr m_userData;

  /** The current visibility of the cursor */
  bool m_cursorVisible;

  /** The current grabbed state of the cursor */
  eAnchorGrabCursorMode m_cursorGrab;

  /** Accumulated offset from m_cursorGrabInitPos. */
  AnchorS32 m_cursorGrabAccumPos[2];

  /** The current shape of the cursor */
  eAnchorStandardCursor m_cursorShape;

  /** The presence of progress indicator with the application icon */
  bool m_progressBarVisible;

  /** The acceptance of the "drop candidate" of the current drag'n'drop operation */
  bool m_canAcceptDragOperation;

  /** Modified state : are there unsaved changes */
  bool m_isUnsavedChanges;

  /** Stores whether this is a full screen window. */
  bool m_fullScreen;

  /** Whether to attempt to initialize a context with a stereo frame-buffer. */
  bool m_wantStereoVisual;

  /** Full-screen width */
  AnchorU32 m_fullScreenWidth;
  /** Full-screen height */
  AnchorU32 m_fullScreenHeight;

  /* macOS only, retina screens */
  float m_nativePixelSize;
};