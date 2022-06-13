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

#include "ANCHOR_api.h"
#include "ANCHOR_internal.h"
#include "ANCHOR_curve_edit.h"

#include <stdint.h>
#include <set>
#include <vector>

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <malloc.h>
#endif
#if !defined(_MSC_VER) && !defined(__MINGW64_VERSION_MAJOR)
#  define _malloca(x) alloca(x)
#  define _freea(x)
#endif

WABI_NAMESPACE_USING

namespace AnchorCurveEdit
{
  static float smoothstep(float edge0, float edge1, float x)
  {
    x = AnchorClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3 - 2 * x);
  }

  static float distance(float x, float y, float x1, float y1, float x2, float y2)
  {
    float A = x - x1;
    float B = y - y1;
    float C = x2 - x1;
    float D = y2 - y1;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = -1.f;
    if (len_sq > FLT_EPSILON)
      param = dot / len_sq;

    float xx, yy;

    if (param < 0.f) {
      xx = x1;
      yy = y1;
    } else if (param > 1.f) {
      xx = x2;
      yy = y2;
    } else {
      xx = x1 + param * C;
      yy = y1 + param * D;
    }

    float dx = x - xx;
    float dy = y - yy;
    return sqrtf(dx * dx + dy * dy);
  }

  static int DrawPoint(AnchorDrawList *draw_list,
                       GfVec2f pos,
                       const GfVec2f size,
                       const GfVec2f offset,
                       bool edited)
  {
    int ret = 0;
    AnchorIO &io = ANCHOR::GetIO();

    static const GfVec2f localOffsets[4] = {GfVec2f(1, 0),
                                            GfVec2f(0, 1),
                                            GfVec2f(-1, 0),
                                            GfVec2f(0, -1)};
    GfVec2f offsets[4];
    for (int i = 0; i < 4; i++) {
      offsets[i] = GfVec2f(pos * size) + localOffsets[i] * 4.5f + offset;
    }

    const GfVec2f center = GfVec2f(pos * size) + offset;
    const AnchorBBox anchor(center - GfVec2f(5, 5), center + GfVec2f(5, 5));
    draw_list->AddConvexPolyFilled(offsets, 4, 0xFF000000);
    if (anchor.Contains(io.MousePos)) {
      ret = 1;
      if (io.MouseDown[0])
        ret = 2;
    }
    if (edited)
      draw_list->AddPolyline(offsets, 4, 0xFFFFFFFF, true, 3.0f);
    else if (ret)
      draw_list->AddPolyline(offsets, 4, 0xFF80B0FF, true, 2.0f);
    else
      draw_list->AddPolyline(offsets, 4, 0xFF0080FF, true, 2.0f);

    return ret;
  }

  int Edit(Delegate &delegate,
           const GfVec2f &size,
           unsigned int id,
           const AnchorBBox *clippingRect,
           std::vector<EditPoint> *selectedPoints)
  {
    static bool selectingQuad = false;
    static GfVec2f quadSelection;
    static int overCurve = -1;
    static int movingCurve = -1;
    static bool scrollingV = false;
    static std::set<EditPoint> selection;
    static bool overSelectedPoint = false;

    int ret = 0;

    AnchorIO &io = ANCHOR::GetIO();
    ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(0, 0));
    ANCHOR::PushStyleColor(AnchorCol_Border, 0);
    ANCHOR::BeginChildFrame(id, size);
    delegate.focused = ANCHOR::IsWindowFocused();
    AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
    if (clippingRect)
      draw_list->PushClipRect(clippingRect->Min, clippingRect->Max, true);

    const GfVec2f offset = ANCHOR::GetCursorScreenPos() + GfVec2f(0.f, size[1]);
    const GfVec2f ssize(size[0], -size[1]);
    const AnchorBBox container(offset + GfVec2f(0.f, ssize[1]), offset + GfVec2f(ssize[0], 0.f));
    GfVec2f &d_min = delegate.GetMin();
    GfVec2f &d_max = delegate.GetMax();

    // handle zoom and VScroll
    if (container.Contains(io.MousePos)) {
      if (fabsf(io.MouseWheel) > FLT_EPSILON) {
        const float r = (io.MousePos[1] - offset[1]) / ssize[1];
        float ratioY = AnchorLerp(d_min[1], d_max[1], r);
        auto scaleValue = [&](float v) {
          v -= ratioY;
          v *= (1.f - io.MouseWheel * 0.05f);
          v += ratioY;
          return v;
        };
        d_min[1] = scaleValue(d_min[1]);
        d_max[1] = scaleValue(d_max[1]);
      }
      if (!scrollingV && ANCHOR::IsMouseDown(2)) {
        scrollingV = true;
      }
    }
    GfVec2f range = d_max - d_min + GfVec2f(1.f, 0.f);  // +1 because of inclusive last frame

    const GfVec2f viewSize(size[0], -size[1]);
    const GfVec2f sizeOfPixel = viewSize / 1.0;
    const size_t curveCount = delegate.GetCurveCount();

    if (scrollingV) {
      float deltaH = io.MouseDelta[1] * range[1] * sizeOfPixel[1];
      d_min[1] -= deltaH;
      d_max[1] -= deltaH;
      if (!ANCHOR::IsMouseDown(2))
        scrollingV = false;
    }

    draw_list->AddRectFilled(offset, offset + ssize, delegate.GetBackgroundColor());

    auto pointToRange = [&](GfVec2f pt) {
      return (pt - d_min) / range;
    };
    auto rangeToPoint = [&](GfVec2f pt) {
      return GfVec2f(pt * range) + d_min;
    };

    draw_list->AddLine(GfVec2f(GfVec2f(-1.f, -d_min[1] / range[1]) * viewSize) + offset,
                       GfVec2f(GfVec2f(1.f, -d_min[1] / range[1]) * viewSize) + offset,
                       0xFF000000,
                       1.5f);
    bool overCurveOrPoint = false;

    int localOverCurve = -1;
    // make sure highlighted curve is rendered last
    int *curvesIndex = (int *)_malloca(sizeof(int) * curveCount);
    for (size_t c = 0; c < curveCount; c++)
      curvesIndex[c] = int(c);
    int highLightedCurveIndex = -1;
    if (overCurve != -1 && curveCount) {
      AnchorSwap(curvesIndex[overCurve], curvesIndex[curveCount - 1]);
      highLightedCurveIndex = overCurve;
    }

    for (size_t cur = 0; cur < curveCount; cur++) {
      int c = curvesIndex[cur];
      if (!delegate.IsVisible(c))
        continue;
      const size_t ptCount = delegate.GetPointCount(c);
      if (ptCount < 1)
        continue;
      CurveType curveType = delegate.GetCurveType(c);
      if (curveType == CurveNone)
        continue;
      const GfVec2f *pts = delegate.GetPoints(c);
      uint32_t curveColor = delegate.GetCurveColor(c);
      if ((c == highLightedCurveIndex && selection.empty() && !selectingQuad) || movingCurve == c)
        curveColor = 0xFFFFFFFF;

      for (size_t p = 0; p < ptCount - 1; p++) {
        const GfVec2f p1 = pointToRange(pts[p]);
        const GfVec2f p2 = pointToRange(pts[p + 1]);

        if (curveType == CurveSmooth || curveType == CurveLinear) {
          size_t subStepCount = (curveType == CurveSmooth) ? 20 : 2;
          float step = 1.f / float(subStepCount - 1);
          for (size_t substep = 0; substep < subStepCount - 1; substep++) {
            float t = float(substep) * step;

            const GfVec2f sp1 = AnchorLerp(p1, p2, t);
            const GfVec2f sp2 = AnchorLerp(p1, p2, t + step);

            const float rt1 = smoothstep(p1[0], p2[0], sp1[0]);
            const float rt2 = smoothstep(p1[0], p2[0], sp2[0]);

            const GfVec2f pos1 = GfVec2f(GfVec2f(sp1[0], AnchorLerp(p1[1], p2[1], rt1)) *
                                         viewSize) +
                                 offset;
            const GfVec2f pos2 = GfVec2f(GfVec2f(sp2[0], AnchorLerp(p1[1], p2[1], rt2)) *
                                         viewSize) +
                                 offset;

            if (distance(io.MousePos[0], io.MousePos[1], pos1[0], pos1[1], pos2[0], pos2[1]) <
                  8.f &&
                !scrollingV) {
              localOverCurve = int(c);
              overCurve = int(c);
              overCurveOrPoint = true;
            }

            draw_list->AddLine(pos1, pos2, curveColor, 1.3f);
          }  // substep
        } else if (curveType == CurveDiscrete) {
          GfVec2f dp1 = GfVec2f(p1 * viewSize) + offset;
          GfVec2f dp2 = GfVec2f(GfVec2f(p2[0], p1[1]) * viewSize) + offset;
          GfVec2f dp3 = GfCompMult(p2, GfVec2f(viewSize + offset));
          draw_list->AddLine(dp1, dp2, curveColor, 1.3f);
          draw_list->AddLine(dp2, dp3, curveColor, 1.3f);

          if ((distance(io.MousePos[0], io.MousePos[1], dp1[0], dp1[1], dp3[0], dp1[1]) < 8.f ||
               distance(io.MousePos[0], io.MousePos[1], dp3[0], dp1[1], dp3[0], dp3[1]) < 8.f)
              /*&& localOverCurve == -1*/) {
            localOverCurve = int(c);
            overCurve = int(c);
            overCurveOrPoint = true;
          }
        }
      }  // point loop

      for (size_t p = 0; p < ptCount; p++) {
        const int drawState = DrawPoint(draw_list,
                                        pointToRange(pts[p]),
                                        viewSize,
                                        offset,
                                        (selection.find({int(c), int(p)}) != selection.end() &&
                                         movingCurve == -1 && !scrollingV));
        if (drawState && movingCurve == -1 && !selectingQuad) {
          overCurveOrPoint = true;
          overSelectedPoint = true;
          overCurve = -1;
          if (drawState == 2) {
            if (!io.KeyShift && selection.find({int(c), int(p)}) == selection.end())
              selection.clear();
            selection.insert({int(c), int(p)});
          }
        }
      }
    }  // curves loop

    if (localOverCurve == -1)
      overCurve = -1;

    // move selection
    static bool pointsMoved = false;
    static GfVec2f mousePosOrigin;
    static std::vector<GfVec2f> originalPoints;
    if (overSelectedPoint && io.MouseDown[0]) {
      if ((fabsf(io.MouseDelta[0]) > 0.f || fabsf(io.MouseDelta[1]) > 0.f) && !selection.empty()) {
        if (!pointsMoved) {
          delegate.BeginEdit(0);
          mousePosOrigin = io.MousePos;
          originalPoints.resize(selection.size());
          int index = 0;
          for (auto &sel : selection) {
            const GfVec2f *pts = delegate.GetPoints(sel.curveIndex);
            originalPoints[index++] = pts[sel.pointIndex];
          }
        }
        pointsMoved = true;
        ret = 1;
        auto prevSelection = selection;
        int originalIndex = 0;
        for (auto &sel : prevSelection) {
          const GfVec2f mouseRange = pointToRange(originalPoints[originalIndex]) +
                                     GfVec2f(io.MousePos - mousePosOrigin);
          const GfVec2f p = rangeToPoint(GfVec2f(mouseRange * GfVec2f(sizeOfPixel)));
          const int newIndex = delegate.EditPoint(sel.curveIndex, sel.pointIndex, p);
          if (newIndex != sel.pointIndex) {
            selection.erase(sel);
            selection.insert({sel.curveIndex, newIndex});
          }
          originalIndex++;
        }
      }
    }

    if (overSelectedPoint && !io.MouseDown[0]) {
      overSelectedPoint = false;
      if (pointsMoved) {
        pointsMoved = false;
        delegate.EndEdit();
      }
    }

    // add point
    if (overCurve != -1 && io.MouseDoubleClicked[0]) {
      const GfVec2f np = rangeToPoint((io.MousePos - offset) / viewSize);
      delegate.BeginEdit(overCurve);
      delegate.AddPoint(overCurve, np);
      delegate.EndEdit();
      ret = 1;
    }

    // move curve

    if (movingCurve != -1) {
      const size_t ptCount = delegate.GetPointCount(movingCurve);
      const GfVec2f *pts = delegate.GetPoints(movingCurve);
      if (!pointsMoved) {
        mousePosOrigin = io.MousePos;
        pointsMoved = true;
        originalPoints.resize(ptCount);
        for (size_t index = 0; index < ptCount; index++) {
          originalPoints[index] = pts[index];
        }
      }
      if (ptCount >= 1) {
        for (size_t p = 0; p < ptCount; p++) {
          delegate.EditPoint(movingCurve,
                             int(p),
                             GfVec2f(rangeToPoint(pointToRange(originalPoints[p]) +
                                                  GfVec2f(io.MousePos - mousePosOrigin)) *
                                     sizeOfPixel));
        }
        ret = 1;
      }
      if (!io.MouseDown[0]) {
        movingCurve = -1;
        pointsMoved = false;
        delegate.EndEdit();
      }
    }
    if (movingCurve == -1 && overCurve != -1 && ANCHOR::IsMouseClicked(0) && selection.empty() &&
        !selectingQuad) {
      movingCurve = overCurve;
      delegate.BeginEdit(overCurve);
    }

    // quad selection
    if (selectingQuad) {
      const GfVec2f bmin = AnchorMin(quadSelection, io.MousePos);
      const GfVec2f bmax = AnchorMax(quadSelection, io.MousePos);
      draw_list->AddRectFilled(bmin, bmax, 0x40FF0000, 1.f);
      draw_list->AddRect(bmin, bmax, 0xFFFF0000, 1.f);
      const AnchorBBox selectionQuad(bmin, bmax);
      if (!io.MouseDown[0]) {
        if (!io.KeyShift)
          selection.clear();
        // select everythnig is quad
        for (size_t c = 0; c < curveCount; c++) {
          if (!delegate.IsVisible(c))
            continue;

          const size_t ptCount = delegate.GetPointCount(c);
          if (ptCount < 1)
            continue;

          const GfVec2f *pts = delegate.GetPoints(c);
          for (size_t p = 0; p < ptCount; p++) {
            const GfVec2f center = GfVec2f(pointToRange(pts[p]) * viewSize) + offset;
            if (selectionQuad.Contains(center))
              selection.insert({int(c), int(p)});
          }
        }
        // done
        selectingQuad = false;
      }
    }
    if (!overCurveOrPoint && ANCHOR::IsMouseClicked(0) && !selectingQuad && movingCurve == -1 &&
        !overSelectedPoint && container.Contains(io.MousePos)) {
      selectingQuad = true;
      quadSelection = io.MousePos;
    }
    if (clippingRect)
      draw_list->PopClipRect();

    ANCHOR::EndChildFrame();
    ANCHOR::PopStyleVar();
    ANCHOR::PopStyleColor(1);

    if (selectedPoints) {
      selectedPoints->resize(int(selection.size()));
      int index = 0;
      for (auto &point : selection)
        (*selectedPoints)[index++] = point;
    }
    return ret;
  }
}  // namespace AnchorCurveEdit
