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

import SwiftTreeSitter

#if DEBUG
  extension Tree
  {
    func prettyPrint()
    {
      guard let cursor = rootNode?.treeCursor
      else
      {
        print("NO ROOT NODE")
        return
      }
      guard cursor.currentNode != nil
      else
      {
        print("NO CURRENT NODE")
        return
      }

      func p(_ cursor: TreeCursor, depth: Int)
      {
        guard let node = cursor.currentNode
        else
        {
          return
        }

        let visible = node.isNamed

        if visible
        {
          print(String(repeating: " ", count: depth * 2), terminator: "")
          if let fieldName = cursor.currentFieldName
          {
            print(fieldName, ": ", separator: "", terminator: "")
          }
          print("(", node.nodeType ?? "NONE", " ", node.range, " ", separator: "", terminator: "")
        }

        if cursor.goToFirstChild()
        {
          while true
          {
            if cursor.currentNode != nil, cursor.currentNode!.isNamed
            {
              print("")
            }

            p(cursor, depth: depth + 1)

            if !cursor.gotoNextSibling()
            {
              break
            }
          }

          if !cursor.gotoParent()
          {
            fatalError("Could not go to parent, this tree may be invalid.")
          }
        }

        if visible
        {
          print(")", terminator: "")
        }
      }

      if cursor.currentNode?.childCount == 0
      {
        if !cursor.currentNode!.isNamed
        {
          print("{\(cursor.currentNode!.nodeType ?? "NONE")}")
        }
        else
        {
          print("\"\(cursor.currentNode!.nodeType ?? "NONE")\"")
        }
      }
      else
      {
        p(cursor, depth: 1)
      }
    }
  }

  extension MutableTree
  {
    func prettyPrint()
    {
      guard let cursor = rootNode?.treeCursor
      else
      {
        print("NO ROOT NODE")
        return
      }
      guard cursor.currentNode != nil
      else
      {
        print("NO CURRENT NODE")
        return
      }

      func p(_ cursor: TreeCursor, depth: Int)
      {
        guard let node = cursor.currentNode
        else
        {
          return
        }

        let visible = node.isNamed

        if visible
        {
          print(String(repeating: " ", count: depth * 2), terminator: "")
          if let fieldName = cursor.currentFieldName
          {
            print(fieldName, ": ", separator: "", terminator: "")
          }
          print("(", node.nodeType ?? "NONE", " ", node.range, " ", separator: "", terminator: "")
        }

        if cursor.goToFirstChild()
        {
          while true
          {
            if cursor.currentNode != nil, cursor.currentNode!.isNamed
            {
              print("")
            }

            p(cursor, depth: depth + 1)

            if !cursor.gotoNextSibling()
            {
              break
            }
          }

          if !cursor.gotoParent()
          {
            fatalError("Could not go to parent, this tree may be invalid.")
          }
        }

        if visible
        {
          print(")", terminator: "")
        }
      }

      if cursor.currentNode?.childCount == 0
      {
        if !cursor.currentNode!.isNamed
        {
          print("{\(cursor.currentNode!.nodeType ?? "NONE")}")
        }
        else
        {
          print("\"\(cursor.currentNode!.nodeType ?? "NONE")\"")
        }
      }
      else
      {
        p(cursor, depth: 1)
      }
    }
  }
#endif
