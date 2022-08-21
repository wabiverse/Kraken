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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"

namespace ImZoomSlider
{
  typedef int AnchorZoomSliderFlags;
  enum AnchorPopupFlags_
  {
    AnchorZoomSliderFlags_None = 0,
    AnchorZoomSliderFlags_Vertical = 1,
    AnchorZoomSliderFlags_NoAnchors = 2,
    AnchorZoomSliderFlags_NoMiddleCarets = 4,
    AnchorZoomSliderFlags_NoWheel = 8,
  };

  template<typename T>
  bool ImZoomSlider(const T lower,
                    const T higher,
                    T &viewLower,
                    T &viewHigher,
                    float wheelRatio = 0.01f,
                    AnchorZoomSliderFlags flags = AnchorZoomSliderFlags_None)
  {
    bool interacted = false;
    AnchorIO &io = ANCHOR::GetIO();
    AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();

    static const float handleSize = 12;
    static const float roundRadius = 3.f;
    static const char *controlName = "ImZoomSlider";

    static bool movingScrollBarSvg = false;
    static bool sizingRBarSvg = false;
    static bool sizingLBarSvg = false;
    static AnchorID editingId = (AnchorID)-1;
    static float scrollingSource = 0.f;
    static float saveViewLower;
    static float saveViewHigher;

    const bool isVertical = flags & AnchorZoomSliderFlags_Vertical;
    const GfVec2f canvasPos = ANCHOR::GetCursorScreenPos();
    const GfVec2f canvasSize = ANCHOR::GetContentRegionAvail();
    const float canvasSizeLength = isVertical ? ANCHOR::GetItemRectSize().y : canvasSize.x;
    const GfVec2f scrollBarSize = isVertical ? GfVec2f(14.f, canvasSizeLength) :
                                               GfVec2f(canvasSizeLength, 14.f);

    ANCHOR::InvisibleButton(controlName, scrollBarSize);
    const AnchorID currentId = ANCHOR::GetID(controlName);

    const bool usingEditingId = currentId == editingId;
    const bool canUseControl = usingEditingId || editingId == -1;
    const bool movingScrollBar = usingEditingId ? movingScrollBarSvg : false;
    const bool sizingRBar = usingEditingId ? sizingRBarSvg : false;
    const bool sizingLBar = usingEditingId ? sizingLBarSvg : false;
    const int componentIndex = isVertical ? 1 : 0;
    const GfVec2f scrollBarMin = ANCHOR::GetItemRectMin();
    const GfVec2f scrollBarMax = ANCHOR::GetItemRectMax();
    const GfVec2f scrollBarA = GfVec2f(scrollBarMin.x, scrollBarMin.y) -
                               (isVertical ? GfVec2f(2, 0) : GfVec2f(0, 2));
    const GfVec2f scrollBarB = isVertical ?
                                 GfVec2f(scrollBarMax.x - 1.f, scrollBarMin.y + canvasSizeLength) :
                                 GfVec2f(scrollBarMin.x + canvasSizeLength, scrollBarMax.y - 1.f);
    const float scrollStart = ((viewLower - lower) / (higher - lower)) * canvasSizeLength +
                              scrollBarMin[componentIndex];
    const float scrollEnd = ((viewHigher - lower) / (higher - lower)) * canvasSizeLength +
                            scrollBarMin[componentIndex];
    const float screenSize = scrollEnd - scrollStart;
    const GfVec2f scrollTopLeft = isVertical ? GfVec2f(scrollBarMin.x, scrollStart) :
                                               GfVec2f(scrollStart, scrollBarMin.y);
    const GfVec2f scrollBottomRight = isVertical ? GfVec2f(scrollBarMax.x - 2.f, scrollEnd) :
                                                   GfVec2f(scrollEnd, scrollBarMax.y - 2.f);
    const bool inScrollBar = canUseControl &&
                             AnchorBBox(scrollTopLeft, scrollBottomRight).Contains(io.MousePos);
    const AnchorBBox scrollBarRect(scrollBarA, scrollBarB);
    const float deltaScreen = io.MousePos[componentIndex] - scrollingSource;
    const float deltaView = ((higher - lower) / canvasSizeLength) * deltaScreen;
    const uint32_t barColor = ANCHOR::GetColorU32(
      (inScrollBar || movingScrollBar) ? AnchorCol_FrameBgHovered : AnchorCol_FrameBg);
    const float middleCoord = (scrollStart + scrollEnd) * 0.5f;
    const bool insideControl = canUseControl &&
                               AnchorBBox(scrollBarMin, scrollBarMax).Contains(io.MousePos);
    const bool hasAnchors = !(flags & AnchorZoomSliderFlags_NoAnchors);
    const float viewMinSize = ((3.f * handleSize) / canvasSizeLength) * (higher - lower);
    const auto ClipView = [lower, higher, &viewLower, &viewHigher]() {
      if (viewLower < lower) {
        const float deltaClip = lower - viewLower;
        viewLower += deltaClip;
        viewHigher += deltaClip;
      }
      if (viewHigher > higher) {
        const float deltaClip = viewHigher - higher;
        viewLower -= deltaClip;
        viewHigher -= deltaClip;
      }
    };

    bool onLeft = false;
    bool onRight = false;

    draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, roundRadius);
    draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);
    draw_list->AddRectFilled(scrollTopLeft, scrollBottomRight, barColor, roundRadius);

    if (!(flags & AnchorZoomSliderFlags_NoMiddleCarets)) {
      for (float i = 0.5f; i < 3.f; i += 1.f) {
        const float coordA = middleCoord - handleSize * 0.5f;
        const float coordB = middleCoord + handleSize * 0.5f;
        GfVec2f base = scrollBarMin;
        base.x += scrollBarSize.x * 0.25f * i;
        base.y += scrollBarSize.y * 0.25f * i;

        if (isVertical) {
          draw_list->AddLine(GfVec2f(base.x, coordA),
                             GfVec2f(base.x, coordB),
                             ANCHOR::GetColorU32(AnchorCol_SliderGrab));
        } else {
          draw_list->AddLine(GfVec2f(coordA, base.y),
                             GfVec2f(coordB, base.y),
                             ANCHOR::GetColorU32(AnchorCol_SliderGrab));
        }
      }
    }

    // Mouse wheel
    if (io.MouseClicked[0] && insideControl && !inScrollBar) {
      const float ratio = (io.MousePos[componentIndex] - scrollBarMin[componentIndex]) /
                          (scrollBarMax[componentIndex] - scrollBarMin[componentIndex]);
      const float size = (higher - lower);
      const float halfViewSize = (viewHigher - viewLower) * 0.5f;
      const float middle = ratio * size + lower;
      viewLower = middle - halfViewSize;
      viewHigher = middle + halfViewSize;
      ClipView();
      interacted = true;
    }

    if (!(flags & AnchorZoomSliderFlags_NoWheel) && inScrollBar && fabsf(io.MouseWheel) > 0.f) {
      const float ratio = (io.MousePos[componentIndex] - scrollStart) / (scrollEnd - scrollStart);
      const float amount = io.MouseWheel * wheelRatio * (viewHigher - viewLower);

      viewLower -= ratio * amount;
      viewHigher += (1.f - ratio) * amount;
      ClipView();
      interacted = true;
    }

    if (screenSize > handleSize * 2.f && hasAnchors) {
      const AnchorBBox barHandleLeft(scrollTopLeft,
                                     isVertical ?
                                       GfVec2f(scrollBottomRight.x, scrollTopLeft.y + handleSize) :
                                       GfVec2f(scrollTopLeft.x + handleSize, scrollBottomRight.y));
      const AnchorBBox barHandleRight(
        isVertical ? GfVec2f(scrollTopLeft.x, scrollBottomRight.y - handleSize) :
                     GfVec2f(scrollBottomRight.x - handleSize, scrollTopLeft.y),
        scrollBottomRight);

      onLeft = barHandleLeft.Contains(io.MousePos);
      onRight = barHandleRight.Contains(io.MousePos);

      draw_list->AddRectFilled(barHandleLeft.Min,
                               barHandleLeft.Max,
                               ANCHOR::GetColorU32((onLeft || sizingLBar) ?
                                                     AnchorCol_SliderGrabActive :
                                                     AnchorCol_SliderGrab),
                               roundRadius);
      draw_list->AddRectFilled(barHandleRight.Min,
                               barHandleRight.Max,
                               ANCHOR::GetColorU32((onRight || sizingRBar) ?
                                                     AnchorCol_SliderGrabActive :
                                                     AnchorCol_SliderGrab),
                               roundRadius);
    }

    if (sizingRBar) {
      if (!io.MouseDown[0]) {
        sizingRBarSvg = false;
        editingId = (AnchorID)-1;
      } else {
        viewHigher = AnchorMin(saveViewHigher + deltaView, higher);
      }
    } else if (sizingLBar) {
      if (!io.MouseDown[0]) {
        sizingLBarSvg = false;
        editingId = (AnchorID)-1;
      } else {
        viewLower = AnchorMax(saveViewLower + deltaView, lower);
      }
    } else {
      if (movingScrollBar) {
        if (!io.MouseDown[0]) {
          movingScrollBarSvg = false;
          editingId = (AnchorID)-1;
        } else {
          viewLower = saveViewLower + deltaView;
          viewHigher = saveViewHigher + deltaView;
          ClipView();
        }
      } else {
        if (inScrollBar && ANCHOR::IsMouseClicked(0)) {
          movingScrollBarSvg = true;
          scrollingSource = io.MousePos[componentIndex];
          saveViewLower = viewLower;
          saveViewHigher = viewHigher;
          editingId = currentId;
        }
        if (!sizingRBar && onRight && ANCHOR::IsMouseClicked(0) && hasAnchors) {
          sizingRBarSvg = true;
          editingId = currentId;
        }
        if (!sizingLBar && onLeft && ANCHOR::IsMouseClicked(0) && hasAnchors) {
          sizingLBarSvg = true;
          editingId = currentId;
        }
      }
    }

    // minimal size check
    if ((viewHigher - viewLower) < viewMinSize) {
      const float middle = (viewLower + viewHigher) * 0.5f;
      viewLower = middle - viewMinSize * 0.5f;
      viewHigher = middle + viewMinSize * 0.5f;
      ClipView();
    }

    return movingScrollBar || sizingRBar || sizingLBar || interacted;
  }

}  // namespace ImZoomSlider
