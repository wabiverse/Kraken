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
import CodeView
import TextFormation
import TextStory

extension TextViewController
{
  enum BracketPairs
  {
    static let allValues: [(String, String)] = [
      ("{", "}"),
      ("[", "]"),
      ("(", ")"),
      ("\"", "\""),
      ("'", "'")
    ]

    static let highlightValues: [(String, String)] = [
      ("{", "}"),
      ("[", "]"),
      ("(", ")")
    ]
  }

  // MARK: - Filter Configuration

  /// Initializes any filters for text editing.
  func setUpTextFormation()
  {
    textFilters = []

    // Filters

    setUpOpenPairFilters(pairs: BracketPairs.allValues)
    setUpNewlineTabFilters(indentOption: indentOption)
    setUpDeletePairFilters(pairs: BracketPairs.allValues)
    setUpDeleteWhitespaceFilter(indentOption: indentOption)
  }

  /// Returns a `TextualIndenter` based on available language configuration.
  private func getTextIndenter() -> TextualIndenter
  {
    switch language.id
    {
      case .python:
        TextualIndenter(patterns: TextualIndenter.pythonPatterns)
      default:
        TextualIndenter(patterns: TextualIndenter.basicPatterns)
    }
  }

  /// Configures pair filters and adds them to the `textFilters` array.
  /// - Parameters:
  ///   - pairs: The pairs to configure. Eg: `{` and `}`
  ///   - whitespaceProvider: The whitespace providers to use.
  private func setUpOpenPairFilters(pairs: [(String, String)])
  {
    for pair in pairs
    {
      let filter = StandardOpenPairFilter(open: pair.0, close: pair.1)
      textFilters.append(filter)
    }
  }

  /// Configures newline and tab replacement filters.
  /// - Parameters:
  ///   - whitespaceProvider: The whitespace providers to use.
  ///   - indentationUnit: The unit of indentation to use.
  private func setUpNewlineTabFilters(indentOption: IndentOption)
  {
    let newlineFilter: Filter = NewlineProcessingFilter()
    let tabReplacementFilter: Filter = TabReplacementFilter(indentOption: indentOption)

    textFilters.append(contentsOf: [newlineFilter, tabReplacementFilter])
  }

  /// Configures delete pair filters.
  private func setUpDeletePairFilters(pairs: [(String, String)])
  {
    for pair in pairs
    {
      let filter = DeleteCloseFilter(open: pair.0, close: pair.1)
      textFilters.append(filter)
    }
  }

  /// Configures up the delete whitespace filter.
  private func setUpDeleteWhitespaceFilter(indentOption: IndentOption)
  {
    let filter = DeleteWhitespaceFilter(indentOption: indentOption)
    textFilters.append(filter)
  }

  /// Determines whether or not a text mutation should be applied.
  /// - Parameters:
  ///   - mutation: The text mutation.
  ///   - textView: The textView to use.
  /// - Returns: Return whether or not the mutation should be applied.
  func shouldApplyMutation(_ mutation: TextMutation, to textView: CodeView) -> Bool
  {
    // don't perform any kind of filtering during undo operations
    if textView.undoManager?.isUndoing ?? false || textView.undoManager?.isRedoing ?? false
    {
      return true
    }

    let indentationUnit = indentOption.stringValue
    let indenter: TextualIndenter = getTextIndenter()
    let whitespaceProvider = WhitespaceProviders(
      leadingWhitespace: indenter.substitionProvider(indentationUnit: indentationUnit,
                                                     width: tabWidth),
      trailingWhitespace: { _, _ in "" }
    )

    for filter in textFilters
    {
      let action = filter.processMutation(mutation, in: textView, with: whitespaceProvider)

      switch action
      {
        case .none:
          break
        case .stop:
          return true
        case .discard:
          return false
      }
    }

    return true
  }
}
