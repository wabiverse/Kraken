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

public extension TextLayoutManager
{
  func visibleLines() -> Iterator
  {
    let visibleRect = delegate?.visibleRect ?? NSRect(
      x: 0,
      y: 0,
      width: 0,
      height: estimatedHeight()
    )
    return Iterator(minY: max(visibleRect.minY, 0), maxY: max(visibleRect.maxY, 0), storage: lineStorage)
  }

  struct Iterator: LazySequenceProtocol, IteratorProtocol
  {
    private var storageIterator: TextLineStorage<TextLine>.TextLineStorageYIterator

    init(minY: CGFloat, maxY: CGFloat, storage: TextLineStorage<TextLine>)
    {
      storageIterator = storage.linesStartingAt(minY, until: maxY)
    }

    public mutating func next() -> TextLineStorage<TextLine>.TextLinePosition?
    {
      storageIterator.next()
    }
  }
}
