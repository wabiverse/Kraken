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
import SceneKit
import SwiftUI
#if canImport(GtkBackend)
  import GtkBackend
#endif /* canImport(GtkBackend) */

public struct Kraken: SwiftUI.App
{
  #if canImport(GtkBackend)
    typealias Backend = GtkBackend
  #endif /* canImport(GtkBackend) */

  /* --- xxx --- */

  /** The bundle identifier for Kraken. */
  public static let identifier = "foundation.wabi.Kraken"

  /* --- xxx --- */

  /** Whether to show the splash screen. */
  @State public var showSplash = true

  /**  The currently opened stage. */
  @State private var stage: UsdStageRefPtr = Usd.Stage.createNew(Kraken.IO.Stage.manager.getTmpURL().path, ext: .usda)

  /** The currently opened usd file. */
  @State private var usdFile = Kraken.IO.Stage.manager.makeTmp()

  /* --- xxx --- */

  /** An action in the environment that presents a new document. */
  @Environment(\.newDocument) private var newDocument

  /* --- xxx --- */

  public init()
  {
    Kraken.IO.Stage.manager.save(&stage)

    Msg.logger.info("\(Kraken.versionInfo())")
    Msg.logger.info("Kraken launched.")
  }

  /* --- xxx --- */

  public var body: some SwiftUI.Scene
  {
    Kraken.UI.MicaWindow(title: "Kraken", id: "kraken")
    {
      if showSplash
      {
        Kraken.UI.SplashScreen(
          image: "Splash",
          logo: "wabi.hexagon.fill",
          title: "The Metaversal Creation Suite.",
          showSplash: $showSplash
        )
      }
      else
      {
        HStack
        {
          Kraken.UI.CodeEditor(
            document: $usdFile,
            stage: $stage,
            fileURL: usdFile.fileURL
          )

          VStack
          {
            SceneView(
              scene: try? SCNScene(url: usdFile.fileURL ?? Kraken.IO.Stage.manager.getTmpURL()),
              options: [
                .allowsCameraControl,
                .autoenablesDefaultLighting,
              ],
              antialiasingMode: .multisampling4X
            )
          }
          .frame(maxWidth: .infinity, alignment: .leading)
        }
      }
    }
  }

  /* --- xxx --- */
}