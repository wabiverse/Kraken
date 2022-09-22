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

#include "mtl_context.h"

#include <simd/simd.h>
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

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

  return 0;
}

AnchorSystemCocoa::AnchorSystemCocoa()
{
  int mib[2];
  struct timeval boottime;
  size_t len;
  char *rstring = nullptr;

  m_modifierMask = 0;
  m_outsideLoopEventProcessed = false;
  m_needDelayedEventProcessing = false;

  // NSEvent timeStamp is given in system uptime, state start date is boot time
  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;
  len = sizeof(struct timeval);

  sysctl(mib, 2, &boottime, &len, nullptr, 0);
  m_start_time = ((boottime.tv_sec * 1000) + (boottime.tv_usec / 1000));

  /* Detect multi-touch track-pad. */
  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib, 2, nullptr, &len, nullptr, 0);
  rstring = (char *)malloc(len);
  sysctl(mib, 2, rstring, &len, nullptr, 0);

  free(rstring);
  rstring = nullptr;

  m_ignoreWindowSizedMessages = false;
  m_ignoreMomentumScroll = false;
  m_multiTouchScroll = false;
  m_last_warp_timestamp = 0;
}

eAnchorStatus AnchorSystemCocoa::init()
{
  eAnchorStatus success = AnchorSystem::init();
  if (success) {
    m_swift = KRKN::CreateSystem();
  }
  return success;
}

AnchorU64 AnchorSystemCocoa::getMilliSeconds() const
{
  // Cocoa equivalent exists in 10.6 ([[NSProcessInfo processInfo] systemUptime])
  struct timeval currentTime;

  gettimeofday(&currentTime, nullptr);

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
  AnchorISystemWindow *window = nullptr;

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
    // delete window;
    // window = nullptr;
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
  return m_swift->processEvents();
}

