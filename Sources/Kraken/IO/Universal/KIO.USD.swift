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
import SwiftUI
import UniformTypeIdentifiers
import PixarUSD

public extension Kraken.IO
{
  @Observable
  final class USD: ReferenceFileDocument
  {
    var text: String
    var fileURL: URL
    var stage: UsdStageRefPtr

    public init(text: String = "", fileURL: URL = Kraken.IO.Stage.manager.getTmpURL())
    {
      self.fileURL = fileURL
      self.text = text

      if FileManager.default.fileExists(atPath: fileURL.path)
      {
        self.stage = Usd.Stage.open(fileURL.path)
      }
      else
      {
        self.stage = Usd.Stage.createNew(fileURL.path)
      }

      Kraken.IO.Stage.manager.save(&stage)

      var contents = ""
      self.stage.exportToString(&contents, addSourceFileComment: false)
      self.text = contents
    }

    public static var readableContentTypes: [UTType]
    {
      [
        .sourceCode,
        .plainText,
        .usd,
        .usdz,
        .delimitedText,
        .script
      ]
    }

    public static var blacklistedFileExts: [String]
    {
      [
        "usd",
        "usdc",
        "usdz"
      ]
    }

    public init(configuration: ReadConfiguration) throws
    {
      let url = Kraken.IO.Stage.manager.getTmpURL()
      self.stage = Usd.Stage.open(url.path)
      self.fileURL = url
      self.text = ""

      guard let data = configuration.file.regularFileContents,
            let string = String(data: data, encoding: .utf8)
      else
      {
        var contents: String = ""
        self.stage.exportToString(&contents, addSourceFileComment: false)
        Kraken.IO.Stage.manager.save(
          contentsOfFile: contents,
          atPath: url.path,
          stage: &stage
        )

        self.text = contents
        return
      }

      Kraken.IO.Stage.manager.save(
        contentsOfFile: string,
        atPath: url.path,
        stage: &stage
      )

      var contents: String = ""
      self.stage.exportToString(&contents, addSourceFileComment: false)

      self.text = contents
    }

    public func fileWrapper(snapshot: UsdStageRefPtr, configuration: WriteConfiguration) throws -> FileWrapper
    {
      var usda = ""
      stage.exportToString(&usda, addSourceFileComment: false)
      text = usda

      return .init(regularFileWithContents: usda.data(using: .utf8)!)
    }

    public func snapshot(contentType: UTType) throws -> UsdStageRefPtr
    {
      return self.stage
    }
  }
}

public extension ReferenceFileDocumentConfiguration<Kraken.IO.USD>
{
  /**
   * Whether the document is in binary format. */
  var isBinary: Bool
  {
    Kraken.IO.Stage.isBinary(self)
  }

  /**
   * The detected language of the document.
   *
   * Uses the document's file url to detect the
   * language of the document (swift, usd, etc). */
  var language: Editor.Code.Language
  {
    guard let url = fileURL
    else { return .default }

    return .detectLanguageFrom(url: url)
  }
}

extension Kraken.IO.USD: Equatable
{
  public static func == (lhs: Kraken.IO.USD, rhs: Kraken.IO.USD) -> Bool
  {
    return
      lhs.fileURL == rhs.fileURL &&
      lhs.text == rhs.text
  }
}
