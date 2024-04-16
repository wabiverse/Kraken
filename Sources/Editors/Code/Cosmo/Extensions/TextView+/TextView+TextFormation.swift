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
import TextFormation
import TextStory

extension TextView: TextInterface
{
  public var selectedRange: NSRange
  {
    get
    {
      selectionManager
        .textSelections
        .sorted(by: { $0.range.lowerBound < $1.range.lowerBound })
        .first?
        .range ?? .zero
    }
    set
    {
      selectionManager.setSelectedRange(newValue)
    }
  }

  public var length: Int
  {
    textStorage.length
  }

  public func substring(from range: NSRange) -> String?
  {
    textStorage.substring(from: range)
  }

  /// Applies the mutation to the text view.
  ///
  /// If the mutation is empty it will be ignored.
  ///
  /// - Parameter mutation: The mutation to apply.
  public func applyMutation(_ mutation: TextMutation)
  {
    guard !mutation.isEmpty else { return }

    layoutManager.beginTransaction()
    textStorage.beginEditing()

    layoutManager.willReplaceCharactersInRange(range: mutation.range, with: mutation.string)
    _undoManager?.registerMutation(mutation)
    textStorage.replaceCharacters(in: mutation.range, with: mutation.string)
    selectionManager.didReplaceCharacters(
      in: mutation.range,
      replacementLength: (mutation.string as NSString).length
    )

    textStorage.endEditing()
    layoutManager.endTransaction()
  }
}
