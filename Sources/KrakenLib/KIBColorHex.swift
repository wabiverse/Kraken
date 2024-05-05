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

extension NSColor
{
  /// Initializes a `NSColor` from a HEX String (e.g.: `#1D2E3F`) and an optional alpha value.
  /// - Parameters:
  ///   - hex: A String of a HEX representation of a color (format: `#1D2E3F`)
  ///   - alpha: A Double indicating the alpha value from `0.0` to `1.0`
  convenience init(hex: String, alpha: Double = 1.0)
  {
    let hex = hex.trimmingCharacters(in: .alphanumerics.inverted)
    var int: UInt64 = 0
    Scanner(string: hex).scanHexInt64(&int)
    self.init(hex: Int(int), alpha: alpha)
  }

  /// Initializes a `NSColor` from an Int  (e.g.: `0x1D2E3F`)and an optional alpha value.
  /// - Parameters:
  ///   - hex: An Int of a HEX representation of a color (format: `0x1D2E3F`)
  ///   - alpha: A Double indicating the alpha value from `0.0` to `1.0`
  convenience init(hex: Int, alpha: Double = 1.0)
  {
    let red = (hex >> 16) & 0xFF
    let green = (hex >> 8) & 0xFF
    let blue = hex & 0xFF
    self.init(srgbRed: Double(red) / 255, green: Double(green) / 255, blue: Double(blue) / 255, alpha: alpha)
  }

  /// Returns an Int representing the `NSColor` in hex format (e.g.: 0x112233)
  var hex: Int
  {
    guard let components = cgColor.components, components.count >= 3 else { return 0 }

    let red = lround(Double(components[0]) * 255.0) << 16
    let green = lround(Double(components[1]) * 255.0) << 8
    let blue = lround(Double(components[2]) * 255.0)

    return red | green | blue
  }

  /// Returns a HEX String representing the `NSColor` (e.g.: #112233)
  var hexString: String
  {
    let color = hex

    return "#" + String(format: "%06x", color)
  }
}
