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
import LanguagesBundle
import RegexBuilder
import SwiftTreeSitter
import TreeSitter

public extension Editor.Code
{
  /// A structure holding metadata for code languages
  struct Language
  {
    init(
      id: TreeSitterLanguage,
      tsName: String,
      extensions: Set<String>,
      lineCommentString: String,
      rangeCommentStrings: (String, String),
      documentationCommentStrings: Set<DocumentationComments> = [],
      parentURL: URL? = nil,
      highlights: Set<String>? = nil,
      additionalIdentifiers: Set<String> = []
    )
    {
      self.id = id
      self.tsName = tsName
      self.extensions = extensions
      self.lineCommentString = lineCommentString
      self.rangeCommentStrings = rangeCommentStrings
      self.documentationCommentStrings = documentationCommentStrings
      parentQueryURL = parentURL
      additionalHighlights = highlights
      self.additionalIdentifiers = additionalIdentifiers
    }

    /// The ID of the language
    public let id: TreeSitterLanguage

    /// The display name of the language
    public let tsName: String

    /// A set of file extensions for the language
    ///
    /// In special cases this can also be a file name
    /// (e.g `Dockerfile`, `Makefile`)
    public let extensions: Set<String>

    /// The leading string of a comment line
    public let lineCommentString: String

    /// The leading and trailing string of a multi-line comment
    public let rangeCommentStrings: (String, String)

    /// The leading (and trailing, if there is one) string of a documentation comment
    public let documentationCommentStrings: Set<DocumentationComments>

    /// The query URL of a language this language inherits from. (e.g.: C for C++)
    public let parentQueryURL: URL?

    /// Additional highlight file names (e.g.: JSX for JavaScript)
    public let additionalHighlights: Set<String>?

    /// The query URL for the language if available
    public var queryURL: URL?
    {
      queryURL()
    }

    /// The bundle's resource URL
    var resourceURL: URL? = Bundle.module.resourceURL

    /// A set of aditional identifiers to use for things like shebang matching.
    public let additionalIdentifiers: Set<String>

    /// The tree-sitter language for the language if available
    public var language: SwiftTreeSitter.Language?
    {
      guard let tsLanguage else { return nil }
      return SwiftTreeSitter.Language(language: tsLanguage)
    }

    func queryURL(for highlights: String = "highlights") -> URL?
    {
      guard let url = resourceURL?.appendingPathComponent("tree-sitter-\(tsName)/\(highlights).scm")
      else { Editor.Code.logger.warning("Query URL not found for \(tsName) language."); return nil }

      return url
    }

    /// Gets the TSLanguage from `tree-sitter`
    private var tsLanguage: OpaquePointer?
    {
      switch id
      {
        case .c:
          tree_sitter_c()
        case .cpp:
          tree_sitter_cpp()
        case .galah:
          tree_sitter_galah()
        case .jsdoc:
          tree_sitter_jsdoc()
        case .json:
          tree_sitter_json()
        case .python:
          tree_sitter_python()
        case .rust:
          tree_sitter_rust()
        case .swift:
          tree_sitter_swift()
        case .toml:
          tree_sitter_toml()
        case .usd:
          tree_sitter_usd()
        case .plainText:
          nil
      }
    }
  }
}

extension Editor.Code.Language: Hashable
{
  public static func == (lhs: Self, rhs: Self) -> Bool
  {
    lhs.id == rhs.id
  }

  public func hash(into hasher: inout Hasher)
  {
    hasher.combine(id)
  }
}

public enum DocumentationComments: Hashable
{
  public static func == (lhs: Self, rhs: Self) -> Bool
  {
    switch lhs
    {
      case let .single(lhsString):
        switch rhs
        {
          case let .single(rhsString):
            lhsString == rhsString
          case .pair:
            false
        }
      case let .pair(lhsPair):
        switch rhs
        {
          case .single:
            false
          case let .pair(rhsPair):
            lhsPair.0 == rhsPair.0 && lhsPair.1 == rhsPair.1
        }
    }
  }

  public func hash(into hasher: inout Hasher)
  {
    switch self
    {
      case let .single(string):
        hasher.combine(string)
      case let .pair(pair):
        hasher.combine(pair.0)
        hasher.combine(pair.1)
    }
  }

  case single(String)
  case pair((String, String))
}
