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
import SwiftUI

public extension Editor.Code
{
  /// A collection of `NSColor` used for syntax higlighting
  struct Theme
  {
    public var text: NSColor
    public var insertionPoint: NSColor
    public var invisibles: NSColor
    public var background: NSColor
    public var lineHighlight: NSColor
    public var selection: NSColor
    public var keywords: NSColor
    public var commands: NSColor
    public var types: NSColor
    public var attributes: NSColor
    public var variables: NSColor
    public var values: NSColor
    public var numbers: NSColor
    public var strings: NSColor
    public var characters: NSColor
    public var comments: NSColor

    public init(
      text: NSColor,
      insertionPoint: NSColor,
      invisibles: NSColor,
      background: NSColor,
      lineHighlight: NSColor,
      selection: NSColor,
      keywords: NSColor,
      commands: NSColor,
      types: NSColor,
      attributes: NSColor,
      variables: NSColor,
      values: NSColor,
      numbers: NSColor,
      strings: NSColor,
      characters: NSColor,
      comments: NSColor
    )
    {
      self.text = text
      self.insertionPoint = insertionPoint
      self.invisibles = invisibles
      self.background = background
      self.lineHighlight = lineHighlight
      self.selection = selection
      self.keywords = keywords
      self.commands = commands
      self.types = types
      self.attributes = attributes
      self.variables = variables
      self.values = values
      self.numbers = numbers
      self.strings = strings
      self.characters = characters
      self.comments = comments
    }

    /// Get the color from ``theme`` for the specified capture name.
    /// - Parameter capture: The capture name
    /// - Returns: A `NSColor`
    func colorFor(_ capture: CaptureName?) -> NSColor
    {
      switch capture
      {
        case .include, .constructor, .keyword, .boolean, .variableBuiltin,
             .keywordReturn, .keywordFunction, .repeat, .conditional, .tag:
          keywords
        case .comment: comments
        case .variable: variables
        case .property: commands
        case .function, .method: commands
        case .number, .float: numbers
        case .string: strings
        case .type: types
        case .parameter: attributes
        case .typeAlternate: attributes
        case .namespace: numbers
        case .typeBuiltin, .functionBuiltin: characters
        case .textUri: strings
        case .stringSpecial: attributes
        default: text
      }
    }
  }
}

extension Editor.Code.Theme: Equatable
{
  public static func == (lhs: Self, rhs: Self) -> Bool
  {
    lhs.text == rhs.text &&
      lhs.insertionPoint == rhs.insertionPoint &&
      lhs.invisibles == rhs.invisibles &&
      lhs.background == rhs.background &&
      lhs.lineHighlight == rhs.lineHighlight &&
      lhs.selection == rhs.selection &&
      lhs.keywords == rhs.keywords &&
      lhs.commands == rhs.commands &&
      lhs.types == rhs.types &&
      lhs.attributes == rhs.attributes &&
      lhs.variables == rhs.variables &&
      lhs.values == rhs.values &&
      lhs.numbers == rhs.numbers &&
      lhs.strings == rhs.strings &&
      lhs.characters == rhs.characters &&
      lhs.comments == rhs.comments
  }
}
