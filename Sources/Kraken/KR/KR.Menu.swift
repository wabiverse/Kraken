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
import KrakenLib
import SwiftUI

public extension Kraken
{
  @ViewBuilder
  func createMainMenu() -> some View
  {
    Divider()

    HStack(spacing: 0)
    {
      Button
      {} label:
      {
        Image("wabi.hexagon.fill", bundle: .kraken)
      }
      .buttonStyle(.borderless)
      .font(.system(size: 12, weight: .medium))
      .padding(.init(top: 4, leading: 12, bottom: 4, trailing: 12))
      .foregroundStyle(.white.opacity(0.7))
      .background(RoundedRectangle(cornerRadius: 4, style: .continuous).fill(.clear))

      Button("File")
      {}
      .buttonStyle(.borderless)
      .font(.system(size: 12, weight: .medium))
      .padding(.init(top: 4, leading: 12, bottom: 4, trailing: 12))
      .foregroundStyle(.white.opacity(0.7))
      .background(RoundedRectangle(cornerRadius: 4, style: .continuous).fill(.clear))

      Button("Edit")
      {}
      .buttonStyle(.borderless)
      .font(.system(size: 12, weight: .medium))
      .padding(.init(top: 4, leading: 12, bottom: 4, trailing: 12))
      .foregroundStyle(.white.opacity(0.7))
      .background(RoundedRectangle(cornerRadius: 4, style: .continuous).fill(.clear))

      Button("Render")
      {}
      .buttonStyle(.borderless)
      .font(.system(size: 12, weight: .medium))
      .padding(.init(top: 4, leading: 12, bottom: 4, trailing: 12))
      .foregroundStyle(.white.opacity(0.7))
      .background(RoundedRectangle(cornerRadius: 4, style: .continuous).fill(.clear))

      Button("Window")
      {}
      .buttonStyle(.borderless)
      .font(.system(size: 12, weight: .medium))
      .padding(.init(top: 4, leading: 12, bottom: 4, trailing: 12))
      .foregroundStyle(.white.opacity(0.7))
      .background(RoundedRectangle(cornerRadius: 4, style: .continuous).fill(.clear))

      Spacer()
    }
    .frame(height: 17)
    .padding(4)
    .zIndex(2)
    .background(.black.opacity(0.3))

    Divider()
  }
}
