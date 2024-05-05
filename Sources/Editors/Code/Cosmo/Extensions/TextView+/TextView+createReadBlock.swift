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
import SwiftTreeSitter

extension CodeView
{
  func createReadBlock() -> Parser.ReadBlock
  {
    { [weak self] byteOffset, _ in
      let limit = self?.documentRange.length ?? 0
      let location = byteOffset / 2
      let end = min(location + 1024, limit)
      if location > end || self == nil
      {
        // Ignore and return nothing, tree-sitter's internal tree can be incorrect in some situations.
        return nil
      }
      let range = NSRange(location ..< end)
      return self?.stringForRange(range)?.data(using: String.nativeUTF16Encoding)
    }
  }

  func createReadCallback() -> SwiftTreeSitter.Predicate.TextProvider
  {
    { [weak self] range, _ in
      self?.stringForRange(range)
    }
  }
}
