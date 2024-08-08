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
        VStack(spacing: 0)
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

          HStack(spacing: 0)
          {
            Kraken.UI.CodeEditor(
              document: $usdFile,
              stage: $stage,
              fileURL: usdFile.fileURL
            )

            Divider()

            Kraken.UI.SceneView(
              fileURL: usdFile.fileURL ?? Kraken.IO.Stage.manager.getTmpURL()
            )
          }
        }
      }
    }
  }

  /* --- xxx --- */
}
