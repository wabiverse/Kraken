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

public extension String
{
  /// Calculates the first `n` lines and returns them as a new string.
  /// - Parameters:
  ///   - lines: The number of lines to return.
  ///   - maxLength: The maximum number of characters to copy.
  /// - Returns: A new string containing the lines.
  func getFirstLines(_ lines: Int = 1, maxLength: Int = 512) -> String
  {
    var string = ""
    var foundLines = 0
    var totalLength = 0
    for char in lazy
    {
      if char.isNewline
      {
        foundLines += 1
      }
      totalLength += 1
      if foundLines >= lines || totalLength >= maxLength
      {
        break
      }
      string.append(char)
    }
    return string
  }

  /// Calculates the last `n` lines and returns them as a new string.
  /// - Parameters:
  ///   - lines: The number of lines to return.
  ///   - maxLength: The maximum number of characters to copy.
  /// - Returns: A new string containing the lines.
  func getLastLines(_ lines: Int = 1, maxLength: Int = 512) -> String
  {
    var string = ""
    var foundLines = 0
    var totalLength = 0
    for char in lazy.reversed()
    {
      if char.isNewline
      {
        foundLines += 1
      }
      totalLength += 1
      if foundLines >= lines || totalLength >= maxLength
      {
        break
      }
      string = String(char) + string
    }
    return string
  }
}
