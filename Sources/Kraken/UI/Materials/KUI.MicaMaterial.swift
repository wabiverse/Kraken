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
   * # Mica Material
   *
   * Mica material is a subtle tinted
   * background, depending on the backdrop.
   *
   * Mica adds a subtle blur effect, to
   * make the window appear to be floating
   * above the backdrop.
   */
  struct MicaMaterial: NSViewRepresentable
  {
    public let material: NSVisualEffectView.Material

    public func makeNSView(context _: Context) -> NSVisualEffectView
    {
      let visualEffectView = NSVisualEffectView()
      visualEffectView.material = material
      visualEffectView.state = NSVisualEffectView.State.active
      return visualEffectView
    }

    public func updateNSView(_ visualEffectView: NSVisualEffectView, context _: Context)
    {
      visualEffectView.material = material
    }
  }
}
