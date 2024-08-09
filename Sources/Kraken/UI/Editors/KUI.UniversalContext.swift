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

import CxxStdlib
import Foundation
import KrakenKit
import KrakenLib
import PixarUSD
import SwiftUI

public extension Kraken.UI
{
  /**
   * The Kraken Universal Scene Description Context
   *
   * Represents the root of the UX/UI hierarchy of
   * its data-driven and highly collaborative suite
   * of digital content creation tools. */
  struct UniversalContext: View
  {
    /** The kraken universal scene description context. */
    @Environment(Kraken.IO.USD.self) private var C

    /** The users universal scene description context. */
    @Bindable var context: Kraken.IO.USD

    public var body: some View
    {
      HStack(spacing: 0)
      {
        Kraken.UI.CodeEditor(C: context)

        Divider()

        Kraken.UI.SceneView(C: context)
      }
    }
  }
}
