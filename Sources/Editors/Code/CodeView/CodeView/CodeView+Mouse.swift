/* --------------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                   ::
 * --------------------------------------------------------------
 * @wabistudios :: metaverse :: kraken
 *
 * This program is free software; you can redistribute it, and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. Check out
 * the GNU General Public License for more details.
 *
 * You should have received a copy for this software license, the
 * GNU General Public License along with this program; or, if not
 * write to the Free Software Foundation, Inc., to the address of
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *                            Copyright (C) 2023 Wabi Foundation.
 *                                           All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

import AppKit

public extension CodeView
{
  override func mouseDown(with event: NSEvent)
  {
    // Set cursor
    guard isSelectable,
          event.type == .leftMouseDown,
          let offset = layoutManager.textOffsetAtPoint(convert(event.locationInWindow, from: nil))
    else
    {
      super.mouseDown(with: event)
      return
    }

    switch event.clickCount
    {
      case 1:
        guard isEditable
        else
        {
          super.mouseDown(with: event)
          return
        }
        if event.modifierFlags.intersection(.deviceIndependentFlagsMask).isSuperset(of: [.control, .shift])
        {
          unmarkText()
          selectionManager.addSelectedRange(NSRange(location: offset, length: 0))
        }
        else
        {
          selectionManager.setSelectedRange(NSRange(location: offset, length: 0))
          unmarkTextIfNeeded()
        }
      case 2:
        unmarkText()
        selectWord(nil)
      case 3:
        unmarkText()
        selectLine(nil)
      default:
        break
    }

    mouseDragTimer?.invalidate()
    // https://cocoadev.github.io/AutoScrolling/ (fired at ~45Hz)
    mouseDragTimer = Timer.scheduledTimer(withTimeInterval: 0.022, repeats: true)
    { [weak self] _ in
      if let event = self?.window?.currentEvent, event.type == .leftMouseDragged
      {
        self?.mouseDragged(with: event)
        self?.autoscroll(with: event)
      }
    }
  }

  override func mouseUp(with event: NSEvent)
  {
    mouseDragAnchor = nil
    mouseDragTimer?.invalidate()
    mouseDragTimer = nil
    super.mouseUp(with: event)
  }

  override func mouseDragged(with event: NSEvent)
  {
    guard !(inputContext?.handleEvent(event) ?? false), isSelectable
    else
    {
      return
    }

    if mouseDragAnchor == nil
    {
      mouseDragAnchor = convert(event.locationInWindow, from: nil)
      super.mouseDragged(with: event)
    }
    else
    {
      guard let mouseDragAnchor,
            let startPosition = layoutManager.textOffsetAtPoint(mouseDragAnchor),
            let endPosition = layoutManager.textOffsetAtPoint(convert(event.locationInWindow, from: nil))
      else
      {
        return
      }
      selectionManager.setSelectedRange(
        NSRange(
          location: min(startPosition, endPosition),
          length: max(startPosition, endPosition) - min(startPosition, endPosition)
        )
      )
      setNeedsDisplay()
      autoscroll(with: event)
    }
  }
}
