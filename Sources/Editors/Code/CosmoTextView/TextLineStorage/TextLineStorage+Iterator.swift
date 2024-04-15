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

public extension TextLineStorage
{
  func linesStartingAt(_ minY: CGFloat, until maxY: CGFloat) -> TextLineStorageYIterator
  {
    TextLineStorageYIterator(storage: self, minY: minY, maxY: maxY)
  }

  func linesInRange(_ range: NSRange) -> TextLineStorageRangeIterator
  {
    TextLineStorageRangeIterator(storage: self, range: range)
  }

  struct TextLineStorageYIterator: LazySequenceProtocol, IteratorProtocol
  {
    private let storage: TextLineStorage
    private let minY: CGFloat
    private let maxY: CGFloat
    private var currentPosition: TextLinePosition?

    init(storage: TextLineStorage, minY: CGFloat, maxY: CGFloat, currentPosition: TextLinePosition? = nil)
    {
      self.storage = storage
      self.minY = minY
      self.maxY = maxY
      self.currentPosition = currentPosition
    }

    public mutating func next() -> TextLinePosition?
    {
      if let currentPosition
      {
        guard currentPosition.yPos < maxY,
              let nextPosition = storage.getLine(atOffset: currentPosition.range.max),
              nextPosition.index != currentPosition.index
        else
        {
          return nil
        }
        self.currentPosition = nextPosition
        return self.currentPosition!
      }
      else if let nextPosition = storage.getLine(atPosition: minY)
      {
        currentPosition = nextPosition
        return nextPosition
      }
      else
      {
        return nil
      }
    }
  }

  struct TextLineStorageRangeIterator: LazySequenceProtocol, IteratorProtocol
  {
    private let storage: TextLineStorage
    private let range: NSRange
    private var currentPosition: TextLinePosition?

    init(storage: TextLineStorage, range: NSRange, currentPosition: TextLinePosition? = nil)
    {
      self.storage = storage
      self.range = range
      self.currentPosition = currentPosition
    }

    public mutating func next() -> TextLinePosition?
    {
      if let currentPosition
      {
        guard currentPosition.range.max < range.max,
              let nextPosition = storage.getLine(atOffset: currentPosition.range.max)
        else
        {
          return nil
        }
        self.currentPosition = nextPosition
        return self.currentPosition!
      }
      else if let nextPosition = storage.getLine(atOffset: range.location)
      {
        currentPosition = nextPosition
        return nextPosition
      }
      else
      {
        return nil
      }
    }
  }
}

extension TextLineStorage: LazySequenceProtocol
{
  public func makeIterator() -> TextLineStorageIterator
  {
    TextLineStorageIterator(storage: self, currentPosition: nil)
  }

  public struct TextLineStorageIterator: IteratorProtocol
  {
    private let storage: TextLineStorage
    private var currentPosition: TextLinePosition?

    init(storage: TextLineStorage, currentPosition: TextLinePosition? = nil)
    {
      self.storage = storage
      self.currentPosition = currentPosition
    }

    public mutating func next() -> TextLinePosition?
    {
      if let currentPosition
      {
        guard currentPosition.range.max < storage.length,
              let nextPosition = storage.getLine(atOffset: currentPosition.range.max)
        else
        {
          return nil
        }
        self.currentPosition = nextPosition
        return self.currentPosition!
      }
      else if let nextPosition = storage.getLine(atOffset: 0)
      {
        currentPosition = nextPosition
        return nextPosition
      }
      else
      {
        return nil
      }
    }
  }
}
