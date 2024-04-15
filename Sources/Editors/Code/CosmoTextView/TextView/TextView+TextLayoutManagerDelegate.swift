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

extension TextView: TextLayoutManagerDelegate
{
  public func layoutManagerHeightDidUpdate(newHeight _: CGFloat)
  {
    updateFrameIfNeeded()
  }

  public func layoutManagerMaxWidthDidChange(newWidth _: CGFloat)
  {
    updateFrameIfNeeded()
  }

  public func layoutManagerTypingAttributes() -> [NSAttributedString.Key: Any]
  {
    typingAttributes
  }

  public func textViewportSize() -> CGSize
  {
    if let scrollView
    {
      var size = scrollView.contentSize
      size.height -= scrollView.contentInsets.top + scrollView.contentInsets.bottom
      return size
    }
    else
    {
      return CGSize(width: CGFloat.infinity, height: CGFloat.infinity)
    }
  }

  public func layoutManagerYAdjustment(_ yAdjustment: CGFloat)
  {
    var point = scrollView?.documentVisibleRect.origin ?? .zero
    point.y += yAdjustment
    scrollView?.documentView?.scroll(point)
  }
}