eAnchorStatus AnchorSystemCocoa::handleApplicationBecomeActiveEvent()
{
  // for (AnchorISystemWindow *iwindow : m_windowManager->getWindows()) {
  //   AnchorAppleMetal *window = (AnchorAppleMetal *)iwindow;
  //   // void *cocoaWindow = window->getWindow();
  // //   if ([cocoaWindow getIsDialog]) {
  // //     [[cocoaWindow getCocoaWindow] makeKeyAndOrderFront:nil];
  // //   }
  // }

  // unsigned long modifiers;
  // AnchorISystemWindow *window = m_windowManager->getActiveWindow();

  // if (!window) {
  //   m_needDelayedEventProcessing = true;
  //   return ANCHOR_FAILURE;
  // }
  // else {
  //   m_needDelayedEventProcessing = false;
  // }

  // modifiers = [[[NSApplication sharedApplication] currentEvent] modifierFlags];

  // if ((modifiers & NSEventModifierFlagShift) != (m_modifierMask & NSEventModifierFlagShift)) {
  //   pushEvent(new AnchorEventKey(getMilliSeconds(),
  //                                (modifiers & NSEventModifierFlagShift) ? AnchorEventTypeKeyDown
  //                                :
  //                                                                         AnchorEventTypeKeyUp,
  //                                window,
  //                                AnchorKeyLeftShift,
  //                                false));
  // }
  // if ((modifiers & NSEventModifierFlagControl) != (m_modifierMask & NSEventModifierFlagControl))
  // {
  //   pushEvent(new AnchorEventKey(getMilliSeconds(),
  //                                (modifiers & NSEventModifierFlagControl) ?
  //                                AnchorEventTypeKeyDown :
  //                                                                           AnchorEventTypeKeyUp,
  //                                window,
  //                                AnchorKeyLeftControl,
  //                                false));
  // }
  // if ((modifiers & NSEventModifierFlagOption) != (m_modifierMask & NSEventModifierFlagOption)) {
  //   pushEvent(new AnchorEventKey(getMilliSeconds(),
  //                                (modifiers & NSEventModifierFlagOption) ?
  //                                AnchorEventTypeKeyDown :
  //                                                                          AnchorEventTypeKeyUp,
  //                                window,
  //                                AnchorKeyLeftAlt,
  //                                false));
  // }
  // if ((modifiers & NSEventModifierFlagCommand) != (m_modifierMask & NSEventModifierFlagCommand))
  // {
  //   pushEvent(new AnchorEventKey(getMilliSeconds(),
  //                                (modifiers & NSEventModifierFlagCommand) ?
  //                                AnchorEventTypeKeyDown :
  //                                                                           AnchorEventTypeKeyUp,
  //                                window,
  //                                AnchorKeyOS,
  //                                false));
  // }

  // m_modifierMask = modifiers;

  // m_outsideLoopEventProcessed = true;
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorSystemCocoa::handleKeyEvent(void *eventPtr)
{
  return ANCHOR_SUCCESS;
}

#pragma mark Clipboard get/set

char *AnchorSystemCocoa::getClipboard(bool selection) const
{
  // char *temp_buff;
  // size_t pastedTextSize;

  // @autoreleasepool {

  // NSPasteboard *pasteBoard = [NSPasteboard generalPasteboard];

  // NSString *textPasted = [pasteBoard stringForType:NSPasteboardTypeString];

  // if (textPasted == nil) {
  //   return nullptr;
  // }

  // pastedTextSize = [textPasted lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

  // temp_buff = (char *)malloc(pastedTextSize + 1);

  // if (temp_buff == nullptr) {
  //   return nullptr;
  // }

  // strncpy(temp_buff, [textPasted cStringUsingEncoding:NSUTF8StringEncoding], pastedTextSize);

  // temp_buff[pastedTextSize] = '\0';

  // if (temp_buff) {
  //   return temp_buff;
  // }
  // else {
  //   return nullptr;
  // }
  // }

  return nullptr;
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

void AnchorSystemCocoa::getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const
{
  CGRect contentRect = m_swift->getMainDisplayDimensions();

  width = (AnchorU32)contentRect.size.width;
  height = (AnchorU32)contentRect.size.height;
}

void AnchorSystemCocoa::getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const
{
  getMainDisplayDimensions(width, height);
}

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
  // NSString *filepath = (NSString *)filepathStr;
  // NSArray *windowsList;
  // char *temp_buff;
  // size_t filenameTextSize;

  // windowsList = [NSApp orderedWindows];
  // if ([windowsList count]) {
  //   [[windowsList objectAtIndex:0] makeKeyAndOrderFront:nil];
  // }

  // AnchorSystemWindow *window = (AnchorSystemWindow *)m_windowManager->getActiveWindow();

  // if (!window) {
  //   return NO;
  // }

  // if (window && window->getCursorGrabModeIsWarp()) {
  //   return NO;
  // }

  // filenameTextSize = [filepath lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  // temp_buff = (char *)malloc(filenameTextSize + 1);

  // if (temp_buff == nullptr) {
  //   return ANCHOR_FAILURE;
  // }

  // strncpy(temp_buff, [filepath cStringUsingEncoding:NSUTF8StringEncoding], filenameTextSize);
  // temp_buff[filenameTextSize] = '\0';

  // pushEvent(new AnchorEventString(getMilliSeconds(), AnchorEventTypeOpenMainFile, window,
  // (AnchorEventDataPtr)temp_buff));

  return YES;
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
    m_metalKitView(nil),
    m_device(nil),
    m_systemCocoa(systemCocoa),
    m_cursor(nullptr),
    m_immediateDraw(false),
    m_debug_context(false),
    m_is_dialog(dialog),
    m_time(mach_absolute_time())
{
  /* create the window on metal with swift. */
  m_window = KRKN::CreateWindow(NS::String::string(title, NS::UTF8StringEncoding),
                                (CGFloat)left,
                                (CGFloat)top,
                                (CGFloat)width,
                                (CGFloat)height,
                                dialog,
                                nil);

  // m_metalKitView = [m_window getMetalView];

  /* now we're ready for it. */
  newDrawingContext(ANCHOR_DrawingContextTypeMetal);
  activateDrawingContext();

  if (state == AnchorWindowStateFullScreen)
    setState(AnchorWindowStateFullScreen);

  setNativePixelSize();
}

AnchorAppleMetal::~AnchorAppleMetal()
{
  if (m_cursor) {
    // m_cursor->release();
    m_cursor = nil;
  }

  if (m_metalKitView) {
    m_metalKitView->release();
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
  // CGSize size = m_metalKitView->drawableSize();

  AnchorRect rect;
  getClientBounds(rect);

  // m_nativePixelSize = (float)size.width / (float)rect.getWidth();
}

void AnchorAppleMetal::getClientBounds(AnchorRect &bounds) const
{
  CGSize rect;
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
  // bounds.m_t = contentRect.size.height - (rect.origin.y + rect.size.height -
  // contentRect.origin.y);

  // [pool drain];
}

#pragma mark Swift Accessors

KRKN::Window *AnchorAppleMetal::getWindow()
{
  return m_window;
}

#pragma mark Drawing context


void AnchorAppleMetal::SetupMetal()
{
  m_device = MTL::CreateSystemDefaultDevice();

  m_hgi = new wabi::HgiMetal(m_device);

  /**
   * Setup ANCHOR context. */

  ANCHOR_CHECKVERSION();
  ANCHOR::CreateContext();

  m_metalCmdQueue = m_hgi->GetQueue();

  ANCHOR::StyleColorsDefault();

  kraken::gpu::InitContext(m_device);
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
  if (ANCHOR::GetCurrentContext() != nullptr) {
    return ANCHOR_SUCCESS;
  }

  return ANCHOR_FAILURE;
}

eAnchorStatus AnchorAppleMetal::swapBuffers()
{
  AnchorIO &io = ANCHOR::GetIO();
  // io.DisplaySize[0] = m_metalKitView->drawableSize().width;
  // io.DisplaySize[1] = m_metalKitView->drawableSize().height;
  // m_metalKitView->setDrawableSize(CGSizeMake((CGFloat)io.DisplaySize[0],
  // (CGFloat)io.DisplaySize[1]));

#if TARGET_OS_OSX
  CGFloat framebufferScale = CGFloat(1);
#else
  CGFloat framebufferScale = m_metalKitView.window.screen.scale ?: UIScreen.mainScreen.scale;
#endif
  io.DisplayFramebufferScale = wabi::GfVec2f(framebufferScale, framebufferScale);

  MTL::CommandBuffer *commandBuffer = m_metalCmdQueue->commandBuffer();

  MTL::RenderPassDescriptor *renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
  if (renderPassDescriptor == nil) {
    commandBuffer->commit();
    return ANCHOR_FAILURE;
  }

  kraken::gpu::NewFrame(renderPassDescriptor);

  ANCHOR::NewFrame();

  static wabi::GfVec4f clear_color = wabi::GfVec4f(0.45f, 0.55f, 0.60f, 1.00f);

  ANCHOR::Begin("Kraken");
  ANCHOR::Text("Computer Graphics of the Modern Age.");
  ANCHOR::End();

  ANCHOR::Render();

  AnchorDrawData *drawData = ANCHOR::GetDrawData();

  renderPassDescriptor->colorAttachments()->object(0)->setClearColor(
    MTL::ClearColor::Make(clear_color[0] * clear_color[3],
                          clear_color[1] * clear_color[3],
                          clear_color[2] * clear_color[3],
                          clear_color[3]));
  MTL::RenderCommandEncoder *renderEncoder = commandBuffer->renderCommandEncoder(
    renderPassDescriptor);
  renderEncoder->pushDebugGroup(
    NS::String::string("Anchor is Rendering...", NS::UTF8StringEncoding));
  kraken::gpu::ViewDraw(drawData, commandBuffer, renderEncoder);
  renderEncoder->popDebugGroup();
  renderEncoder->endEncoding();

  // commandBuffer->presentDrawable(m_metalKitView->nextDrawable());
  commandBuffer->commit();

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

void AnchorAppleMetal::setIcon(const char *icon) {}

bool AnchorAppleMetal::getValid() const
{
  return AnchorSystemWindow::getValid() && m_window != nil /* && m_metalKitView != nil*/;
}

void *AnchorAppleMetal::getOSWindow() const
{
  return (void *)m_window;
}

void AnchorAppleMetal::setTitle(const char *title)
{
  /* convert the title string for swift. */
  NS::String *titleutf = NS::String::string(title, NS::UTF8StringEncoding);

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

void AnchorAppleMetal::getWindowBounds(AnchorRect &bounds) const {}

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

void AnchorAppleMetal::screenToClient(AnchorS32 inX,
                                      AnchorS32 inY,
                                      AnchorS32 &outX,
                                      AnchorS32 &outY) const
{}

void AnchorAppleMetal::clientToScreen(AnchorS32 inX,
                                      AnchorS32 inY,
                                      AnchorS32 &outX,
                                      AnchorS32 &outY) const
{}

eAnchorStatus AnchorAppleMetal::setOrder(eAnchorWindowOrder order)
{
  return ANCHOR_SUCCESS;
}

static void *getImageCursor(eAnchorStandardCursor shape, NS::String *name)
{
  // static void *cursors[(int)ANCHOR_StandardCursorNumCursors] = {0};
  // static bool loaded[(int)ANCHOR_StandardCursorNumCursors] = {false};

  // const int index = (int)shape;
  // if (!loaded[index]) {
  //   /* Load image from file in application Resources folder. */
  //   /* clang-format off */
  //   // @autoreleasepool {
  //     /* clang-format on */
  //     NS::Image *image = [NSImage imageNamed:name];
  //     if (image != nullptr) {
  //       cursors[index] = [[NSCursor alloc] initWithImage:image hotSpot:hotspot];
  //     }
  //   // }

  //   loaded[index] = true;
  // }

  // return cursors[index];
  return nullptr;
}

eAnchorStatus AnchorAppleMetal::setWindowCursorVisibility(bool visible)
{
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

  if (/*m_window->isVisible()*/ m_window) {
    loadCursor(visible, getCursorShape());
  }

  pool->drain();
  return ANCHOR_SUCCESS;
}

void *AnchorAppleMetal::getStandardCursor(eAnchorStandardCursor shape) const
{
  switch (shape) {
    case ANCHOR_StandardCursorCustom:
      if (m_cursor) {
        return m_cursor;
      } else {
        return nullptr;
      }
    case ANCHOR_StandardCursorDestroy:
      // return NSCursor->disappearingItemCursor();
      return nullptr;
    case ANCHOR_StandardCursorText:
      // return NSCursor->IBeamCursor();
      return nullptr;
    case ANCHOR_StandardCursorCrosshair:
      // return NSCursor->crosshairCursor();
      return nullptr;
    case ANCHOR_StandardCursorUpDown:
      // return NSCursor->resizeUpDownCursor();
      return nullptr;
    case ANCHOR_StandardCursorLeftRight:
      // return NSCursor->resizeLeftRightCursor();
      return nullptr;
    case ANCHOR_StandardCursorTopSide:
      // return NSCursor->resizeUpCursor();
      return nullptr;
    case ANCHOR_StandardCursorBottomSide:
      // return NSCursor->resizeDownCursor();
      return nullptr;
    case ANCHOR_StandardCursorLeftSide:
      // return NSCursor->resizeLeftCursor();
      return nullptr;
    case ANCHOR_StandardCursorRightSide:
      // return NSCursor->resizeRightCursor();
      return nullptr;
    case ANCHOR_StandardCursorCopy:
      // return NSCursor->dragCopyCursor();
      return nullptr;
    case ANCHOR_StandardCursorStop:
      // return NSCursor->operationNotAllowedCursor();
      return nullptr;
    case ANCHOR_StandardCursorMove:
      // return NSCursor->pointingHandCursor();
      return nullptr;
    case ANCHOR_StandardCursorDefault:
      // return NSCursor->arrowCursor();
      return nullptr;
    case ANCHOR_StandardCursorKnife:
      return getImageCursor(shape, NS::String::string("knife.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorEraser:
      return getImageCursor(shape, NS::String::string("eraser.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorPencil:
      return getImageCursor(shape, NS::String::string("pen.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorEyedropper:
      return getImageCursor(shape, NS::String::string("eyedropper.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorZoomIn:
      return getImageCursor(shape, NS::String::string("zoomin.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorZoomOut:
      return getImageCursor(shape, NS::String::string("zoomout.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorNSEWScroll:
      return getImageCursor(shape, NS::String::string("scrollnsew.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorNSScroll:
      return getImageCursor(shape, NS::String::string("scrollns.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorEWScroll:
      return getImageCursor(shape, NS::String::string("scrollew.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorUpArrow:
      return getImageCursor(shape, NS::String::string("arrowup.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorDownArrow:
      return getImageCursor(shape, NS::String::string("arrowdown.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorLeftArrow:
      return getImageCursor(shape, NS::String::string("arrowleft.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorRightArrow:
      return getImageCursor(shape, NS::String::string("arrowright.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorVerticalSplit:
      return getImageCursor(shape, NS::String::string("splitv.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorHorizontalSplit:
      return getImageCursor(shape, NS::String::string("splith.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorCrosshairA:
      return getImageCursor(shape,
                            NS::String::string("paint_cursor_cross.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorCrosshairB:
      return getImageCursor(shape,
                            NS::String::string("paint_cursor_dot.pdf", NS::UTF8StringEncoding));
    case ANCHOR_StandardCursorCrosshairC:
      return getImageCursor(shape, NS::String::string("crossc.pdf", NS::UTF8StringEncoding));
    default:
      return nullptr;
  }
}

/** Reverse the bits in a uint16_t */
static uint16_t uns16ReverseBits(uint16_t shrt)
{
  shrt = ((shrt >> 1) & 0x5555) | ((shrt << 1) & 0xAAAA);
  shrt = ((shrt >> 2) & 0x3333) | ((shrt << 2) & 0xCCCC);
  shrt = ((shrt >> 4) & 0x0F0F) | ((shrt << 4) & 0xF0F0);
  shrt = ((shrt >> 8) & 0x00FF) | ((shrt << 8) & 0xFF00);
  return shrt;
}

eAnchorStatus AnchorAppleMetal::setWindowCustomCursorShape(uint8_t *bitmap,
                                                           uint8_t *mask,
                                                           int sizex,
                                                           int sizey,
                                                           int hotX,
                                                           int hotY,
                                                           bool canInvertColor)
{
// todo: rewrite for cxx/swift.
//   int y, nbUns16;
//   NS::Point hotSpotPoint;
//   NS::BitmapImageRep *cursorImageRep;
//   NS::Image *cursorImage;
//   NS::Size imSize;
//   uint16_t *cursorBitmap;

//   NS::AutoreleasePool *pool = NS::AutoreleasePool->alloc()->init();

//   if (m_customCursor) {
//     [m_customCursor release];
//     m_customCursor = nil;
//   }

//   cursorImageRep = [[NSBitmapImageRep alloc]
//     initWithBitmapDataPlanes:nil
//                   pixelsWide:sizex
//                   pixelsHigh:sizey
//                bitsPerSample:1
//              samplesPerPixel:2
//                     hasAlpha:YES
//                     isPlanar:YES
//               colorSpaceName:NSDeviceWhiteColorSpace
//                  bytesPerRow:(sizex / 8 + (sizex % 8 > 0 ? 1 : 0))
//                 bitsPerPixel:1];

//   cursorBitmap = (uint16_t *)[cursorImageRep bitmapData];
//   nbUns16 = [cursorImageRep bytesPerPlane] / 2;

//   for (y = 0; y < nbUns16; y++) {
// #if !defined(__LITTLE_ENDIAN__)
//     cursorBitmap[y] = uns16ReverseBits((bitmap[2 * y] << 0) | (bitmap[2 * y + 1] << 8));
//     cursorBitmap[nbUns16 + y] = uns16ReverseBits((mask[2 * y] << 0) | (mask[2 * y + 1] << 8));
// #else
//     cursorBitmap[y] = uns16ReverseBits((bitmap[2 * y + 1] << 0) | (bitmap[2 * y] << 8));
//     cursorBitmap[nbUns16 + y] = uns16ReverseBits((mask[2 * y + 1] << 0) | (mask[2 * y] << 8));
// #endif

//     /* Flip white cursor with black outline to black cursor with white outline
//      * to match macOS platform conventions. */
//     if (canInvertColor) {
//       cursorBitmap[y] = ~cursorBitmap[y];
//     }
//   }

//   imSize.width = sizex;
//   imSize.height = sizey;
//   cursorImage = [[NSImage alloc] initWithSize:imSize];
//   [cursorImage addRepresentation:cursorImageRep];

//   hotSpotPoint.x = hotX;
//   hotSpotPoint.y = hotY;

//   // foreground and background color parameter is not handled for now (10.6)
//   m_customCursor = [[NSCursor alloc] initWithImage:cursorImage hotSpot:hotSpotPoint];

//   [cursorImageRep release];
//   [cursorImage release];

//   if ([m_window isVisible]) {
//     loadCursor(getCursorVisibility(), ANCHOR_StandardCursorCustom);
//   }
//   [pool drain];
  return ANCHOR_SUCCESS;
}

eAnchorStatus AnchorAppleMetal::hasCursorShape(eAnchorStandardCursor shape)
{
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
  eAnchorStatus success = (getStandardCursor(shape)) ? ANCHOR_SUCCESS : ANCHOR_FAILURE;
  pool->drain();
  return success;
}

void AnchorAppleMetal::loadCursor(bool visible, eAnchorStandardCursor cursorShape) const
{
  static bool systemCursorVisible = true;
  if (visible != systemCursorVisible) {
    if (visible) {
      // [NSCursor unhide];
      systemCursorVisible = true;
    } else {
      // [NSCursor hide];
      systemCursorVisible = false;
    }
  }

  // NSCursor *cursor = getStandardCursor(cursorShape);
  // if (cursor == nullptr) {
  //   cursor = getStandardCursor(ANCHOR_StandardCursorDefault);
  // }

  // [cursor set];
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