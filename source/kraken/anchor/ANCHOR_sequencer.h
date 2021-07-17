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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include <cstddef>

struct AnchorDrawList;
struct AnchorRect;
namespace ImSequencer
{
   enum SEQUENCER_OPTIONS
   {
      SEQUENCER_EDIT_NONE = 0,
      SEQUENCER_EDIT_STARTEND = 1 << 1,
      SEQUENCER_CHANGE_FRAME = 1 << 3,
      SEQUENCER_ADD = 1 << 4,
      SEQUENCER_DEL = 1 << 5,
      SEQUENCER_COPYPASTE = 1 << 6,
      SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
   };

   struct SequenceInterface
   {
      bool focused = false;
      virtual int GetFrameMin() const = 0;
      virtual int GetFrameMax() const = 0;
      virtual int GetItemCount() const = 0;

      virtual void BeginEdit(int) {}
      virtual void EndEdit() {}
      virtual int GetItemTypeCount() const { return 0; }
      virtual const char* GetItemTypeName(int) const { return ""; }
      virtual const char* GetItemLabel(int) const { return ""; }

      virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) = 0;
      virtual void Add(int) {}
      virtual void Del(int) {}
      virtual void Duplicate(int) {}

      virtual void Copy() {}
      virtual void Paste() {}

      virtual size_t GetCustomHeight(int) { return 0; }
      virtual void DoubleClick(int) {}
      virtual void CustomDraw(int, AnchorDrawList*, const AnchorRect&, const AnchorRect&, const AnchorRect&, const AnchorRect&) {}
      virtual void CustomDrawCompact(int, AnchorDrawList*, const AnchorRect&, const AnchorRect&) {}
   };


   // return true if selection is made
   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions);

}
