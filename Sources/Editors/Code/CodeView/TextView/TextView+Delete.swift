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

extension TextView
{
  override open func deleteBackward(_: Any?)
  {
    delete(direction: .backward, destination: .character)
  }

  override open func deleteBackwardByDecomposingPreviousCharacter(_: Any?)
  {
    delete(direction: .backward, destination: .character, decomposeCharacters: true)
  }

  override open func deleteForward(_: Any?)
  {
    delete(direction: .forward, destination: .character)
  }

  override open func deleteWordBackward(_: Any?)
  {
    delete(direction: .backward, destination: .word)
  }

  override open func deleteWordForward(_: Any?)
  {
    delete(direction: .forward, destination: .word)
  }

  override open func deleteToBeginningOfLine(_: Any?)
  {
    delete(direction: .backward, destination: .line)
  }

  override open func deleteToEndOfLine(_: Any?)
  {
    delete(direction: .forward, destination: .line)
  }

  override open func deleteToBeginningOfParagraph(_: Any?)
  {
    delete(direction: .backward, destination: .line)
  }

  override open func deleteToEndOfParagraph(_: Any?)
  {
    delete(direction: .forward, destination: .line)
  }

  private func delete(
    direction: TextSelectionManager.Direction,
    destination: TextSelectionManager.Destination,
    decomposeCharacters _: Bool = false
  )
  {
    // Extend each selection by a distance specified by `destination`, then update both storage and the selection.
    for textSelection in selectionManager.textSelections
    {
      let extendedRange = selectionManager.rangeOfSelection(
        from: textSelection.range.location,
        direction: direction,
        destination: destination
      )
      guard extendedRange.location >= 0 else { continue }
      textSelection.range.formUnion(extendedRange)
    }
    replaceCharacters(in: selectionManager.textSelections.map(\.range), with: "")
    unmarkTextIfNeeded()
  }
}
