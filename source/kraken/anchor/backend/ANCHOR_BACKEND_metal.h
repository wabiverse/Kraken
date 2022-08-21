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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#pragma once

#include "ANCHOR_api.h"
#include "ANCHOR_display_manager.h"
#include "ANCHOR_event.h"
#include "ANCHOR_system.h"
#include "ANCHOR_window.h"

#if WITH_METAL
# include <wabi/imaging/hgiMetal/hgi.h>
# include <wabi/imaging/hgiMetal/capabilities.h>
#endif /* WITH_METAL */

@class AnchorWindowApple;
@class CAMetalLayer;
@class CocoaMetalView;
@class CocoaWindow;
@class NSCursor;
@class NSObject;

class AnchorSystemCocoa;

class AnchorAppleMetal : public AnchorSystemWindow
{
 public:

  AnchorAppleMetal(AnchorSystemCocoa *systemCocoa,
                    const char *title,
                    AnchorS32 left,
                    AnchorS32 top,
                    AnchorU32 width,
                    AnchorU32 height,
                    eAnchorWindowState state,
                    bool dialog = false);

  ~AnchorAppleMetal();

  bool getValid() const;

  eAnchorStatus activateDrawingContext();

  eAnchorStatus swapBuffers();

  AnchorU16 getDPIHint();

  eAnchorStatus setModifiedState(bool isUnsavedChanges);
  bool getModifiedState();

  void newDrawingContext(eAnchorDrawingContextType type);

  void *getOSWindow() const;

  void setTitle(const char *title);
  std::string getTitle() const;

  void setIcon(const char *icon);

  void getWindowBounds(AnchorRect &bounds) const;

  eAnchorStatus setClientSize(AnchorU32 width, AnchorU32 height);
  void getClientBounds(AnchorRect &bounds) const;

  eAnchorStatus setState(eAnchorWindowState state);
  eAnchorWindowState getState() const;

  void screenToClient(AnchorS32 inX, AnchorS32 inY, AnchorS32 &outX, AnchorS32 &outY) const;
  void clientToScreen(AnchorS32 inX, AnchorS32 inY, AnchorS32 &outX, AnchorS32 &outY) const;

  eAnchorStatus setOrder(eAnchorWindowOrder order);

  NSCursor *getStandardCursor(eAnchorStandardCursor shape) const;
  eAnchorStatus hasCursorShape(eAnchorStandardCursor shape);
  void loadCursor(bool visible, eAnchorStandardCursor cursorShape) const;

  bool isDialog() const;

  eAnchorStatus setProgressBar(float progress);
  eAnchorStatus endProgressBar();

  eAnchorStatus beginFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

  eAnchorStatus endFullScreen() const
  {
    return ANCHOR_FAILURE;
  }

 protected:

  void SetupMetal();

 protected:
  /* The swift window which holds the metal view. */
  AnchorWindowApple *m_window;

  /* The Metal view. */
  CocoaMetalView *m_metalView;
  CAMetalLayer *m_metalLayer;

  /* The SystemCocoa class to send events. */
  AnchorSystemCocoa *m_systemCocoa;

  /* To set the cursor. */
  NSCursor *m_cursor;

  bool m_immediateDraw;
  bool m_debug_context;
  bool m_is_dialog;

  wabi::HgiMetal *m_hgi;
};