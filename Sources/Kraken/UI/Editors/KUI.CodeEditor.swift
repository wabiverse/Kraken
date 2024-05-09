/* ----------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                            ::
 * ----------------------------------------------------------------
 * This software is Licensed under the terms of the Apache License,
 * version 2.0 (the "Apache License") with the following additional
 * modification; you may not use this file except within compliance
 * of the Apache License and the following modification made to it.
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * Trademarks. This License does not grant permission to use any of
 * its trade names, trademarks, service marks, or the product names
 * of this Licensor or its affiliates, except as required to comply
 * with Section 4(c.) of this License, and to reproduce the content
 * of the NOTICE file.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND without even an
 * implied warranty of MERCHANTABILITY, or FITNESS FOR A PARTICULAR
 * PURPOSE. See the Apache License for more details.
 *
 * You should have received a copy for this software license of the
 * Apache License along with this program; or, if not, please write
 * to the Free Software Foundation Inc., with the following address
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *         Copyright (C) 2024 Wabi Foundation. All Rights Reserved.
 * ----------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ---------------------------------------------------------------- */

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

        Divider()

        HStack
        {
          LanguagePicker(language: $language)
            .frame(maxWidth: 35)

          Spacer()

          Text(getLabel(cursorPositions))
            .font(.system(size: 8, weight: .bold, design: .monospaced))
        }
        .padding(4)
        .zIndex(2)
        .background(.ultraThinMaterial)
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
