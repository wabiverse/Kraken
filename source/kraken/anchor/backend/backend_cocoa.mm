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

#include "ANCHOR_api.h"
#include "ANCHOR_BACKEND_cocoa.h"
#include "ANCHOR_BACKEND_metal.h"

#import "Anchor.h"
#import <kraken_anchor-Swift.h>

#include <Carbon/Carbon.h>

#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include <mach/mach_time.h>


#pragma mark Utility functions

#define FIRSTFILEBUFLG 512
static bool g_hasFirstFile = false;
static char g_firstFileBuf[512];

// TODO: Need to investigate this.
// Function called too early in creator.cpp to have g_hasFirstFile == true
int ANCHOR_HACK_getFirstFile(char buf[FIRSTFILEBUFLG])
{
  if (g_hasFirstFile) {
    strncpy(buf, g_firstFileBuf, FIRSTFILEBUFLG - 1);
    buf[FIRSTFILEBUFLG - 1] = '\0';
    return 1;
  }
  else {
    return 0;
  }
}


eAnchorStatus AnchorSystemCocoa::init()
{
  eAnchorStatus success = AnchorSystem::init();
  if (success) {
    @autoreleasepool {
      AnchorSystemApple *appDelegate = [[AnchorSystemApple alloc] init];
    }
  }
  return success;
}

eAnchorStatus AnchorSystemCocoa::exit()
{
  return ANCHOR_SUCCESS;
}

AnchorISystemWindow *AnchorSystemCocoa::createWindow(const char *title,
                                                     const char *icon,
                                                     AnchorS32 left,
                                                     AnchorS32 top,
                                                     AnchorU32 width,
                                                     AnchorU32 height,
                                                     eAnchorWindowState state,
                                                     eAnchorDrawingContextType type,
                                                     int vkSettings,
                                                     const bool exclusive,
                                                     const bool is_dialog,
                                                     const AnchorISystemWindow *parentWindow)
{
  AnchorISystemWindow *window = NULL;

  window = new AnchorAppleMetal(this, title, left, top, width, height, state, is_dialog);
  if (window->getValid()) {
    /* Store pointer to window in window manager, set it to active, and push events. */
  } else {
    /* don't destroy this until the metal context is fully implemented and expected. */
    //delete window;
    //window = NULL;
  }

  return window;
}

AnchorU64 AnchorSystemCocoa::performanceCounterToMillis(int perf_ticks) const
{
  return 0;
}


AnchorU64 AnchorSystemCocoa::tickCountToMillis(int ticks) const
{
  return 0;
}

AnchorU64 AnchorSystemCocoa::getMilliSeconds() const
{
  return 0;
}

bool AnchorSystemCocoa::processEvents(bool waitForEvent)
{
  bool anyProcessed = [AnchorSystemApple processEvents];
  return anyProcessed;
}

eAnchorStatus AnchorSystemCocoa::getModifierKeys(AnchorModifierKeys &keys) const
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystemCocoa::getButtons(AnchorButtons &buttons) const
{
  return ANCHOR_SUCCESS;
}

AnchorU8 AnchorSystemCocoa::getNumDisplays() const
{
  return 0;
}

void AnchorSystemCocoa::getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const {}

void AnchorSystemCocoa::getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const {}

/**
 * Creates a window event.
 * @param type: The type of event to create.
 * @param window: The window receiving the event (the active window).
 * @return The event created. */
AnchorEvent *AnchorSystemCocoa::processWindowEvent(eAnchorEventType type,
                                                   AnchorISystemWindow *window)
{
  return nullptr;
}

/**
 * Creates tablet events from pointer events.
 * @param type: The type of pointer event.
 * @param window: The window receiving the event (the active window).
 * @param wParam: The wParam from the wndproc.
 * @param lParam: The lParam from the wndproc.
 * @param eventhandled: True if the method handled the event. */
void AnchorSystemCocoa::processPointerEvent(int type,
                                            AnchorISystemWindow *window,
                                            AnchorS32 wParam,
                                            AnchorS32 lParam,
                                            bool &eventhandled)
{}

/**
 * Creates tablet events from pointer events.
 * @param type: The type of pointer event.
 * @param window: The window receiving the event (the active window).
 * @param wParam: The wParam from the wndproc.
 * @param lParam: The lParam from the wndproc.
 * @param eventhandled: True if the method handled the event. */
AnchorEventCursor *AnchorSystemCocoa::processCursorEvent(AnchorISystemWindow *window)
{
  return nullptr;
}

