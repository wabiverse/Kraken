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
import KrakenLib
import PixarUSD
import SwiftUI

public extension Kraken.UI
{
  struct CodeEditor: View
  {
    /** The document contents to edit. */
    @Binding var document: Kraken.IO.USD

    /** The stage to persist these changes. */
    @Binding var stage: UsdStageRefPtr

    /** The file url to this document. */
    let fileURL: URL?

    /* -------------------------------------------------------- */

    /** default code editor language setting. */
    @State private var language: Editor.Code.Language = .default

    /** default code editor theme setting for syntax highlighting. */
    @State private var theme: Editor.Code.Theme = .gruvbox

    /** default code editor font setting for text. */
    @State private var font: NSFont = .monospacedSystemFont(ofSize: 12, weight: .bold)

    /** default code editor cursor positions. */
    @State private var cursorPositions: [CursorPosition] = []

    /* -------------------------------------------------------- */

    /** perisistent setting whether lines wrap to the editor's width. */
    @AppStorage("wrapLines") private var wrapLines: Bool = true

    /* -------------------------------------------------------- */

    public var body: some View
    {
      VStack(spacing: 0)
      {
        HStack
        {
          Text("Language")

          LanguagePicker(language: $language)
            .frame(maxWidth: 100)

          Toggle("Wrap Lines", isOn: $wrapLines)
          Spacer()
          Text(getLabel(cursorPositions))
        }
        .padding(4)
        .zIndex(2)
        .background(Color(NSColor.windowBackgroundColor))

        Divider()

        Editor.Code.Cosmo(
          $document.text,
          language: language,
          theme: theme,
          font: font,
          tabWidth: 4,
          lineHeight: 1.2,
          wrapLines: wrapLines,
          cursorPositions: $cursorPositions,
          useThemeBackground: false
        )
        .onChange(of: document.text)
        {
          let bakDir = Kraken.IO.Stage.manager.getTmpURL()

          Kraken.IO.Stage.manager.save(
            contentsOfFile: document.text,
            atPath: fileURL?.path ?? bakDir.path,
            stage: &stage
          )
        }
      }
      .onAppear
      {
        language = detectLanguage(fileURL: fileURL) ?? .default
      }
    }

    /**
     * Automatically detect the language of the document from it's file url.
     * - Parameter fileURL: The file url to the document.
     * - Returns: The detected language of the document. */
    private func detectLanguage(fileURL: URL?) -> Editor.Code.Language?
    {
      guard let fileURL else { return nil }
      return Editor.Code.Language.detectLanguageFrom(
        url: fileURL,
        prefixBuffer: document.text.getFirstLines(5),
        suffixBuffer: document.text.getLastLines(5)
      )
    }

    /**
     * Create a label string for cursor positions.
     * - Parameter cursorPositions: The cursor positions to create the label for.
     * - Returns: A string describing the user's location in a document. */
    func getLabel(_ cursorPositions: [CursorPosition]) -> String
    {
      if cursorPositions.isEmpty
      {
        return ""
      }

      // More than one selection, display the number of selections.
      if cursorPositions.count > 1
      {
        return "\(cursorPositions.count) selected ranges"
      }

      // When there's a single cursor, display the line and column.
      return "Line: \(cursorPositions[0].line)  Col: \(cursorPositions[0].column)"
    }
  }
}