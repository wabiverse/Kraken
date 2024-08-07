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

public protocol TextSelectionManagerDelegate: AnyObject
{
  var visibleTextRange: NSRange? { get }

  func setNeedsDisplay()
  func estimatedLineHeight() -> CGFloat
}

/// Manages an array of text selections representing cursors (0-length ranges) and selections (>0-length ranges).
///
/// Draws selections using a draw method similar to the `TextLayoutManager` class, and adds cursor views when
/// appropriate.
public class TextSelectionManager: NSObject
{
  // MARK: - TextSelection

  public class TextSelection: Hashable, Equatable
  {
    public var range: NSRange
    weak var view: NSView?
    var boundingRect: CGRect = .zero
    var suggestedXPos: CGFloat?
    /// The position this selection should 'rotate' around when modifying selections.
    var pivot: Int?

    init(range: NSRange, view: CursorView? = nil)
    {
      self.range = range
      self.view = view
    }

    var isCursor: Bool
    {
      range.length == 0
    }

    public func hash(into hasher: inout Hasher)
    {
      hasher.combine(range)
    }

    public static func == (lhs: TextSelection, rhs: TextSelection) -> Bool
    {
      lhs.range == rhs.range
    }
  }

  public enum Destination
  {
    case character
    case word
    case line
    case visualLine
    /// Eg: Bottom of screen
    case container
    case document
  }

  public enum Direction
  {
    case up
    case down
    case forward
    case backward
  }

  // MARK: - Properties

  // swiftlint:disable:next line_length
  public static let selectionChangedNotification: Notification.Name = .init("foundation.wabi.TextSelectionManager.TextSelectionChangedNotification")

  public var insertionPointColor: NSColor = .labelColor
  {
    didSet
    {
      textSelections.compactMap { $0.view as? CursorView }.forEach { $0.color = insertionPointColor }
    }
  }

  public var highlightSelectedLine: Bool = true
  public var selectedLineBackgroundColor: NSColor = .selectedTextBackgroundColor.withSystemEffect(.disabled)
  public var selectionBackgroundColor: NSColor = .selectedTextBackgroundColor
  public var useSystemCursor: Bool = false
  {
    didSet
    {
      updateSelectionViews()
    }
  }

  public internal(set) var textSelections: [TextSelection] = []
  weak var layoutManager: TextLayoutManager?
  weak var textStorage: NSTextStorage?
  weak var textView: CodeView?
  weak var delegate: TextSelectionManagerDelegate?
  var cursorTimer: CursorTimer

  init(
    layoutManager: TextLayoutManager,
    textStorage: NSTextStorage,
    textView: CodeView?,
    delegate: TextSelectionManagerDelegate?,
    useSystemCursor _: Bool = false
  )
  {
    self.layoutManager = layoutManager
    self.textStorage = textStorage
    self.textView = textView
    self.delegate = delegate
    cursorTimer = CursorTimer()
    super.init()
    textSelections = []
    updateSelectionViews()
  }

  // MARK: - Selected Ranges

  public func setSelectedRange(_ range: NSRange)
  {
    textSelections.forEach { $0.view?.removeFromSuperview() }
    let selection = TextSelection(range: range)
    selection.suggestedXPos = layoutManager?.rectForOffset(range.location)?.minX
    textSelections = [selection]
    if textView?.isFirstResponder ?? false
    {
      updateSelectionViews()
      NotificationCenter.default.post(Notification(name: Self.selectionChangedNotification, object: self))
    }
  }

  public func setSelectedRanges(_ ranges: [NSRange])
  {
    textSelections.forEach { $0.view?.removeFromSuperview() }
    // Remove duplicates, invalid ranges, update suggested X position.
    textSelections = Set(ranges)
      .filter
      {
        (0 ... (textStorage?.length ?? 0)).contains($0.location)
          && (0 ... (textStorage?.length ?? 0)).contains($0.max)
      }
      .map
      {
        let selection = TextSelection(range: $0)
        selection.suggestedXPos = layoutManager?.rectForOffset($0.location)?.minX
        return selection
      }
    if textView?.isFirstResponder ?? false
    {
      updateSelectionViews()
      NotificationCenter.default.post(Notification(name: Self.selectionChangedNotification, object: self))
    }
  }

  public func addSelectedRange(_ range: NSRange)
  {
    let newTextSelection = TextSelection(range: range)
    var didHandle = false
    for textSelection in textSelections
    {
      if textSelection.range == newTextSelection.range
      {
        // Duplicate range, ignore
        return
      }
      else if (range.length > 0 && textSelection.range.intersection(range) != nil)
        || textSelection.range.max == range.location
      {
        // Range intersects existing range, modify this range to be the union of both and don't add the new
        // selection
        textSelection.range = textSelection.range.union(range)
        didHandle = true
      }
    }
    if !didHandle
    {
      textSelections.append(newTextSelection)
    }

    if textView?.isFirstResponder ?? false
    {
      updateSelectionViews()
      NotificationCenter.default.post(Notification(name: Self.selectionChangedNotification, object: self))
    }
  }

  // MARK: - Selection Views

