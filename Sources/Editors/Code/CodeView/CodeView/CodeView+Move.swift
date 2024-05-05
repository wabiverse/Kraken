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

public extension CodeView
{
  private func updateAfterMove()
  {
    unmarkTextIfNeeded()
    scrollSelectionToVisible()
  }

  /// Moves the cursors up one character.
  override func moveUp(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .character)
    updateAfterMove()
  }

  /// Moves the cursors up one character extending the current selection.
  override func moveUpAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .character, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors down one character.
  override func moveDown(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .character)
    updateAfterMove()
  }

  /// Moves the cursors down one character extending the current selection.
  override func moveDownAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .character, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors left one character.
  override func moveLeft(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .character)
    updateAfterMove()
  }

  /// Moves the cursors left one character extending the current selection.
  override func moveLeftAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .character, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors right one character.
  override func moveRight(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .character)
    updateAfterMove()
  }

  /// Moves the cursors right one character extending the current selection.
  override func moveRightAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .character, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors left one word.
  override func moveWordLeft(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .word)
    updateAfterMove()
  }

  /// Moves the cursors left one word extending the current selection.
  override func moveWordLeftAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .word, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors right one word.
  override func moveWordRight(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .word)
    updateAfterMove()
  }

  /// Moves the cursors right one word extending the current selection.
  override func moveWordRightAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .word, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors left to the end of the line.
  override func moveToLeftEndOfLine(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .visualLine)
    updateAfterMove()
  }

  /// Moves the cursors left to the end of the line extending the current selection.
  override func moveToLeftEndOfLineAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .backward, destination: .visualLine, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors right to the end of the line.
  override func moveToRightEndOfLine(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .visualLine)
    updateAfterMove()
  }

  /// Moves the cursors right to the end of the line extending the current selection.
  override func moveToRightEndOfLineAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .forward, destination: .visualLine, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors to the beginning of the line, if pressed again selects the next line up.
  override func moveToBeginningOfParagraph(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .line)
    updateAfterMove()
  }

  /// Moves the cursors to the beginning of the line, if pressed again selects the next line up extending the current
  /// selection.
  override func moveToBeginningOfParagraphAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .line, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors to the end of the line, if pressed again selects the next line up.
  override func moveToEndOfParagraph(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .line)
    updateAfterMove()
  }

  /// Moves the cursors to the end of the line, if pressed again selects the next line up extending the current
  /// selection.
  override func moveToEndOfParagraphAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .line, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors to the beginning of the document.
  override func moveToBeginningOfDocument(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .document)
    updateAfterMove()
  }

  /// Moves the cursors to the beginning of the document extending the current selection.
  override func moveToBeginningOfDocumentAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .up, destination: .document, modifySelection: true)
    updateAfterMove()
  }

  /// Moves the cursors to the end of the document.
  override func moveToEndOfDocument(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .document)
    updateAfterMove()
  }

  /// Moves the cursors to the end of the document extending the current selection.
  override func moveToEndOfDocumentAndModifySelection(_: Any?)
  {
    selectionManager.moveSelections(direction: .down, destination: .document, modifySelection: true)
    updateAfterMove()
  }
}
