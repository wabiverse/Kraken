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

import CodeEditSourceEditor
import Foundation

extension EditorTheme
{
  static var standard: EditorTheme
  {
    EditorTheme(
      text: .init(hex: "D9D9D9"),
      insertionPoint: .init(hex: "D9D9D9"),
      invisibles: .init(hex: "424D5B"),
      background: .init(hex: "1F1F24"),
      lineHighlight: .init(hex: "23252B"),
      selection: .init(hex: "515B70"),
      keywords: .init(hex: "FF7AB3"),
      commands: .init(hex: "67B7A4"),
      types: .init(hex: "5DD8FF"),
      attributes: .init(hex: "D0A8FF"),
      variables: .init(hex: "41A1C0"),
      values: .init(hex: "A167E6"),
      numbers: .init(hex: "D0BF69"),
      strings: .init(hex: "FC6A5D"),
      characters: .init(hex: "D0BF69"),
      comments: .init(hex: "73A74E")
    )
  }
}
