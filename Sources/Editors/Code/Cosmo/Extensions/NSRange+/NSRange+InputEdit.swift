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

import CodeView
import Foundation
import SwiftTreeSitter

extension InputEdit
{
  init?(range: NSRange, delta: Int, oldEndPoint: Point, textView: TextView)
  {
    let newEndLocation = NSMaxRange(range) + delta

    if newEndLocation < 0
    {
      assertionFailure("Invalid range/delta")
      return nil
    }

    let newRange = NSRange(location: range.location, length: range.length + delta)
    let startPoint = textView.pointForLocation(newRange.location) ?? .zero
    let newEndPoint = textView.pointForLocation(newEndLocation) ?? .zero

    self.init(
      startByte: UInt32(range.location * 2),
      oldEndByte: UInt32(NSMaxRange(range) * 2),
      newEndByte: UInt32(newEndLocation * 2),
      startPoint: startPoint,
      oldEndPoint: oldEndPoint,
      newEndPoint: newEndPoint
    )
  }
}

extension NSRange
{
  // swiftlint:disable line_length
  /// Modifies the range to account for an edit.
  /// Largely based on code from
  /// [tree-sitter](https://github.com/tree-sitter/tree-sitter/blob/ddeaa0c7f534268b35b4f6cb39b52df082754413/lib/src/subtree.c#L691-L720)
  mutating func applyInputEdit(_ edit: InputEdit)
  {
    // swiftlint:enable line_length
    let endIndex = NSMaxRange(self)
    let isPureInsertion = edit.oldEndByte == edit.startByte

    // Edit is after the range
    if (edit.startByte / 2) > endIndex
    {
      return
    }
    else if edit.oldEndByte / 2 < location
    {
      // If the edit is entirely before this range
      location += (Int(edit.newEndByte) - Int(edit.oldEndByte)) / 2
    }
    else if edit.startByte / 2 < location
    {
      // If the edit starts in the space before this range and extends into this range
      length -= Int(edit.oldEndByte) / 2 - location
      location = Int(edit.newEndByte) / 2
    }
    else if edit.startByte / 2 == location, isPureInsertion
    {
      // If the edit is *only* an insertion right at the beginning of the range
      location = Int(edit.newEndByte) / 2
    }
    else
    {
      // Otherwise, the edit is entirely within this range
      if edit.startByte / 2 < endIndex || (edit.startByte / 2 == endIndex && isPureInsertion)
      {
        length = (Int(edit.newEndByte) / 2 - location) + (length - (Int(edit.oldEndByte) / 2 - location))
      }
    }
  }
}
