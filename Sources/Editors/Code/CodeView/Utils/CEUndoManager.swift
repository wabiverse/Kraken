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
import TextStory

/// Maintains a history of edits applied to the editor and allows for undo/redo actions using those edits.
///
/// This object also groups edits into sequences that make for a better undo/redo editing experience such as:
/// - Breaking undo groups on newlines
/// - Grouping pasted text
///
/// If needed, the automatic undo grouping can be overridden using the `beginGrouping()` and `endGrouping()` methods.
public class CEUndoManager
{
  /// An `UndoManager` subclass that forwards relevant actions to a `CEUndoManager`.
  /// Allows for objects like `CodeView` to use the `UndoManager` API
  /// while CETV manages the undo/redo actions.
  public class DelegatedUndoManager: UndoManager
  {
    weak var parent: CEUndoManager?

    override public var isUndoing: Bool { parent?.isUndoing ?? false }
    override public var isRedoing: Bool { parent?.isRedoing ?? false }
    override public var canUndo: Bool { parent?.canUndo ?? false }
    override public var canRedo: Bool { parent?.canRedo ?? false }

    public func registerMutation(_ mutation: TextMutation)
    {
      parent?.registerMutation(mutation)
      removeAllActions()
    }

    override public func undo()
    {
      parent?.undo()
    }

    override public func redo()
    {
      parent?.redo()
    }

    override public func registerUndo(withTarget _: Any, selector _: Selector, object _: Any?)
    {
      // no-op, but just in case to save resources:
      removeAllActions()
    }
  }

  /// Represents a group of mutations that should be treated as one mutation when undoing/redoing.
  private struct UndoGroup
  {
    var mutations: [Mutation]
  }

  /// A single undo mutation.
  private struct Mutation
  {
    var mutation: TextMutation
    var inverse: TextMutation
  }

  public let manager: DelegatedUndoManager
  public private(set) var isUndoing: Bool = false
  public private(set) var isRedoing: Bool = false

  public var canUndo: Bool
  {
    !undoStack.isEmpty
  }

  public var canRedo: Bool
  {
    !redoStack.isEmpty
  }

  /// A stack of operations that can be undone.
  private var undoStack: [UndoGroup] = []
  /// A stack of operations that can be redone.
  private var redoStack: [UndoGroup] = []

  private weak var textView: CodeView?
  public private(set) var isGrouping: Bool = false
  /// True when the manager is ignoring mutations.
  private var isDisabled: Bool = false

  // MARK: - Init

  public init()
  {
    manager = DelegatedUndoManager()
    manager.parent = self
  }

  convenience init(textView: CodeView)
  {
    self.init()
    self.textView = textView
  }

  // MARK: - Undo/Redo

  /// Performs an undo operation if there is one available.
  public func undo()
  {
    guard !isDisabled, let item = undoStack.popLast(), let textView
    else
    {
      return
    }
    isUndoing = true
    NotificationCenter.default.post(name: .NSUndoManagerWillUndoChange, object: manager)
    textView.textStorage.beginEditing()
    for mutation in item.mutations.reversed()
    {
      textView.replaceCharacters(in: mutation.inverse.range, with: mutation.inverse.string)
    }
    textView.textStorage.endEditing()
    NotificationCenter.default.post(name: .NSUndoManagerDidUndoChange, object: manager)
    redoStack.append(item)
    isUndoing = false
  }

  /// Performs a redo operation if there is one available.
  public func redo()
  {
    guard !isDisabled, let item = redoStack.popLast(), let textView
    else
    {
      return
    }
    isRedoing = true
    NotificationCenter.default.post(name: .NSUndoManagerWillRedoChange, object: manager)
    textView.textStorage.beginEditing()
    for mutation in item.mutations
    {
      textView.replaceCharacters(in: mutation.mutation.range, with: mutation.mutation.string)
    }
    textView.textStorage.endEditing()
    NotificationCenter.default.post(name: .NSUndoManagerDidRedoChange, object: manager)
    undoStack.append(item)
    isRedoing = false
  }

