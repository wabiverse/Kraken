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

extension TextLineStorage where Data == TextLine
{
  /// Builds the line storage object from the given `NSTextStorage`.
  /// - Parameters:
  ///   - textStorage: The text storage object to use.
  ///   - estimatedLineHeight: The estimated height of each individual line.
  func buildFromTextStorage(_ textStorage: NSTextStorage, estimatedLineHeight: CGFloat)
  {
    var index = 0
    var lines: [BuildItem] = []
    while let range = textStorage.getNextLine(startingAt: index)
    {
      lines.append(BuildItem(data: TextLine(), length: range.max - index, height: estimatedLineHeight))
      index = NSMaxRange(range)
    }
    // Create the last line
    if textStorage.length - index > 0
    {
      lines.append(BuildItem(data: TextLine(), length: textStorage.length - index, height: estimatedLineHeight))
    }

    if textStorage.length == 0
      || LineEnding(rawValue: textStorage.mutableString.substring(from: textStorage.length - 1)) != nil
    {
      lines.append(BuildItem(data: TextLine(), length: 0, height: estimatedLineHeight))
    }

    // Use an efficient tree building algorithm rather than adding lines sequentially
    build(from: lines, estimatedLineHeight: estimatedLineHeight)
  }
}
