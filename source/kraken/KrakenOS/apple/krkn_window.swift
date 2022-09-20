/*
 * KrakenOS.Window
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

@_cdecl("KRKNCreateWindow")
public func KRKNCreateWindow(title: String, 
                             left: CGFloat, 
                             top: CGFloat, 
                             width: CGFloat, 
                             height: CGFloat,
                             dialog: Bool,
                             parent: KRKNWindow?) -> KRKNWindow
{
  let frame = (NSScreen.main?.visibleFrame)!
  let contentRect: NSRect = NSWindow.contentRect(forFrameRect: frame, styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)

  var bottom = (contentRect.size.height - 1) - height - top

  /* Ensures window top left is inside this available rect. */
  let leftAdjust = left > contentRect.origin.x ? left : contentRect.origin.x

  /* Add contentRect.origin.y to respect docksize. */
  bottom = bottom > contentRect.origin.y ? bottom + contentRect.origin.y : contentRect.origin.y

  let rect = NSRect(origin: CGPoint(x: leftAdjust, y: bottom), size: CGSize(width: width, height: height))

  var styleMask: NSWindowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
  if (!dialog) {
    styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
  }

  return KRKNWindow(contentRect: rect, styleMask: styleMask, backing: NSWindow.BackingStoreType.buffered, defer: false, title: title)
}

open class KRKNMetalView : MTKView
{
  @objc
  public override init(frame: NSRect, device: MTLDevice?)
  {
    super.init(frame: frame, device: device)

    self.device = device
    self.wantsLayer = true

    self.allowedTouchTypes = (NSTouchTypeMaskDirect | NSTouchTypeMaskIndirect)
  }

  public required init(coder aDecoder: NSCoder) {
    super.init(coder: aDecoder)
  }
}

open class KRKNWindow : NSWindow
{
  var metalView: KRKNMetalView?
  var metalDevice: MTLDevice?
  let strongDelegate = KRKNWindowDelegate()

  @objc
  public init(contentRect: NSRect, 
              styleMask: NSWindowStyleMask, 
              backing: NSWindow.BackingStoreType, 
              defer flag: Bool,
              title: String)
  {
    super.init(contentRect: contentRect, 
               styleMask: styleMask, 
               backing: backing, 
               defer: flag)
  
    self.contentMinSize = NSSize(width: 320, height: 240)
    self.metalDevice = MTLCreateSystemDefaultDevice()
    self.metalView = KRKNMetalView(frame: contentRect, device: self.metalDevice)
    self.initialFirstResponder = self.metalView
    self.contentView = self.metalView

    self.makeKeyAndOrderFront(nil)
    self.title = title

    self.delegate = strongDelegate

    self.acceptsMouseMovedEvents = true

    self.registerForDraggedTypes([
      NSPasteboard.PasteboardType("NSFilenamesPboardType"),
      NSPasteboard.PasteboardType("NSPasteboardTypeString"),
      NSPasteboard.PasteboardType("NSPasteboardTypeTIFF")
    ])

    self.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary
  }
}

open class KRKNWindowDelegate : NSObject, NSWindowDelegate
{
  @objc
  public override init()
  {
    super.init()
  }

  public func windowDidMove(_ notification: Notification) {
    fputs("Window moved\n", stderr)
  }
}