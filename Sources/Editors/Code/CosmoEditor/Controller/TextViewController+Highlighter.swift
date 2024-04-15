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
import SwiftTreeSitter

extension TextViewController
{
  func setUpHighlighter()
  {
    if let highlighter
    {
      textView.removeStorageDelegate(highlighter)
      self.highlighter = nil
    }

    highlighter = Highlighter(
      textView: textView,
      highlightProvider: highlightProvider,
      theme: theme,
      attributeProvider: self,
      language: language
    )
    textView.addStorageDelegate(highlighter!)
    setHighlightProvider(highlightProvider)
  }

  func setHighlightProvider(_ highlightProvider: HighlightProviding? = nil)
  {
    var provider: HighlightProviding?

    if let highlightProvider
    {
      provider = highlightProvider
    }
    else
    {
      treeSitterClient = TreeSitterClient()
      provider = treeSitterClient!
    }

    if let provider
    {
      self.highlightProvider = provider
      highlighter?.setHighlightProvider(provider)
    }
  }
}

extension TextViewController: ThemeAttributesProviding
{
  public func attributesFor(_ capture: CaptureName?) -> [NSAttributedString.Key: Any]
  {
    [
      .font: font,
      .foregroundColor: theme.colorFor(capture),
      .kern: textView.kern
    ]
  }
}
