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

import CodeEditSourceEditor
import Foundation
import SwiftUI

public struct CodeEditor: View
{
  @Binding public var text: String

  @State var theme = EditorTheme(
    text: .orange,
    insertionPoint: .purple,
    invisibles: .lightGray,
    background: .windowBackgroundColor,
    lineHighlight: .gray,
    selection: .darkGray,
    keywords: .red,
    commands: .green,
    types: .yellow,
    attributes: .orange,
    variables: .white,
    values: .blue,
    numbers: .purple,
    strings: .yellow,
    characters: .blue,
    comments: .lightGray
  )
  @State var font = NSFont.monospacedSystemFont(ofSize: 11, weight: .bold)
  @State var tabWidth = 2
  @State var lineHeight = 1.2
  @State var editorOverscroll = 0.3
  @State var cursorPositions = [CursorPosition(range: .notFound)]

  public init(text: Binding<String>)
  {
    _text = text
  }

  public var body: some View
  {
    CodeEditSourceEditor(
      $text,
      language: .swift,
      theme: theme,
      font: font,
      tabWidth: tabWidth,
      lineHeight: lineHeight,
      wrapLines: true,
      editorOverscroll: editorOverscroll,
      cursorPositions: $cursorPositions
    )
  }
}

#if canImport(AppKit)
  extension NSTextView
  {
    override open var frame: CGRect
    {
      didSet
      {
        isAutomaticQuoteSubstitutionEnabled = false
        isAutomaticDashSubstitutionEnabled = false
        isAutomaticTextReplacementEnabled = false
        isAutomaticSpellingCorrectionEnabled = false
        isAutomaticDataDetectionEnabled = false
        isAutomaticLinkDetectionEnabled = false
        isAutomaticTextCompletionEnabled = false
        isAutomaticTextReplacementEnabled = false
      }
    }
  }

#elseif canImport(UIKit)
  extension UITextView
  {
    override open var frame: CGRect
    {
      didSet
      {
        autocorrectionType = .no
        autocapitalizationType = .none
        smartQuotesType = .no
        smartDashesType = .no
        smartInsertDeleteType = .no
      }
    }
  }
#endif
