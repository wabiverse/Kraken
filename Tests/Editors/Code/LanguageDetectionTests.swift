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

import RegexBuilder
import XCTest
@testable import CodeLanguages

// swiftlint:disable all
final class LanguageDetectionTests: XCTestCase
{
  func test_shebang()
  {
    let validCases = [
      "#! /usr/bin/env swift",
      "#! /usr/bin/env -S swift",
      "#!/usr/bin/env swift",
      "#!     /usr/bin/env         swift",
      "#! swift",
      "#!swift",
    ]

    let invalidCases = [
      "1234",
      "import CodeLanguages",
      "",
      "\n",
      "# This is a regular comment",
      "#!",
      "#",
      "!",
    ]

    for validCase in validCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: validCase)
      XCTAssert(
        detectedLanguage == .swift,
        "Parser failed to find valid shebang language. Given \"\(validCase)\", found \(detectedLanguage), expected swift."
      )
    }

    for invalidCase in invalidCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: invalidCase)
      XCTAssert(
        detectedLanguage == .default,
        "Parser unexpectedly found valid shebang language. Given \"\(invalidCase)\", found \(detectedLanguage), expected default."
      )
    }
  }

  func test_vimModeline()
  {
    let validCases = [
      "// vim: ft=swift",
      "/* vim: ft=swift */",
      "/* vim: other=param a=b ft=swift b=d\n*/",
      "// vim: other=param a=b ft=swift b=d",
    ]

    let invalidCases = [
      "//vim:",
      "//vim",
      "// This is a normal comment",
      "/*vim",
      "/* vim",
      "/* this is also a normal comment */",
      "1234",
      "import CodeLanguages",
      "",
      "\n",
    ]

    for validCase in validCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: validCase)
      XCTAssert(
        detectedLanguage == .swift,
        "Parser failed to find valid shebang language. Given \"\(validCase)\", found \(detectedLanguage), expected swift."
      )
    }

    for invalidCase in invalidCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: invalidCase)
      XCTAssert(
        detectedLanguage == .default,
        "Parser unexpectedly found valid shebang language. Given \"\(invalidCase)\", found \(detectedLanguage), expected default."
      )
    }
  }

  func test_emacsModeline()
  {
    let validCases = [
      "-*- mode:swift -*-",
      "-*- indent-style: tabs mode: swift -*-",
      "-*-mode: swift-*-",
    ]

    let invalidCases = [
      "1234",
      "import CodeLanguages",
      "",
      "\n",
      "-*-",
      "-*- -*-",
      "-- -*-",
      "mode:swift",
    ]

    for validCase in validCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: validCase)
      XCTAssert(
        detectedLanguage == .swift,
        "Parser failed to find valid shebang language. Given \"\(validCase)\", found \(detectedLanguage), expected swift."
      )
    }

    for invalidCase in invalidCases
    {
      let detectedLanguage = Editor.Code.Language.detectLanguageFrom(url: URL(filePath: ""), prefixBuffer: invalidCase)
      XCTAssert(
        detectedLanguage == .default,
        "Parser unexpectedly found valid shebang language. Given \"\(invalidCase)\", found \(detectedLanguage), expected default."
      )
    }
  }
}

// swiftlint:enable all