  /// Update all selection cursors. Placing them in the correct position for each text selection and reseting the
  /// blink timer.
  func updateSelectionViews()
  {
    var didUpdate = false

    for textSelection in textSelections
    {
      if textSelection.range.isEmpty
      {
        let cursorOrigin = (layoutManager?.rectForOffset(textSelection.range.location) ?? .zero).origin

        // If using the system cursor, macOS will change the origin and height by about 0.5,
        // so we do an approximate equals in that case to avoid extra updates.
        let doesViewNeedReposition: Bool = if useSystemCursor, #available(macOS 14.0, *)
        {
          !textSelection.boundingRect.origin.approxEqual(cursorOrigin)
            || !textSelection.boundingRect.height.approxEqual(layoutManager?.estimateLineHeight() ?? 0)
        }
        else
        {
          textSelection.boundingRect.origin != cursorOrigin
            || textSelection.boundingRect.height != layoutManager?.estimateLineHeight() ?? 0
        }

        if textSelection.view == nil || doesViewNeedReposition
        {
          let cursorView: NSView

          if let existingCursorView = textSelection.view
          {
            cursorView = existingCursorView
          }
          else
          {
            textSelection.view?.removeFromSuperview()
            textSelection.view = nil

            if useSystemCursor, #available(macOS 14.0, *)
            {
              let systemCursorView = NSTextInsertionIndicator(frame: .zero)
              cursorView = systemCursorView
              systemCursorView.displayMode = .automatic
            }
            else
            {
              let internalCursorView = CursorView(color: insertionPointColor)
              cursorView = internalCursorView
              cursorTimer.register(internalCursorView)
            }

            textView?.addSubview(cursorView)
          }

          cursorView.frame.origin = cursorOrigin
          cursorView.frame.size.height = layoutManager?.estimateLineHeight() ?? 0

          textSelection.view = cursorView
          textSelection.boundingRect = cursorView.frame

          didUpdate = true
        }
      }
      else if !textSelection.range.isEmpty, textSelection.view != nil
      {
        textSelection.view?.removeFromSuperview()
        textSelection.view = nil
        didUpdate = true
      }
    }

    if didUpdate
    {
      delegate?.setNeedsDisplay()
      cursorTimer.resetTimer()
    }
  }

  /// Removes all cursor views and stops the cursor blink timer.
  func removeCursors()
  {
    cursorTimer.stopTimer()
    for textSelection in textSelections
    {
      textSelection.view?.removeFromSuperview()
    }
  }

  // MARK: - Draw

  /// Draws line backgrounds and selection rects for each selection in the given rect.
  /// - Parameter rect: The rect to draw in.
  func drawSelections(in rect: NSRect)
  {
    guard let context = NSGraphicsContext.current?.cgContext else { return }
    context.saveGState()
    var highlightedLines: Set<UUID> = []
    // For each selection in the rect
    for textSelection in textSelections
    {
      if textSelection.range.isEmpty
      {
        drawHighlightedLine(
          in: rect,
          for: textSelection,
          context: context,
          highlightedLines: &highlightedLines
        )
      }
      else
      {
        drawSelectedRange(in: rect, for: textSelection, context: context)
      }
    }
    context.restoreGState()
  }

  /// Draws a highlighted line in the given rect.
  /// - Parameters:
  ///   - rect: The rect to draw in.
  ///   - textSelection: The selection to draw.
  ///   - context: The context to draw in.
  ///   - highlightedLines: The set of all lines that have already been highlighted, used to avoid highlighting lines
  ///                       twice and updated if this function comes across a new line id.
  private func drawHighlightedLine(
    in rect: NSRect,
    for textSelection: TextSelection,
    context: CGContext,
    highlightedLines: inout Set<UUID>
  )
  {
    guard let linePosition = layoutManager?.textLineForOffset(textSelection.range.location),
          !highlightedLines.contains(linePosition.data.id)
    else
    {
      return
    }
    highlightedLines.insert(linePosition.data.id)
    context.saveGState()
    let selectionRect = CGRect(
      x: rect.minX,
      y: linePosition.yPos,
      width: rect.width,
      height: linePosition.height
    )
    if selectionRect.intersects(rect)
    {
      context.setFillColor(selectedLineBackgroundColor.cgColor)
      context.fill(selectionRect)
    }
    context.restoreGState()
  }

  /// Draws a selected range in the given context.
  /// - Parameters:
  ///   - rect: The rect to draw in.
  ///   - range: The range to highlight.
  ///   - context: The context to draw in.
  private func drawSelectedRange(in rect: NSRect, for textSelection: TextSelection, context: CGContext)
  {
    context.saveGState()

    let fillColor = (textView?.isFirstResponder ?? false)
      ? selectionBackgroundColor.cgColor
      : selectionBackgroundColor.grayscale.cgColor

    context.setFillColor(fillColor)

    let fillRects = getFillRects(in: rect, for: textSelection)

    let min = fillRects.min(by: { $0.origin.y < $1.origin.y })?.origin ?? .zero
    let max = fillRects.max(by: { $0.origin.y < $1.origin.y }) ?? .zero
    let size = CGSize(width: max.maxX - min.x, height: max.maxY - min.y)
    textSelection.boundingRect = CGRect(origin: min, size: size)

    context.fill(fillRects)
    context.restoreGState()
  }
}

// MARK: - Private TextSelection

private extension TextSelectionManager.TextSelection
{
  func didInsertText(length: Int, retainLength: Bool = false)
  {
    if !retainLength
    {
      range.length = 0
    }
    range.location += length
  }
}
