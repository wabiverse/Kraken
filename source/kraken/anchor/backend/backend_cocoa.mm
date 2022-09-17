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
#include "ANCHOR_event.h"
#include "ANCHOR_BACKEND_cocoa.h"
#include "ANCHOR_BACKEND_metal.h"

// #import "Anchor.h"
#include "mtl_context.hh"

#include <simd/simd.h>
#include <AppKit/AppKit.h>
#include <MetalKit/MetalKit.h>
#include <Carbon/Carbon.h>

#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include <mach/mach_time.h>

KRAKEN_NAMESPACE_USING

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

AnchorSystemCocoa::AnchorSystemCocoa(void *shared)
{
  m_swift = shared;

  int mib[2];
  struct timeval boottime;
  size_t len;
  char *rstring = NULL;

  m_modifierMask = 0;
  m_outsideLoopEventProcessed = false;
  m_needDelayedEventProcessing = false;

  // NSEvent timeStamp is given in system uptime, state start date is boot time
  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;
  len = sizeof(struct timeval);

  sysctl(mib, 2, &boottime, &len, NULL, 0);
  m_start_time = ((boottime.tv_sec * 1000) + (boottime.tv_usec / 1000));

  /* Detect multi-touch track-pad. */
  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib, 2, NULL, &len, NULL, 0);
  rstring = (char *)malloc(len);
  sysctl(mib, 2, rstring, &len, NULL, 0);

  free(rstring);
  rstring = NULL;

  m_ignoreWindowSizedMessages = false;
  m_ignoreMomentumScroll = false;
  m_multiTouchScroll = false;
  m_last_warp_timestamp = 0;
}

eAnchorStatus AnchorSystemCocoa::init()
{
  eAnchorStatus success = AnchorSystem::init();
  if (success) {
    
    // @autoreleasepool {
      /* cxx system: sets this system instance pointer inside of CocoaAppDelegate. */
      // CocoaAppDelegate *cxxDelegate = [[CocoaAppDelegate alloc] init];
      // [cxxDelegate setSystemCocoa:(void *)this];

      /* swift system: recieves this system instance, so it can call into this class. */
      // [[AnchorSystemApple alloc] initWithCocoa:cxxDelegate];
    // }
  }
  return success;
}

AnchorU64 AnchorSystemCocoa::getMilliSeconds() const
{
  // Cocoa equivalent exists in 10.6 ([[NSProcessInfo processInfo] systemUptime])
  struct timeval currentTime;

  gettimeofday(&currentTime, NULL);

  // Return timestamp of system uptime

  return ((currentTime.tv_sec * 1000) + (currentTime.tv_usec / 1000) - m_start_time);
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
    ANCHOR_ASSERT(m_windowManager);
    m_windowManager->addWindow(window);
    m_windowManager->setActiveWindow(window);
    pushEvent(new AnchorEvent(getMilliSeconds(), AnchorEventTypeWindowActivate, window));
    pushEvent(new AnchorEvent(getMilliSeconds(), AnchorEventTypeWindowSize, window));
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

bool AnchorSystemCocoa::processEvents(bool waitForEvent)
{
  bool anyProcessed = false;
  NSEvent *event;
  do {
    @autoreleasepool {
      event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                 untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES];
      if (event == nil) {
        break;
      }

      anyProcessed = true;

      if ([event type] == NSEventTypeKeyDown && [event keyCode] == kVK_Tab &&
          ([event modifierFlags] & NSEventModifierFlagControl)) {
        handleKeyEvent(event);
      }
      else {
        if ([event type] == NSEventTypeKeyUp &&
            ([event modifierFlags] & (NSEventModifierFlagCommand | NSEventModifierFlagOption)))
          handleKeyEvent(event);

        [NSApp sendEvent:event];
      }
    }
  } while (event != nil);

  if (m_needDelayedEventProcessing)
    handleApplicationBecomeActiveEvent();

  if (m_outsideLoopEventProcessed) {
    m_outsideLoopEventProcessed = false;
    return true;
  }

  m_ignoreWindowSizedMessages = false;

  return anyProcessed;
}

