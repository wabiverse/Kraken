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

import AppKit
import CodeView

extension TextViewController
{
  /// Method called when CMD + / key sequence recognized, comments cursor's current line of code
  public func commandSlashCalled()
  {
    guard let cursorPosition = cursorPositions.first
    else
    {
      print("There is no cursor \(#function)")
      return
    }
    // Many languages require a character sequence at the beginning of the line to comment the line.
    // (ex. python #, C++ //)
    // If such a sequence exists, we will insert that sequence at the beginning of the line
    if !language.lineCommentString.isEmpty
    {
      toggleCharsAtBeginningOfLine(chars: language.lineCommentString, lineNumber: cursorPosition.line)
    }
    // In other cases, languages require a character sequence at beginning and end of a line, aka a range comment
    // (Ex. HTML <!--line here -->)
    // We treat the line as a one-line range to comment it out using rangeCommentStrings on both sides of the line
    else
    {
      let (openComment, closeComment) = language.rangeCommentStrings
      toggleCharsAtEndOfLine(chars: closeComment, lineNumber: cursorPosition.line)
      toggleCharsAtBeginningOfLine(chars: openComment, lineNumber: cursorPosition.line)
    }
  }

  ///  Toggles comment string at the beginning of a specified line (lineNumber is 1-indexed)
  private func toggleCharsAtBeginningOfLine(chars: String, lineNumber: Int)
  {
    guard let lineInfo = textView.layoutManager.textLineForIndex(lineNumber - 1)
    else
    {
      print("There are no characters/lineInfo \(#function)")
      return
    }
    guard let lineString = textView.textStorage.substring(from: lineInfo.range)
    else
    {
      print("There are no characters/lineString \(#function)")
      return
    }
    let firstNonWhiteSpaceCharIndex = lineString.firstIndex(where: { !$0.isWhitespace }) ?? lineString.startIndex
    let numWhitespaceChars = lineString.distance(from: lineString.startIndex, to: firstNonWhiteSpaceCharIndex)
    let firstCharsInLine = lineString.suffix(from: firstNonWhiteSpaceCharIndex).prefix(chars.count)
    // toggle comment off
    if firstCharsInLine == chars
    {
      textView.replaceCharacters(in: NSRange(
        location: lineInfo.range.location + numWhitespaceChars,
        length: chars.count
      ), with: "")
    }
    // toggle comment on
    else
    {
      textView.replaceCharacters(in: NSRange(
        location: lineInfo.range.location + numWhitespaceChars,
        length: 0
      ), with: chars)
    }
  }

  ///  Toggles a specific string of characters at the end of a specified line. (lineNumber is 1-indexed)
  private func toggleCharsAtEndOfLine(chars: String, lineNumber: Int)
  {
    guard let lineInfo = textView.layoutManager.textLineForIndex(lineNumber - 1)
    else
    {
      print("There are no characters/lineInfo \(#function)")
      return
    }
    guard let lineString = textView.textStorage.substring(from: lineInfo.range)
    else
    {
      print("There are no characters/lineString \(#function)")
      return
    }
    let lineLastCharIndex = lineInfo.range.location + lineInfo.range.length - 1
    let closeCommentLength = chars.count
    let closeCommentRange = NSRange(
      location: lineLastCharIndex - closeCommentLength,
      length: closeCommentLength
    )
    let lastCharsInLine = textView.textStorage.substring(from: closeCommentRange)
    // toggle comment off
    if lastCharsInLine == chars
    {
      textView.replaceCharacters(in: NSRange(
        location: lineLastCharIndex - closeCommentLength,
        length: closeCommentLength
      ), with: "")
    }
    // toggle comment on
    else
    {
      textView.replaceCharacters(in: NSRange(location: lineLastCharIndex, length: 0), with: chars)
    }
  }
}
