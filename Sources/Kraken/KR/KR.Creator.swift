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
import PixarUSD
#if canImport(PyBundle)
  import PyBundle
  import Python
#endif /* canImport(PyBundle) */

/**
 * this is the main entry point for Kraken,
 * it initializes the usd plugins, any/all
 * resources, and python, which is required
 * for the application to run. */
@main
public enum Creator
{
  static func main()
  {
    /* setup usd plugins & resources. */
    Pixar.Bundler.shared.setup(.resources)

    #if canImport(PyBundle)
      /* embed & init python. */
      PyBundler.shared.pyInit()
      PyBundler.shared.pyInfo()
    #endif /* canImport(PyBundle) */

    /* kraken main entry point. */
    Kraken.main()
  }
}
