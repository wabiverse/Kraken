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
import Foundation

/// Represents a displayable line of text.
public final class TextLine: Identifiable, Equatable
{
  public let id: UUID = .init()
  private var needsLayout: Bool = true
  var maxWidth: CGFloat?
  private(set) var typesetter: Typesetter = .init()

  /// The line fragments contained by this text line.
  public var lineFragments: TextLineStorage<LineFragment>
  {
    typesetter.lineFragments
  }

  /// Marks this line as needing layout and clears all typesetting data.
  public func setNeedsLayout()
  {
    needsLayout = true
    typesetter = Typesetter()
  }

  /// Determines if the line needs to be laid out again.
  /// - Parameter maxWidth: The new max width to check.
  /// - Returns: True, if this line has been marked as needing layout using ``TextLine/setNeedsLayout()`` or if the
  ///            line needs to find new line breaks due to a new constraining width.
  func needsLayout(maxWidth: CGFloat) -> Bool
  {
    needsLayout || maxWidth != self.maxWidth
  }

  /// Prepares the line for display, generating all potential line breaks and calculating the real height of the line.
  /// - Parameters:
  ///   - displayData: Information required to display a text line.
  ///   - range: The range this text range represents in the entire document.
  ///   - stringRef: A reference to the string storage for the document.
  ///   - markedRanges: Any marked ranges in the line.
  ///   - breakStrategy: Determines how line breaks are calculated.
  func prepareForDisplay(
    displayData: DisplayData,
    range: NSRange,
    stringRef: NSTextStorage,
    markedRanges: MarkedTextManager.MarkedRanges?,
    breakStrategy: LineBreakStrategy
  )
  {
    let string = stringRef.attributedSubstring(from: range)
    maxWidth = displayData.maxWidth
    typesetter.typeset(
      string,
      displayData: displayData,
      breakStrategy: breakStrategy,
      markedRanges: markedRanges
    )
    needsLayout = false
  }

  public static func == (lhs: TextLine, rhs: TextLine) -> Bool
  {
    lhs.id == rhs.id
  }

  /// Contains all required data to perform a typeset and layout operation on a text line.
  struct DisplayData
  {
    let maxWidth: CGFloat
    let lineHeightMultiplier: CGFloat
    let estimatedLineHeight: CGFloat
  }
}
