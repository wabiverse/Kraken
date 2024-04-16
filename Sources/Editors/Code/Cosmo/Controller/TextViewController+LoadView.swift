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
import CodeView

public extension TextViewController
{
  // swiftlint:disable:next function_body_length
  override func loadView()
  {
    scrollView = NSScrollView()
    textView.postsFrameChangedNotifications = true
    textView.translatesAutoresizingMaskIntoConstraints = false

    scrollView.translatesAutoresizingMaskIntoConstraints = false
    scrollView.contentView.postsFrameChangedNotifications = true
    scrollView.hasVerticalScroller = true
    scrollView.hasHorizontalScroller = true
    scrollView.documentView = textView
    scrollView.contentView.postsBoundsChangedNotifications = true

    gutterView = GutterView(
      font: font.rulerFont,
      textColor: .secondaryLabelColor,
      textView: textView,
      delegate: self
    )
    gutterView.updateWidthIfNeeded()
    scrollView.addFloatingSubview(
      gutterView,
      for: .horizontal
    )

    view = scrollView
    if let _undoManager
    {
      textView.setUndoManager(_undoManager)
    }

    styleTextView()
    styleScrollView()
    styleGutterView()
    setUpHighlighter()
    setUpTextFormation()

    NSLayoutConstraint.activate([
      scrollView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
      scrollView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
      scrollView.topAnchor.constraint(equalTo: view.topAnchor),
      scrollView.bottomAnchor.constraint(equalTo: view.bottomAnchor)
    ])

    if !cursorPositions.isEmpty
    {
      setCursorPositions(cursorPositions)
    }

    // Layout on scroll change
    NotificationCenter.default.addObserver(
      forName: NSView.boundsDidChangeNotification,
      object: scrollView.contentView,
      queue: .main
    )
    { [weak self] _ in
      self?.textView.updatedViewport(self?.scrollView.documentVisibleRect ?? .zero)
      self?.gutterView.needsDisplay = true
    }

    // Layout on frame change
    NotificationCenter.default.addObserver(
      forName: NSView.frameDidChangeNotification,
      object: scrollView.contentView,
      queue: .main
    )
    { [weak self] _ in
      self?.textView.updatedViewport(self?.scrollView.documentVisibleRect ?? .zero)
      self?.gutterView.needsDisplay = true
      if self?.bracketPairHighlight == .flash
      {
        self?.removeHighlightLayers()
      }
    }

    NotificationCenter.default.addObserver(
      forName: NSView.frameDidChangeNotification,
      object: textView,
      queue: .main
    )
    { [weak self] _ in
      self?.gutterView.frame.size.height = (self?.textView.frame.height ?? 0) + 10
      self?.gutterView.needsDisplay = true
    }

    NotificationCenter.default.addObserver(
      forName: TextSelectionManager.selectionChangedNotification,
      object: textView.selectionManager,
      queue: .main
    )
    { [weak self] _ in
      self?.updateCursorPosition()
      self?.highlightSelectionPairs()
    }

    textView.updateFrameIfNeeded()

    NSApp.publisher(for: \.effectiveAppearance)
      .receive(on: RunLoop.main)
      .sink
      { [weak self] newValue in
        guard let self else { return }

        if systemAppearance != newValue.name
        {
          systemAppearance = newValue.name
        }
      }
      .store(in: &cancellables)
  }
}
