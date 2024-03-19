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

#if os(macOS)
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
  public struct MicaMaterial: NSViewRepresentable
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
#else /* !os(macOS) */
  /**
   * # Background Style
   *
   * Does nothing on platforms that do not
   * import SwiftUI, and is provided to be
   * compatible with platforms that use SwiftUI */
  public struct BackgroundStyle
  {
    public init()
    {}
  }

  /**
   * # Rounded Rectangle
   *
   * Does nothing on platforms that do not
   * import SwiftUI, and is provided to be
   * compatible with platforms that use SwiftUI */
  public struct RoundedRectangle: Shape
  {
    public func path(in _: CGRect) -> Path
    {
      Path()
    }

    public enum Style
    {
      case continuous
    }

    public init(cornerRadius _: CGFloat, style _: Style)
    {}

    public var body: some View
    {
      EmptyView()
    }
  }

  public extension Image
  {
    init(_ name: String, bundle: Bundle?)
    {
      guard let bundle
      else { self.init(name) }

      self.init("\(bundle.resourcePath)/Assets/\(name)")
    }
  }

  /**
   * # Mica Material
   *
   * Does nothing on platforms that are not macOS,
   * and is provided to be compatible with the macOS
   * material API without having to use the if #available
   * syntax to workaround it. */
  public struct MicaMaterial
  {
    public enum MaterialTypes
    {
      case sidebar
    }

    public let material: MaterialTypes

    public func ignoresSafeArea() -> BackgroundStyle
    {
      BackgroundStyle()
    }
  }
#endif /* os(macOS) */
