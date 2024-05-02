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

import CodeLanguages
import CosmoEditor
import Foundation
import PixarUSD
import SwiftUI

public extension Kraken.UI
{
  struct CodeEditor: View
  {
    /** The document contents to edit. */
    @Binding var document: Kraken.IO.USD

    /** Whether the document is in binary format. */
    @State var isBinary: Bool

    /* -------------------------------------------------------- */

    /** perisistent setting whether lines wrap to the editor's width. */
    @AppStorage("wrapLines") private var wrapLines: Bool = true

    /* -------------------------------------------------------- */

    /** default code editor language setting. */
    @State private var language: Editor.Code.Language = .default

    /** default code editor theme setting for syntax highlighting. */
    @State private var theme: Editor.Code.Theme = .standard

    /** default code editor font setting for text. */
    @State private var font: NSFont = .monospacedSystemFont(ofSize: 12, weight: .bold)

    /** default code editor cursor positions. */
    @State private var cursorPositions: [CursorPosition] = []

    /* -------------------------------------------------------- */

    public var body: some View
    {
      if !isBinary
      {
        Editor.Code.Cosmo(
          $document.text,
          language: language,
          theme: theme,
          font: font,
          tabWidth: 2,
          indentOption: .spaces(count: 2),
          lineHeight: 1.2,
          wrapLines: wrapLines,
          cursorPositions: $cursorPositions
        )
      }
    }
  }
}
