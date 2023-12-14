/* --------------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                   ::
 * --------------------------------------------------------------
 * @wabistudios :: multiverse :: kraken
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
import KrakenKit
import KrakenUI
import Pixar
import PyBundle
import Python
import SwiftUI

/* --- xxx --- */

@main
struct Kraken: App
{
  init()
  {
    /* Embed & init python. */
    PyBundle.shared.pyInit()
    PyBundle.shared.pyInfo()

    /* Using Pixar's USD (Arch) from Swift. */
    let cwd = Pixar.Arch.getCwd()
    let exePath = Pixar.Arch.getExecutablePath()
    let isMain = Pixar.Arch.isMainThread()
    let threadId = Pixar.Arch.getMainThreadId()
    let pageSize = Pixar.Arch.getPageSize()

    PXRMsg.Log.point("Current working directory", to: cwd)
    PXRMsg.Log.point("Path to running executable", to: exePath)
    PXRMsg.Log.point("Are we on the main thread?", to: isMain)
    PXRMsg.Log.point("The id of the main thread", to: threadId)
    PXRMsg.Log.point("System memory paging size", to: pageSize)

    /* Using Pixar's USD (Gf) from Swift (no namespace). */
    let vecA = GfVec2f(1, 2)
    let vecB = GfVec2f(3, 4)
    var vecC = vecA + vecB
    vecC *= 2

    PXRMsg.Log.point("The value of vecC", to: vecC)

    /* Using Pixar's USD (Gf) from Swift (Pixar.Gf namespace). */
    let pxrVecA = Pixar.Gf.Vec2f(1, 2)
    let pxrVecB = Pixar.Gf.Vec2f(3, 4)
    var pxrVecC = pxrVecA + pxrVecB
    pxrVecC *= 2

    PXRMsg.Log.point("The value of pxrVecC", to: pxrVecC)

    /* Using Pixar's USD (Js) from Swift (no namespace). */
    let jsonvalue = JsValue(true)

    PXRMsg.Log.point("The value of jsonvalue", to: jsonvalue.GetBool())

    /* Using Pixar's USD (Js) from Swift (Pixar.Js namespace). */
    let pxrValue = Pixar.Js.Value(true)

    PXRMsg.Log.point("The value of pxrValue", to: pxrValue.GetBool())

    print("Kraken launched.")
  }

  /* --- xxx --- */

  var body: some Scene
  {
    MicaWindow(title: "Kraken", id: "kraken")
    {
      SplashScreen(
        image: "Splash",
        logo: "wabi.hexagon.fill",
        title: "The Metaversal Creation Suite"
      )
    }
  }
}

/* --- xxx --- */
