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

/// # Notes
///
/// This implementation considers the entire document as one element, ignoring all subviews and lines.
/// Another idea would be to make each line fragment an accessibility element, with options for navigating through
/// lines from there. The text view would then only handle text input, and lines would handle reading out useful data
/// to the user.
/// More research needs to be done for the best option here.
extension CodeView
{
  override open func isAccessibilityElement() -> Bool
  {
    true
  }

  override open func isAccessibilityEnabled() -> Bool
  {
    true
  }

  override open func isAccessibilityFocused() -> Bool
  {
    isFirstResponder
  }

  override open func accessibilityLabel() -> String?
  {
    "Text Editor"
  }

  override open func accessibilityRole() -> NSAccessibility.Role?
  {
    .textArea
  }

  override open func accessibilityValue() -> Any?
  {
    string
  }

  override open func setAccessibilityValue(_ accessibilityValue: Any?)
  {
    guard let string = accessibilityValue as? String
    else
    {
      return
    }

    self.string = string
  }

  override open func accessibilityString(for range: NSRange) -> String?
  {
    textStorage.substring(
      from: textStorage.mutableString.rangeOfComposedCharacterSequences(for: range)
    )
  }

  // MARK: Selections

  override open func accessibilitySelectedText() -> String?
  {
    guard let selection = selectionManager
      .textSelections
      .sorted(by: { $0.range.lowerBound < $1.range.lowerBound })
      .first
    else
    {
      return nil
    }
    let range = (textStorage.string as NSString).rangeOfComposedCharacterSequences(for: selection.range)
    return textStorage.substring(from: range)
  }

  override open func accessibilitySelectedTextRange() -> NSRange
  {
    guard let selection = selectionManager
      .textSelections
      .sorted(by: { $0.range.lowerBound < $1.range.lowerBound })
      .first
    else
    {
      return .zero
    }
    return textStorage.mutableString.rangeOfComposedCharacterSequences(for: selection.range)
  }

  override open func accessibilitySelectedTextRanges() -> [NSValue]?
  {
    selectionManager.textSelections.map
    { selection in
      textStorage.mutableString.rangeOfComposedCharacterSequences(for: selection.range) as NSValue
    }
  }

  override open func accessibilityInsertionPointLineNumber() -> Int
  {
    guard let selection = selectionManager
      .textSelections
      .sorted(by: { $0.range.lowerBound < $1.range.lowerBound })
      .first,
      let linePosition = layoutManager.textLineForOffset(selection.range.location)
    else
    {
      return 0
    }
    return linePosition.index
  }

  override open func setAccessibilitySelectedTextRange(_ accessibilitySelectedTextRange: NSRange)
  {
    selectionManager.setSelectedRange(accessibilitySelectedTextRange)
  }

  override open func setAccessibilitySelectedTextRanges(_ accessibilitySelectedTextRanges: [NSValue]?)
  {
    let ranges = accessibilitySelectedTextRanges?.compactMap { $0 as? NSRange } ?? []
    selectionManager.setSelectedRanges(ranges)
  }

  // MARK: Text Ranges

  override open func accessibilityNumberOfCharacters() -> Int
  {
    string.count
  }

  override open func accessibilityRange(forLine line: Int) -> NSRange
  {
    guard line >= 0, layoutManager.lineStorage.count > line,
          let linePosition = layoutManager.textLineForIndex(line)
    else
    {
      return .zero
    }
    return linePosition.range
  }

  override open func accessibilityRange(for point: NSPoint) -> NSRange
  {
    guard let location = layoutManager.textOffsetAtPoint(point) else { return .zero }
    return NSRange(location: location, length: 0)
  }

  override open func accessibilityRange(for index: Int) -> NSRange
  {
    textStorage.mutableString.rangeOfComposedCharacterSequence(at: index)
  }
}
