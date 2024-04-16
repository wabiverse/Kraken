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

// swiftlint:disable file_length

//
//  Editor.Code.Language+Definitions.swift
//
//
//  Created by Lukas Pistrol on 15.01.23.
//

import Foundation

public extension Editor.Code.Language
{
  /// An array of all language structures.
  static let allLanguages: [Editor.Code.Language] = [
    .c,
    .cpp,
    .json,
    .python,
    .rust,
    .swift,
    .toml,
    .usd,
  ]

  /// A language structure for `C`
  static let c: Editor.Code.Language = .init(
    id: .c,
    tsName: "c",
    extensions: ["c", "h"],
    lineCommentString: "//",
    rangeCommentStrings: ("/*", "*/")
  )

  /// A language structure for `C++`
  static let cpp: Editor.Code.Language = .init(
    id: .cpp,
    tsName: "cpp",
    extensions: ["cc", "cpp", "c++", "hpp", "h"],
    lineCommentString: Editor.Code.Language.c.lineCommentString,
    rangeCommentStrings: Editor.Code.Language.c.rangeCommentStrings,
    documentationCommentStrings: [.pair(("/**", "*/"))],
    parentURL: Editor.Code.Language.c.queryURL,
    highlights: ["injections"]
  )

  /// A language structure for `JSON`
  static let json: Editor.Code.Language = .init(
    id: .json,
    tsName: "json",
    extensions: ["json"],
    lineCommentString: "//",
    rangeCommentStrings: ("/*", "*/")
  )

  /// A language structure for `Python`
  static let python: Editor.Code.Language = .init(
    id: .python,
    tsName: "python",
    extensions: ["py"],
    lineCommentString: "#",
    rangeCommentStrings: ("", ""),
    documentationCommentStrings: [.pair(("\"\"\"", "\"\"\""))],
    additionalIdentifiers: ["python2", "python3"]
  )

  /// A language structure for `Rust`
  static let rust: Editor.Code.Language = .init(
    id: .rust,
    tsName: "rust",
    extensions: ["rs"],
    lineCommentString: "//",
    rangeCommentStrings: ("/*", "*/"),
    documentationCommentStrings: [
      .single("///"),
      .single("//!"),
      .pair(("/**", "*/")),
      .pair(("/*!", "*/"))
    ],
    highlights: ["injections"]
  )

  /// A language structure for `Swift`
  static let swift: Editor.Code.Language = .init(
    id: .swift,
    tsName: "swift",
    extensions: ["swift"],
    lineCommentString: "//",
    rangeCommentStrings: ("/*", "*/"),
    documentationCommentStrings: [.single("///"), .pair(("/**", "*/"))]
  )

  /// A language structure for `TOML`
  static let toml: Editor.Code.Language = .init(
    id: .toml,
    tsName: "toml",
    extensions: ["toml"],
    lineCommentString: "#",
    rangeCommentStrings: ("", "")
  )

  /// A language structure for `USD`
  static let usd: Editor.Code.Language = .init(
    id: .usd,
    tsName: "usd",
    extensions: ["usda"],
    lineCommentString: "#",
    rangeCommentStrings: ("", ""),
    documentationCommentStrings: [.pair(("\"", "\""))]
  )

  /// The default language (plain text)
  static let `default`: Editor.Code.Language = .init(
    id: .plainText,
    tsName: "PlainText",
    extensions: ["txt"],
    lineCommentString: "",
    rangeCommentStrings: ("", "")
  )
}

// swiftlint:enable file_length
