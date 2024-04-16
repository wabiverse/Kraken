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

/// Animates a cursor. Will sync animation with any other cursor views.
open class CursorView: NSView
{
  /// The color of the cursor.
  public var color: NSColor
  {
    didSet
    {
      layer?.backgroundColor = color.cgColor
    }
  }

  /// The width of the cursor.
  private let width: CGFloat
  /// The timer observer.
  private var observer: NSObjectProtocol?

  override open var isFlipped: Bool
  {
    true
  }

  override open func hitTest(_: NSPoint) -> NSView? { nil }

  /// Create a cursor view.
  /// - Parameters:
  ///   - blinkDuration: The duration to blink, leave as nil to never blink.
  ///   - color: The color of the cursor.
  ///   - width: How wide the cursor should be.
  init(
    color: NSColor = NSColor.labelColor,
    width: CGFloat = 1.0
  )
  {
    self.color = color
    self.width = width

    super.init(frame: .zero)

    frame.size.width = width
    wantsLayer = true
    layer?.backgroundColor = color.cgColor
  }

  func blinkTimer(_ shouldHideCursor: Bool)
  {
    isHidden = shouldHideCursor
  }

  @available(*, unavailable)
  public required init?(coder _: NSCoder)
  {
    fatalError("init(coder:) has not been implemented")
  }
}
