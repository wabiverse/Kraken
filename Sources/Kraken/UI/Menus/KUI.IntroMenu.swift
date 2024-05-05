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
  struct IntroMenu: View
  {
    @Binding public var showSplash: Bool

    public var body: some View
    {
      VStack
      {
        Button
        {
          showSplash = false

          NSWorkspace.shared.open(URL(string: "kraken://create")!)
        }
        label:
        {
          HStack
          {
            Image(systemName: "globe.americas.fill")
            #if !os(Linux)
              .aspectRatio(contentMode: .fill)
              .foregroundStyle(.green.opacity(0.8))
              .font(.system(size: 16))
            #endif /* !os(Linux) */
              .padding(2)
              .background(RoundedRectangle(cornerRadius: 8, style: .continuous).fill(.blue.opacity(0.2)))
              .frame(width: 85, alignment: .trailing)

            Spacer()

            Text("New Project")
            #if !os(Linux)
              .fontWeight(.bold)
            #endif /* !os(Linux) */
              .foregroundStyle(.secondary)
              .frame(width: 165, alignment: .leading)
          }
          .frame(width: 250, alignment: .center)
          .padding()
          .background(RoundedRectangle(cornerRadius: 12, style: .continuous).fill(.quaternary))
        }
        #if !os(Linux)
        .buttonStyle(PlainButtonStyle())
        #endif

        Button
        {}
        label:
        {
          HStack
          {
            Image(systemName: "network")
            #if !os(Linux)
              .aspectRatio(contentMode: .fill)
            #endif /* !os(Linux) */
              .foregroundStyle(.purple.opacity(0.8))
              .font(.system(size: 16))
              .padding(2)
              .background(RoundedRectangle(cornerRadius: 8, style: .continuous).fill(.purple.opacity(0.2)))
              .frame(width: 85, alignment: .trailing)

            Spacer()

            Text("Metaverse Portal")
            #if !os(Linux)
              .fontWeight(.bold)
            #endif /* !os(Linux) */
              .foregroundStyle(.secondary)
              .frame(width: 165, alignment: .leading)
          }
          .frame(width: 250, alignment: .center)
          .padding()
          .background(RoundedRectangle(cornerRadius: 12, style: .continuous).fill(.quaternary))
        }
        #if !os(Linux)
        .buttonStyle(PlainButtonStyle())
        #endif /* !os(Linux) */
        .padding(.bottom, 4)

        Spacer()
      }
    }
  }
}
