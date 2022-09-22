/*
 * KrakenOS.System
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 *
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
 */

import Foundation
import MetalKit
import AppKit
import IOKit
import Carbon

@_cdecl("KRKNCreateSystem")
public func KRKNCreateSystem() -> KRKNSystem
{
  return KRKNSystem()
}

open class KRKNSystem : NSObject
{
  let app = NSApplication.shared
  let strongDelegate = KRKNAppDelegate()
  let menu = KRKNAppMenu(title: "Kraken", app: NSApplication.shared)

  @objc
  public override init() 
  {
    self.app.delegate = self.strongDelegate
    self.app.mainMenu = self.menu
    self.app.windowsMenu = self.menu.items[1].submenu

    NSWindow.allowsAutomaticWindowTabbing = false

    self.app.setActivationPolicy(NSApplication.ActivationPolicy.regular)
    self.app.finishLaunching()
  }

  @objc
  public func getMainDisplayDimensions() -> NSRect
  {
    fputs("got called\n", stderr)

    let frame = (NSScreen.main?.visibleFrame)!

    // Returns max window contents (excluding title bar...)
    let contentRect: NSRect = NSWindow.contentRect(forFrameRect: frame, styleMask: (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable))

    return contentRect
  }

  @objc
  public func processEvents() -> Bool
  { 
    var receivedEvent = false
    var anyProcessed = false

    repeat {
      if let event: NSEvent = self.app.nextEvent(matchingMask: NSEventMaskAny, until: NSDate.distantPast, inMode: RunLoop.Mode.default, dequeue: true) {

        anyProcessed = true
        receivedEvent = true

        if (event.type == .keyDown && 
            event.keyCode == kVK_Tab && 
            event.modifierFlags == NSEventModifierFlagControl) {
          handleEvent(keyEvent: event)
        } else {
          if (event.type == .keyUp &&
              (event.modifierFlags == (NSEventModifierFlagCommand | NSEventModifierFlagOption))) {
            handleEvent(keyEvent: event)
          }

          self.app.sendEvent(event)
        }

      } else {
        receivedEvent = false
      }

    } while (receivedEvent)

    return anyProcessed
  }

  @objc
  public func handleEvent(keyEvent: NSEvent)
  {
    let event = keyEvent

    var modifiers: NSEventModifierFlags?
    var characters: String?
    var convertedCharacters: NSData?
    var charactersIgnoringModifiers: String?

    if let window = event.window {
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
      //     if (m_modifierMask & NSEventModifierFlagCommand)
      //       utf8_buf[0] = '\0';

      //     if ((keyCode == AnchorKeyQ) && (m_modifierMask & NSEventModifierFlagCommand))
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

      //     if ((modifiers & NSEventModifierFlagShift) != (m_modifierMask & NSEventModifierFlagShift)) {
      //       pushEvent(new AnchorEventKey([event timestamp] * 1000,
      //                                   (modifiers & NSEventModifierFlagShift) ? AnchorEventTypeKeyDown :
      //                                                                             AnchorEventTypeKeyUp,
      //                                   window,
      //                                   AnchorKeyLeftShift,
      //                                   false));
      //     }
      //     if ((modifiers & NSEventModifierFlagControl) !=
      //         (m_modifierMask & NSEventModifierFlagControl)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & NSEventModifierFlagControl) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
      //           window,
      //           AnchorKeyLeftControl,
      //           false));
      //     }
      //     if ((modifiers & NSEventModifierFlagOption) !=
      //         (m_modifierMask & NSEventModifierFlagOption)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & NSEventModifierFlagOption) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
      //           window,
      //           AnchorKeyLeftAlt,
      //           false));
      //     }
      //     if ((modifiers & NSEventModifierFlagCommand) !=
      //         (m_modifierMask & NSEventModifierFlagCommand)) {
      //       pushEvent(new AnchorEventKey(
      //           [event timestamp] * 1000,
      //           (modifiers & NSEventModifierFlagCommand) ? AnchorEventTypeKeyDown : AnchorEventTypeKeyUp,
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
    } else {
      fputs("No window for event " + event.type.rawValue.description + "\n", stderr);
    }
  }
}

