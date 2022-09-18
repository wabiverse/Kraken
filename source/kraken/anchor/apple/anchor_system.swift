/**
 *  backend_apple.swift
 *  Anchor System for Kraken on macOS.
 *
 *  The interface gets auto-generated into
 *  Anchor.h and the Anchor backend for Cocoa
 *  calls directly into this swift class.
 *
 *  This system is in charge of managing the
 *  underlying system time, displays & window
 *  events, and other Operating System bits.
 *
 *  Created by Wabi Animation Studios
 *  Copyright © 2022 Wabi. All rights reserved.
 */

import Foundation
import MetalKit
import AppKit
import IOKit

/* Pixar USD. */
import Pixar

/* Kraken Anchor. */
import Anchor

public struct AnchorSystemApple
{
  let app = NSApplication.shared
  let strongDelegate = AppDelegate()
  let menu = AppMenu()

  public init() 
  {
    /** 
     * @UNIFIED Anchor System.
     * recieves cocoa instance, so swift can use 
     * the same instanced cxx anchor system. */
    // self.strongDelegate.cocoa = cocoa

    self.app.delegate = self.strongDelegate
    self.app.mainMenu = self.menu

    self.app.setActivationPolicy(.regular)
    self.app.finishLaunching()
  }

  public static func createWindow(title: String, left: CGFloat, top: CGFloat, width: CGFloat, height: CGFloat, state: eAnchorWindowState, isDialog: Bool) -> AnchorWindowApple?
  {
    var window: AnchorWindowApple?

    let frame = (NSScreen.main?.visibleFrame)!
    // let contentRect = NSWindow.contentRect(forFrameRect: frame, styleMask: [.titled, .closable, .miniaturizable])
    let contentRect = NSWindow.contentRect(forFrameRect: frame, styleMask: NSWindowStyleMask.init(integerLiteral: NSWindowStyleMaskTitled + NSWindowStyleMaskClosable + NSWindowStyleMaskMiniaturizable))

    var bottom = (contentRect.size.height - 1) - height - top

    /* Ensures window top left is inside this available rect. */
    let leftC = left > contentRect.origin.x ? left : contentRect.origin.x
    /* Add contentRect.origin.y to respect docksize. */
    bottom = bottom > contentRect.origin.y ? bottom + contentRect.origin.y : contentRect.origin.y

    window = AnchorWindowApple(title: title, 
                               left: leftC, 
                               bottom: bottom, 
                               width: width, 
                               height: height, 
                               state: state, 
                               dialog: isDialog,
                               parent: nil)

    if let window = window {
      /* proceed... */
    } else {
      fputs("AnchorWindowApple::createWindow(): window invalid\n", stderr)
      window = nil
    }

    return window
  }
}

