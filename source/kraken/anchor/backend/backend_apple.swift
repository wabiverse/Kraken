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
import Darwin
import Cocoa
import AppKit
import Metal
import QuartzCore

@objc
public enum AnchorWindowState: Int {
  case WindowStateNormal = 0
  case WindowStateMaximized = 1
  case WindowStateMinimized = 2
  case WindowStateFullScreen = 3
  case WindowStateEmbedded = 4
}

open class AnchorSystemApple : NSObject
{
  let app = NSApplication.shared
  let strongDelegate = AppDelegate()
  let menu = AppMenu()

  @objc 
  public init(cocoa: CocoaAppDelegate) 
  {
    super.init()
    fputs("Hello Anchor.\n", stderr)

    self.strongDelegate.cocoa = cocoa

    self.app.delegate = self.strongDelegate
    self.app.mainMenu = self.menu

    self.app.setActivationPolicy(.regular)
    self.app.finishLaunching()
  }

  @objc
  public static func processEvents() -> Bool
  {
    var anyProcessed = false

    while let event = NSApp.nextEvent(matching: .any, until: .distantPast, inMode: .default, dequeue: true) {
      anyProcessed = true
      NSApp.sendEvent(event)
    }

    return anyProcessed
  }

  @objc 
  public static func createWindow(title: String, left: CGFloat, top: CGFloat, width: CGFloat, height: CGFloat, state: AnchorWindowState, isDialog: Bool) -> AnchorWindowApple?
  {
    var window: AnchorWindowApple?

    let frame = (NSScreen.main?.visibleFrame)!
    let contentRect = NSWindow.contentRect(forFrameRect: frame, styleMask: [.titled, .closable, .miniaturizable])

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
      NSMenuItem(title: "Hide Kraken", action: #selector(NSApplication.hide(_:)), keyEquivalent: "h", modifier: [.command]),
      NSMenuItem(title: "Hide Others", action: #selector(NSApplication.hideOtherApplications(_:)), keyEquivalent: "h", modifier: [.option, .command]),
      NSMenuItem(title: "Show All", action: #selector(NSApplication.unhideAllApplications(_:)), keyEquivalent: ""),
      NSMenuItem.separator(),
      NSMenuItem(title: "Quit Kraken", action: #selector(NSApplication.shared.terminate(_:)), keyEquivalent: "q", modifier: [.command])
    ]

    let windowMenu = NSMenuItem()
    windowMenu.submenu = NSMenu(title: "Window")
    windowMenu.submenu?.items = [
      NSMenuItem(title: "Minimize", action: #selector(NSWindow.miniaturize(_:)), keyEquivalent: "m", modifier: [.command]),
      NSMenuItem(title: "Zoom", action: #selector(NSWindow.performZoom(_:)), keyEquivalent: ""),
      NSMenuItem(title: "Enter Full Screen", action: #selector(NSWindow.toggleFullScreen(_:)), keyEquivalent: "f", modifier: [.control, .command]),
      NSMenuItem(title: "Close", action: #selector(NSWindow.performClose(_:)), keyEquivalent: "w", modifier: [.command])
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
                   target: AnyObject = self as AnyObject, 
                   action selector: Selector?, 
                   keyEquivalent charCode: String, 
                   modifier: NSEvent.ModifierFlags = .command)
  {
    self.init(title: string, action: selector, keyEquivalent: charCode)
    keyEquivalentModifierMask = modifier
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
  var metalLayer: CAMetalLayer!
  var metalView: NSView
  
  var state: AnchorWindowState

  init(title: String, 
       left: CGFloat, 
       bottom: CGFloat, 
       width: CGFloat, 
       height: CGFloat, 
       state: AnchorWindowState, 
       dialog: Bool,
       parent: AnchorWindowApple?) 
  {
    self.state = state

    let rect = NSRect(origin: CGPoint(x: left, y: bottom), size: CGSize(width: width, height: height))
    let minSize = NSSize(width: 320, height: 240)

    var styleMask: NSWindow.StyleMask = [.titled, .closable, .resizable]
    if (!dialog) {
      styleMask.insert(.miniaturizable)
    }

    self.window = NSWindow(contentRect: rect, styleMask: styleMask, backing: NSWindow.BackingStoreType.buffered, defer: false)
    self.window.contentMinSize = minSize

    /* setup metal device. */
    self.metalDevice = MTLCreateSystemDefaultDevice()
    
    /* setup metal layer. */
    self.metalLayer = CAMetalLayer()
    self.metalLayer.edgeAntialiasingMask = CAEdgeAntialiasingMask(rawValue: 0)
    self.metalLayer.masksToBounds = false
    self.metalLayer.isOpaque = true
    self.metalLayer.framebufferOnly = true
    self.metalLayer.presentsWithTransaction = false
    self.metalLayer.removeAllAnimations()
    self.metalLayer.device = self.metalDevice

    /* setup metal view. */
    self.metalView = NSView(frame: rect)
    self.metalView.wantsLayer = true
    self.metalView.layer = self.metalLayer

    /* attach to window. */
    self.window.contentView = self.metalView
    self.window.initialFirstResponder = self.metalView

    /**
     * support other types: OpenGL. Vulkan.
     * ... ??? ---------------------------- */

    self.window.makeKeyAndOrderFront(nil)
    self.window.title = title
  }

  @objc
  public func getMetalView() -> NSView
  {
    return self.metalView
  }

  @objc
  public func getCocoaWindow() -> NSWindow
  {
    return self.window
  }

  @objc
  public func setCocoaTitle(title: String) -> Void
  {
    self.window.title = title
  }

  @objc
  public func getCocoaTitle() -> String
  {
    return self.window.title
  }

  @objc 
  public func closeCocoaWindow() -> Void
  {
    self.window.close()
  }

  @objc 
  public func getCocoaState() -> AnchorWindowState
  {
    return self.state
  }
}

class AppDelegate : NSObject, NSApplicationDelegate
{
  private var m_windowFocus = true
  var cocoa: CocoaAppDelegate!

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
      
    if let windowNumbers = NSWindow.windowNumbers(options: .init(rawValue: 0)) {
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

  func applicationWillTerminate(_ notification: Notification) 
  {

  }

  func application(_ sender: NSApplication, openFile filename: String) -> Bool
  {    
    return self.cocoa.handleOpenDocumentRequest(filename)
  }
}