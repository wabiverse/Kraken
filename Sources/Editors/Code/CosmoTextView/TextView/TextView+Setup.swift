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

extension TextView
{
  func setUpLayoutManager(lineHeightMultiplier: CGFloat, wrapLines: Bool) -> TextLayoutManager
  {
    TextLayoutManager(
      textStorage: textStorage,
      lineHeightMultiplier: lineHeightMultiplier,
      wrapLines: wrapLines,
      textView: self,
      delegate: self
    )
  }

  func setUpSelectionManager() -> TextSelectionManager
  {
    TextSelectionManager(
      layoutManager: layoutManager,
      textStorage: textStorage,
      textView: self,
      delegate: self
    )
  }

  func setUpScrollListeners(scrollView: NSScrollView)
  {
    NotificationCenter.default.removeObserver(self, name: NSScrollView.willStartLiveScrollNotification, object: nil)
    NotificationCenter.default.removeObserver(self, name: NSScrollView.didEndLiveScrollNotification, object: nil)

    NotificationCenter.default.addObserver(
      self,
      selector: #selector(scrollViewWillStartScroll),
      name: NSScrollView.willStartLiveScrollNotification,
      object: scrollView
    )

    NotificationCenter.default.addObserver(
      self,
      selector: #selector(scrollViewDidEndScroll),
      name: NSScrollView.didEndLiveScrollNotification,
      object: scrollView
    )
  }

  @objc func scrollViewWillStartScroll()
  {
    if #available(macOS 14.0, *)
    {
      inputContext?.textInputClientWillStartScrollingOrZooming()
    }
  }

  @objc func scrollViewDidEndScroll()
  {
    if #available(macOS 14.0, *)
    {
      inputContext?.textInputClientDidEndScrollingOrZooming()
    }
  }
}