class AppMenu: NSMenu
{
  override init(title: String) {
    super.init(title: title)

    let mainMenu = NSMenuItem()
    mainMenu.submenu = NSMenu(title: "Kraken")
    mainMenu.submenu?.items = [
      NSMenuItem(title: "About Kraken", action: #selector(NSApplication.orderFrontStandardAboutPanel(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Hide Kraken", action: #selector(NSApplication.hide(_:)), keyEquivalent: "h", modifier: NSEventModifierFlagCommand),
      NSMenuItem(title: "Hide Others", action: #selector(NSApplication.hideOtherApplications(_:)), keyEquivalent: "h", modifier: NSEventModifierFlagCommand + NSEventModifierFlagOption),
      NSMenuItem(title: "Show All", action: #selector(NSApplication.unhideAllApplications(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Quit Kraken", action: #selector(NSApplication.shared.terminate(_:)), keyEquivalent: "q", modifier: NSEventModifierFlagCommand)
    ]

    let windowMenu = NSMenuItem()
    windowMenu.submenu = NSMenu(title: "Window")
    windowMenu.submenu?.items = [
      NSMenuItem(title: "Minimize", action: #selector(NSWindow.miniaturize(_:)), keyEquivalent: "m", modifier: NSEventModifierFlagCommand),
      NSMenuItem(title: "Zoom", action: #selector(NSWindow.performZoom(_:)), keyEquivalent: ""),
      NSMenuItem(title: "Enter Full Screen", action: #selector(NSWindow.toggleFullScreen(_:)), keyEquivalent: "f", modifier: NSEventModifierFlagCommand + NSEventModifierFlagControl),
      NSMenuItem(title: "Close", action: #selector(NSWindow.performClose(_:)), keyEquivalent: "w", modifier: NSEventModifierFlagCommand)
    ]

    items = [mainMenu, windowMenu]
  }

  required init(coder: NSCoder) {
    super.init(coder: coder)
  }
}

extension NSMenuItem : @unchecked Sendable
{
  convenience init(title string: String, 
                   target: AnyObject = NSMenuItem.self as AnyObject, 
                   action selector: Selector?, 
                   keyEquivalent charCode: String, 
                   modifier: NSEventModifierFlags)
  {
    self.init(title: string, action: selector, keyEquivalent: charCode)

    // keyEquivalentModifierMask = .command

    self.target = target
  }

  convenience init(title string: String, submenuItems: [NSMenuItem])
  {
    self.init(title: string, action: nil, keyEquivalent: "")
    self.submenu = NSMenu()
    self.submenu?.items = submenuItems
  }
}

open class AnchorWindowApple : NSObject
{
  var window: NSWindow
  var metalDevice: MTLDevice!
  var metalView: MTKView!
  var dialog: Bool = false
  var state: eAnchorWindowState
  var windowDelegate: CocoaWindowDelegate

  init(title: String, 
       left: CGFloat, 
       bottom: CGFloat, 
       width: CGFloat, 
       height: CGFloat, 
       state: eAnchorWindowState, 
       dialog: Bool,
       parent: AnchorWindowApple?) 
  {
    self.state = state
    self.dialog = dialog

    let rect = NSRect(origin: CGPoint(x: left, y: bottom), size: CGSize(width: width, height: height))
    let minSize = NSSize(width: 320, height: 240)

    var styleMask: NSWindowStyleMask = NSWindowStyleMask.init(integerLiteral: NSWindowStyleMaskTitled + NSWindowStyleMaskClosable + NSWindowStyleMaskResizable)
    if (!dialog) {
      styleMask += NSWindowStyleMask.init(integerLiteral: NSWindowStyleMaskMiniaturizable)
    }

    self.window = NSWindow(contentRect: rect, styleMask: styleMask, backing: .buffered, defer: false)
    self.window.contentMinSize = minSize

    /* setup metal device. */
    self.metalDevice = MTLCreateSystemDefaultDevice()

    /* setup metal view. */
    self.metalView = MTKView(frame: rect, device: self.metalDevice)
    self.metalView.wantsLayer = true

    /* attach to window. */
    self.window.contentView = self.metalView
    self.window.initialFirstResponder = self.metalView

    /**
     * support other types: OpenGL. Vulkan.
     * ... ??? ---------------------------- */

    self.window.makeKeyAndOrderFront(nil)
    self.window.title = title

    self.windowDelegate = CocoaWindowDelegate()
    self.window.delegate = self.windowDelegate
  }

  public func getMetalView() -> MTKView
  {
    return self.metalView
  }

  public func getCocoaWindow() -> NSWindow
  {
    return self.window
  }

  public func setCocoaTitle(title: String) -> Void
  {
    self.window.title = title
  }

  public func getCocoaTitle() -> String
  {
    return self.window.title
  }

  public func closeCocoaWindow() -> Void
  {
    self.window.close()
  }

  public func getCocoaState() -> eAnchorWindowState
  {
    return self.state
  }

  public func setCocoaState(newState: eAnchorWindowState)
  {
    self.state = newState
  }

  public func getScreen() -> NSScreen
  {
    return self.window.screen!
  }

  public func getIsDialog() -> Bool
  {
    return self.dialog
  }
}

class CocoaWindowDelegate : NSObject, NSWindowDelegate
{
  override init()
  {
    super.init()
  }

  func windowDidMove(_ notification: Notification) {
    fputs("Window moved\n", stderr)
  }
}

class AppDelegate : NSObject, NSApplicationDelegate
{
  private var m_windowFocus = true
  // var cocoa: CocoaAppDelegate!

  override init() 
  {
    super.init()
    // NotificationCenter.default.addObserver(self, selector: #selector(self.windowWillClose(_:)), name: NSWindow.willCloseNotification, object: nil)
    fputs("Kraken is LIVE.\n", stderr)
  }

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