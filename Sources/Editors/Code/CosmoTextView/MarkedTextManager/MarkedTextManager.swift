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

/// Manages marked ranges
class MarkedTextManager
{
  struct MarkedRanges
  {
    let ranges: [NSRange]
    let attributes: [NSAttributedString.Key: Any]
  }

  /// All marked ranges being tracked.
  private(set) var markedRanges: [NSRange] = []

  /// The attributes to use for marked text. Defaults to a single underline when `nil`
  var markedTextAttributes: [NSAttributedString.Key: Any]?

  /// True if there is marked text being tracked.
  var hasMarkedText: Bool
  {
    !markedRanges.isEmpty
  }

  /// Removes all marked ranges.
  func removeAll()
  {
    markedRanges.removeAll()
  }

  /// Updates the stored marked ranges.
  /// - Parameters:
  ///   - insertLength: The length of the string being inserted.
  ///   - replacementRange: The range to replace with marked text.
  ///   - selectedRange: The selected range from `NSTextInput`.
  ///   - textSelections: The current text selections.
  func updateMarkedRanges(
    insertLength: Int,
    replacementRange: NSRange,
    selectedRange: NSRange,
    textSelections: [TextSelectionManager.TextSelection]
  )
  {
    if replacementRange.location == NSNotFound
    {
      markedRanges = textSelections.map
      {
        NSRange(location: $0.range.location, length: insertLength)
      }
    }
    else
    {
      markedRanges = [selectedRange]
    }
  }

  /// Finds any marked ranges for a line and returns them.
  /// - Parameter lineRange: The range of the line.
  /// - Returns: A `MarkedRange` struct with information about attributes and ranges. `nil` if there is no marked
  ///            text for this line.
  func markedRanges(in lineRange: NSRange) -> MarkedRanges?
  {
    let attributes = markedTextAttributes ?? [.underlineStyle: NSUnderlineStyle.single.rawValue]
    let ranges = markedRanges.compactMap
    {
      $0.intersection(lineRange)
    }.map
    {
      NSRange(location: $0.location - lineRange.location, length: $0.length)
    }
    if ranges.isEmpty
    {
      return nil
    }
    else
    {
      return MarkedRanges(ranges: ranges, attributes: attributes)
    }
  }

  /// Updates marked text ranges for a new set of selections.
  /// - Parameter textSelections: The new text selections.
  /// - Returns: `True` if the marked text needs layout.
  func updateForNewSelections(textSelections: [TextSelectionManager.TextSelection]) -> Bool
  {
    // Ensure every marked range has a matching selection.
    // If any marked ranges do not have a matching selection, unmark.
    // Matching, in this context, means having a selection in the range location...max
    var markedRanges = markedRanges
    for textSelection in textSelections
    {
      if let markedRangeIdx = markedRanges.firstIndex(where: {
        ($0.location ... $0.max).contains(textSelection.range.location)
          && ($0.location ... $0.max).contains(textSelection.range.max)
      })
      {
        markedRanges.remove(at: markedRangeIdx)
      }
      else
      {
        return true
      }
    }

    // If any remaining marked ranges, we need to unmark.
    if !markedRanges.isEmpty
    {
      return false
    }
    else
    {
      return true
    }
  }
}
