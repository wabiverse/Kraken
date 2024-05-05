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
import TextStory

public extension CodeView
{
  override func selectAll(_: Any?)
  {
    selectionManager.setSelectedRange(documentRange)
    unmarkTextIfNeeded()
    needsDisplay = true
  }

  override func selectLine(_: Any?)
  {
    let newSelections = selectionManager.textSelections.compactMap
    { textSelection -> NSRange? in
      guard let linePosition = layoutManager.textLineForOffset(textSelection.range.location)
      else
      {
        return nil
      }
      return linePosition.range
    }
    selectionManager.setSelectedRanges(newSelections)
    unmarkTextIfNeeded()
    needsDisplay = true
  }

  override func selectWord(_: Any?)
  {
    let newSelections = selectionManager.textSelections.compactMap
    { textSelection -> NSRange? in
      guard textSelection.range.isEmpty,
            let char = textStorage.substring(
              from: NSRange(location: textSelection.range.location, length: 1)
            )?.first
      else
      {
        return nil
      }
      let charSet = CharacterSet(charactersIn: String(char))
      let characterSet: CharacterSet
      if CharacterSet.alphanumerics.isSuperset(of: charSet)
      {
        characterSet = .alphanumerics
      }
      else if CharacterSet.whitespaces.isSuperset(of: charSet)
      {
        characterSet = .whitespaces
      }
      else if CharacterSet.newlines.isSuperset(of: charSet)
      {
        characterSet = .newlines
      }
      else if CharacterSet.punctuationCharacters.isSuperset(of: charSet)
      {
        characterSet = .punctuationCharacters
      }
      else
      {
        return nil
      }
      guard let start = textStorage
        .findPrecedingOccurrenceOfCharacter(in: characterSet.inverted, from: textSelection.range.location),
        let end = textStorage
        .findNextOccurrenceOfCharacter(in: characterSet.inverted, from: textSelection.range.max)
      else
      {
        return nil
      }
      return NSRange(
        location: start,
        length: end - start
      )
    }
    selectionManager.setSelectedRanges(newSelections)
    unmarkTextIfNeeded()
    needsDisplay = true
  }
}
