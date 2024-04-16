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
import Foundation

class CursorTimer
{
  // # Properties

  /// The timer that publishes the cursor toggle timer.
  private var timer: Timer?
  /// Maps to all cursor views, uses weak memory to not cause a strong reference cycle.
  private var cursors: NSHashTable<CursorView> = .init(options: .weakMemory)
  /// Tracks whether cursors are hidden or not.
  var shouldHide: Bool = false

  // MARK: - Methods

  /// Resets the cursor blink timer.
  /// - Parameter newBlinkDuration: The duration to blink, leave as nil to never blink.
  func resetTimer(newBlinkDuration: TimeInterval? = 0.5)
  {
    timer?.invalidate()

    guard let newBlinkDuration
    else
    {
      notifyCursors(shouldHide: true)
      return
    }

    shouldHide = false
    notifyCursors(shouldHide: shouldHide)

    timer = Timer.scheduledTimer(withTimeInterval: newBlinkDuration, repeats: true)
    { [weak self] _ in
      self?.assertMain()
      self?.shouldHide.toggle()
      guard let shouldHide = self?.shouldHide else { return }
      self?.notifyCursors(shouldHide: shouldHide)
    }
  }

  func stopTimer()
  {
    shouldHide = true
    notifyCursors(shouldHide: true)
    cursors.removeAllObjects()
    timer?.invalidate()
    timer = nil
  }

  /// Notify all cursors of a new blink state.
  /// - Parameter shouldHide: Whether or not the cursors should be hidden or not.
  private func notifyCursors(shouldHide: Bool)
  {
    for cursor in cursors.allObjects
    {
      cursor.blinkTimer(shouldHide)
    }
  }

  /// Register a new cursor view with the timer.
  /// - Parameter newCursor: The cursor to blink.
  func register(_ newCursor: CursorView)
  {
    cursors.add(newCursor)
  }

  deinit
  {
    timer?.invalidate()
    timer = nil
    cursors.removeAllObjects()
  }

  private func assertMain()
  {
    #if DEBUG
      assert(Thread.isMainThread, "CursorTimer used from non-main thread")
    #endif
  }
}