eAnchorStatus AnchorSystemCocoa::handleApplicationBecomeActiveEvent()
{
  for (AnchorISystemWindow *iwindow : m_windowManager->getWindows()) {
    AnchorAppleMetal *window = (AnchorAppleMetal *)iwindow;
    // void *cocoaWindow = window->getWindow();
  //   if ([cocoaWindow getIsDialog]) {
  //     [[cocoaWindow getCocoaWindow] makeKeyAndOrderFront:nil];
  //   }
  }

  unsigned long modifiers;
  AnchorISystemWindow *window = m_windowManager->getActiveWindow();

  if (!window) {
    m_needDelayedEventProcessing = true;
    return ANCHOR_FAILURE;
  }
  else {
    m_needDelayedEventProcessing = false;
  }

  modifiers = [[[NSApplication sharedApplication] currentEvent] modifierFlags];

  if ((modifiers & NSEventModifierFlagShift) != (m_modifierMask & NSEventModifierFlagShift)) {
    pushEvent(new AnchorEventKey(getMilliSeconds(),
                                 (modifiers & NSEventModifierFlagShift) ? AnchorEventTypeKeyDown :
                                                                          AnchorEventTypeKeyUp,
                                 window,
                                 AnchorKeyLeftShift,
                                 false));
  }
  if ((modifiers & NSEventModifierFlagControl) != (m_modifierMask & NSEventModifierFlagControl)) {
    pushEvent(new AnchorEventKey(getMilliSeconds(),
                                 (modifiers & NSEventModifierFlagControl) ? AnchorEventTypeKeyDown :
                                                                            AnchorEventTypeKeyUp,
                                 window,
                                 AnchorKeyLeftControl,
                                 false));
  }
  if ((modifiers & NSEventModifierFlagOption) != (m_modifierMask & NSEventModifierFlagOption)) {
    pushEvent(new AnchorEventKey(getMilliSeconds(),
                                 (modifiers & NSEventModifierFlagOption) ? AnchorEventTypeKeyDown :
                                                                           AnchorEventTypeKeyUp,
                                 window,
                                 AnchorKeyLeftAlt,
                                 false));
  }
  if ((modifiers & NSEventModifierFlagCommand) != (m_modifierMask & NSEventModifierFlagCommand)) {
    pushEvent(new AnchorEventKey(getMilliSeconds(),
                                 (modifiers & NSEventModifierFlagCommand) ? AnchorEventTypeKeyDown :
                                                                            AnchorEventTypeKeyUp,
                                 window,
                                 AnchorKeyOS,
                                 false));
  }

  m_modifierMask = modifiers;

  m_outsideLoopEventProcessed = true;
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystemCocoa::handleKeyEvent(void *eventPtr)
{
  NSEvent *event = (NSEvent *)eventPtr;
  AnchorISystemWindow *window;
  unsigned long modifiers;
  NSString *characters;
  NSData *convertedCharacters;
  eAnchorKey keyCode;
  NSString *charsIgnoringModifiers;

  window = m_windowManager->getWindowAssociatedWithOSWindow((void *)[event window]);
  if (!window) {
    // printf("\nW failure for event 0x%x",[event type]);
    return ANCHOR_FAILURE;
  }

  char utf8_buf[6] = {'\0'};

  switch ([event type]) {

    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
      charsIgnoringModifiers = [event charactersIgnoringModifiers];
      if ([charsIgnoringModifiers length] > 0) {
        keyCode = convertKey([event keyCode],
                             [charsIgnoringModifiers characterAtIndex:0],
                             [event type] == NSEventTypeKeyDown ? kUCKeyActionDown :
                                                                  kUCKeyActionUp);
      }
      else {
        keyCode = convertKey([event keyCode],
                             0,
                             [event type] == NSEventTypeKeyDown ? kUCKeyActionDown :
                                                                  kUCKeyActionUp);
      }

      characters = [event characters];
      if ([characters length] > 0) {
        convertedCharacters = [characters dataUsingEncoding:NSUTF8StringEncoding];

        for (int x = 0; x < [convertedCharacters length]; x++) {
          utf8_buf[x] = ((char *)[convertedCharacters bytes])[x];
        }
      }

      /* arrow keys should not have utf8 */
      if ((keyCode >= AnchorKeyLeftArrow) && (keyCode <= AnchorKeyDownArrow)) {
        utf8_buf[0] = '\0';
      }

      /* F keys should not have utf8 */
      if ((keyCode >= AnchorKeyF1) && (keyCode <= AnchorKeyF20))
        utf8_buf[0] = '\0';

      /* no text with command key pressed */
      if (m_modifierMask & NSEventModifierFlagCommand)
        utf8_buf[0] = '\0';

      if ((keyCode == AnchorKeyQ) && (m_modifierMask & NSEventModifierFlagCommand))
        break;  // Cmd-Q is directly handled by Cocoa

      if ([event type] == NSEventTypeKeyDown) {
        pushEvent(new AnchorEventKey([event timestamp] * 1000,
                                     AnchorEventTypeKeyDown,
                                     window,
                                     (eAnchorKey)keyCode,
                                     [event isARepeat],
                                     utf8_buf));
      }
      else {
        pushEvent(new AnchorEventKey([event timestamp] * 1000, 
                                     AnchorEventTypeKeyUp, 
                                     window, 
                                     keyCode, 
                                     false, 
                                     NULL));
      }
      m_ignoreMomentumScroll = true;
      break;

    case NSEventTypeFlagsChanged:
      modifiers = [event modifierFlags];

      if ((modifiers & NSEventModifierFlagShift) != (m_modifierMask & NSEventModifierFlagShift)) {
        pushEvent(new AnchorEventKey([event timestamp] * 1000,
                                     (modifiers & NSEventModifierFlagShift) ? AnchorEventTypeKeyDown :
                                                                              AnchorEventTypeKeyUp,
                                     window,
                                     AnchorKeyLeftShift,
                                     false));
      }
      if ((modifiers & NSEventModifierFlagControl) !=
          (m_modifierMask & NSEventModifierFlagControl)) {
        pushEvent(new AnchorEventKey(
            [event timestamp] * 1000,
            (modifiers & NSEventModifierFlagControl) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
            window,
            AnchorKeyLeftControl,
            false));
      }
      if ((modifiers & NSEventModifierFlagOption) !=
          (m_modifierMask & NSEventModifierFlagOption)) {
        pushEvent(new AnchorEventKey(
            [event timestamp] * 1000,
            (modifiers & NSEventModifierFlagOption) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
            window,
            AnchorKeyLeftAlt,
            false));
      }
      if ((modifiers & NSEventModifierFlagCommand) !=
          (m_modifierMask & NSEventModifierFlagCommand)) {
        pushEvent(new AnchorEventKey(
            [event timestamp] * 1000,
            (modifiers & NSEventModifierFlagCommand) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
            window,
            AnchorKeyOS,
            false));
      }

      m_modifierMask = modifiers;
      m_ignoreMomentumScroll = true;
      break;

    default:
      return ANCHOR_FAILURE;
      break;
  }

  return ANCHOR_SUCCESS;
}

#pragma mark Clipboard get/set

char *AnchorSystemCocoa::getClipboard(bool selection) const
{
  char *temp_buff;
  size_t pastedTextSize;

  @autoreleasepool {

    NSPasteboard *pasteBoard = [NSPasteboard generalPasteboard];

    NSString *textPasted = [pasteBoard stringForType:NSPasteboardTypeString];

    if (textPasted == nil) {
      return NULL;
    }

    pastedTextSize = [textPasted lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

    temp_buff = (char *)malloc(pastedTextSize + 1);

    if (temp_buff == NULL) {
      return NULL;
    }

    strncpy(temp_buff, [textPasted cStringUsingEncoding:NSUTF8StringEncoding], pastedTextSize);

    temp_buff[pastedTextSize] = '\0';

    if (temp_buff) {
      return temp_buff;
    }
    else {
      return NULL;
    }
  }
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

bool AnchorSystemCocoa::handleOpenDocumentRequest(void *filepathStr)
{
  NSString *filepath = (NSString *)filepathStr;
  NSArray *windowsList;
  char *temp_buff;
  size_t filenameTextSize;

  windowsList = [NSApp orderedWindows];
  if ([windowsList count]) {
    [[windowsList objectAtIndex:0] makeKeyAndOrderFront:nil];
  }

  AnchorSystemWindow *window = (AnchorSystemWindow *)m_windowManager->getActiveWindow();

  if (!window) {
    return NO;
  }

  if (window && window->getCursorGrabModeIsWarp()) {
    return NO;
  }

  filenameTextSize = [filepath lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  temp_buff = (char *)malloc(filenameTextSize + 1);

  if (temp_buff == NULL) {
    return ANCHOR_FAILURE;
  }

  strncpy(temp_buff, [filepath cStringUsingEncoding:NSUTF8StringEncoding], filenameTextSize);
  temp_buff[filenameTextSize] = '\0';

  pushEvent(new AnchorEventString(getMilliSeconds(), AnchorEventTypeOpenMainFile, window, (AnchorEventDataPtr)temp_buff));

  return YES;
}


static void anchor_fatal_error_dialog(const char *msg)
{
  /* clang-format off */
  @autoreleasepool {
    /* clang-format on */
    NSString *message = [NSString stringWithFormat:@"Error opening window:\n%s", msg];

    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"Quit"];
    [alert setMessageText:@"Kraken"];
    [alert setInformativeText:message];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];
  }

  exit(1);
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
    m_systemCocoa(systemCocoa),
    m_cursor(0),
    m_immediateDraw(false),
    m_debug_context(false),
    m_is_dialog(dialog),
    m_time(mach_absolute_time())
{
  /* convert the title string for swift. */
  NSString *titleutf = [[[NSString alloc] initWithUTF8String:title] autorelease];

  /* create the window on metal with swift. */
  // m_window = [AnchorSystemApple createWindowWithTitle:titleutf left:left top:top width:width height:height state:state isDialog:dialog];
  // m_metalKitView = [m_window getMetalView];

  /* now we're ready for it. */
  newDrawingContext(ANCHOR_DrawingContextTypeMetal);
  activateDrawingContext();

  // [[m_window getCocoaWindow] setAcceptsMouseMovedEvents:YES];

  // NSView *contentview = [[m_window getCocoaWindow] contentView];
  // [contentview setAllowedTouchTypes:(NSTouchTypeMaskDirect | NSTouchTypeMaskIndirect)];

  // [[m_window getCocoaWindow] registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,
                                                      // NSStringPboardType,
                                                      // NSTIFFPboardType,
                                                      // nil]];

  if (/*dialog && parentWindow*/false) {
    // [parentWindow->getCocoaWindow() addChildWindow:m_window ordered:NSWindowAbove];
    // [m_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenAuxiliary];
  }
  else {
    // [[m_window getCocoaWindow] setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
  }

  if (state == AnchorWindowStateFullScreen)
    setState(AnchorWindowStateFullScreen);

  setNativePixelSize();
}

AnchorAppleMetal::~AnchorAppleMetal()
{
  if (m_cursor) {
    [m_cursor release];
    m_cursor = nil;
  }

  if (m_metalKitView) {
    [m_metalKitView release];
    m_metalKitView = nil;
  }

  // if (m_window) {
  //   [m_window closeCocoaWindow];
  // }

  /* Check for other kraken opened windows and make the front-most key
   * NOTE: for some reason the closed window is still in the list. */

  // NSArray *windowsList = [NSApp orderedWindows];
  // for (int a = 0; a < [windowsList count]; a++) {
  //   if ([m_window getCocoaWindow] != (NSWindow *)[windowsList objectAtIndex:a]) {
  //     [[windowsList objectAtIndex:a] makeKeyWindow];
  //     break;
  //   }
  // }

  m_window = nil;
}

/* called for event, when window leaves monitor to another */
void AnchorAppleMetal::setNativePixelSize(void)
{
  NSRect backingBounds = [m_metalKitView convertRectToBacking:[m_metalKitView bounds]];

  AnchorRect rect;
  getClientBounds(rect);

  m_nativePixelSize = (float)backingBounds.size.width / (float)rect.getWidth();
}

void AnchorAppleMetal::getClientBounds(AnchorRect &bounds) const
{
  NSRect rect;
  ANCHOR_ASSERT(getValid());

  // NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  // NSRect screenSize = [[m_window getScreen] visibleFrame];

  // Max window contents as screen size (excluding title bar...)
  // NSRect contentRect = [NSWindow contentRectForFrameRect:screenSize
                                              //  styleMask:[[m_window getCocoaWindow] styleMask]];

  // rect = [[m_window getCocoaWindow] contentRectForFrameRect:[[m_window getCocoaWindow] frame]];

  // bounds.m_b = contentRect.size.height - (rect.origin.y - contentRect.origin.y);
  // bounds.m_l = rect.origin.x - contentRect.origin.x;
  // bounds.m_r = rect.origin.x - contentRect.origin.x + rect.size.width;
  // bounds.m_t = contentRect.size.height - (rect.origin.y + rect.size.height - contentRect.origin.y);

  // [pool drain];
}

#pragma mark Swift Accessors

void *AnchorAppleMetal::getWindow()
{
  return m_window;
}

#pragma mark Drawing context


void AnchorAppleMetal::SetupMetal()
{
  m_hgi = new wabi::HgiMetal((__bridge MTL::Device *)m_metalKitView.device);

  /**
  * Setup ANCHOR context. */

  ANCHOR_CHECKVERSION();
  ANCHOR::CreateContext();

  AnchorIO &io = ANCHOR::GetIO();

  id<MTLDevice> device = m_metalKitView.device;

  m_metalCmdQueue = (MTLCommandQueue *)m_hgi->GetQueue();

  ANCHOR::StyleColorsDefault();

  kraken::gpu::InitContext((__bridge MTL::Device *)m_metalKitView.device);
}

static void SetFont()
{
  AnchorIO &io = ANCHOR::GetIO();
  AnchorFont *font = io.Fonts->AddFontDefault();

  io.FontDefault = font;
}

void AnchorAppleMetal::newDrawingContext(eAnchorDrawingContextType type)
{
  if (type == ANCHOR_DrawingContextTypeMetal) {

    /**
     * Create Metal Resources. */

    SetupMetal();

    /**
     * Setup Keyboard & Gamepad controls. */

    AnchorIO &io = ANCHOR::GetIO();
    io.ConfigFlags |= AnchorConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= AnchorConfigFlags_NavEnableGamepad;

    /**
     * Create Pixar Hydra Graphics Interface. */

    wabi::HdDriver driver;
    wabi::HgiUniquePtr hgi = wabi::HgiUniquePtr(m_hgi);
    driver.name = wabi::HgiTokens->renderDriver;
    driver.driver = wabi::VtValue(hgi.get());

    /**
     * Setup Pixar Driver & Engine. */

    ANCHOR::GetPixarDriver().name = driver.name;
    ANCHOR::GetPixarDriver().driver = driver.driver;

    /**
     * Set default fonts. */

    SetFont();
  }
}

eAnchorStatus AnchorAppleMetal::activateDrawingContext()
{
  if (ANCHOR::GetCurrentContext() != NULL)
    return ANCHOR_SUCCESS;
  else
    return ANCHOR_FAILURE;
}

eAnchorStatus AnchorAppleMetal::swapBuffers()
{
  AnchorIO &io = ANCHOR::GetIO();
  io.DisplaySize[0] = m_metalKitView.bounds.size.width;
  io.DisplaySize[1] = m_metalKitView.bounds.size.height;
  m_metalKitView.drawableSize = CGSizeMake(io.DisplaySize[0], io.DisplaySize[1]);

#if TARGET_OS_OSX
  CGFloat framebufferScale = m_metalKitView.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
#else
  CGFloat framebufferScale = m_metalKitView.window.screen.scale ?: UIScreen.mainScreen.scale;
#endif
  io.DisplayFramebufferScale = wabi::GfVec2f(framebufferScale, framebufferScale);

  id<MTLCommandBuffer> commandBuffer = [m_metalCmdQueue commandBuffer];

  MTLRenderPassDescriptor* renderPassDescriptor = m_metalKitView.currentRenderPassDescriptor;
  if (renderPassDescriptor == nil)
  {
    [commandBuffer commit];
    return;
  }

  kraken::gpu::NewFrame((__bridge MTL::RenderPassDescriptor *)renderPassDescriptor);

  ANCHOR::NewFrame();

  static wabi::GfVec4f clear_color = wabi::GfVec4f(0.45f, 0.55f, 0.60f, 1.00f);
  
  ANCHOR::Begin("Kraken");
  ANCHOR::Text("Computer Graphics of the Modern Age.");
  ANCHOR::End();

  ANCHOR::Render();

  AnchorDrawData *drawData = ANCHOR::GetDrawData();

  renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], 
                                                                          clear_color[1] * clear_color[3], 
                                                                          clear_color[2] * clear_color[3], 
                                                                          clear_color[3]);
  id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
  [renderEncoder pushDebugGroup:@"Anchor is Rendering..."];
  kraken::gpu::ViewDraw(drawData, (__bridge MTL::CommandBuffer *)commandBuffer, (__bridge MTL::RenderCommandEncoder *)renderEncoder);
  [renderEncoder popDebugGroup];
  [renderEncoder endEncoding];

  [commandBuffer presentDrawable:m_metalKitView.currentDrawable]; 
  [commandBuffer commit];

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

void AnchorAppleMetal::setIcon(const char *icon)
{

}

bool AnchorAppleMetal::getValid() const
{
  return AnchorSystemWindow::getValid() && m_window != NULL && m_metalKitView != NULL;
}

void *AnchorAppleMetal::getOSWindow() const
{
  return (void *)m_window;
}

void AnchorAppleMetal::setTitle(const char *title)
{
  /* convert the title string for swift. */
  NSString *titleutf = [[[NSString alloc] initWithUTF8String:title] autorelease];

  // [m_window setCocoaTitleWithTitle:titleutf];
}

std::string AnchorAppleMetal::getTitle() const
{
  // NSString *windowTitle = [m_window getCocoaTitle];

  std::string title;
  // if (windowTitle != nil) {
  //   title = [windowTitle UTF8String];
  // }

  return title;
}

void AnchorAppleMetal::getWindowBounds(AnchorRect &bounds) const
{

}

eAnchorStatus AnchorAppleMetal::setClientSize(AnchorU32 width, AnchorU32 height)
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorAppleMetal::setState(eAnchorWindowState state)
{
  // [m_window setCocoaState:nsstate];

  return ANCHOR_SUCCESS;
}

eAnchorWindowState AnchorAppleMetal::getState() const
{
  eAnchorWindowState state = AnchorWindowStateNormal;

  /* convert swift enum to cxx enum. */
  // AnchorWindowState nsstate = [m_window getCocoaState];
  // switch (nsstate) {
  //   case AnchorWindowStateWindowStateNormal:
  //     state = AnchorWindowStateNormal;
  //     break;
  //   case AnchorWindowStateWindowStateMaximized:
  //     state = AnchorWindowStateMaximized;
  //     break;
  //   case AnchorWindowStateWindowStateMinimized:
  //     state = AnchorWindowStateMinimized;
  //     break;
  //   case AnchorWindowStateWindowStateFullScreen:
  //     state = AnchorWindowStateFullScreen;
  //     break;
  //   case AnchorWindowStateWindowStateEmbedded:
  //     state = AnchorWindowStateEmbedded;
  //     break;
  //   default:
  //     state = AnchorWindowStateNormal;
  //     break;
  // }
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