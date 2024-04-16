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

    /** The usd file path. */
    var filePath: String

    /** The stage identified by this file. */
    @State private var stage: UsdStageRefPtr

    /** Whether the document is in binary format. */
    @State private var isBinary: Bool

    /**
     * Creates a new code editor and stage for the provided document,
     * if the file URL is nil or invalid, a new stage will be created.
     * - Parameters:
     *   - document: The document to edit.
     *   - fileURL: The file URL to edit. */
    public init(document: Binding<Kraken.IO.USD>, fileURL: URL?)
    {
      /* set the document. */
      _document = document

      /* validate usd filepath. */
      filePath = Kraken.IO.Stage.manager.validate(url: fileURL)

      /* open an existing usd stage. */
      _stage = State(initialValue: Usd.Stage.open(filePath))

      /* check if the file is binary. */
      isBinary = fileURL?.pathExtension != "usda"
    }

    /* -------------------------------------------------------- */

    @State private var language: Editor.Code.Language = .default
    @State private var theme: Editor.Code.Theme = .standard
    @State private var font: NSFont = .monospacedSystemFont(ofSize: 12, weight: .regular)
    @AppStorage("wrapLines") private var wrapLines: Bool = true
    @State private var cursorPositions: [CursorPosition] = []

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
        .onChange(of: document.text)
        {
          /* live visual reloading of stage changes. */
          // Kraken.IO.Stage.manager.save(
          //   contentsOfFile: document.text,
          //   atPath: filePath,
          //   stage: &stage
          // )
        }
      }
    }
  }
}