/**
 * Handles a mouse wheel event.
 * @param window: The window receiving the event (the active window).
 * @param wParam: The wParam from the wndproc.
 * @param lParam: The lParam from the wndproc.
 * @param isHorizontal: Whether the wheel event is horizontal or (false) for vertical. */
void AnchorSystemCocoa::processWheelEvent(AnchorISystemWindow *window,
                                          AnchorS32 wParam,
                                          AnchorS32 lParam,
                                          bool isHorizontal)
{}

/**
 * Handles minimum window size.
 * @param minmax: The MINMAXINFO structure. */
void AnchorSystemCocoa::processMinMaxInfo(void *minmax) {}

eAnchorStatus AnchorSystemCocoa::getCursorPosition(AnchorS32 &x, AnchorS32 &y) const
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystemCocoa::setCursorPosition(AnchorS32 x, AnchorS32 y)
{
  return ANCHOR_SUCCESS;
}

int AnchorSystemCocoa::toggleConsole(int action)
{
  return 0;
}

AnchorEventKey *AnchorSystemCocoa::processKeyEvent(AnchorISystemWindow *window,
                                                   AnchorS32 const &raw)
{
  return nullptr;
}

AnchorEvent *AnchorSystemCocoa::processWindowSizeEvent(AnchorISystemWindow *window)
{
  return nullptr;
}

AnchorEventButton *AnchorSystemCocoa::processButtonEvent(eAnchorEventType type,
                                                         AnchorISystemWindow *window,
                                                         eAnchorButtonMask mask)
{
  return nullptr;
}

eAnchorKey AnchorSystemCocoa::convertKey(short vKey, short ScanCode, short extend) const
{
  return eAnchorKey::AnchorKeyEnter;
}

eAnchorKey AnchorSystemCocoa::hardKey(AnchorS32 const &raw,
                                      bool *r_keyDown,
                                      bool *r_is_repeated_modifier)
{
  return eAnchorKey::AnchorKeyEnter;
}

eAnchorKey AnchorSystemCocoa::processSpecialKey(short vKey, short scanCode) const
{
  return eAnchorKey::AnchorKeyEnter;
}


AnchorAppleMetal::AnchorAppleMetal(AnchorSystemCocoa *systemCocoa,
                                     const char *title,
                                     AnchorS32 left,
                                     AnchorS32 top,
                                     AnchorU32 width,
                                     AnchorU32 height,
                                     eAnchorWindowState state,
                                     bool dialog)
  : AnchorSystemWindow(width, height, state, false, false),
    m_metalView(nil),
    m_metalLayer(nil),
    m_systemCocoa(systemCocoa),
    m_cursor(0),
    m_immediateDraw(false),
    m_debug_context(false),
    m_is_dialog(dialog)
{
  /* convert the title string for swift. */
  NSString *titleutf = [[[NSString alloc] initWithUTF8String:title] autorelease];

  /* convert the cxx enum to the swift enum. */
  AnchorWindowState nsstate = AnchorWindowStateWindowStateNormal;
  switch (state) {
    case AnchorWindowStateNormal:
      nsstate = AnchorWindowStateWindowStateNormal;
      break;
    case AnchorWindowStateMaximized:
      nsstate = AnchorWindowStateWindowStateMaximized;
      break;
    case AnchorWindowStateMinimized:
      nsstate = AnchorWindowStateWindowStateMinimized;
      break;
    case AnchorWindowStateFullScreen:
      nsstate = AnchorWindowStateWindowStateFullScreen;
      break;
    case AnchorWindowStateEmbedded:
      nsstate = AnchorWindowStateWindowStateEmbedded;
      break;
    default:
      nsstate = AnchorWindowStateWindowStateNormal;
      break;
  }

  /* create the window on metal with swift. */
  m_window = [AnchorSystemApple createWindowWithTitle:titleutf left:left top:top width:width height:height state:nsstate isDialog:dialog];
}

AnchorAppleMetal::~AnchorAppleMetal()
{
  if (m_cursor) {
    [m_cursor release];
    m_cursor = nil;
  }

  if (m_metalView) {
    [m_metalView release];
    m_metalView = nil;
  }

  if (m_metalLayer) {
    [m_metalLayer release];
    m_metalLayer = nil;
  }

  if (m_window) {
    [m_window closeCocoaWindow];
  }

  /* Check for other kraken opened windows and make the front-most key
   * NOTE: for some reason the closed window is still in the list.
   * @TODO: do this in swift... */

  // NSArray *windowsList = [NSApp orderedWindows];
  // for (int a = 0; a < [windowsList count]; a++) {
  //   if (m_window != (CocoaWindow *)[windowsList objectAtIndex:a]) {
  //     [[windowsList objectAtIndex:a] makeKeyWindow];
  //     break;
  //   }
  // }

  m_window = nil;
}

