/* --------------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                   ::
 * --------------------------------------------------------------
 * @wabistudios :: multiverse :: kraken
 *
 * CREDITS.
 *
 * T.Furby                 @furby-tm       <devs@wabi.foundation>
 *
 *            Copyright (C) 2023 Wabi Animation Studios, Ltd. Co.
 *                                           All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

import AppKit
import Carbon
import Foundation
import IOKit
import MetalKit

@_cdecl("KRKNCreateSystem")
public func KRKNCreateSystem() -> KRKNSystem
{
  KRKNSystem()
}

open class KRKNSystem: NSObject
{
  let app = NSApplication.shared
  let strongDelegate = KRKNAppDelegate()
  let menu = KRKNAppMenu(title: "Kraken", app: NSApplication.shared)

  @objc
  override public init()
  {
    app.delegate = strongDelegate
    app.mainMenu = menu
    app.windowsMenu = menu.items[1].submenu

    NSWindow.allowsAutomaticWindowTabbing = false

    app.setActivationPolicy(NSApplication.ActivationPolicy.regular)
    app.finishLaunching()
  }

  @objc
  public func getMainDisplayDimensions() -> NSRect
  {
    fputs("got called\n", stderr)

    let frame = (NSScreen.main?.visibleFrame)!

    // Returns max window contents (excluding title bar...)
    let contentRect: NSRect = NSWindow.contentRect(forFrameRect: frame, styleMask: [.titled, .closable, .miniaturizable])

    return contentRect
  }

  @objc
  public func processEvents() -> Bool
  {
    var receivedEvent = false
    var anyProcessed = false

    repeat
    {
      if let event: NSEvent = app.nextEvent(matching: .any, until: .distantPast, inMode: .default, dequeue: true)
      {
        anyProcessed = true
        receivedEvent = true

        if event.type == NSEvent.EventType.keyDown,
           event.keyCode == kVK_Tab,
           event.modifierFlags == .control
        {
          handleEvent(keyEvent: event)
        }
        else
        {
          if event.type == NSEvent.EventType.keyUp,
             event.modifierFlags == [.command, .option]
          {
            handleEvent(keyEvent: event)
          }

          app.sendEvent(event)
        }
      }
      else
      {
        receivedEvent = false
      }
    }
    while receivedEvent

    return anyProcessed
  }

  @objc
  public func handleEvent(keyEvent: NSEvent)
  {
    let event = keyEvent

    var modifiers: NSEvent.ModifierFlags?
    var characters: String?
    var convertedCharacters: NSData?
    var charactersIgnoringModifiers: String?

    if let window = event.window
    {
      // char utf8_buf[6] = {'\0'};

      // switch ([event type]) {

      //   case NSEventTypeKeyDown:
      //   case NSEventTypeKeyUp:
      //     charsIgnoringModifiers = [event charactersIgnoringModifiers];
      //     if ([charsIgnoringModifiers length] > 0) {
      //       keyCode = convertKey([event keyCode],
      //                           [charsIgnoringModifiers characterAtIndex:0],
      //                           [event type] == NSEventTypeKeyDown ? kUCKeyActionDown :
      //                                                                 kUCKeyActionUp);
      //     }
      //     else {
      //       keyCode = convertKey([event keyCode],
      //                           0,
      //                           [event type] == NSEventTypeKeyDown ? kUCKeyActionDown :
      //                                                                 kUCKeyActionUp);
      //     }

      //     characters = [event characters];
      //     if ([characters length] > 0) {
      //       convertedCharacters = [characters dataUsingEncoding:NSUTF8StringEncoding];

      //       for (int x = 0; x < [convertedCharacters length]; x++) {
      //         utf8_buf[x] = ((char *)[convertedCharacters bytes])[x];
      //       }
      //     }

      //     /* arrow keys should not have utf8 */
      //     if ((keyCode >= AnchorKeyLeftArrow) && (keyCode <= AnchorKeyDownArrow)) {
      //       utf8_buf[0] = '\0';
      //     }

      //     /* F keys should not have utf8 */
      //     if ((keyCode >= AnchorKeyF1) && (keyCode <= AnchorKeyF20))
      //       utf8_buf[0] = '\0';

      //     /* no text with command key pressed */
      //     if (m_modifierMask & .command)
      //       utf8_buf[0] = '\0';

      //     if ((keyCode == AnchorKeyQ) && (m_modifierMask & .command))
      //       break;  // Cmd-Q is directly handled by Cocoa

      //     if ([event type] == NSEventTypeKeyDown) {
      //       pushEvent(new AnchorEventKey([event timestamp] * 1000,
      //                                   AnchorEventTypeKeyDown,
      //                                   window,
      //                                   (eAnchorKey)keyCode,
      //                                   [event isARepeat],
      //                                   utf8_buf));
      //     }
      //     else {
      //       pushEvent(new AnchorEventKey([event timestamp] * 1000,
      //                                   AnchorEventTypeKeyUp,
      //                                   window,
      //                                   keyCode,
      //                                   false,
      //                                   NULL));
      //     }
      //     m_ignoreMomentumScroll = true;
      //     break;

      //   case NSEventTypeFlagsChanged:
      //     modifiers = [event modifierFlags];

      //     if ((modifiers & .shift) != (m_modifierMask & .shift)) {
      //       pushEvent(new AnchorEventKey([event timestamp] * 1000,
      //                                   (modifiers & .shift) ? AnchorEventTypeKeyDown :
      //                                                                             AnchorEventTypeKeyUp,
      //                                   window,
      //                                   AnchorKeyLeftShift,
      //                                   false));
      //     }
      //     if ((modifiers & .control) !=
      //         (m_modifierMask & .control)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & .control) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
      //           window,
      //           AnchorKeyLeftControl,
      //           false));
      //     }
      //     if ((modifiers & .option) !=
      //         (m_modifierMask & .option)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & .option) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
      //           window,
      //           AnchorKeyLeftAlt,
      //           false));
      //     }
      //     if ((modifiers & .command) !=
      //         (m_modifierMask & .command)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & .command) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
      //           window,
      //           AnchorKeyOS,
      //           false));
      //     }

      //     m_modifierMask = modifiers;
      //     m_ignoreMomentumScroll = true;
      //     break;

      //   default:
      //     return ANCHOR_FAILURE;
      //     break;
      // }
    }
    else
    {
      fputs("No window for event " + event.type.rawValue.description + "\n", stderr)
    }
  }
}

