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
  /// Applies the given edit to the current state and calls the editState's completion handler.
  /// - Parameter edit: The edit to apply to the internal tree sitter state.
  /// - Returns: The set of ranges invalidated by the edit operation.
  func applyEdit(edit: InputEdit) -> IndexSet
  {
    guard let state, let readBlock, let readCallback else { return IndexSet() }

    var invalidatedRanges = IndexSet()
    var touchedLayers = Set<LanguageLayer>()

    // Loop through all layers, apply edits & find changed byte ranges.
    for (idx, layer) in state.layers.enumerated().reversed()
    {
      if layer.id != state.primaryLayer.id
      {
        // Reversed for safe removal while looping
        for rangeIdx in (0 ..< layer.ranges.count).reversed()
        {
          layer.ranges[rangeIdx].applyInputEdit(edit)

          if layer.ranges[rangeIdx].length <= 0
          {
            layer.ranges.remove(at: rangeIdx)
          }
        }
        if layer.ranges.isEmpty
        {
          state.removeLanguageLayer(at: idx)
          continue
        }

        touchedLayers.insert(layer)
      }

      layer.parser.includedRanges = layer.ranges.map(\.tsRange)
      do
      {
        try invalidatedRanges.insert(
          ranges: layer.findChangedByteRanges(
            edit: edit,
            timeout: Constants.parserTimeout,
            readBlock: readBlock
          )
        )
      }
      catch
      {
        Self.logger.error("Error applying edit to state: \(error.localizedDescription, privacy: .public)")
        return IndexSet()
      }
    }

    // Update the state object for any new injections that may have been caused by this edit.
    invalidatedRanges.formUnion(
      state.updateInjectedLayers(readCallback: readCallback, readBlock: readBlock, touchedLayers: touchedLayers)
    )

    return invalidatedRanges
  }
}
