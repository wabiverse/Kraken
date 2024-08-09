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
import PixarUSD
import SwiftUI
import UniformTypeIdentifiers

public extension Kraken.IO
{
  /// temporary maximum number of characters (6000 lines * 90 wide)
  /// for treesitter, since it becomes unstable with massive files
  /// TODO: find the culprit in treesitter, and remove the maximum
  /// threshold once fixed.
  static let TREE_SITTER_MAX = 6000 * 90

  @Observable
  final class USD: ReferenceFileDocument
  {
    var context: Kraken.IO.USD.Context

    public init(fileURL: URL = Kraken.IO.Stage.manager.getRandomTmpURL())
    {
      context = Kraken.IO.USD.Context(fileURL: fileURL)
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
      context = Kraken.IO.USD.Context()

      guard let data = configuration.file.regularFileContents,
            let string = String(data: data, encoding: .utf8)
      else
      {
        Kraken.IO.Stage.manager.reloadAndSave(stage: &context.stage)

        var contents = ""
        context.stage.exportToString(&contents, addSourceFileComment: false)
        context.usda = String(contents.prefix(Kraken.IO.TREE_SITTER_MAX))
        return
      }
      context.usda = String(string.prefix(Kraken.IO.TREE_SITTER_MAX))
    }

    public func fileWrapper(snapshot _: Kraken.IO.USD.Context, configuration _: WriteConfiguration) throws -> FileWrapper
    {
      var contents = ""
      context.stage.exportToString(&contents, addSourceFileComment: false)
      context.usda = String(contents.prefix(Kraken.IO.TREE_SITTER_MAX))

      return .init(regularFileWithContents: context.usda.data(using: .utf8)!)
    }

    public func snapshot(contentType _: UTType) throws -> Kraken.IO.USD.Context
    {
      context
    }
  }
}

public extension Kraken.IO.USD
{
  @Observable
  class Context: Identifiable
  {
    public var usda: String
    public var fileURL: URL
    public var stage: UsdStageRefPtr
    public var stageCache: UsdStageCache

    public var id: Int
    {
      stageCache.GetId(stage).ToLongInt()
    }

    public init(fileURL: URL = Kraken.IO.Stage.manager.getRandomTmpURL())
    {
      self.fileURL = fileURL
      usda = ""
      stage = Usd.Stage.createNew(fileURL.path)

      stageCache = UsdStageCache()
      UsdStageCacheContext.bind(cache: &stageCache)

      stage = Usd.Stage.open(fileURL.path)

      Kraken.IO.Stage.manager.save(&stage)

      var contents = ""
      stage.exportToString(&contents, addSourceFileComment: false)
      usda = String(contents.prefix(Kraken.IO.TREE_SITTER_MAX))
    }

    public func open(fileURL: URL)
    {
      self.fileURL = fileURL
      stage = Usd.Stage.open(fileURL.path)

      Kraken.IO.Stage.manager.save(&stage)

      var contents = ""
      stage.exportToString(&contents, addSourceFileComment: false)
      usda = String(contents.prefix(Kraken.IO.TREE_SITTER_MAX))
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

extension Kraken.IO.USD: Hashable
{
  public func hash(into hasher: inout Hasher)
  {
    hasher.combine(context.fileURL)
  }
}

extension Kraken.IO.USD: Equatable
{
  public static func == (lhs: Kraken.IO.USD, rhs: Kraken.IO.USD) -> Bool
  {
    lhs.context.fileURL == rhs.context.fileURL &&
      lhs.context.id == rhs.context.id &&
      lhs.context.usda == rhs.context.usda
  }
}
