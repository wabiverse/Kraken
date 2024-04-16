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

extension TreeSitterClient
{
  func queryHighlightsForRange(range: NSRange) -> [HighlightRange]
  {
    guard let state else { return [] }

    var highlights: [HighlightRange] = []
    var injectedSet = IndexSet(integersIn: range)

    for layer in state.layers where layer.id != state.primaryLayer.id
    {
      // Query injected only if a layer's ranges intersects with `range`
      for layerRange in layer.ranges
      {
        if let rangeIntersection = range.intersection(layerRange)
        {
          let queryResult = queryLayerHighlights(
            layer: layer,
            range: rangeIntersection
          )

          highlights.append(contentsOf: queryResult)
          injectedSet.remove(integersIn: rangeIntersection)
        }
      }
    }

    // Query primary for any ranges that weren't used in the injected layers.
    for range in injectedSet.rangeView
    {
      let queryResult = queryLayerHighlights(
        layer: state.layers[0],
        range: NSRange(range)
      )
      highlights.append(contentsOf: queryResult)
    }

    return highlights
  }

  /// Queries the given language layer for any highlights.
  /// - Parameters:
  ///   - layer: The layer to query.
  ///   - range: The range to query for.
  /// - Returns: Any ranges to highlight.
  func queryLayerHighlights(
    layer: LanguageLayer,
    range: NSRange
  ) -> [HighlightRange]
  {
    guard let tree = layer.tree,
          let rootNode = tree.rootNode
    else
    {
      return []
    }

    // This needs to be on the main thread since we're going to use the `textProvider` in
    // the `highlightsFromCursor` method, which uses the textView's text storage.
    guard let queryCursor = layer.languageQuery?.execute(node: rootNode, in: tree)
    else
    {
      return []
    }
    queryCursor.setRange(range)
    queryCursor.matchLimit = Constants.treeSitterMatchLimit

    var highlights: [HighlightRange] = []

    // if layer.id == .jsdoc
    // {
    //   highlights.append(HighlightRange(range: range, capture: .comment))
    // }

    highlights += highlightsFromCursor(cursor: queryCursor, includedRange: range)

    return highlights
  }

  /// Resolves a query cursor to the highlight ranges it contains.
  /// **Must be called on the main thread**
  /// - Parameters:
  ///     - cursor: The cursor to resolve.
  ///     - includedRange: The range to include highlights from.
  /// - Returns: Any highlight ranges contained in the cursor.
  func highlightsFromCursor(
    cursor: QueryCursor,
    includedRange: NSRange
  ) -> [HighlightRange]
  {
    cursor
      .flatMap(\.captures)
      .compactMap
      {
        // Sometimes `cursor.setRange` just doesn't work :( so we have to do a redundant check for a valid range
        // in the included range
        let intersectionRange = $0.range.intersection(includedRange) ?? .zero
        // Check that the capture name is one CESE can parse. If not, ignore it completely.
        if intersectionRange.length > 0, let captureName = CaptureName.fromString($0.name)
        {
          return HighlightRange(range: intersectionRange, capture: captureName)
        }
        return nil
      }
  }
}
