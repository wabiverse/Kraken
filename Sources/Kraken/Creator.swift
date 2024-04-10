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
import KrakenKit
import KrakenLib
import KrakenUI
import PixarUSD
#if canImport(PyBundle)
  import PyBundle
  import Python
#endif /* canImport(PyBundle) */
import SwiftUI
#if canImport(GtkBackend)
  import GtkBackend
#endif /* canImport(GtkBackend) */

@main
struct Kraken: App
{
  #if canImport(GtkBackend)
    typealias Backend = GtkBackend
  #endif /* canImport(GtkBackend) */

  static let identifier = "foundation.wabi.Kraken"
  static let version = Pixar.GfVec3i(1, 0, 7)

  init()
  {
    /* setup usd plugins & resources. */
    Pixar.Bundler.shared.setup(.resources)

    /* embed & init python. */
    PyBundler.shared.pyInit()
    PyBundler.shared.pyInfo()

    /* -------------------------------------------------------- */

    let stage = Usd.Stage.createNew("ExampleScene", ext: .usda)

    UsdGeom.Xform.define(stage, path: "/Hello")
    UsdGeom.Sphere.define(stage, path: "/Hello/World")

    stage.save()

    /* -------------------------------------------------------- */

    /* hello metaverse. */
    UsdStage("Kraken", ext: .usda)
    {
      // 👋.
      UsdPrim("Hello", type: .xform)
      {
        // 🌌.
        UsdPrim("Metaverse", type: .xform)
        {
          // 🦑.
          UsdPrim("Kraken", type: .cylinder)
        }
      }
    }
    .set(doc: "Kraken v\(versionStr)")
    .save()

    Msg.logger.log(level: .info, "Kraken launched.")
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

extension Kraken
{
  var versionStr: String
  {
    let v = ".".join(array: Kraken.version)

    Msg.logger.log(level: .info, "\("Kraken".magenta) \("v".yellow)\(v.yellow) | \("PixarUSD".magenta) \("v".yellow)\(Pixar.version.yellow)")

    return v
  }
}

/* --- xxx --- */
