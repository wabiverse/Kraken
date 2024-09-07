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
import MetaversePlugin
import PixarUSD

public extension Kraken
{
  enum Plugin
  {
    public typealias InitFunction = @convention(c) () -> UnsafeMutableRawPointer

    public static func load(at path: String) -> MetaversePlugin
    {
      let openRes = dlopen(path, RTLD_NOW | RTLD_LOCAL)
      if openRes != nil
      {
        defer
        {
          dlclose(openRes)
        }

        let symbolName = "createPlugin"
        let sym = dlsym(openRes, symbolName)

        if sym != nil
        {
          let f: InitFunction = unsafeBitCast(sym, to: InitFunction.self)
          let pluginPointer = f()
          let builder = Unmanaged<MetaversePluginBuilder>.fromOpaque(pluginPointer).takeRetainedValue()
          return builder.build()
        }
        else
        {
          fatalError("error loading lib: symbol \(symbolName) not found, path: \(path)")
        }
      }
      else
      {
        if let err = dlerror()
        {
          fatalError("error opening lib: \(String(format: "%s", err)), path: \(path)")
        }
        else
        {
          fatalError("error opening lib: unknown error, path: \(path)")
        }
      }
    }

    public static func info(at path: String)
    {
      let plugin = Kraken.Plugin.load(at: path)
      let name = plugin.name

      Msg.logger.info("Loaded plugin { name: \(name), path: \(path) }")
    }
  }
}
