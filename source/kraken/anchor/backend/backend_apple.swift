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

open class KrakenApplication : NSApplication
{
  let strongDelegate = AppDelegate()

  public override init() 
  {
    super.init()
    self.delegate = strongDelegate
  }

  public required init?(coder: NSCoder) 
  {
    fatalError("init(coder:) has not been implemented")
  }

  @objc 
  public static func sayHello()
  {
    fputs("Hello Anchor.\n", stderr)

    _ = KrakenApplication.shared

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

    NSApp.finishLaunching()
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