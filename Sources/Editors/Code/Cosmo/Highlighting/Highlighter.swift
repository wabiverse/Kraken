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
import CodeLanguages
import CodeView
import Foundation
import SwiftTreeSitter

/// The `Highlighter` class handles efficiently highlighting the `CodeView` it's provided with.
/// It will listen for text and visibility changes, and highlight syntax as needed.
///
/// One should rarely have to direcly modify or call methods on this class. Just keep it alive in
/// memory and it will listen for bounds changes, text changes, etc. However, to completely invalidate all
/// highlights use the ``invalidate()`` method to re-highlight all (visible) text, and the ``setLanguage``
/// method to update the highlighter with a new language if needed.
@MainActor
class Highlighter: NSObject
{
  // MARK: - Index Sets

  /// Any indexes that highlights have been requested for, but haven't been applied.
  /// Indexes/ranges are added to this when highlights are requested and removed
  /// after they are applied
  private var pendingSet: IndexSet = .init()

  /// The set of valid indexes
  private var validSet: IndexSet = .init()

  /// The set of visible indexes in tht text view
  private lazy var visibleSet: IndexSet = .init(integersIn: textView?.visibleTextRange ?? NSRange())

  // MARK: - UI

  /// The text view to highlight
  private weak var textView: CodeView?

  /// The editor theme
  private var theme: Editor.Code.Theme

  /// The object providing attributes for captures.
  private weak var attributeProvider: ThemeAttributesProviding!

  /// The current language of the editor.
  private var language: Editor.Code.Language

  /// Calculates invalidated ranges given an edit.
  private(set) weak var highlightProvider: HighlightProviding?

  /// The length to chunk ranges into when passing to the highlighter.
  private let rangeChunkLimit = 1024

  // MARK: - Init

  /// Initializes the `Highlighter`
  /// - Parameters:
  ///   - textView: The text view to highlight.
  ///   - treeSitterClient: The tree-sitter client to handle tree updates and highlight queries.
  ///   - theme: The theme to use for highlights.
  init(
    textView: CodeView,
    highlightProvider: HighlightProviding?,
    theme: Editor.Code.Theme,
    attributeProvider: ThemeAttributesProviding,
    language: Editor.Code.Language
  )
  {
    self.textView = textView
    self.highlightProvider = highlightProvider
    self.theme = theme
    self.attributeProvider = attributeProvider
    self.language = language

    super.init()

    highlightProvider?.setUp(textView: textView, codeLanguage: language)

    if let scrollView = textView.enclosingScrollView
    {
      NotificationCenter.default.addObserver(
        self,
        selector: #selector(visibleTextChanged(_:)),
        name: NSView.frameDidChangeNotification,
        object: scrollView
      )

      NotificationCenter.default.addObserver(
        self,
        selector: #selector(visibleTextChanged(_:)),
        name: NSView.boundsDidChangeNotification,
        object: scrollView.contentView
      )
    }
  }

  // MARK: - Public

  /// Invalidates all text in the textview. Useful for updating themes.
  public func invalidate()
  {
    guard let textView else { return }
    updateVisibleSet(textView: textView)
    invalidate(range: textView.documentRange)
  }

  /// Sets the language and causes a re-highlight of the entire text.
  /// - Parameter language: The language to update to.
  public func setLanguage(language: Editor.Code.Language)
  {
    guard let textView else { return }
    // Remove all current highlights. Makes the language setting feel snappier and tells the user we're doing
    // something immediately.
    textView.textStorage.setAttributes(
      attributeProvider.attributesFor(nil),
      range: NSRange(location: 0, length: textView.textStorage.length)
    )
    textView.layoutManager.invalidateLayoutForRect(textView.visibleRect)
    validSet.removeAll()
    pendingSet.removeAll()
    highlightProvider?.setUp(textView: textView, codeLanguage: language)
    invalidate()
  }

  /// Sets the highlight provider. Will cause a re-highlight of the entire text.
  /// - Parameter provider: The provider to use for future syntax highlights.
  public func setHighlightProvider(_ provider: HighlightProviding)
  {
    highlightProvider = provider
    guard let textView else { return }
    highlightProvider?.setUp(textView: textView, codeLanguage: language)
    invalidate()
  }

  deinit
  {
    self.attributeProvider = nil
    self.textView = nil
    self.highlightProvider = nil
  }
}

// MARK: - Highlighting

private extension Highlighter
{
  /// Invalidates a given range and adds it to the queue to be highlighted.
  /// - Parameter range: The range to invalidate.
  func invalidate(range: NSRange)
  {
    let set = IndexSet(integersIn: range)

    if set.isEmpty
    {
      return
    }

    validSet.subtract(set)

    highlightInvalidRanges()
  }

  /// Begins highlighting any invalid ranges
  func highlightInvalidRanges()
  {
    // If there aren't any more ranges to highlight, don't do anything, otherwise continue highlighting
    // any available ranges.
    var rangesToQuery: [NSRange] = []
    while let range = getNextRange()
    {
      rangesToQuery.append(range)
      pendingSet.insert(range: range)
    }

    queryHighlights(for: rangesToQuery)
  }

