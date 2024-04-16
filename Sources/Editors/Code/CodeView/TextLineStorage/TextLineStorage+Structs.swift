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

extension TextLineStorage where Data: Identifiable
{
  public struct TextLinePosition
  {
    init(data: Data, range: NSRange, yPos: CGFloat, height: CGFloat, index: Int)
    {
      self.data = data
      self.range = range
      self.yPos = yPos
      self.height = height
      self.index = index
    }

    init(position: NodePosition)
    {
      data = position.node.data
      range = NSRange(location: position.textPos, length: position.node.length)
      yPos = position.yPos
      height = position.node.height
      index = position.index
    }

    /// The data stored at the position
    public let data: Data
    /// The range represented by the data
    public let range: NSRange
    /// The y position of the data, on a top down y axis
    public let yPos: CGFloat
    /// The height of the stored data
    public let height: CGFloat
    /// The index of the position.
    public let index: Int
  }

  struct NodePosition
  {
    /// The node storing information and the data stored at the position.
    let node: Node<Data>
    /// The y position of the data, on a top down y axis
    let yPos: CGFloat
    /// The location of the node in the document
    let textPos: Int
    /// The index of the node in the document.
    let index: Int
  }

  struct NodeSubtreeMetadata
  {
    let height: CGFloat
    let offset: Int
    let count: Int

    static var zero: NodeSubtreeMetadata
    {
      NodeSubtreeMetadata(height: 0, offset: 0, count: 0)
    }

    static func + (lhs: NodeSubtreeMetadata, rhs: NodeSubtreeMetadata) -> NodeSubtreeMetadata
    {
      NodeSubtreeMetadata(
        height: lhs.height + rhs.height,
        offset: lhs.offset + rhs.offset,
        count: lhs.count + rhs.count
      )
    }
  }

  public struct BuildItem
  {
    public let data: Data
    public let length: Int
    public let height: CGFloat?
  }
}
