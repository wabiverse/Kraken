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

/// A ``LineFragment`` represents a subrange of characters in a line. Every text line contains at least one line
/// fragments, and any lines that need to be broken due to width constraints will contain more than one fragment.
public final class LineFragment: Identifiable, Equatable
{
  public let id = UUID()
  public private(set) var ctLine: CTLine
  public let width: CGFloat
  public let height: CGFloat
  public let descent: CGFloat
  public let scaledHeight: CGFloat

  /// The difference between the real text height and the scaled height
  public var heightDifference: CGFloat
  {
    scaledHeight - height
  }

  init(
    ctLine: CTLine,
    width: CGFloat,
    height: CGFloat,
    descent: CGFloat,
    lineHeightMultiplier: CGFloat
  )
  {
    self.ctLine = ctLine
    self.width = width
    self.height = height
    self.descent = descent
    scaledHeight = height * lineHeightMultiplier
  }

  public static func == (lhs: LineFragment, rhs: LineFragment) -> Bool
  {
    lhs.id == rhs.id
  }
}
