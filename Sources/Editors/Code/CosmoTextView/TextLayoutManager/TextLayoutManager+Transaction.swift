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

public extension TextLayoutManager
{
  /// Begins a transaction, preventing the layout manager from performing layout until the `endTransaction` is called.
  /// Useful for grouping attribute modifications into one layout pass rather than laying out every update.
  ///
  /// You can nest transaction start/end calls, the layout manager will not cause layout until the last transaction
  /// group is ended.
  ///
  /// Ensure there is a balanced number of begin/end calls. If there is a missing endTranscaction call, the layout
  /// manager will never lay out text. If there is a end call without matching a start call an assertionFailure
  /// will occur.
  func beginTransaction()
  {
    transactionCounter += 1
  }

  /// Ends a transaction. When called, the layout manager will layout any necessary lines.
  func endTransaction(forceLayout: Bool = false)
  {
    transactionCounter -= 1
    if transactionCounter == 0
    {
      if forceLayout
      {
        setNeedsLayout()
      }
      layoutLines()
    }
    else if transactionCounter < 0
    {
      assertionFailure(
        "TextLayoutManager.endTransaction called without a matching TextLayoutManager.beginTransaction call"
      )
    }
  }
}
