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

@objc
public enum AnchorWindowState: Int {
  case WindowStateNormal = 0
  case WindowStateMaximized = 1
  case WindowStateMinimized = 2
  case WindowStateFullScreen = 3
  case WindowStateEmbedded = 4
}

open class KrakenApplication : NSObject
{
  @objc 
  public static func graphicsBegin()
  {
    fputs("Hello Anchor.\n", stderr)

    let app = NSApplication.shared
    let strongDelegate = AppDelegate()
    app.delegate = strongDelegate

    NSApp = app
    NSApp.delegate = strongDelegate

    if (NSApp.mainMenu == nil) {
      var mainMenuBar = NSMenu()
      var menuItem = NSMenuItem()
      var windowMenu = NSMenu()
      var appMenu = NSMenu()

      appMenu.title = "Kraken"
      appMenu.addItem(withTitle: "About Kraken", action: #selector(NSApplication.orderFrontStandardAboutPanel(_:)), keyEquivalent: "")
      appMenu.addItem(NSMenuItem.separator())

      let hide = appMenu.addItem(withTitle: "Hide Kraken", action: #selector(NSApplication.hide), keyEquivalent: "h")
      hide.keyEquivalentModifierMask = [.command]
      let others = appMenu.addItem(withTitle: "Hide Others", action: #selector(NSApplication.hideOtherApplications), keyEquivalent: "h")
      others.keyEquivalentModifierMask = [.option, .command]
      appMenu.addItem(withTitle: "Show All", action: #selector(NSApplication.unhideAllApplications), keyEquivalent: "")
      let q = appMenu.addItem(withTitle: "Quit Kraken", action: #selector(NSApplication.terminate), keyEquivalent: "q")
      q.keyEquivalentModifierMask = [.command]
      menuItem.submenu = appMenu
      mainMenuBar.addItem(menuItem)

      // NSApp.performSelector(inBackground: Selector("setAppleMenu"), with: appMenu)

      windowMenu.title = "Window"
      let mini = windowMenu.addItem(withTitle: "Minimize", action: #selector(NSApplication.miniaturizeAll), keyEquivalent: "m")
      mini.keyEquivalentModifierMask = [.command]
      windowMenu.addItem(withTitle: "Zoom", action: #selector(NSApplication.accessibilityZoomButton), keyEquivalent: "")
      let fs = windowMenu.addItem(withTitle: "Enter Full Screen", action: #selector(NSApplication.accessibilityFullScreenButton), keyEquivalent: "f")
      fs.keyEquivalentModifierMask = [.control, .command]
      let x = windowMenu.addItem(withTitle: "Close", action: #selector(NSApplication.terminate), keyEquivalent: "w")
      x.keyEquivalentModifierMask = [.command]

      NSApp.mainMenu = mainMenuBar
      NSApp.windowsMenu = windowMenu
    }

    // NSApp.finishLaunching()
  }

  @objc
  public static func processEvents() -> Bool
  {
    var anyProcessed = false
    var event: NSEvent?

    repeat {
      let kill = autoreleasepool {
        event = NSApp.nextEvent(matching: .any, until: .distantPast, inMode: .default, dequeue: true)

        if (event == nil) {
          return true
        }

        anyProcessed = true

        NSApp.sendEvent(event!)

        return false
      }

      if (kill) {
        break
      }

    } while (event != nil)

    return anyProcessed
  }

  @objc 
  public static func createWindow(title: String, left: CGFloat, top: CGFloat, width: CGFloat, height: CGFloat, state: AnchorWindowState, isDialog: Bool)
  {
    var window: AnchorWindowApple?
    autoreleasepool {

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

      /**
       * might as well pass execution onto cocoa here,
       * just to keep kraken running with a window until
       * we have metal fleshed out on the swapchain. */
      NSApp.run()
    }
  }
}

class AnchorWindowApple : NSObject
{
  var window: NSWindow
  var metalDevice: MTLDevice!
  var metalLayer: CAMetalLayer!

  init(title: String, 
       left: CGFloat, 
       bottom: CGFloat, 
       width: CGFloat, 
       height: CGFloat, 
       state: AnchorWindowState, 
       dialog: Bool,
       parent: AnchorWindowApple?) 
  {
    let rect = NSRect(origin: CGPoint(x: left, y: bottom), size: CGSize(width: width, height: height))
    let minSize = NSSize(width: 320, height: 240)

    var styleMask: NSWindow.StyleMask = [.titled, .closable, .resizable]
    if (!dialog) {
      styleMask.insert(.miniaturizable)
    }

    self.window = NSWindow(contentRect: rect, styleMask: styleMask, backing: NSWindow.BackingStoreType.buffered, defer: false)
    self.window.contentMinSize = minSize

    self.metalDevice = MTLCreateSystemDefaultDevice()
    var view: NSView?

    if (metalDevice != nil) {
      self.metalLayer = CAMetalLayer()
      self.metalLayer.edgeAntialiasingMask = CAEdgeAntialiasingMask(rawValue: 0)
      self.metalLayer.masksToBounds = false
      self.metalLayer.isOpaque = false
      self.metalLayer.framebufferOnly = true
      self.metalLayer.presentsWithTransaction = false
      self.metalLayer.removeAllAnimations()
      self.metalLayer.device = metalDevice

      /* todo: create metal view. */
    }

    self.window.makeKeyAndOrderFront(nil)

    self.window.title = title
  }
}

class AppDelegate : NSObject, NSApplicationDelegate
{
  private var m_windowFocus = true

  override init() 
  {
    super.init()
    fputs("Kraken is LIVE.\n", stderr)
  }

  func applicationDidFinishLaunching(_ aNotification: Notification)
  {
    fputs("Kraken launched successfully.\n", stderr)

    if (self.m_windowFocus) {
      NSApp.activate(ignoringOtherApps: true)
    }

    NSEvent.isMouseCoalescingEnabled = false
  }

  func applicationWillTerminate(_ aNotification: Notification) 
  {

  }
}