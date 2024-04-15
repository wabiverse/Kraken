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

/// A collection of possible capture names for `tree-sitter` with their respected raw values.
public enum CaptureName: String, CaseIterable, Sendable
{
  case include
  case constructor
  case keyword
  case boolean
  case `repeat`
  case conditional
  case tag
  case comment
  case variable
  case property
  case function
  case method
  case number
  case float
  case string
  case type
  case parameter
  case typeAlternate = "type_alternate"
  case variableBuiltin = "variable.builtin"
  case keywordReturn = "keyword.return"
  case keywordFunction = "keyword.function"

  /// Returns a specific capture name case from a given string.
  /// - Parameter string: A string to get the capture name from
  /// - Returns: A `CaptureNames` case
  static func fromString(_ string: String?) -> CaptureName?
  {
    CaptureName(rawValue: string ?? "")
  }

  var alternate: CaptureName
  {
    switch self
    {
      case .type:
        .typeAlternate
      default:
        self
    }
  }
}