eAnchorStatus AnchorAppleMetal::activateDrawingContext()
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorAppleMetal::swapBuffers()
{
  return ANCHOR_SUCCESS;
}

AnchorU16 AnchorAppleMetal::getDPIHint()
{
  return 96;
}

eAnchorStatus AnchorAppleMetal::setModifiedState(bool isUnsavedChanges)
{
  return ANCHOR_SUCCESS;
}

bool AnchorAppleMetal::getModifiedState()
{
  return false;
}

void AnchorAppleMetal::newDrawingContext(eAnchorDrawingContextType type)
{

}

void AnchorAppleMetal::setIcon(const char *icon)
{

}

bool AnchorAppleMetal::getValid() const
{
  return AnchorSystemWindow::getValid() && m_window != NULL;
}

void *AnchorAppleMetal::getOSWindow() const
{
  return (void *)[m_window getCocoaWindow];
}

void AnchorAppleMetal::setTitle(const char *title)
{
  /* convert the title string for swift. */
  NSString *titleutf = [[[NSString alloc] initWithUTF8String:title] autorelease];

  [m_window setCocoaTitleWithTitle:titleutf];
}

std::string AnchorAppleMetal::getTitle() const
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  NSString *windowTitle = [m_window getCocoaTitle];

  std::string title;
  if (windowTitle != nil) {
    title = [windowTitle UTF8String];
  }

  [pool drain];

  return title;
}

void AnchorAppleMetal::getWindowBounds(AnchorRect &bounds) const
{

}

eAnchorStatus AnchorAppleMetal::setClientSize(AnchorU32 width, AnchorU32 height)
{
  return ANCHOR_SUCCESS;
}

void AnchorAppleMetal::getClientBounds(AnchorRect &bounds) const
{

}

eAnchorStatus AnchorAppleMetal::setState(eAnchorWindowState state)
{
  return ANCHOR_SUCCESS;
}

eAnchorWindowState AnchorAppleMetal::getState() const
{
  eAnchorWindowState state;

  AnchorWindowState nsstate = [m_window getCocoaState];
  switch (nsstate) {
    case AnchorWindowStateWindowStateNormal:
      state = AnchorWindowStateNormal;
      break;
    case AnchorWindowStateWindowStateMaximized:
      state = AnchorWindowStateMaximized;
      break;
    case AnchorWindowStateWindowStateMinimized:
      state = AnchorWindowStateMinimized;
      break;
    case AnchorWindowStateWindowStateFullScreen:
      state = AnchorWindowStateFullScreen;
      break;
    case AnchorWindowStateWindowStateEmbedded:
      state = AnchorWindowStateEmbedded;
      break;
    default:
      state = AnchorWindowStateNormal;
      break;
  }
  return state;
}

void AnchorAppleMetal::screenToClient(AnchorS32 inX, AnchorS32 inY, AnchorS32 &outX, AnchorS32 &outY) const
{

}

void AnchorAppleMetal::clientToScreen(AnchorS32 inX, AnchorS32 inY, AnchorS32 &outX, AnchorS32 &outY) const
{

}

eAnchorStatus AnchorAppleMetal::setOrder(eAnchorWindowOrder order)
{
  return ANCHOR_SUCCESS;
}

static NSCursor *getImageCursor(eAnchorStandardCursor shape, NSString *name, NSPoint hotspot)
{
  static NSCursor *cursors[(int)ANCHOR_StandardCursorNumCursors] = {0};
  static bool loaded[(int)ANCHOR_StandardCursorNumCursors] = {false};

  const int index = (int)shape;
  if (!loaded[index]) {
    /* Load image from file in application Resources folder. */
    /* clang-format off */
    @autoreleasepool {
      /* clang-format on */
      NSImage *image = [NSImage imageNamed:name];
      if (image != NULL) {
        cursors[index] = [[NSCursor alloc] initWithImage:image hotSpot:hotspot];
      }
    }

    loaded[index] = true;
  }

  return cursors[index];
}

