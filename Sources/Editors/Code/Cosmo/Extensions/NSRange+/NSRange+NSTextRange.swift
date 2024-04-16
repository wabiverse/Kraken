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

public extension NSTextRange
{
  convenience init?(_ range: NSRange, provider: NSTextElementProvider)
  {
    let docLocation = provider.documentRange.location

    guard let start = provider.location?(docLocation, offsetBy: range.location)
    else
    {
      return nil
    }

    guard let end = provider.location?(start, offsetBy: range.length)
    else
    {
      return nil
    }

    self.init(location: start, end: end)
  }

  /// Creates an `NSRange` using document information from the given provider.
  /// - Parameter provider: The `NSTextElementProvider` to use to convert this range into an `NSRange`
  /// - Returns: An `NSRange` if possible
  func nsRange(using provider: NSTextElementProvider) -> NSRange?
  {
    guard let location = provider.offset?(from: provider.documentRange.location, to: location)
    else
    {
      return nil
    }
    guard let length = provider.offset?(from: self.location, to: endLocation)
    else
    {
      return nil
    }
    return NSRange(location: location, length: length)
  }
}
