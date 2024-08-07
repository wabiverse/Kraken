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

import CodeLanguages
import Foundation
import SwiftTreeSitter

public class LanguageLayer: Hashable
{
  /// Initialize a language layer
  /// - Parameters:
  ///   - id: The ID of the layer.
  ///   - parser: A parser to use for the layer.
  ///   - supportsInjections: Set to true when the langauge supports the `injections` query.
  ///   - tree: The tree-sitter tree generated while editing/parsing a document.
  ///   - languageQuery: The language query used for fetching the associated `queries.scm` file
  ///   - ranges: All ranges this layer acts on. Must be kept in order and w/o overlap.
  init(
    id: TreeSitterLanguage,
    parser: Parser,
    supportsInjections: Bool,
    tree: MutableTree? = nil,
    languageQuery: Query? = nil,
    ranges: [NSRange]
  )
  {
    self.id = id
    self.parser = parser
    self.supportsInjections = supportsInjections
    self.tree = tree
    self.languageQuery = languageQuery
    self.ranges = ranges

    self.parser.timeout = TreeSitterClient.Constants.parserTimeout
  }

  let id: TreeSitterLanguage
  let parser: Parser
  let supportsInjections: Bool
  var tree: MutableTree?
  var languageQuery: Query?
  var ranges: [NSRange]

  func copy() -> LanguageLayer
  {
    LanguageLayer(
      id: id,
      parser: parser,
      supportsInjections: supportsInjections,
      tree: tree?.mutableCopy(),
      languageQuery: languageQuery,
      ranges: ranges
    )
  }

  public static func == (lhs: LanguageLayer, rhs: LanguageLayer) -> Bool
  {
    lhs.id == rhs.id && lhs.ranges == rhs.ranges
  }

  public func hash(into hasher: inout Hasher)
  {
    hasher.combine(id)
    hasher.combine(ranges)
  }

  /// Calculates a series of ranges that have been invalidated by a given edit.
  /// - Parameters:
  ///   - edit: The edit to act on.
  ///   - timeout: The maximum time interval the parser can run before halting.
  ///   - readBlock: A callback for fetching blocks of text.
  /// - Returns: An array of distinct `NSRanges` that need to be re-highlighted.
  func findChangedByteRanges(
    edit: InputEdit,
    timeout: TimeInterval?,
    readBlock: @escaping Parser.ReadBlock
  ) throws -> [NSRange]
  {
    parser.timeout = timeout ?? 0

    let newTree = calculateNewState(
      tree: tree?.mutableCopy(),
      parser: parser,
      edit: edit,
      readBlock: readBlock
    )

    if tree == nil, newTree == nil
    {
      // There was no existing tree, make a new one and return all indexes.
      tree = parser.parse(tree: nil as Tree?, readBlock: readBlock)
      return [tree?.rootNode?.range ?? .zero]
    }
    else if tree != nil, newTree == nil
    {
      // The parser timed out,
      throw Error.parserTimeout
    }

    let ranges = changedByteRanges(tree, newTree).map(\.range)

    tree = newTree

    return ranges
  }

  /// Applies the edit to the current `tree` and returns the old tree and a copy of the current tree with the
  /// processed edit.
  /// - Parameters:
  ///   - tree: The tree before an edit used to parse the new tree.
  ///   - parser: The parser used to parse the new tree.
  ///   - edit: The edit to apply.
  ///   - readBlock: The block to use to read text.
  /// - Returns: (The old state, the new state).
  func calculateNewState(
    tree: MutableTree?,
    parser: Parser,
    edit: InputEdit,
    readBlock: @escaping Parser.ReadBlock
  ) -> MutableTree?
  {
    guard let tree
    else
    {
      return nil
    }

    // Apply the edit to the old tree
    tree.edit(edit)

    // Check every timeout to see if the task is canceled to avoid parsing after the editor has been closed.
    // We can continue a parse after a timeout causes it to cancel by calling parse on the same tree.
    var newTree: MutableTree?
    while newTree == nil, !Task.isCancelled
    {
      newTree = parser.parse(tree: tree, readBlock: readBlock)
    }

    return newTree
  }

  /// Calculates the changed byte ranges between two trees.
  /// - Parameters:
  ///   - lhs: The first (older) tree.
  ///   - rhs: The second (newer) tree.
  /// - Returns: Any changed ranges.
  func changedByteRanges(_ lhs: MutableTree?, _ rhs: MutableTree?) -> [Range<UInt32>]
  {
    switch (lhs, rhs)
    {
      case let (tree1?, tree2?):
        return tree1.changedRanges(from: tree2).map(\.bytes)
      case (nil, let tree2?):
        let range = tree2.rootNode?.byteRange

        return range.flatMap { [$0] } ?? []
      case (_, nil):
        return []
    }
  }

  enum Error: Swift.Error, LocalizedError
  {
    case parserTimeout
  }
}
