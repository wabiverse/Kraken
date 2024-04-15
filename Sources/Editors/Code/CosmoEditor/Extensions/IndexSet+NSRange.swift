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

import Foundation

extension NSRange
{
  /// Convenience getter for safely creating a `Range<Int>` from an `NSRange`
  var intRange: Range<Int>
  {
    location ..< NSMaxRange(self)
  }
}

/// Helpers for working with `NSRange`s and `IndexSet`s.
extension IndexSet
{
  /// Initializes the  index set with a range of integers
  init(integersIn range: NSRange)
  {
    self.init(integersIn: range.intRange)
  }

  /// Remove all the integers in the `NSRange`
  mutating func remove(integersIn range: NSRange)
  {
    remove(integersIn: range.intRange)
  }

  /// Insert all the integers in the `NSRange`
  mutating func insert(integersIn range: NSRange)
  {
    insert(integersIn: range.intRange)
  }

  /// Returns true if self contains all of the integers in range.
  func contains(integersIn range: NSRange) -> Bool
  {
    contains(integersIn: range.intRange)
  }
}
