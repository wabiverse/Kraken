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

import SwiftUI

public extension Kraken.UI
{
  /**
   * Mica windows are subtly tinted with
   * the user's desktop background color,
   * and have a subtle blur effect.
   */
  struct MicaWindow<ContentItems: View>: Scene
  {
    public init(title: String,
                id: String,
                @ViewBuilder view: @escaping (Kraken.IO.USD) -> ContentItems)
    {
      self.title = title
      self.id = id
      self.view = view
    }

    public let title: String
    public let id: String
    public var view: (Kraken.IO.USD) -> ContentItems

    public var body: some Scene
    {
      DocumentGroup(newDocument: { Kraken.IO.USD() })
      { file in
        ZStack
        {
          Color.clear.ignoresSafeArea()

          view(file.document)
        }
        .background(Kraken.UI.MicaMaterial(material: .sidebar).ignoresSafeArea())
        .frame(minWidth: 800, maxWidth: .infinity, minHeight: 500, maxHeight: .infinity)
        .onAppear
        {
          if let url = file.fileURL
          {
            Task
            {
              file.document.context.open(fileURL: url)
            }
          }
        }
      }
      #if os(macOS)
      .windowStyle(.hiddenTitleBar)
      #endif /* os(macOS) */
    }
  }
}