  /// Highlights the given ranges
  /// - Parameter ranges: The ranges to request highlights for.
  func queryHighlights(for rangesToHighlight: [NSRange])
  {
    guard let textView else { return }

    if !Thread.isMainThread
    {
      DispatchQueue.main.async
      { [weak self] in
        for range in rangesToHighlight
        {
          self?.highlightProvider?.queryHighlightsFor(
            textView: textView,
            range: range
          )
          { [weak self] highlights in
            self?.applyHighlightResult(highlights, rangeToHighlight: range)
          }
        }
      }
    }
    else
    {
      for range in rangesToHighlight
      {
        highlightProvider?.queryHighlightsFor(textView: textView, range: range)
        { [weak self] highlights in
          self?.applyHighlightResult(highlights, rangeToHighlight: range)
        }
      }
    }
  }

  /// Applies a highlight query result to the text view.
  /// - Parameters:
  ///   - results: The result of a highlight query.
  ///   - rangeToHighlight: The range to apply the highlight to.
  private func applyHighlightResult(_ results: [HighlightRange], rangeToHighlight: NSRange)
  {
    guard let attributeProvider
    else
    {
      return
    }

    pendingSet.remove(integersIn: rangeToHighlight)
    guard visibleSet.intersects(integersIn: rangeToHighlight)
    else
    {
      return
    }
    validSet.formUnion(IndexSet(integersIn: rangeToHighlight))

    // Loop through each highlight and modify the textStorage accordingly.
    textView?.layoutManager.beginTransaction()
    textView?.textStorage.beginEditing()

    // Create a set of indexes that were not highlighted.
    var ignoredIndexes = IndexSet(integersIn: rangeToHighlight)

    // Apply all highlights that need color
    for highlight in results
      where textView?.documentRange.upperBound ?? 0 > highlight.range.upperBound
    {
      textView?.textStorage.setAttributes(
        attributeProvider.attributesFor(highlight.capture),
        range: highlight.range
      )

      // Remove highlighted indexes from the "ignored" indexes.
      ignoredIndexes.remove(integersIn: highlight.range)
    }

    // For any indices left over, we need to apply normal attributes to them
    // This fixes the case where characters are changed to have a non-text color, and then are skipped when
    // they need to be changed back.
    for ignoredRange in ignoredIndexes.rangeView
      where textView?.documentRange.upperBound ?? 0 > ignoredRange.upperBound
    {
      textView?.textStorage.setAttributes(
        attributeProvider.attributesFor(nil),
        range: NSRange(ignoredRange)
      )
    }

    textView?.textStorage.endEditing()
    textView?.layoutManager.endTransaction()
  }

  /// Gets the next `NSRange` to highlight based on the invalid set, visible set, and pending set.
  /// - Returns: An `NSRange` to highlight if it could be fetched.
  func getNextRange() -> NSRange?
  {
    let set = IndexSet(integersIn: textView?.documentRange ?? .zero) // All text
      .subtracting(validSet) // Subtract valid = Invalid set
      .intersection(visibleSet) // Only visible indexes
      .subtracting(pendingSet) // Don't include pending indexes

    guard let range = set.rangeView.first
    else
    {
      return nil
    }

    // Chunk the ranges in sets of rangeChunkLimit characters.
    return NSRange(
      location: range.lowerBound,
      length: min(rangeChunkLimit, range.upperBound - range.lowerBound)
    )
  }
}

// MARK: - Visible Content Updates

private extension Highlighter
{
  private func updateVisibleSet(textView: CodeView)
  {
    if let newVisibleRange = textView.visibleTextRange
    {
      visibleSet = IndexSet(integersIn: newVisibleRange)
    }
  }

  /// Updates the view to highlight newly visible text when the textview is scrolled or bounds change.
  @objc func visibleTextChanged(_ notification: Notification)
  {
    let textView: CodeView
    if let clipView = notification.object as? NSClipView,
       let documentView = clipView.enclosingScrollView?.documentView as? CodeView
    {
      textView = documentView
    }
    else if let scrollView = notification.object as? NSScrollView,
            let documentView = scrollView.documentView as? CodeView
    {
      textView = documentView
    }
    else
    {
      return
    }

    updateVisibleSet(textView: textView)

    // Any indices that are both *not* valid and in the visible text range should be invalidated
    let newlyInvalidSet = visibleSet.subtracting(validSet)

    for range in newlyInvalidSet.rangeView.map({ NSRange($0) })
    {
      invalidate(range: range)
    }
  }
}

// MARK: - Editing

extension Highlighter
{
  func storageDidEdit(editedRange: NSRange, delta: Int)
  {
    guard let textView else { return }

    let range = NSRange(location: editedRange.location, length: editedRange.length - delta)
    if delta > 0
    {
      visibleSet.insert(range: editedRange)
    }

    updateVisibleSet(textView: textView)

    highlightProvider?.applyEdit(textView: textView, range: range, delta: delta)
    { [weak self] invalidIndexSet in
      let indexSet = invalidIndexSet
        .union(IndexSet(integersIn: editedRange))

      for range in indexSet.rangeView
      {
        self?.invalidate(range: NSRange(range))
      }
    }
  }

  func storageWillEdit(editedRange: NSRange)
  {
    guard let textView else { return }
    highlightProvider?.willApplyEdit(textView: textView, range: editedRange)
  }
}
