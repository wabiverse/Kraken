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
import PixarUSD

public extension String
{
  /// Join a given scalar type by returning a new
  /// single string whose elements are separated
  /// by the content of the current string.
  ///
  /// This example joins each of the elements of
  /// a ``Pixar.GfVec3i`` as a new string, with
  /// each element separated by the given `"."`
  /// string.
  /// ```swift
  /// let version = Pixar.GfVec3i(1, 0, 0)
  ///
  /// print(".".join(array: version))
  /// // Prints "1.0.0"
  /// ```
  func join(array: some Scalar) -> String
  {
    var str = ""
    for (index, item) in array.enumerated()
    {
      str += "\(item)"
      if index < array.getArray().count - 1
      {
        str += self
      }
    }
    return str
  }
}
