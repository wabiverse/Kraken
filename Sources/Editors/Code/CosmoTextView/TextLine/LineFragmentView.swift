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
// import CosmoTextViewObjC

/// Displays a line fragment.
final class LineFragmentView: NSView
{
  private weak var lineFragment: LineFragment?

  override var isFlipped: Bool
  {
    true
  }

  override var isOpaque: Bool
  {
    false
  }

  override func hitTest(_: NSPoint) -> NSView? { nil }

  /// Prepare the view for reuse, clears the line fragment reference.
  override func prepareForReuse()
  {
    super.prepareForReuse()
    lineFragment = nil
  }

  /// Set a new line fragment for this view, updating view size.
  /// - Parameter newFragment: The new fragment to use.
  public func setLineFragment(_ newFragment: LineFragment)
  {
    lineFragment = newFragment
    frame.size = CGSize(width: newFragment.width, height: newFragment.scaledHeight)
  }

  /// Draws the line fragment in the graphics context.
  override func draw(_: NSRect)
  {
    guard let lineFragment, let context = NSGraphicsContext.current?.cgContext
    else
    {
      return
    }
    context.saveGState()

    context.setAllowsAntialiasing(true)
    context.setShouldAntialias(true)
    context.setAllowsFontSmoothing(false)
    context.setShouldSmoothFonts(false)
    context.setAllowsFontSubpixelPositioning(true)
    context.setShouldSubpixelPositionFonts(true)
    context.setAllowsFontSubpixelQuantization(true)
    context.setShouldSubpixelQuantizeFonts(true)

    // ContextSetHiddenSmoothingStyle(context, 16)

    context.textMatrix = .init(scaleX: 1, y: -1)
    context.textPosition = CGPoint(
      x: 0,
      y: lineFragment.height - lineFragment.descent + (lineFragment.heightDifference / 2)
    ).pixelAligned

    CTLineDraw(lineFragment.ctLine, context)
    context.restoreGState()
  }
}
