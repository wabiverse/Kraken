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

import Foundation

extension TextSelectionManager
{
  public func didReplaceCharacters(in range: NSRange, replacementLength: Int)
  {
    let delta = replacementLength == 0 ? -range.length : replacementLength
    for textSelection in textSelections
    {
      if textSelection.range.location > range.max
      {
        textSelection.range.location = max(0, textSelection.range.location + delta)
        textSelection.range.length = 0
      }
      else if textSelection.range.intersection(range) != nil
        || textSelection.range == range
        || (textSelection.range.isEmpty && textSelection.range.location == range.max)
      {
        if replacementLength > 0
        {
          textSelection.range.location = range.location + replacementLength
        }
        else
        {
          textSelection.range.location = range.location
        }
        textSelection.range.length = 0
      }
      else
      {
        textSelection.range.length = 0
      }
    }

    // Clean up duplicate selection ranges
    var allRanges: Set<NSRange> = []
    for (idx, selection) in textSelections.enumerated().reversed()
    {
      if allRanges.contains(selection.range)
      {
        textSelections.remove(at: idx)
      }
      else
      {
        allRanges.insert(selection.range)
      }
    }
  }

  func notifyAfterEdit()
  {
    updateSelectionViews()
    NotificationCenter.default.post(Notification(name: Self.selectionChangedNotification, object: self))
  }
}
