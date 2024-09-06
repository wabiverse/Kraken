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

/// A singleton class to manage `tree-sitter` queries and keep them in memory.
public class TreeSitterModel
{
  /// The singleton/shared instance of ``TreeSitterModel``.
  public static let shared: TreeSitterModel = .init()

  /// Get a query for a specific language
  /// - Parameter language: The language to request the query for.
  /// - Returns: A Query if available. Returns `nil` for not implemented languages
  public func query(for language: TreeSitterLanguage) -> Query?
  {
    // swiftlint:disable:previous cyclomatic_complexity function_body_length
    switch language
    {
      case .c:
        cQuery
      case .cpp:
        cppQuery
      case .galah:
        galahQuery
      case .jsdoc:
        jsdocQuery
      case .json:
        jsonQuery
      case .python:
        pythonQuery
      case .rust:
        rustQuery
      case .swift:
        swiftQuery
      case .toml:
        tomlQuery
      case .usd:
        usdQuery
      case .plainText:
        nil
    }
  }

  /// Query for `C` files.
  public private(set) lazy var cQuery: Query? = queryFor(.c)

  /// Query for `C++` files.
  public private(set) lazy var cppQuery: Query? = queryFor(.cpp)

  /// Query for `Galah` files.
  public private(set) lazy var galahQuery: Query? = queryFor(.galah)

  /// Query for `JSDoc` files.
  public private(set) lazy var jsdocQuery: Query? = queryFor(.jsdoc)

  /// Query for `JSON` files.
  public private(set) lazy var jsonQuery: Query? = queryFor(.json)

  /// Query for `Python` files.
  public private(set) lazy var pythonQuery: Query? = queryFor(.python)

  /// Query for `Rust` files.
  public private(set) lazy var rustQuery: Query? = queryFor(.rust)

  /// Query for `Swift` files.
  public private(set) lazy var swiftQuery: Query? = queryFor(.swift)

  /// Query for `TOML` files.
  public private(set) lazy var tomlQuery: Query? = queryFor(.toml)

  /// Query for `USD` files.
  public private(set) lazy var usdQuery: Query? = queryFor(.usd)

  private func queryFor(_ codeLanguage: Editor.Code.Language) -> Query?
  {
    // get the tree-sitter language and query url if available
    guard let language = codeLanguage.language,
          let url = codeLanguage.queryURL else { return nil }

    // 1. if the language depends on another language combine the query files
    // 2. if the language has additional query files combine them with the main one
    // 3. otherwise return the query file
    if let parentURL = codeLanguage.parentQueryURL,
       let data = combinedQueryData(for: [url, parentURL])
    {
      return try? Query(language: language, data: data)
    }
    else if let additionalHighlights = codeLanguage.additionalHighlights
    {
      var addURLs = additionalHighlights.compactMap { codeLanguage.queryURL(for: $0) }
      addURLs.append(url)
      guard let data = combinedQueryData(for: addURLs) else { return nil }
      return try? Query(language: language, data: data)
    }
    else
    {
      return try? language.query(contentsOf: url)
    }
  }

  private func combinedQueryData(for fileURLs: [URL]) -> Data?
  {
    let rawQuery = fileURLs.compactMap { try? String(contentsOf: $0) }.joined(separator: "\n")
    if !rawQuery.isEmpty
    {
      return rawQuery.data(using: .utf8)
    }
    else
    {
      return nil
    }
  }

  private init() {}
}
