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

public struct HorizontalEdgeInsets: Codable, Sendable, Equatable
{
  public var left: CGFloat
  public var right: CGFloat

  public var horizontal: CGFloat
  {
    left + right
  }

  public init(left: CGFloat, right: CGFloat)
  {
    self.left = left
    self.right = right
  }

  public init(from decoder: Decoder) throws
  {
    let container = try decoder.container(keyedBy: CodingKeys.self)
    left = try container.decode(CGFloat.self, forKey: .left)
    right = try container.decode(CGFloat.self, forKey: .right)
  }

  public static let zero: HorizontalEdgeInsets = .init(left: 0, right: 0)
}