  /// Clears the undo/redo stacks.
  public func clearStack()
  {
    undoStack.removeAll()
    redoStack.removeAll()
  }

  // MARK: - Mutations

  /// Registers a mutation into the undo stack.
  ///
  /// Calling this method while the manager is in an undo/redo operation will result in a no-op.
  /// - Parameter mutation: The mutation to register for undo/redo
  public func registerMutation(_ mutation: TextMutation)
  {
    guard let textView,
          let textStorage = textView.textStorage,
          !isUndoing,
          !isRedoing
    else
    {
      return
    }
    let newMutation = Mutation(mutation: mutation, inverse: textStorage.inverseMutation(for: mutation))
    if !undoStack.isEmpty, let lastMutation = undoStack.last?.mutations.last
    {
      if isGrouping || shouldContinueGroup(newMutation, lastMutation: lastMutation)
      {
        undoStack[undoStack.count - 1].mutations.append(newMutation)
      }
      else
      {
        undoStack.append(UndoGroup(mutations: [newMutation]))
      }
    }
    else
    {
      undoStack.append(
        UndoGroup(mutations: [newMutation])
      )
    }

    redoStack.removeAll()
  }

  // MARK: - Grouping

  /// Groups all incoming mutations.
  public func beginGrouping()
  {
    guard !isGrouping
    else
    {
      assertionFailure("UndoManager already in a group. Call `endGrouping` before this can be called.")
      return
    }
    isGrouping = true
  }

  /// Stops grouping all incoming mutations.
  public func endGrouping()
  {
    guard isGrouping
    else
    {
      assertionFailure("UndoManager not in a group. Call `beginGrouping` before this can be called.")
      return
    }
    isGrouping = false
  }

  /// Determines whether or not two mutations should be grouped.
  ///
  /// Will break group if:
  /// - Last mutation is delete and new is insert, and vice versa *(insert and delete)*.
  /// - Last mutation was not whitespace, new is whitespace *(insert)*.
  /// - New mutation is a newline *(insert and delete)*.
  /// - New mutation is not sequential with the last one *(insert and delete)*.
  ///
  /// - Parameters:
  ///   - mutation: The current mutation.
  ///   - lastMutation: The last mutation applied to the document.
  /// - Returns: Whether or not the given mutations can be grouped.
  private func shouldContinueGroup(_ mutation: Mutation, lastMutation: Mutation) -> Bool
  {
    // If last mutation was delete & new is insert or vice versa, split group
    if (mutation.mutation.range.length > 0 && lastMutation.mutation.range.length == 0)
      || (mutation.mutation.range.length == 0 && lastMutation.mutation.range.length > 0)
    {
      return false
    }

    if mutation.mutation.string.isEmpty
    {
      // Deleting
      return
        lastMutation.mutation.range.location == mutation.mutation.range.max
          && LineEnding(line: lastMutation.inverse.string) == nil
    }
    else
    {
      // Inserting

      // Only attempt this check if the mutations are small enough.
      // If the last mutation was not whitespace, and the new one is, break the group.
      if lastMutation.mutation.string.count < 1024,
         mutation.mutation.string.count < 1024,
         !lastMutation.mutation.string.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty,
         mutation.mutation.string.trimmingCharacters(in: .whitespaces).isEmpty
      {
        return false
      }

      return
        lastMutation.mutation.range.max + 1 == mutation.mutation.range.location
          && LineEnding(line: mutation.mutation.string) == nil
    }
  }

  // MARK: - Disable

  /// Sets the undo manager to ignore incoming mutations until the matching `enable` method is called.
  /// Cannot be nested.
  public func disable()
  {
    isDisabled = true
  }

  /// Sets the undo manager to begin receiving incoming mutations after a call to `disable`
  /// Cannot be nested.
  public func enable()
  {
    isDisabled = false
  }

  // MARK: - Internal

  /// Sets a new text view to use for mutation registration, undo/redo operations.
  /// - Parameter newTextView: The new text view.
  func setTextView(_ newTextView: CodeView)
  {
    textView = newTextView
  }
}
