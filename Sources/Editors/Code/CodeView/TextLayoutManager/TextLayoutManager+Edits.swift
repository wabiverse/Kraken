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

// MARK: - Edits

extension TextLayoutManager: NSTextStorageDelegate
{
  /// Notifies the layout manager of an edit.
  ///
  /// Used by the `CodeView` to tell the layout manager about any edits that will happen.
  /// Use this to keep the layout manager's line storage in sync with the text storage.
  ///
  /// - Parameters:
  ///   - range: The range of the edit.
  ///   - string: The string to replace in the given range.
  public func willReplaceCharactersInRange(range: NSRange, with string: String)
  {
    // Loop through each line being replaced in reverse, updating and removing where necessary.
    for linePosition in lineStorage.linesInRange(range).reversed()
    {
      // Two cases: Updated line, deleted line entirely
      guard let intersection = linePosition.range.intersection(range), !intersection.isEmpty else { continue }
      if intersection == linePosition.range, linePosition.range.max != lineStorage.length
      {
        // Delete line
        lineStorage.delete(lineAt: linePosition.range.location)
      }
      else if intersection.max == linePosition.range.max,
              let nextLine = lineStorage.getLine(atOffset: linePosition.range.max)
      {
        // Need to merge line with one after it after updating this line to remove the end of the line
        lineStorage.delete(lineAt: nextLine.range.location)
        let delta = -intersection.length + nextLine.range.length
        if delta != 0
        {
          lineStorage.update(atIndex: linePosition.range.location, delta: delta, deltaHeight: 0)
        }
      }
      else
      {
        lineStorage.update(atIndex: linePosition.range.location, delta: -intersection.length, deltaHeight: 0)
      }
    }

    // Loop through each line being inserted, inserting & splitting where necessary
    if !string.isEmpty
    {
      var index = 0
      while let nextLine = (string as NSString).getNextLine(startingAt: index)
      {
        let lineRange = NSRange(location: index, length: nextLine.max - index)
        applyLineInsert((string as NSString).substring(with: lineRange) as NSString, at: range.location + index)
        index = nextLine.max
      }

      if index < (string as NSString).length
      {
        // Get the last line.
        applyLineInsert(
          (string as NSString).substring(from: index) as NSString,
          at: range.location + index
        )
      }
    }
    setNeedsLayout()
  }

  /// Applies a line insert to the internal line storage tree.
  /// - Parameters:
  ///   - insertedString: The string being inserted.
  ///   - location: The location the string is being inserted into.
  private func applyLineInsert(_ insertedString: NSString, at location: Int)
  {
    if LineEnding(line: insertedString as String) != nil
    {
      if location == textStorage?.length ?? 0
      {
        // Insert a new line at the end of the document, need to insert a new line 'cause there's nothing to
        // split. Also, append the new text to the last line.
        lineStorage.update(atIndex: location, delta: insertedString.length, deltaHeight: 0.0)
        lineStorage.insert(
          line: TextLine(),
          atOffset: location + insertedString.length,
          length: 0,
          height: estimateLineHeight()
        )
      }
      else
      {
        // Need to split the line inserting into and create a new line with the split section of the line
        guard let linePosition = lineStorage.getLine(atOffset: location) else { return }
        let splitLocation = location + insertedString.length
        let splitLength = linePosition.range.max - location
        let lineDelta = insertedString.length - splitLength // The difference in the line being edited
        if lineDelta != 0
        {
          lineStorage.update(atIndex: location, delta: lineDelta, deltaHeight: 0.0)
        }

        lineStorage.insert(
          line: TextLine(),
          atOffset: splitLocation,
          length: splitLength,
          height: estimateLineHeight()
        )
      }
    }
    else
    {
      lineStorage.update(atIndex: location, delta: insertedString.length, deltaHeight: 0.0)
    }
  }

  /// This method is to simplify keeping the layout manager in sync with attribute changes in the storage object.
  /// This does not handle cases where characters have been inserted or removed from the storage.
  /// For that, see the `willPerformEdit` method.
  public func textStorage(
    _: NSTextStorage,
    didProcessEditing editedMask: NSTextStorageEditActions,
    range editedRange: NSRange,
    changeInLength delta: Int
  )
  {
    if editedMask.contains(.editedAttributes), delta == 0
    {
      invalidateLayoutForRange(editedRange)
    }
  }
}
