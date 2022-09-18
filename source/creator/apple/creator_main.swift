/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

import Foundation
import MetalKit
import AppKit

/* Pixar USD. */
import Pixar

/* Anchor System. */
import AnchorSystem

/** 
 The main entry point for swift. 
 
 As apart of the main startup sequence in CXX, this is the entry for
 Swift. This function gets called and injects a Swift instance of the
 single underlying Anchor System, in addition to both Python and CXX.
 Meaning tools, libraries, and plugins that makeup the Kraken platform
 are language agnostic and unified by design. */
@_cdecl("CreatorMain") 
public func CreatorMain() -> UnsafeMutableRawPointer
{
  fputs("hello from swift.\n", stderr)

  var anchor: AnchorSystemApple = AnchorSystemApple()

  return withUnsafeMutablePointer(to: &anchor) { inst in
    let shared = UnsafeMutableRawPointer(inst)
    return shared
  }
}