NSCursor *AnchorAppleMetal::getStandardCursor(eAnchorStandardCursor shape) const
{
  switch (shape) {
    case ANCHOR_StandardCursorCustom:
      if (m_cursor) {
        return m_cursor;
      }
      else {
        return NULL;
      }
    case ANCHOR_StandardCursorDestroy:
      return [NSCursor disappearingItemCursor];
    case ANCHOR_StandardCursorText:
      return [NSCursor IBeamCursor];
    case ANCHOR_StandardCursorCrosshair:
      return [NSCursor crosshairCursor];
    case ANCHOR_StandardCursorUpDown:
      return [NSCursor resizeUpDownCursor];
    case ANCHOR_StandardCursorLeftRight:
      return [NSCursor resizeLeftRightCursor];
    case ANCHOR_StandardCursorTopSide:
      return [NSCursor resizeUpCursor];
    case ANCHOR_StandardCursorBottomSide:
      return [NSCursor resizeDownCursor];
    case ANCHOR_StandardCursorLeftSide:
      return [NSCursor resizeLeftCursor];
    case ANCHOR_StandardCursorRightSide:
      return [NSCursor resizeRightCursor];
    case ANCHOR_StandardCursorCopy:
      return [NSCursor dragCopyCursor];
    case ANCHOR_StandardCursorStop:
      return [NSCursor operationNotAllowedCursor];
    case ANCHOR_StandardCursorMove:
      return [NSCursor pointingHandCursor];
    case ANCHOR_StandardCursorDefault:
      return [NSCursor arrowCursor];
    case ANCHOR_StandardCursorKnife:
      return getImageCursor(shape, @"knife.pdf", NSMakePoint(6, 24));
    case ANCHOR_StandardCursorEraser:
      return getImageCursor(shape, @"eraser.pdf", NSMakePoint(6, 24));
    case ANCHOR_StandardCursorPencil:
      return getImageCursor(shape, @"pen.pdf", NSMakePoint(6, 24));
    case ANCHOR_StandardCursorEyedropper:
      return getImageCursor(shape, @"eyedropper.pdf", NSMakePoint(6, 24));
    case ANCHOR_StandardCursorZoomIn:
      return getImageCursor(shape, @"zoomin.pdf", NSMakePoint(8, 7));
    case ANCHOR_StandardCursorZoomOut:
      return getImageCursor(shape, @"zoomout.pdf", NSMakePoint(8, 7));
    case ANCHOR_StandardCursorNSEWScroll:
      return getImageCursor(shape, @"scrollnsew.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorNSScroll:
      return getImageCursor(shape, @"scrollns.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorEWScroll:
      return getImageCursor(shape, @"scrollew.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorUpArrow:
      return getImageCursor(shape, @"arrowup.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorDownArrow:
      return getImageCursor(shape, @"arrowdown.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorLeftArrow:
      return getImageCursor(shape, @"arrowleft.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorRightArrow:
      return getImageCursor(shape, @"arrowright.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorVerticalSplit:
      return getImageCursor(shape, @"splitv.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorHorizontalSplit:
      return getImageCursor(shape, @"splith.pdf", NSMakePoint(16, 16));
    case ANCHOR_StandardCursorCrosshairA:
      return getImageCursor(shape, @"paint_cursor_cross.pdf", NSMakePoint(16, 15));
    case ANCHOR_StandardCursorCrosshairB:
      return getImageCursor(shape, @"paint_cursor_dot.pdf", NSMakePoint(16, 15));
    case ANCHOR_StandardCursorCrosshairC:
      return getImageCursor(shape, @"crossc.pdf", NSMakePoint(16, 16));
    default:
      return NULL;
  }
}

eAnchorStatus AnchorAppleMetal::hasCursorShape(eAnchorStandardCursor shape)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  eAnchorStatus success = (getStandardCursor(shape)) ? ANCHOR_SUCCESS : ANCHOR_FAILURE;
  [pool drain];
  return success;
}

void AnchorAppleMetal::loadCursor(bool visible, eAnchorStandardCursor cursorShape) const
{
  static bool systemCursorVisible = true;
  if (visible != systemCursorVisible) {
    if (visible) {
      [NSCursor unhide];
      systemCursorVisible = true;
    }
    else {
      [NSCursor hide];
      systemCursorVisible = false;
    }
  }

  NSCursor *cursor = getStandardCursor(cursorShape);
  if (cursor == NULL) {
    cursor = getStandardCursor(ANCHOR_StandardCursorDefault);
  }

  [cursor set];
}

bool AnchorAppleMetal::isDialog() const
{
  return m_is_dialog;
}

eAnchorStatus AnchorAppleMetal::setProgressBar(float progress)
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorAppleMetal::endProgressBar()
{
  return ANCHOR_SUCCESS;
}