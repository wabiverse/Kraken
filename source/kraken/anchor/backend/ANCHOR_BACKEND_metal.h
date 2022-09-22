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
#  include <Metal/Metal.hpp>
#  include <KrakenOS/Kraken.OS.hpp>
#  include <wabi/imaging/hgiMetal/hgi.h>
#  include <wabi/imaging/hgiMetal/capabilities.h>
#endif /* WITH_METAL */

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

  /**
   * Sets the cursor visibility on the window using
   * native window system calls.
   */
  eAnchorStatus setWindowCursorVisibility(bool visible);

  void SetupRenderState(MTL::RenderCommandEncoder *enc,
                        MTL::Buffer *vertexBuffer,
                        AnchorDrawData *draw_data,
                        size_t vertexBufferOffset);

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

  /**
   * Sets the cursor shape on the window using
   * native window system calls.
   */
  eAnchorStatus setWindowCustomCursorShape(uint8_t *bitmap,
                                           uint8_t *mask,
                                           int sizex,
                                           int sizey,
                                           int hotX,
                                           int hotY,
                                           bool canInvertColor);

  void *getStandardCursor(eAnchorStandardCursor shape) const;
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

  KRKN::Window *getWindow();

  void setNativePixelSize(void);

 protected:

  void SetupMetal();

  /* The swift window which holds the metal view. */
  KRKN::Window *m_window;

  /* The Metal view. */
  CA::MetalLayer *m_metalKitView;
  MTL::CommandQueue *m_metalCmdQueue;
  MTL::RenderPipelineState *m_metalRenderPipeline;
  MTL::Texture *m_defaultFramebufferMetalTexture;
  MTL::Device *m_device;

  /* The SystemCocoa class to send events. */
  AnchorSystemCocoa *m_systemCocoa;

  /* To set the cursor. */
  void *m_cursor;

  bool m_immediateDraw;
  bool m_debug_context;
  bool m_is_dialog;

  wabi::HgiMetal *m_hgi;

  AnchorU64 m_time;

  /* test buffer. */
  MTL::Texture *m_font_tex;
};