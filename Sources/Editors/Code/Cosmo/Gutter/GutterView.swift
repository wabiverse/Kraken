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

public protocol GutterViewDelegate: AnyObject
{
  func gutterViewWidthDidUpdate(newWidth: CGFloat)
}

public class GutterView: NSView
{
  struct EdgeInsets: Equatable, Hashable
  {
    let leading: CGFloat
    let trailing: CGFloat

    var horizontal: CGFloat
    {
      leading + trailing
    }
  }

  @Invalidating(.display)
  var textColor: NSColor = .secondaryLabelColor

  @Invalidating(.display)
  var font: NSFont = .systemFont(ofSize: 13)

  @Invalidating(.display)
  var edgeInsets: EdgeInsets = .init(leading: 20, trailing: 12)

  @Invalidating(.display)
  var backgroundColor: NSColor? = NSColor.controlBackgroundColor

  @Invalidating(.display)
  var highlightSelectedLines: Bool = true

  @Invalidating(.display)
  var selectedLineTextColor: NSColor? = .textColor

  @Invalidating(.display)
  var selectedLineColor: NSColor = .selectedTextBackgroundColor.withSystemEffect(.disabled)

  public private(set) var gutterWidth: CGFloat = 0

  private weak var textView: CodeView?
  private weak var delegate: GutterViewDelegate?
  private var maxWidth: CGFloat = 0
  /// The maximum number of digits found for a line number.
  private var maxLineLength: Int = 0

  override public var isFlipped: Bool
  {
    true
  }

  public init(
    font: NSFont,
    textColor: NSColor,
    textView: CodeView,
    delegate: GutterViewDelegate? = nil
  )
  {
    self.font = font
    self.textColor = textColor
    self.textView = textView
    self.delegate = delegate

    super.init(frame: .zero)
    clipsToBounds = true
    wantsLayer = true
    layerContentsRedrawPolicy = .onSetNeedsDisplay
    translatesAutoresizingMaskIntoConstraints = false
    layer?.masksToBounds = true

    NotificationCenter.default.addObserver(
      forName: TextSelectionManager.selectionChangedNotification,
      object: nil,
      queue: .main
    )
    { _ in
      self.needsDisplay = true
    }
  }

  override init(frame frameRect: NSRect)
  {
    super.init(frame: frameRect)
  }

  @available(*, unavailable)
  required init?(coder _: NSCoder)
  {
    fatalError("init(coder:) has not been implemented")
  }

  /// Updates the width of the gutter if needed.
  func updateWidthIfNeeded()
  {
    guard let textView else { return }
    let attributes: [NSAttributedString.Key: Any] = [
      .font: font,
      .foregroundColor: textColor
    ]
    let originalMaxWidth = maxWidth
    // Reserve at least 3 digits of space no matter what
    let lineStorageDigits = max(3, String(textView.layoutManager.lineCount).count)

    if maxLineLength < lineStorageDigits
    {
      // Update the max width
      let maxCtLine = CTLineCreateWithAttributedString(
        NSAttributedString(string: String(repeating: "0", count: lineStorageDigits), attributes: attributes)
      )
      let width = CTLineGetTypographicBounds(maxCtLine, nil, nil, nil)
      maxWidth = max(maxWidth, width)
      maxLineLength = lineStorageDigits
    }

    if originalMaxWidth != maxWidth
    {
      gutterWidth = maxWidth + edgeInsets.horizontal
      delegate?.gutterViewWidthDidUpdate(newWidth: maxWidth + edgeInsets.horizontal)
    }
  }

  private func drawSelectedLines(_ context: CGContext)
  {
    guard let textView,
          let selectionManager = textView.selectionManager,
          let visibleRange = textView.visibleTextRange,
          highlightSelectedLines
    else
    {
      return
    }
    context.saveGState()
    var highlightedLines: Set<UUID> = []
    context.setFillColor(selectedLineColor.cgColor)
    for selection in selectionManager.textSelections
      where selection.range.isEmpty
    {
      guard let line = textView.layoutManager.textLineForOffset(selection.range.location),
            visibleRange.intersection(line.range) != nil || selection.range.location == textView.length,
            !highlightedLines.contains(line.data.id)
      else
      {
        continue
      }
      highlightedLines.insert(line.data.id)
      context.fill(
        CGRect(
          x: 0.0,
          y: line.yPos,
          width: maxWidth + edgeInsets.horizontal,
          height: line.height
        )
      )
    }
    context.restoreGState()
  }

  private func drawLineNumbers(_ context: CGContext)
  {
    guard let textView else { return }
    var attributes: [NSAttributedString.Key: Any] = [.font: font]

    var selectionRangeMap = IndexSet()
    textView.selectionManager?.textSelections.forEach
    {
      if $0.range.isEmpty
      {
        selectionRangeMap.insert($0.range.location)
      }
      else
      {
        selectionRangeMap.insert(range: $0.range)
      }
    }

    context.saveGState()

    context.setAllowsAntialiasing(true)
    context.setShouldAntialias(true)
    context.setAllowsFontSmoothing(false)
    context.setAllowsFontSubpixelPositioning(true)
    context.setShouldSubpixelPositionFonts(true)
    context.setAllowsFontSubpixelQuantization(true)
    context.setShouldSubpixelQuantizeFonts(true)
    // ContextSetHiddenSmoothingStyle(context, 16)

    context.textMatrix = CGAffineTransform(scaleX: 1, y: -1)
    for linePosition in textView.layoutManager.visibleLines()
    {
      if selectionRangeMap.intersects(integersIn: linePosition.range)
      {
        attributes[.foregroundColor] = selectedLineTextColor ?? textColor
      }
      else
      {
        attributes[.foregroundColor] = textColor
      }

      let ctLine = CTLineCreateWithAttributedString(
        NSAttributedString(string: "\(linePosition.index + 1)", attributes: attributes)
      )
      let fragment: LineFragment? = linePosition.data.lineFragments.first?.data
      var ascent: CGFloat = 0
      let lineNumberWidth = CTLineGetTypographicBounds(ctLine, &ascent, nil, nil)

      let yPos = linePosition.yPos + ascent + (fragment?.heightDifference ?? 0) / 2
      // Leading padding + (width - linewidth)
      let xPos = edgeInsets.leading + (maxWidth - lineNumberWidth)

      context.textPosition = CGPoint(x: xPos, y: yPos).pixelAligned
      CTLineDraw(ctLine, context)
    }
    context.restoreGState()
  }

  override public func draw(_: NSRect)
  {
    guard let context = NSGraphicsContext.current?.cgContext
    else
    {
      return
    }
    CATransaction.begin()
    superview?.clipsToBounds = false
    superview?.layer?.masksToBounds = false
    layer?.backgroundColor = backgroundColor?.cgColor
    updateWidthIfNeeded()
    drawSelectedLines(context)
    drawLineNumbers(context)
    CATransaction.commit()
  }

  deinit
  {
    NotificationCenter.default.removeObserver(self)
  }
}
