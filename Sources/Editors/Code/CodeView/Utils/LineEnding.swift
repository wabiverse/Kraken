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

public enum LineEnding: String, CaseIterable
{
  /// The default unix `\n` character
  case lineFeed = "\n"
  /// MacOS line ending `\r` character
  case carriageReturn = "\r"
  /// Windows line ending sequence `\r\n`
  case carriageReturnLineFeed = "\r\n"

  /// Initialize a line ending from a line string.
  /// - Parameter line: The line to use
  public init?(line: String)
  {
    guard let lastChar = line.last,
          let lineEnding = LineEnding(rawValue: String(lastChar)) else { return nil }
    self = lineEnding
  }

  /// Attempts to detect the line ending from a line storage.
  /// - Parameter lineStorage: The line storage to enumerate.
  /// - Returns: A line ending. Defaults to `.lf` if none could be found.
  public static func detectLineEnding(
    lineStorage: TextLineStorage<TextLine>,
    textStorage: NSTextStorage
  ) -> LineEnding
  {
    var histogram: [LineEnding: Int] = LineEnding.allCases.reduce(into: [LineEnding: Int]())
    {
      $0[$1] = 0
    }
    var shouldContinue = true
    var lineIterator = lineStorage.makeIterator()

    while let line = lineIterator.next(), shouldContinue
    {
      guard let lineString = textStorage.substring(from: line.range),
            let lineEnding = LineEnding(line: lineString)
      else
      {
        continue
      }
      histogram[lineEnding] = histogram[lineEnding]! + 1
      // after finding 15 lines of a line ending we assume it's correct.
      if histogram[lineEnding]! >= 15
      {
        shouldContinue = false
      }
    }

    let orderedValues = histogram.sorted(by: { $0.value > $1.value })
    // Return the max of the histogram, but if there's no max
    // we default to lineFeed. This should be a parameter in the future.
    if orderedValues.count >= 2
    {
      if orderedValues[0].value == orderedValues[1].value
      {
        return .lineFeed
      }
      else
      {
        return orderedValues[0].key
      }
    }
    else
    {
      return .lineFeed
    }
  }

  /// The UTF-16 Length of the line ending.
  public var length: Int
  {
    rawValue.utf16.count
  }
}
