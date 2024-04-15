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
import TextFormation
import TextStory

/// Filter for quickly deleting indent whitespace
///
/// Will only delete whitespace when it's on the leading side of the line. Will delete back to the nearest tab column.
/// Eg:
/// ```text
/// (| = column delimiter, _ = space, * = cursor)
///
/// ____|___*   <- delete
/// ----*       <- final
/// ```
/// Will also move the cursor to the trailing side of the whitespace if it is not there already:
/// ```text
/// ____|_*___|__   <- delete
/// ____|____*      <- final
/// ```
struct DeleteWhitespaceFilter: Filter
{
  let indentOption: IndentOption

  func processMutation(
    _ mutation: TextMutation,
    in interface: TextInterface,
    with _: WhitespaceProviders
  ) -> FilterAction
  {
    guard mutation.delta < 0,
          mutation.string == "",
          mutation.range.length == 1,
          indentOption != .tab
    else
    {
      return .none
    }

    let lineRange = interface.lineRange(containing: mutation.range.location)
    guard let leadingWhitespace = interface.leadingRange(in: lineRange, within: .whitespacesWithoutNewlines),
          leadingWhitespace.contains(mutation.range.location)
    else
    {
      return .none
    }

    // Move to right of the whitespace and delete to the left-most tab column
    let indentLength = indentOption.stringValue.count
    var numberOfExtraSpaces = leadingWhitespace.length % indentLength
    if numberOfExtraSpaces == 0
    {
      numberOfExtraSpaces = indentLength
    }

    interface.applyMutation(
      TextMutation(
        delete: NSRange(location: leadingWhitespace.max - numberOfExtraSpaces, length: numberOfExtraSpaces),
        limit: mutation.limit
      )
    )

    return .discard
  }
}
