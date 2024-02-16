/* ----------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                            ::
 * ----------------------------------------------------------------
 * This software is Licensed under the terms of the Apache License,
 * version 2.0 (the "Apache License") with the following additional
 * modification; you may not use this file except within compliance
 * of the Apache License and the following modification made to it.
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * Trademarks. This License does not grant permission to use any of
 * its trade names, trademarks, service marks, or the product names
 * of this Licensor or its affiliates, except as required to comply
 * with Section 4(c.) of this License, and to reproduce the content
 * of the NOTICE file.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND without even an
 * implied warranty of MERCHANTABILITY, or FITNESS FOR A PARTICULAR
 * PURPOSE. See the Apache License for more details.
 *
 * You should have received a copy for this software license of the
 * Apache License along with this program; or, if not, please write
 * to the Free Software Foundation Inc., with the following address
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *         Copyright (C) 2024 Wabi Foundation. All Rights Reserved.
 * ----------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ---------------------------------------------------------------- */

import Foundation
import KrakenKit
import KrakenUI
import OCIOBundle
import OpenColorIO
import OpenImageIO
import PixarUSD
import PyBundle
import Python
import SwiftUI

/* --- xxx --- */

public typealias OCIO = OpenColorIO_v2_3
public typealias OIIO = OpenImageIO_v2_5

/* --- xxx --- */

@main
struct Kraken: App
{
  static let version = Pixar.Gf.Vec3i(1, 0, 5)

  init()
  {
    /* setup usd plugins & resources. */
    Pixar.Bundler.shared.setup(.resources)

    /* embed & init python. */
    PyBundler.shared.pyInit()
    PyBundler.shared.pyInfo()

    /* -------------------------------------------------------- */

    /* test some opencolorio api. */
    OCIOBundler.shared.ocioInit(config: .aces)
    OCIO.GetCurrentConfig()

    /* test some openimageio api. */
    let dtp = OIIO.TypeDesc.TypeFloat
    let fmt = OIIO.ImageSpec(512, 89, 4, dtp)

    var fg = OIIO.ImageBuf(.init(std.string("fg.exr")), fmt, OIIO.InitializePixels.Yes)
    let bg = OIIO.ImageBuf(.init(std.string("bg.exr")), fmt, OIIO.InitializePixels.Yes)

    fg.set_origin(512, 89, 0)

    let comp = OIIO.ImageBufAlgo.over(fg, bg, fmt.roi(), 0)
    if !comp.has_error()
    {
      if comp.write(.init(std.string("composite.exr")), OIIO.TypeDesc(.init("UNKNOWN")), .init(""), nil, nil) == false || comp.has_error()
      {
        Msg.logger.log(level: .info, "Error writing image: \(comp.geterror(true))")
      }
    }
    else
    {
      Msg.logger.log(level: .info, "Error writing image: \(comp.geterror(true))")
    }

    /* -------------------------------------------------------- */

    /* test some usd symbols, ensure @_spi errors are gone. */
    let stage = Pixar.Usd.Stage.createNew("ExampleScene.usda")

    Pixar.UsdGeom.Xform.define(stage, path: "/Hello")
    Pixar.UsdGeom.Sphere.define(stage, path: "/Hello/World")

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
    let v = Kraken.version.getArray().map
    {
      String($0)
    }
    .joined(separator: ".")

    Msg.logger.log(level: .info, "\("Kraken".magenta) \("v".yellow)\(v.yellow)")

    return v
  }
}

/* --- xxx --- */
