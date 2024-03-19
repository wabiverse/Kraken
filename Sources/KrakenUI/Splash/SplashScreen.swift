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

/**
 * A view that displays a splash screen,
 * consisting of a featured image, a logo
 * badge overlay, a welcome title, and a
 * menu stack of introductory options.
 */
public struct SplashScreen: View
{
  public init(image: String, logo: String, title: String)
  {
    self.image = image
    self.logo = logo
    self.title = title
  }

  public let image: String
  public let logo: String
  public let title: String

  public var body: some View
  {
    VStack
    {
      SplashFeature(image: image, logo: logo)

      SplashTitle(title: title)
        .padding(.top, 7)

      IntroStack()
        .padding()

      Spacer()
    }
    .frame(width: 400, height: 400)
    .background(.quaternary)
    .clipShape(RoundedRectangle(cornerRadius: 12, style: .continuous))
    .padding()
  }
}
