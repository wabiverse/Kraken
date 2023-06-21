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
import Foundation
import MetalKit

@_cdecl("KRKNCreateWindow")
public func KRKNCreateWindow(title: String,
                             left: CGFloat,
                             top: CGFloat,
                             width: CGFloat,
                             height: CGFloat,
                             dialog: Bool,
                             parent _: KRKNWindow?) -> KRKNWindow
{
  let frame = (NSScreen.main?.visibleFrame)!
  let contentRect: NSRect = NSWindow.contentRect(forFrameRect: frame, styleMask: [.titled, .closable, .miniaturizable])

  var bottom = (contentRect.size.height - 1) - height - top

  /* Ensures window top left is inside this available rect. */
  let leftAdjust = left > contentRect.origin.x ? left : contentRect.origin.x

  /* Add contentRect.origin.y to respect docksize. */
  bottom = bottom > contentRect.origin.y ? bottom + contentRect.origin.y : contentRect.origin.y

  let rect = NSRect(origin: CGPoint(x: leftAdjust, y: bottom), size: CGSize(width: width, height: height))

  var styleMask: NSWindow.StyleMask = [.titled, .closable, .resizable]
  if !dialog
  {
    styleMask = [.titled, .closable, .resizable, .miniaturizable]
  }

  return KRKNWindow(contentRect: rect, styleMask: styleMask, backing: NSWindow.BackingStoreType.buffered, defer: false, title: title)
}

open class KRKNMetalView: MTKView
{
  @objc
  override public init(frame: NSRect, device: MTLDevice?)
  {
    super.init(frame: frame, device: device)

    self.device = device
    wantsLayer = true

    allowedTouchTypes = [NSTouch.TouchTypeMask.direct, NSTouch.TouchTypeMask.indirect]
  }

  public required init(coder aDecoder: NSCoder)
  {
    super.init(coder: aDecoder)
  }
}

open class KRKNWindow: NSWindow
{
  var metalView: KRKNMetalView?
  var metalDevice: MTLDevice?
  let strongDelegate = KRKNWindowDelegate()

  @objc
  public init(contentRect: NSRect,
              styleMask: NSWindow.StyleMask,
              backing: NSWindow.BackingStoreType,
              defer flag: Bool,
              title: String)
  {
    super.init(contentRect: contentRect,
               styleMask: styleMask,
               backing: backing,
               defer: flag)

    contentMinSize = NSSize(width: 320, height: 240)
    metalDevice = MTLCreateSystemDefaultDevice()
    metalView = KRKNMetalView(frame: contentRect, device: metalDevice)
    initialFirstResponder = metalView
    contentView = metalView

    makeKeyAndOrderFront(nil)
    self.title = title

    delegate = strongDelegate

    acceptsMouseMovedEvents = true

    registerForDraggedTypes([
      NSPasteboard.PasteboardType("NSFilenamesPboardType"),
      NSPasteboard.PasteboardType("NSPasteboardTypeString"),
      NSPasteboard.PasteboardType("NSPasteboardTypeTIFF"),
    ])

    collectionBehavior = .fullScreenPrimary
  }
}

open class KRKNWindowDelegate: NSObject, NSWindowDelegate
{
  @objc
  override public init()
  {
    super.init()
  }

  public func windowDidMove(_: Notification)
  {
    fputs("Window moved\n", stderr)
  }
}