class KRKNAppMenu: NSMenu
{
  init(title: String, app: NSApplication)
  {
    super.init(title: title)

    let mainMenu = NSMenuItem()
    mainMenu.submenu = NSMenu(title: "Kraken")
    mainMenu.submenu?.items = [
      NSMenuItem(title: "About Kraken", target: NSApplication.self, action: #selector(app.orderFrontStandardAboutPanel(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Hide Kraken", target: NSApplication.self, action: #selector(app.hide(_:)), keyEquivalent: "h", modifier: .command),
      NSMenuItem(title: "Hide Others", target: NSApplication.self, action: #selector(app.hideOtherApplications(_:)), keyEquivalent: "h", modifier: [.option, .command]),
      NSMenuItem(title: "Show All", target: NSApplication.self, action: #selector(app.unhideAllApplications(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Quit Kraken", target: NSApplication.self, action: #selector(app.terminate(_:)), keyEquivalent: "q", modifier: .command),
    ]

    let windowMenu = NSMenuItem()
    windowMenu.submenu = NSMenu(title: "Window")
    windowMenu.submenu?.items = [
      NSMenuItem(title: "Minimize", target: NSApplication.self, action: #selector(NSWindow.performMiniaturize(_:)), keyEquivalent: "m", modifier: .command),
      NSMenuItem(title: "Zoom", target: NSApplication.self, action: #selector(NSWindow.performZoom(_:)), keyEquivalent: ""),
      NSMenuItem(title: "Enter Full Screen", target: NSApplication.self, action: #selector(NSWindow.toggleFullScreen(_:)), keyEquivalent: "f", modifier: [.control, .command]),
      NSMenuItem(title: "Close", target: NSApplication.self, action: #selector(NSWindow.performClose(_:)), keyEquivalent: "w", modifier: .command),
    ]

    items = [mainMenu, windowMenu]
  }

  required init(coder: NSCoder)
  {
    super.init(coder: coder)
  }
}

extension NSMenuItem
{
  convenience init(title string: String,
                   target: AnyObject = NSMenuItem.self,
                   action selector: Selector?,
                   keyEquivalent charCode: String,
                   modifier: NSEvent.ModifierFlags = .init(rawValue: 0))
  {
    self.init(title: string, action: selector, keyEquivalent: charCode)

    if modifier != .init(rawValue: 0)
    {
      keyEquivalentModifierMask = modifier
    }

    self.target = target
  }

  convenience init(title string: String, submenuItems: [NSMenuItem])
  {
    self.init(title: string, action: nil, keyEquivalent: "")
    submenu = NSMenu()
    submenu?.items = submenuItems
  }
}

class KRKNAppDelegate: NSObject, NSApplicationDelegate
{
  private var m_windowFocus = true
  // var cocoa: CocoaAppDelegate!

  override init()
  {
    super.init()
    NotificationCenter.default.addObserver(self, selector: #selector(windowWillClose(_:)), name: NSWindow.willCloseNotification, object: nil)
    fputs("Kraken is LIVE.\n", stderr)
  }

  @objc
  func windowWillClose(_ notification: Notification)
  {
    let closingWindow = notification.object as! NSWindow

    if let index = NSApp.orderedWindows.firstIndex(of: closingWindow)
    {
      if index != NSNotFound
      {
        return
      }
    }

    for currentWindow in NSApp.orderedWindows
    {
      if currentWindow == closingWindow
      {
        continue
      }

      if currentWindow.isOnActiveSpace, currentWindow.canBecomeMain
      {
        currentWindow.makeKeyAndOrderFront(nil)
        return
      }
    }

    if let windowNumbers = NSWindow.windowNumbers(options: .init(rawValue: 0))
    {
      for windowNumber in windowNumbers
      {
        if let currentWindow = NSApp.window(withWindowNumber: windowNumber.intValue)
        {
          if currentWindow == closingWindow
          {
            continue
          }

          if currentWindow.canBecomeKey
          {
            currentWindow.makeKeyAndOrderFront(nil)
            return
          }
        }
      }
    }
  }

  func applicationDidFinishLaunching(_: Notification)
  {
    fputs("Kraken launched successfully.\n", stderr)

    if m_windowFocus
    {
      NSApp.activate(ignoringOtherApps: true)
    }

    NSEvent.isMouseCoalescingEnabled = false
  }

  /*
   * @NOTE: Event processing is handled & dispatched in the central Anchor System.
   * This is so all platforms can share the same underlying system, vs rewriting
   * it all for swift. */

  func applicationWillBecomeActive(_: Notification)
  {
    /* calls instanced cxx anchor system function. */
    // self.cocoa.handleApplicationBecomeActiveEvent()
  }

  func application(_: NSApplication, openFile _: String) -> Bool
  {
    /* calls instanced cxx anchor system function. */
    // return self.cocoa.handleOpenDocumentRequest(filename)
    true
  }
}
