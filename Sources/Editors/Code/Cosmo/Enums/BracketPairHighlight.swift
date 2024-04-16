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

/// An enum representing the type of highlight to use for bracket pairs.
public enum BracketPairHighlight: Equatable
{
  /// Highlight both the opening and closing character in a pair with a bounding box.
  /// The boxes will stay on screen until the cursor moves away from the bracket pair.
  case bordered(color: NSColor)
  /// Flash a yellow highlight box on only the opposite character in the pair.
  /// This is closely matched to Xcode's flash highlight for bracket pairs, and animates in and out over the course
  /// of `0.75` seconds.
  case flash
  /// Highlight both the opening and closing character in a pair with an underline.
  /// The underline will stay on screen until the cursor moves away from the bracket pair.
  case underline(color: NSColor)

  public static func == (lhs: BracketPairHighlight, rhs: BracketPairHighlight) -> Bool
  {
    switch (lhs, rhs)
    {
      case (.flash, .flash):
        true
      case let (.bordered(lhsColor), .bordered(rhsColor)):
        lhsColor == rhsColor
      case let (.underline(lhsColor), .underline(rhsColor)):
        lhsColor == rhsColor
      default:
        false
    }
  }

  /// Returns `true` if the highlight should act on both the opening and closing bracket.
  var highlightsSourceBracket: Bool
  {
    switch self
    {
      case .bordered, .underline:
        true
      case .flash:
        false
    }
  }
}
