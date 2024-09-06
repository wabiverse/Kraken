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

import SwiftTreeSitter
import XCTest
@testable import CodeLanguages
@testable import CosmoEditor

final class CodeLanguagesTests: XCTestCase
{
  let bundleURL = Bundle.module.resourceURL

  // MARK: - C

  func test_CodeLanguageC() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.c")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .c)
  }

  func test_CodeLanguageC2() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.h")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .c)
  }

  func test_FetchQueryC() throws
  {
    var language = Editor.Code.Language.c
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - C++

  func test_CodeLanguageCPP() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.cc")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .cpp)
  }

  func test_CodeLanguageCPP2() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.cpp")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .cpp)
  }

  func test_CodeLanguageCPP3() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.hpp")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .cpp)
  }

  func test_FetchQueryCPP() throws
  {
    var language = Editor.Code.Language.cpp
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - Galah

  func test_CodeLanguageGalah() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.galah")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .galah)
  }

  func test_FetchQueryGalah() throws
  {
    var language = Editor.Code.Language.galah
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - JSDoc

  func test_FetchQueryJSDoc() throws
  {
    var language = Editor.Code.Language.jsdoc
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - JSON

  func test_CodeLanguageJSON() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.json")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .json)
  }

  func test_FetchQueryJSON() throws
  {
    var language = Editor.Code.Language.json
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - Python

  func test_CodeLanguagePython() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.py")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .python)
  }

  func test_FetchQueryPython() throws
  {
    var language = Editor.Code.Language.python
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - Rust

  func test_CodeLanguageRust() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.rs")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .rust)
  }

  func test_FetchQueryRust() throws
  {
    var language = Editor.Code.Language.rust
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - Swift

  func test_CodeLanguageSwift() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.swift")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .swift)
  }

  func test_FetchQuerySwift() throws
  {
    var language = Editor.Code.Language.swift
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - TOML

  func test_CodeLanguageTOML() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.toml")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .toml)
  }

  func test_FetchQueryTOML() throws
  {
    var language = Editor.Code.Language.toml
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - USD

  func test_CodeLanguageUSD() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.usda")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .usd)
  }

  func test_FetchQueryUSD() throws
  {
    var language = Editor.Code.Language.usd
    language.resourceURL = bundleURL

    let data = try Data(contentsOf: language.queryURL!)
    let query = try? Query(language: language.language!, data: data)
    XCTAssertNotNil(query)
    XCTAssertNotEqual(query?.patternCount, 0)
  }

  // MARK: - Unsupported

  func test_CodeLanguageUnsupported() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file.abc")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .plainText)
  }

  func test_CodeLanguageUnsupportedNoExtension() throws
  {
    let url = URL(fileURLWithPath: "~/path/to/file")
    let language = Editor.Code.Language.detectLanguageFrom(url: url)

    XCTAssertEqual(language.id, .plainText)
  }
}
