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

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"
#include "ANCHOR_internal.h"
#include "ANCHOR_gradient.h"

KRAKEN_NAMESPACE_USING

namespace AnchorGradient
{
  static int DrawPoint(AnchorDrawList *draw_list,
                       wabi::GfVec4f color,
                       const wabi::GfVec2f size,
                       bool editing,
                       wabi::GfVec2f pos)
  {
    AnchorIO &io = ANCHOR::GetIO();

    wabi::GfVec2f p1 = AnchorLerp(pos, wabi::GfVec2f(pos + wabi::GfVec2f(size[0] - size[1], 0.f)), color[3]) +
                 wabi::GfVec2f(3, 3);
    wabi::GfVec2f p2 = AnchorLerp(pos + wabi::GfVec2f(size[1], size[1]), wabi::GfVec2f(pos + size), color[3]) -
                 wabi::GfVec2f(3, 3);
    AnchorBBox rc(p1, p2);

    color[3] = 1.f;
    draw_list->AddRectFilled(p1, p2, AnchorColor(color));
    if (editing)
      draw_list->AddRect(p1, p2, 0xFFFFFFFF, 2.f, 15, 2.5f);
    else
      draw_list->AddRect(p1, p2, 0x80FFFFFF, 2.f, 15, 1.25f);

    if (rc.Contains(io.MousePos)) {
      if (io.MouseClicked[0])
        return 2;
      return 1;
    }
    return 0;
  }

  bool Edit(Delegate &delegate, const wabi::GfVec2f &size, int &selection)
  {
    bool ret = false;
    AnchorIO &io = ANCHOR::GetIO();
    ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, wabi::GfVec2f(0, 0));
    ANCHOR::BeginChildFrame(137, size);

    AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
    const wabi::GfVec2f offset = ANCHOR::GetCursorScreenPos();

    const wabi::GfVec4f *pts = delegate.GetPoints();
    static int currentSelection = -1;
    static int movingPt = -1;
    if (currentSelection >= int(delegate.GetPointCount()))
      currentSelection = -1;
    if (movingPt != -1) {
      wabi::GfVec4f current = pts[movingPt];
      current[3] += io.MouseDelta[0] / size[0];
      current[3] = AnchorClamp(current[3], 0.f, 1.f);
      delegate.EditPoint(movingPt, current);
      ret = true;
      if (!io.MouseDown[0])
        movingPt = -1;
    }
    for (size_t i = 0; i < delegate.GetPointCount(); i++) {
      int ptSel = DrawPoint(draw_list, pts[i], size, i == currentSelection, offset);
      if (ptSel == 2) {
        currentSelection = int(i);
        ret = true;
      }
      if (ptSel == 1 && io.MouseDown[0] && movingPt == -1) {
        movingPt = int(i);
      }
    }
    AnchorBBox rc(offset, offset + size);
    if (rc.Contains(io.MousePos) && io.MouseDoubleClicked[0]) {
      float t = (io.MousePos[0] - offset[0]) / size[0];
      delegate.AddPoint(delegate.GetPoint(t));
      ret = true;
    }
    ANCHOR::EndChildFrame();
    ANCHOR::PopStyleVar();

    selection = currentSelection;
    return ret;
  }
}  // namespace AnchorGradient
