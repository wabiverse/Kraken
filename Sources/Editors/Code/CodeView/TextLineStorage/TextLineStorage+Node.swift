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

extension TextLineStorage
{
  func isRightChild(_ node: Node<Data>) -> Bool
  {
    node.parent?.right === node
  }

  func isLeftChild(_ node: Node<Data>) -> Bool
  {
    node.parent?.left === node
  }

  /// Transplants a node with another node.
  ///
  /// ```
  ///         [a]
  ///     [u]_/ \_[b]
  /// [c]_/ \_[v]
  ///
  /// call: transplant(u, v)
  ///
  ///         [a]
  ///     [v]_/ \_[b]
  /// [c]_/
  ///
  /// ```
  /// - Note: Leaves the task of updating tree metadata to the caller.
  /// - Parameters:
  ///   - nodeU: The node to replace.
  ///   - nodeV: The node to insert in place of `nodeU`
  func transplant(_ nodeU: borrowing Node<Data>, with nodeV: Node<Data>?)
  {
    if nodeU.parent == nil
    {
      root = nodeV
    }
    else if isLeftChild(nodeU)
    {
      nodeU.parent?.left = nodeV
    }
    else
    {
      nodeU.parent?.right = nodeV
    }
    nodeV?.parent = nodeU.parent
  }

  enum Color
  {
    case red
    case black
  }

  final class Node<NodeData: Identifiable>
  {
    /// The length of the text line
    var length: Int
    /// The height of this text line
    var height: CGFloat
    var data: NodeData

    /// The offset in characters of the entire left subtree
    var leftSubtreeOffset: Int
    /// The sum of the height of the nodes in the left subtree
    var leftSubtreeHeight: CGFloat
    /// The number of nodes in the left subtree
    var leftSubtreeCount: Int

    var left: Node<NodeData>?
    var right: Node<NodeData>?
    unowned var parent: Node<NodeData>?
    var color: Color

    init(
      length: Int,
      data: NodeData,
      leftSubtreeOffset: Int,
      leftSubtreeHeight: CGFloat,
      leftSubtreeCount: Int,
      height: CGFloat,
      left: Node<NodeData>? = nil,
      right: Node<NodeData>? = nil,
      parent: Node<NodeData>? = nil,
      color: Color
    )
    {
      self.length = length
      self.data = data
      self.leftSubtreeOffset = leftSubtreeOffset
      self.leftSubtreeHeight = leftSubtreeHeight
      self.leftSubtreeCount = leftSubtreeCount
      self.height = height
      self.left = left
      self.right = right
      self.parent = parent
      self.color = color
    }

    convenience init(length: Int, data: NodeData, height: CGFloat)
    {
      self.init(
        length: length,
        data: data,
        leftSubtreeOffset: 0,
        leftSubtreeHeight: 0.0,
        leftSubtreeCount: 0,
        height: height,
        color: .black
      )
    }

    func sibling() -> Node<NodeData>?
    {
      if parent?.left === self
      {
        parent?.right
      }
      else
      {
        parent?.left
      }
    }

    func minimum() -> Node<NodeData>
    {
      if let left
      {
        left.minimum()
      }
      else
      {
        self
      }
    }

    func maximum() -> Node<NodeData>
    {
      if let right
      {
        right.maximum()
      }
      else
      {
        self
      }
    }

    func getSuccessor() -> Node<NodeData>?
    {
      // If node has right child: successor is the min of this right tree
      if let right
      {
        return right.minimum()
      }
      else
      {
        // Else go upward until node is a left child
        var currentNode = self
        var parent = currentNode.parent
        while currentNode.parent?.right === currentNode
        {
          if let parent
          {
            currentNode = parent
          }
          parent = currentNode.parent
        }
        return parent
      }
    }

    deinit
    {
      left = nil
      right = nil
      parent = nil
    }
  }
}