class KRKNAppMenu: NSMenu
{
  init(title: String, app: NSApplication) {
    super.init(title: title)

    let mainMenu = NSMenuItem()
    mainMenu.submenu = NSMenu(title: "Kraken")
    mainMenu.submenu?.items = [
      NSMenuItem(title: "About Kraken", target: app, action: #selector(app.orderFrontStandardAboutPanel(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Hide Kraken", target: app, action: #selector(app.hide(_:)), keyEquivalent: "h", modifier: NSEventModifierFlagCommand),
      NSMenuItem(title: "Hide Others", target: app, action: #selector(app.hideOtherApplications(_:)), keyEquivalent: "h", modifier: (NSEventModifierFlagOption | NSEventModifierFlagCommand)),
      NSMenuItem(title: "Show All", target: app, action: #selector(app.unhideAllApplications(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Quit Kraken", target: app, action: #selector(app.terminate(_:)), keyEquivalent: "q", modifier: NSEventModifierFlagCommand)
    ]

    let windowMenu = NSMenuItem()
    windowMenu.submenu = NSMenu(title: "Window")
    windowMenu.submenu?.items = [
      NSMenuItem(title: "Minimize", target: app, action: #selector(NSWindow.performMiniaturize(_:)), keyEquivalent: "m", modifier: NSEventModifierFlagCommand),
      NSMenuItem(title: "Zoom", target: app, action: #selector(NSWindow.performZoom(_:)), keyEquivalent: ""),
      NSMenuItem(title: "Enter Full Screen", target: app, action: #selector(NSWindow.toggleFullScreen(_:)), keyEquivalent: "f", modifier: (NSEventModifierFlagControl | NSEventModifierFlagCommand)),
      NSMenuItem(title: "Close", target: app, action: #selector(NSWindow.performClose(_:)), keyEquivalent: "w", modifier: NSEventModifierFlagCommand)
    ]

    items = [mainMenu, windowMenu]
  }

  required init(coder: NSCoder) {
    super.init(coder: coder)
  }
}

extension NSMenuItem
{ 
  convenience init(title string: String, 
                   target: AnyObject = NSMenuItem.self, 
                   action selector: Selector?, 
                   keyEquivalent charCode: String, 
                   modifier: NSEventModifierFlags = 0)
  {
    self.init(title: string, action: selector, keyEquivalent: charCode)

    if (modifier != 0) {
      self.keyEquivalentModifierMask = modifier
    }

    self.target = target
  }

  convenience init(title string: String, submenuItems: [NSMenuItem])
  {
    self.init(title: string, action: nil, keyEquivalent: "")
    self.submenu = NSMenu()
    self.submenu?.items = submenuItems
  }
}

class KRKNAppDelegate : NSObject, NSApplicationDelegate
{
  private var m_windowFocus = true
  // var cocoa: CocoaAppDelegate!

  override init() 
  {
    super.init()
    NotificationCenter.default.addObserver(self, selector: #selector(self.windowWillClose(_:)), name: NSWindow.willCloseNotification, object: nil)
    fputs("Kraken is LIVE.\n", stderr)
  }

  @objc
  func windowWillClose(_ notification: Notification)
  {
    let closingWindow = notification.object as! NSWindow

    if let index = NSApp.orderedWindows.firstIndex(of: closingWindow) {
      if (index != NSNotFound) {
        return
      }
    }

    for currentWindow in NSApp.orderedWindows {
      if (currentWindow == closingWindow) {
        continue
      }

      if (currentWindow.isOnActiveSpace && currentWindow.canBecomeMain) {
        currentWindow.makeKeyAndOrderFront(nil)
        return
      }
    }
    
    if let windowNumbers = NSWindow.windowNumbers(withOptions: .init(0)) {
      for windowNumber in windowNumbers {
        if let currentWindow = NSApp.window(withWindowNumber: windowNumber.intValue) {

          if (currentWindow == closingWindow) {
            continue
          }

          if (currentWindow.canBecomeKey) {
            currentWindow.makeKeyAndOrderFront(nil)
            return
          }
        }
      }
    }
  }

  func applicationDidFinishLaunching(_ notification: Notification)
  {
    fputs("Kraken launched successfully.\n", stderr)

    if (self.m_windowFocus) {
      NSApp.activate(ignoringOtherApps: true)
    }

    NSEvent.isMouseCoalescingEnabled = false
  }

  /**
   * @NOTE: Event processing is handled & dispatched in the central Anchor System.
   * This is so all platforms can share the same underlying system, vs rewriting
   * it all for swift. */

  func applicationWillBecomeActive(_ notification: Notification) 
  {
    /* calls instanced cxx anchor system function. */
    // self.cocoa.handleApplicationBecomeActiveEvent()
  }

  func application(_ sender: NSApplication, openFile filename: String) -> Bool
  { 
    /* calls instanced cxx anchor system function. */
    // return self.cocoa.handleOpenDocumentRequest(filename)
    return true
  }
}