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

extension CodeView
{
  @objc open func copy(_: AnyObject)
  {
    guard let textSelections = selectionManager?
      .textSelections
      .compactMap({ textStorage.attributedSubstring(from: $0.range) }),
      !textSelections.isEmpty
    else
    {
      return
    }
    NSPasteboard.general.clearContents()
    NSPasteboard.general.writeObjects(textSelections)
  }

  @objc open func paste(_: AnyObject)
  {
    guard let stringContents = NSPasteboard.general.string(forType: .string) else { return }
    insertText(stringContents, replacementRange: NSRange(location: NSNotFound, length: 0))
  }

  @objc open func cut(_ sender: AnyObject)
  {
    copy(sender)
    deleteBackward(sender)
  }

  @objc open func delete(_ sender: AnyObject)
  {
    deleteBackward(sender)
  }
}
