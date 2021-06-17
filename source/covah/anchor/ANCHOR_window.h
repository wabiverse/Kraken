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
   * Returns the associated OS object/handle
   * @return The associated OS object/handle */
  virtual void *getOSWindow() const = 0;

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

  /**
   * Sets the window "modified" status, indicating unsaved changes
   * @param isUnsavedChanges: Unsaved changes or not.
   * @return Indication of success. */
  virtual eAnchorStatus setModifiedState(bool isUnsavedChanges) = 0;

  /**
   * Returns the window user data.
   * @return The window user data. */
  virtual ANCHOR_UserPtr getUserData() const = 0;

  /**
   * Gets the window "modified" status, indicating unsaved changes
   * @return True if there are unsaved changes */
  virtual bool getModifiedState() = 0;

  /**
   * Swaps front and back buffers of a window.
   * @return A boolean success indicator. */
  virtual eAnchorStatus swapBuffers() = 0;

  /** */
  virtual eAnchorStatus beginFullScreen() const = 0;
  virtual eAnchorStatus endFullScreen() const = 0;

  virtual float getNativePixelSize(void) = 0;

  /**
   * Returns the recommended DPI for this window.
   * @return The recommended DPI for this window. */
  virtual AnchorU16 getDPIHint() = 0;
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
                      const bool exclusive = false);

  virtual ~ANCHOR_SystemWindow();

  /**
   * Returns indication as to whether the window is valid.
   * @return The validity of the window. */
  virtual bool getValid() const
  {
    return (ANCHOR::GetCurrentContext() != NULL);
  }

  /**
   * Returns the associated OS object/handle
   * @return The associated OS object/handle */
  virtual void *getOSWindow() const;

  /**
   * Swaps front and back buffers of a window.
   * @return A boolean success indicator. */
  virtual eAnchorStatus swapBuffers();

  /**
   * Tries to install a rendering context in this window.
   * Child classes do not need to overload this method,
   * They should overload #newDrawingContext instead.
   * @param type: The type of rendering context installed.
   * @return Indication as to whether installation has succeeded. */
  eAnchorStatus setDrawingContextType(eAnchorDrawingContextType type);

  /**
   * Returns the window user data.
   * @return The window user data. */
  inline ANCHOR_UserPtr getUserData() const
  {
    return m_userData;
  }

  /**
   * Returns the recommended DPI for this window.
   * @return The recommended DPI for this window. */
  virtual inline AnchorU16 getDPIHint()
  {
    return 96;
  }

  float getNativePixelSize(void)
  {
    if (m_nativePixelSize > 0.0f)
      return m_nativePixelSize;
    return 1.0f;
  }

  /**
   * Sets the window "modified" status, indicating unsaved changes
   * @param isUnsavedChanges: Unsaved changes or not.
   * @return Indication of success. */
  virtual eAnchorStatus setModifiedState(bool isUnsavedChanges);

  /**
   * Gets the window "modified" status, indicating unsaved changes
   * @return True if there are unsaved changes */
  virtual bool getModifiedState();

 protected:
  /**
   * Tries to install a rendering context in this window.
   * @param type: The type of rendering context installed.
   * @return Indication as to whether installation has succeeded. */
  virtual void newDrawingContext(eAnchorDrawingContextType type) = 0;

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
