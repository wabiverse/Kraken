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

open class AnchorWindowApple : NSObject
{
  var window: NSWindow
  var metalDevice: MTLDevice!
  var metalView: MTKView!
  var dialog: Bool = false
  var state: eAnchorWindowState
  var windowDelegate: CocoaWindowDelegate

  public init(title: String, 
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
