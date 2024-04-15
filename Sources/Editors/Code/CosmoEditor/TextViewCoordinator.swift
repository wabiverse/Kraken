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

import AppKit

/// # TextViewCoordinator
///
/// A protocol that can be used to provide extra functionality to ``CosmoEditor/CosmoEditor`` while
/// avoiding some of the inefficiencies of SwiftUI.
///
public protocol TextViewCoordinator: AnyObject
{
  /// Called when an instance of ``TextViewController`` is available. Use this method to install any delegates,
  /// perform any modifications on the text view or controller, or capture the text view for later use in your app.
  ///
  /// - Parameter controller: The text controller. This is safe to keep a weak reference to, as long as it is
  ///                         dereferenced when ``TextViewCoordinator/destroy()-9nzfl`` is called.
  func prepareCoordinator(controller: TextViewController)

  /// Called when the text view's text changed.
  /// - Parameter controller: The text controller.
  func textViewDidChangeText(controller: TextViewController)

  /// Called after the text view updated it's cursor positions.
  /// - Parameter newPositions: The new positions of the cursors.
  func textViewDidChangeSelection(controller: TextViewController, newPositions: [CursorPosition])

  /// Called when the text controller is being destroyed. Use to free any necessary resources.
  func destroy()
}

/// Default implementations
public extension TextViewCoordinator
{
  func textViewDidChangeText(controller _: TextViewController) {}
  func textViewDidChangeSelection(controller _: TextViewController, newPositions _: [CursorPosition]) {}
  func destroy() {}
}
