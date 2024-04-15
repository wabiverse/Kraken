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

/// Filter for replacing tab characters with the user-defined indentation unit.
/// - Note: The undentation unit can be another tab character, this is merely a point at which this can be configured.
struct TabReplacementFilter: Filter
{
  let indentOption: IndentOption

  func processMutation(
    _ mutation: TextMutation,
    in interface: TextInterface,
    with _: WhitespaceProviders
  ) -> FilterAction
  {
    if mutation.string == "\t", indentOption != .tab, mutation.delta > 0
    {
      interface.applyMutation(
        TextMutation(
          insert: indentOption.stringValue,
          at: mutation.range.location,
          limit: mutation.limit
        )
      )
      return .discard
    }
    else
    {
      return .none
    }
  }
}
