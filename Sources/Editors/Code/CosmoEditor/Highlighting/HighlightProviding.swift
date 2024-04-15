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
import CosmoLanguages
import CosmoTextView
import Foundation

/// The protocol a class must conform to to be used for highlighting.
public protocol HighlightProviding: AnyObject
{
  /// Called once to set up the highlight provider with a data source and language.
  /// - Parameters:
  ///   - textView: The text view to use as a text source.
  ///   - codeLanguage: The language that should be used by the highlighter.
  func setUp(textView: TextView, codeLanguage: CodeLanguage)

  /// Notifies the highlighter that an edit is going to happen in the given range.
  /// - Parameters:
  ///   - textView: The text view to use.
  ///   - range: The range of the incoming edit.
  func willApplyEdit(textView: TextView, range: NSRange)

  /// Notifies the highlighter of an edit and in exchange gets a set of indices that need to be re-highlighted.
  /// The returned `IndexSet` should include all indexes that need to be highlighted, including any inserted text.
  /// - Parameters:
  ///   - textView: The text view to use.
  ///   - range: The range of the edit.
  ///   - delta: The length of the edit, can be negative for deletions.
  /// - Returns: an `IndexSet` containing all Indices to invalidate.
  func applyEdit(textView: TextView, range: NSRange, delta: Int, completion: @escaping (IndexSet) -> Void)

  /// Queries the highlight provider for any ranges to apply highlights to. The highlight provider should return an
  /// array containing all ranges to highlight, and the capture type for the range. Any ranges or indexes
  /// excluded from the returned array will be treated as plain text and highlighted as such.
  /// - Parameters:
  ///   - textView: The text view to use.
  ///   - range: The range to query.
  /// - Returns: All highlight ranges for the queried ranges.
  func queryHighlightsFor(textView: TextView, range: NSRange, completion: @escaping ([HighlightRange]) -> Void)
}

public extension HighlightProviding
{
  func willApplyEdit(textView _: TextView, range _: NSRange) {}
}
