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

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ImGradient.h"
#include "ANCHOR_api.h"
#include "imgui_internal.h"

namespace ImGradient
{
#ifndef ANCHOR_DEFINE_MATH_OPERATORS
   static inline GfVec2f operator*(const GfVec2f& lhs, const float rhs) { return GfVec2f(lhs.x * rhs, lhs.y * rhs); }
   static inline GfVec2f operator/(const GfVec2f& lhs, const float rhs) { return GfVec2f(lhs.x / rhs, lhs.y / rhs); }
   static inline GfVec2f operator+(const GfVec2f& lhs, const GfVec2f& rhs) { return GfVec2f(lhs.x + rhs.x, lhs.y + rhs.y); }
   static inline GfVec2f operator-(const GfVec2f& lhs, const GfVec2f& rhs) { return GfVec2f(lhs.x - rhs.x, lhs.y - rhs.y); }
   static inline GfVec2f operator*(const GfVec2f& lhs, const GfVec2f& rhs) { return GfVec2f(lhs.x * rhs.x, lhs.y * rhs.y); }
   static inline GfVec2f operator/(const GfVec2f& lhs, const GfVec2f& rhs) { return GfVec2f(lhs.x / rhs.x, lhs.y / rhs.y); }
#endif

   static int DrawPoint(AnchorDrawList* draw_list, GfVec4f color, const GfVec2f size, bool editing, GfVec2f pos)
   {
      AnchorIO& io = Anchor::GetIO();

      GfVec2f p1 = AnchorLerp(pos, GfVec2f(pos + GfVec2f(size.x - size.y, 0.f)), color.w) + GfVec2f(3, 3);
      GfVec2f p2 = AnchorLerp(pos + GfVec2f(size.y, size.y), GfVec2f(pos + size), color.w) - GfVec2f(3, 3);
      AnchorRect rc(p1, p2);

      color.w = 1.f;
      draw_list->AddRectFilled(p1, p2, ImColor(color));
      if (editing)
         draw_list->AddRect(p1, p2, 0xFFFFFFFF, 2.f, 15, 2.5f);
      else
         draw_list->AddRect(p1, p2, 0x80FFFFFF, 2.f, 15, 1.25f);

      if (rc.Contains(io.MousePos))
      {
         if (io.MouseClicked[0])
            return 2;
         return 1;
      }
      return 0;
   }

   bool Edit(Delegate& delegate, const GfVec2f& size, int& selection)
   {
      bool ret = false;
      AnchorIO& io = Anchor::GetIO();
      Anchor::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(0, 0));
      Anchor::BeginChildFrame(137, size);

      AnchorDrawList* draw_list = Anchor::GetWindowDrawList();
      const GfVec2f offset = Anchor::GetCursorScreenPos();

      const GfVec4f* pts = delegate.GetPoints();
      static int currentSelection = -1;
      static int movingPt = -1;
      if (currentSelection >= int(delegate.GetPointCount()))
         currentSelection = -1;
      if (movingPt != -1)
      {
         GfVec4f current = pts[movingPt];
         current.w += io.MouseDelta.x / size.x;
         current.w = AnchorClamp(current.w, 0.f, 1.f);
         delegate.EditPoint(movingPt, current);
         ret = true;
         if (!io.MouseDown[0])
            movingPt = -1;
      }
      for (size_t i = 0; i < delegate.GetPointCount(); i++)
      {
         int ptSel = DrawPoint(draw_list, pts[i], size, i == currentSelection, offset);
         if (ptSel == 2)
         {
            currentSelection = int(i);
            ret = true;
         }
         if (ptSel == 1 && io.MouseDown[0] && movingPt == -1)
         {
            movingPt = int(i);
         }
      }
      AnchorRect rc(offset, offset + size);
      if (rc.Contains(io.MousePos) && io.MouseDoubleClicked[0])
      {
         float t = (io.MousePos.x - offset.x) / size.x;
         delegate.AddPoint(delegate.GetPoint(t));
         ret = true;
      }
      Anchor::EndChildFrame();
      Anchor::PopStyleVar();

      selection = currentSelection;
      return ret;
   }
}
