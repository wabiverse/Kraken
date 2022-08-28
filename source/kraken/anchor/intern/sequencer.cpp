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
#include "ANCHOR_sequencer.h"

#include <cstdlib>

KRAKEN_NAMESPACE_USING

namespace AnchorSequencer
{
  static bool SequencerAddDelButton(AnchorDrawList *draw_list, wabi::GfVec2f pos, bool add = true)
  {
    AnchorIO &io = ANCHOR::GetIO();
    AnchorBBox delRect(pos, wabi::GfVec2f(pos[0] + 16, pos[1] + 16));
    bool overDel = delRect.Contains(io.MousePos);
    int delColor = overDel ? 0xFFAAAAAA : 0x50000000;
    float midy = pos[1] + 16 / 2 - 0.5f;
    float midx = pos[0] + 16 / 2 - 0.5f;
    draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
    draw_list->AddLine(wabi::GfVec2f(delRect.Min[0] + 3, midy),
                       wabi::GfVec2f(delRect.Max[0] - 3, midy),
                       delColor,
                       2);
    if (add)
      draw_list->AddLine(wabi::GfVec2f(midx, delRect.Min[1] + 3),
                         wabi::GfVec2f(midx, delRect.Max[1] - 3),
                         delColor,
                         2);
    return overDel;
  }

  bool Sequencer(SequenceInterface *sequence,
                 int *currentFrame,
                 bool *expanded,
                 int *selectedEntry,
                 int *firstFrame,
                 int sequenceOptions)
  {
    bool ret = false;
    AnchorIO &io = ANCHOR::GetIO();
    int cx = (int)(io.MousePos[0]);
    int cy = (int)(io.MousePos[1]);
    static float framePixelWidth = 10.f;
    static float framePixelWidthTarget = 10.f;
    int legendWidth = 200;

    static int movingEntry = -1;
    static int movingPos = -1;
    static int movingPart = -1;
    int delEntry = -1;
    int dupEntry = -1;
    int ItemHeight = 20;

    bool popupOpened = false;
    int sequenceCount = sequence->GetItemCount();
    if (!sequenceCount)
      return false;
    ANCHOR::BeginGroup();

    AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
    wabi::GfVec2f canvas_pos =
      ANCHOR::GetCursorScreenPos();  // AnchorDrawList API uses screen coordinates!
    wabi::GfVec2f canvas_size = ANCHOR::GetContentRegionAvail();  // Resize canvas to what's available
    int firstFrameUsed = firstFrame ? *firstFrame : 0;


    int controlHeight = sequenceCount * ItemHeight;
    for (int i = 0; i < sequenceCount; i++)
      controlHeight += int(sequence->GetCustomHeight(i));
    int frameCount = AnchorMax(sequence->GetFrameMax() - sequence->GetFrameMin(), 1);

    static bool MovingScrollBar = false;
    static bool MovingCurrentFrame = false;
    struct CustomDraw
    {
      int index;
      AnchorBBox customRect;
      AnchorBBox legendRect;
      AnchorBBox clippingRect;
      AnchorBBox legendClippingRect;
    };
    std::vector<CustomDraw> customDraws;
    std::vector<CustomDraw> compactCustomDraws;
    // zoom in/out
    const int visibleFrameCount = (int)floorf((canvas_size[0] - legendWidth) / framePixelWidth);
    const float barWidthRatio = AnchorMin(visibleFrameCount / (float)frameCount, 1.f);
    const float barWidthInPixels = barWidthRatio * (canvas_size[0] - legendWidth);

    AnchorBBox regionRect(canvas_pos, canvas_pos + canvas_size);

    static bool panningView = false;
    static wabi::GfVec2f panningViewSource;
    static int panningViewFrame;
    if (ANCHOR::IsWindowFocused() && io.KeyAlt && io.MouseDown[2]) {
      if (!panningView) {
        panningViewSource = io.MousePos;
        panningView = true;
        panningViewFrame = *firstFrame;
      }
      *firstFrame = panningViewFrame -
                    int((io.MousePos[0] - panningViewSource[0]) / framePixelWidth);
      *firstFrame = AnchorClamp(*firstFrame,
                                sequence->GetFrameMin(),
                                sequence->GetFrameMax() - visibleFrameCount);
    }
    if (panningView && !io.MouseDown[2]) {
      panningView = false;
    }
    framePixelWidthTarget = AnchorClamp(framePixelWidthTarget, 0.1f, 50.f);

    framePixelWidth = AnchorLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

    frameCount = sequence->GetFrameMax() - sequence->GetFrameMin();
    if (visibleFrameCount >= frameCount && firstFrame)
      *firstFrame = sequence->GetFrameMin();


    // --
    if (expanded && !*expanded) {
      ANCHOR::InvisibleButton("canvas",
                              wabi::GfVec2f(canvas_size[0] - canvas_pos[0], (float)ItemHeight));
      draw_list->AddRectFilled(canvas_pos,
                               wabi::GfVec2f(canvas_size[0] + canvas_pos[0], canvas_pos[1] + ItemHeight),
                               0xFF3D3837,
                               0);
      char tmps[512];
      AnchorFormatString(tmps,
                         ANCHOR_ARRAYSIZE(tmps),
                         "%d Frames / %d entries",
                         frameCount,
                         sequenceCount);
      draw_list->AddText(wabi::GfVec2f(canvas_pos[0] + 26, canvas_pos[1] + 2), 0xFFFFFFFF, tmps);
    } else {
      bool hasScrollBar(true);
      /*
           int framesPixelWidth = int(frameCount * framePixelWidth);
           if ((framesPixelWidth + legendWidth) >= canvas_size[0])
           {
               hasScrollBar = true;
           }
           */
      // test scroll area
      wabi::GfVec2f headerSize(canvas_size[0], (float)ItemHeight);
      wabi::GfVec2f scrollBarSize(canvas_size[0], 14.f);
      ANCHOR::InvisibleButton("topBar", headerSize);
      draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
      wabi::GfVec2f childFramePos = ANCHOR::GetCursorScreenPos();
      wabi::GfVec2f childFrameSize(canvas_size[0],
                             canvas_size[1] - 8.f - headerSize[1] -
                               (hasScrollBar ? scrollBarSize[1] : 0));
      ANCHOR::PushStyleColor(AnchorCol_FrameBg, 0);
      ANCHOR::BeginChildFrame(889, childFrameSize);
      sequence->focused = ANCHOR::IsWindowFocused();
      ANCHOR::InvisibleButton("contentBar", wabi::GfVec2f(canvas_size[0], float(controlHeight)));
      const wabi::GfVec2f contentMin = ANCHOR::GetItemRectMin();
      const wabi::GfVec2f contentMax = ANCHOR::GetItemRectMax();
      const AnchorBBox contentRect(contentMin, contentMax);
      const float contentHeight = contentMax[1] - contentMin[1];

      // full background
      draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);

      // current frame top
      AnchorBBox topRect(wabi::GfVec2f(canvas_pos[0] + legendWidth, canvas_pos[1]),
                         wabi::GfVec2f(canvas_pos[0] + canvas_size[0], canvas_pos[1] + ItemHeight));

      if (!MovingCurrentFrame && !MovingScrollBar && movingEntry == -1 &&
          sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 &&
          topRect.Contains(io.MousePos) && io.MouseDown[0]) {
        MovingCurrentFrame = true;
      }
      if (MovingCurrentFrame) {
        if (frameCount) {
          *currentFrame = (int)((io.MousePos[0] - topRect.Min[0]) / framePixelWidth) +
                          firstFrameUsed;
          if (*currentFrame < sequence->GetFrameMin())
            *currentFrame = sequence->GetFrameMin();
          if (*currentFrame >= sequence->GetFrameMax())
            *currentFrame = sequence->GetFrameMax();
        }
        if (!io.MouseDown[0])
          MovingCurrentFrame = false;
      }

      // header
      draw_list->AddRectFilled(canvas_pos,
                               wabi::GfVec2f(canvas_size[0] + canvas_pos[0], canvas_pos[1] + ItemHeight),
                               0xFF3D3837,
                               0);
      if (sequenceOptions & SEQUENCER_ADD) {
        if (SequencerAddDelButton(
              draw_list,
              wabi::GfVec2f(canvas_pos[0] + legendWidth - ItemHeight, canvas_pos[1] + 2),
              true) &&
            io.MouseReleased[0])
          ANCHOR::OpenPopup("addEntry");

        if (ANCHOR::BeginPopup("addEntry")) {
          for (int i = 0; i < sequence->GetItemTypeCount(); i++)
            if (ANCHOR::Selectable(sequence->GetItemTypeName(i))) {
              sequence->Add(i);
              *selectedEntry = sequence->GetItemCount() - 1;
            }

          ANCHOR::EndPopup();
          popupOpened = true;
        }
      }

      // header frame number and lines
      int modFrameCount = 10;
      int frameStep = 1;
      while ((modFrameCount * framePixelWidth) < 150) {
        modFrameCount *= 2;
        frameStep *= 2;
      };
      int halfModFrameCount = modFrameCount / 2;

      auto drawLine = [&](int i, int regionHeight) {
        bool baseIndex = ((i % modFrameCount) == 0) ||
                         (i == sequence->GetFrameMax() || i == sequence->GetFrameMin());
        bool halfIndex = (i % halfModFrameCount) == 0;
        int px = (int)canvas_pos[0] + int(i * framePixelWidth) + legendWidth -
                 int(firstFrameUsed * framePixelWidth);
        int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
        int tiretEnd = baseIndex ? regionHeight : ItemHeight;

        if (px <= (canvas_size[0] + canvas_pos[0]) && px >= (canvas_pos[0] + legendWidth)) {
          draw_list->AddLine(wabi::GfVec2f((float)px, canvas_pos[1] + (float)tiretStart),
                             wabi::GfVec2f((float)px, canvas_pos[1] + (float)tiretEnd - 1),
                             0xFF606060,
                             1);

          draw_list->AddLine(wabi::GfVec2f((float)px, canvas_pos[1] + (float)ItemHeight),
                             wabi::GfVec2f((float)px, canvas_pos[1] + (float)regionHeight - 1),
                             0x30606060,
                             1);
        }

        if (baseIndex && px > (canvas_pos[0] + legendWidth)) {
          char tmps[512];
          AnchorFormatString(tmps, ANCHOR_ARRAYSIZE(tmps), "%d", i);
          draw_list->AddText(wabi::GfVec2f((float)px + 3.f, canvas_pos[1]), 0xFFBBBBBB, tmps);
        }
      };

      auto drawLineContent = [&](int i, int /*regionHeight*/) {
        int px = (int)canvas_pos[0] + int(i * framePixelWidth) + legendWidth -
                 int(firstFrameUsed * framePixelWidth);
        int tiretStart = int(contentMin[1]);
        int tiretEnd = int(contentMax[1]);

        if (px <= (canvas_size[0] + canvas_pos[0]) && px >= (canvas_pos[0] + legendWidth)) {
          // draw_list->AddLine(wabi::GfVec2f((float)px, canvas_pos[1] + (float)tiretStart),
          // wabi::GfVec2f((float)px, canvas_pos[1] + (float)tiretEnd - 1), 0xFF606060, 1);

          draw_list->AddLine(wabi::GfVec2f(float(px), float(tiretStart)),
                             wabi::GfVec2f(float(px), float(tiretEnd)),
                             0x30606060,
                             1);
        }
      };
      for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep) {
        drawLine(i, ItemHeight);
      }
      drawLine(sequence->GetFrameMin(), ItemHeight);
      drawLine(sequence->GetFrameMax(), ItemHeight);
      /*
                    draw_list->AddLine(canvas_pos, wabi::GfVec2f(canvas_pos[0], canvas_pos[1] +
         controlHeight), 0xFF000000, 1); draw_list->AddLine(wabi::GfVec2f(canvas_pos[0], canvas_pos[1] +
         ItemHeight), wabi::GfVec2f(canvas_size[0], canvas_pos[1] + ItemHeight), 0xFF000000, 1);
                    */
      // clip content

      draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

      // draw item names in the legend rect on the left
      size_t customHeight = 0;
      for (int i = 0; i < sequenceCount; i++) {
        int type;
        sequence->Get(i, NULL, NULL, &type, NULL);
        wabi::GfVec2f tpos(contentMin[0] + 3, contentMin[1] + i * ItemHeight + 2 + customHeight);
        draw_list->AddText(tpos, 0xFFFFFFFF, sequence->GetItemLabel(i));

        if (sequenceOptions & SEQUENCER_DEL) {
          bool overDel = SequencerAddDelButton(
            draw_list,
            wabi::GfVec2f(contentMin[0] + legendWidth - ItemHeight + 2 - 10, tpos[1] + 2),
            false);
          if (overDel && io.MouseReleased[0])
            delEntry = i;

          bool overDup = SequencerAddDelButton(
            draw_list,
            wabi::GfVec2f(contentMin[0] + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos[1] + 2),
            true);
          if (overDup && io.MouseReleased[0])
            dupEntry = i;
        }
        customHeight += sequence->GetCustomHeight(i);
      }

      // clipping rect so items bars are not visible in the legend on the left when scrolled
      //

      // slots background
      customHeight = 0;
      for (int i = 0; i < sequenceCount; i++) {
        unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

        size_t localCustomHeight = sequence->GetCustomHeight(i);
        wabi::GfVec2f pos = wabi::GfVec2f(contentMin[0] + legendWidth,
                              contentMin[1] + ItemHeight * i + 1 + customHeight);
        wabi::GfVec2f sz = wabi::GfVec2f(canvas_size[0] + canvas_pos[0],
                             pos[1] + ItemHeight - 1 + localCustomHeight);
        if (!popupOpened && cy >= pos[1] && cy < pos[1] + (ItemHeight + localCustomHeight) &&
            movingEntry == -1 && cx > contentMin[0] && cx < contentMin[0] + canvas_size[0]) {
          col += 0x80201008;
          pos[0] -= legendWidth;
        }
        draw_list->AddRectFilled(pos, sz, col, 0);
        customHeight += localCustomHeight;
      }

      draw_list->PushClipRect(childFramePos + wabi::GfVec2f(float(legendWidth), 0.f),
                              childFramePos + childFrameSize);

      // vertical frame lines in content area
      for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep) {
        drawLineContent(i, int(contentHeight));
      }
      drawLineContent(sequence->GetFrameMin(), int(contentHeight));
      drawLineContent(sequence->GetFrameMax(), int(contentHeight));

      // selection
      bool selected = selectedEntry && (*selectedEntry >= 0);
      if (selected) {
        customHeight = 0;
        for (int i = 0; i < *selectedEntry; i++)
          customHeight += sequence->GetCustomHeight(i);
        ;
        draw_list->AddRectFilled(
          wabi::GfVec2f(contentMin[0], contentMin[1] + ItemHeight * *selectedEntry + customHeight),
          wabi::GfVec2f(contentMin[0] + canvas_size[0],
                  contentMin[1] + ItemHeight * (*selectedEntry + 1) + customHeight),
          0x801080FF,
          1.f);
      }

      // slots
      customHeight = 0;
      for (int i = 0; i < sequenceCount; i++) {
        int *start, *end;
        unsigned int color;
        sequence->Get(i, &start, &end, NULL, &color);
        size_t localCustomHeight = sequence->GetCustomHeight(i);

        wabi::GfVec2f pos = wabi::GfVec2f(contentMin[0] + legendWidth - firstFrameUsed * framePixelWidth,
                              contentMin[1] + ItemHeight * i + 1 + customHeight);
        wabi::GfVec2f slotP1(pos[0] + *start * framePixelWidth, pos[1] + 2);
        wabi::GfVec2f slotP2(pos[0] + *end * framePixelWidth + framePixelWidth, pos[1] + ItemHeight - 2);
        wabi::GfVec2f slotP3(pos[0] + *end * framePixelWidth + framePixelWidth,
                       pos[1] + ItemHeight - 2 + localCustomHeight);
        unsigned int slotColor = color | 0xFF000000;
        unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

        if (slotP1[0] <= (canvas_size[0] + contentMin[0]) &&
            slotP2[0] >= (contentMin[0] + legendWidth)) {
          draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
          draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
        }
        if (AnchorBBox(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0]) {
          sequence->DoubleClick(i);
        }
        AnchorBBox rects[3] = {
          AnchorBBox(slotP1, wabi::GfVec2f(slotP1[0] + framePixelWidth / 2, slotP2[1])),
          AnchorBBox(wabi::GfVec2f(slotP2[0] - framePixelWidth / 2, slotP1[1]), slotP2),
          AnchorBBox(slotP1, slotP2)};

        const unsigned int quadColor[] = {0xFFFFFFFF,
                                          0xFFFFFFFF,
                                          slotColor + (selected ? 0 : 0x202020)};
        if (movingEntry == -1 &&
            (sequenceOptions &
             SEQUENCER_EDIT_STARTEND))  // TODOFOCUS && backgroundRect.Contains(io.MousePos))
        {
          for (int j = 2; j >= 0; j--) {
            AnchorBBox &rc = rects[j];
            if (!rc.Contains(io.MousePos))
              continue;
            draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
          }

          for (int j = 0; j < 3; j++) {
            AnchorBBox &rc = rects[j];
            if (!rc.Contains(io.MousePos))
              continue;
            if (!AnchorBBox(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
              continue;
            if (ANCHOR::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame) {
              movingEntry = i;
              movingPos = cx;
              movingPart = j + 1;
              sequence->BeginEdit(movingEntry);
              break;
            }
          }
        }

        // custom draw
        if (localCustomHeight > 0) {
          wabi::GfVec2f rp(canvas_pos[0], contentMin[1] + ItemHeight * i + 1 + customHeight);
          AnchorBBox customRect(
            rp + wabi::GfVec2f(legendWidth -
                           (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth,
                         float(ItemHeight)),
            rp + wabi::GfVec2f(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) *
                                         framePixelWidth,
                         float(localCustomHeight + ItemHeight)));
          AnchorBBox clippingRect(
            rp + wabi::GfVec2f(float(legendWidth), float(ItemHeight)),
            rp + wabi::GfVec2f(canvas_size[0], float(localCustomHeight + ItemHeight)));

          AnchorBBox legendRect(rp + wabi::GfVec2f(0.f, float(ItemHeight)),
                                rp + wabi::GfVec2f(float(legendWidth), float(localCustomHeight)));
          AnchorBBox legendClippingRect(
            canvas_pos + wabi::GfVec2f(0.f, float(ItemHeight)),
            canvas_pos + wabi::GfVec2f(float(legendWidth), float(localCustomHeight + ItemHeight)));
          customDraws.push_back({i, customRect, legendRect, clippingRect, legendClippingRect});
        } else {
          wabi::GfVec2f rp(canvas_pos[0], contentMin[1] + ItemHeight * i + customHeight);
          AnchorBBox customRect(
            rp + wabi::GfVec2f(legendWidth -
                           (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth,
                         float(0.f)),
            rp + wabi::GfVec2f(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) *
                                         framePixelWidth,
                         float(ItemHeight)));
          AnchorBBox clippingRect(rp + wabi::GfVec2f(float(legendWidth), float(0.f)),
                                  rp + wabi::GfVec2f(canvas_size[0], float(ItemHeight)));

          compactCustomDraws.push_back({i, customRect, AnchorBBox(), clippingRect, AnchorBBox()});
        }
        customHeight += localCustomHeight;
      }


      // moving
      if (/*backgroundRect.Contains(io.MousePos) && */ movingEntry >= 0) {
        ANCHOR::CaptureMouseFromApp();
        int diffFrame = int((cx - movingPos) / framePixelWidth);
        if (std::abs(diffFrame) > 0) {
          int *start, *end;
          sequence->Get(movingEntry, &start, &end, NULL, NULL);
          if (selectedEntry)
            *selectedEntry = movingEntry;
          int &l = *start;
          int &r = *end;
          if (movingPart & 1)
            l += diffFrame;
          if (movingPart & 2)
            r += diffFrame;
          if (l < 0) {
            if (movingPart & 2)
              r -= l;
            l = 0;
          }
          if (movingPart & 1 && l > r)
            l = r;
          if (movingPart & 2 && r < l)
            r = l;
          movingPos += int(diffFrame * framePixelWidth);
        }
        if (!io.MouseDown[0]) {
          // single select
          if (!diffFrame && movingPart && selectedEntry) {
            *selectedEntry = movingEntry;
            ret = true;
          }

          movingEntry = -1;
          sequence->EndEdit();
        }
      }

      // cursor
      if (currentFrame && firstFrame && *currentFrame >= *firstFrame &&
          *currentFrame <= sequence->GetFrameMax()) {
        static const float cursorWidth = 8.f;
        float cursorOffset = contentMin[0] + legendWidth +
                             (*currentFrame - firstFrameUsed) * framePixelWidth +
                             framePixelWidth / 2 - cursorWidth * 0.5f;
        draw_list->AddLine(wabi::GfVec2f(cursorOffset, canvas_pos[1]),
                           wabi::GfVec2f(cursorOffset, contentMax[1]),
                           0xA02A2AFF,
                           cursorWidth);
        char tmps[512];
        AnchorFormatString(tmps, ANCHOR_ARRAYSIZE(tmps), "%d", *currentFrame);
        draw_list->AddText(wabi::GfVec2f(cursorOffset + 10, canvas_pos[1] + 2), 0xFF2A2AFF, tmps);
      }

      draw_list->PopClipRect();
      draw_list->PopClipRect();

      for (auto &customDraw : customDraws)
        sequence->CustomDraw(customDraw.index,
                             draw_list,
                             customDraw.customRect,
                             customDraw.legendRect,
                             customDraw.clippingRect,
                             customDraw.legendClippingRect);
      for (auto &customDraw : compactCustomDraws)
        sequence->CustomDrawCompact(customDraw.index,
                                    draw_list,
                                    customDraw.customRect,
                                    customDraw.clippingRect);

      // copy paste
      if (sequenceOptions & SEQUENCER_COPYPASTE) {
        AnchorBBox rectCopy(wabi::GfVec2f(contentMin[0] + 100, canvas_pos[1] + 2),
                            wabi::GfVec2f(contentMin[0] + 100 + 30, canvas_pos[1] + ItemHeight - 2));
        bool inRectCopy = rectCopy.Contains(io.MousePos);
        unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
        draw_list->AddText(rectCopy.Min, copyColor, "Copy");

        AnchorBBox rectPaste(wabi::GfVec2f(contentMin[0] + 140, canvas_pos[1] + 2),
                             wabi::GfVec2f(contentMin[0] + 140 + 30, canvas_pos[1] + ItemHeight - 2));
        bool inRectPaste = rectPaste.Contains(io.MousePos);
        unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
        draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

        if (inRectCopy && io.MouseReleased[0]) {
          sequence->Copy();
        }
        if (inRectPaste && io.MouseReleased[0]) {
          sequence->Paste();
        }
      }
      //

      ANCHOR::EndChildFrame();
      ANCHOR::PopStyleColor();
      if (hasScrollBar) {
        ANCHOR::InvisibleButton("scrollBar", scrollBarSize);
        wabi::GfVec2f scrollBarMin = ANCHOR::GetItemRectMin();
        wabi::GfVec2f scrollBarMax = ANCHOR::GetItemRectMax();

        // ratio = number of frames visible in control / number to total frames

        float startFrameOffset = ((float)(firstFrameUsed - sequence->GetFrameMin()) /
                                  (float)frameCount) *
                                 (canvas_size[0] - legendWidth);
        wabi::GfVec2f scrollBarA(scrollBarMin[0] + legendWidth, scrollBarMin[1] - 2);
        wabi::GfVec2f scrollBarB(scrollBarMin[0] + canvas_size[0], scrollBarMax[1] - 1);
        draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

        AnchorBBox scrollBarRect(scrollBarA, scrollBarB);
        bool inScrollBar = scrollBarRect.Contains(io.MousePos);

        draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


        wabi::GfVec2f scrollBarC(scrollBarMin[0] + legendWidth + startFrameOffset, scrollBarMin[1]);
        wabi::GfVec2f scrollBarD(scrollBarMin[0] + legendWidth + barWidthInPixels + startFrameOffset,
                           scrollBarMax[1] - 2);
        draw_list->AddRectFilled(scrollBarC,
                                 scrollBarD,
                                 (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050,
                                 6);

        AnchorBBox barHandleLeft(scrollBarC, wabi::GfVec2f(scrollBarC[0] + 14, scrollBarD[1]));
        AnchorBBox barHandleRight(wabi::GfVec2f(scrollBarD[0] - 14, scrollBarC[1]), scrollBarD);

        bool onLeft = barHandleLeft.Contains(io.MousePos);
        bool onRight = barHandleRight.Contains(io.MousePos);

        static bool sizingRBar = false;
        static bool sizingLBar = false;

        draw_list->AddRectFilled(barHandleLeft.Min,
                                 barHandleLeft.Max,
                                 (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666,
                                 6);
        draw_list->AddRectFilled(barHandleRight.Min,
                                 barHandleRight.Max,
                                 (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666,
                                 6);

        AnchorBBox scrollBarThumb(scrollBarC, scrollBarD);
        static const float MinBarWidth = 44.f;
        if (sizingRBar) {
          if (!io.MouseDown[0]) {
            sizingRBar = false;
          } else {
            float barNewWidth = AnchorMax(barWidthInPixels + io.MouseDelta[0], MinBarWidth);
            float barRatio = barNewWidth / barWidthInPixels;
            framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
            int newVisibleFrameCount = int((canvas_size[0] - legendWidth) / framePixelWidthTarget);
            int lastFrame = *firstFrame + newVisibleFrameCount;
            if (lastFrame > sequence->GetFrameMax()) {
              framePixelWidthTarget = framePixelWidth = (canvas_size[0] - legendWidth) /
                                                        float(sequence->GetFrameMax() -
                                                              *firstFrame);
            }
          }
        } else if (sizingLBar) {
          if (!io.MouseDown[0]) {
            sizingLBar = false;
          } else {
            if (fabsf(io.MouseDelta[0]) > FLT_EPSILON) {
              float barNewWidth = AnchorMax(barWidthInPixels - io.MouseDelta[0], MinBarWidth);
              float barRatio = barNewWidth / barWidthInPixels;
              float previousFramePixelWidthTarget = framePixelWidthTarget;
              framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
              int newVisibleFrameCount = int(visibleFrameCount / barRatio);
              int newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
              newFirstFrame = AnchorClamp(
                newFirstFrame,
                sequence->GetFrameMin(),
                AnchorMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
              if (newFirstFrame == *firstFrame) {
                framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
              } else {
                *firstFrame = newFirstFrame;
              }
            }
          }
        } else {
          if (MovingScrollBar) {
            if (!io.MouseDown[0]) {
              MovingScrollBar = false;
            } else {
              float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
              *firstFrame = int((io.MousePos[0] - panningViewSource[0]) / framesPerPixelInBar) -
                            panningViewFrame;
              *firstFrame = AnchorClamp(
                *firstFrame,
                sequence->GetFrameMin(),
                AnchorMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
            }
          } else {
            if (scrollBarThumb.Contains(io.MousePos) && ANCHOR::IsMouseClicked(0) && firstFrame &&
                !MovingCurrentFrame && movingEntry == -1) {
              MovingScrollBar = true;
              panningViewSource = io.MousePos;
              panningViewFrame = -*firstFrame;
            }
            if (!sizingRBar && onRight && ANCHOR::IsMouseClicked(0))
              sizingRBar = true;
            if (!sizingLBar && onLeft && ANCHOR::IsMouseClicked(0))
              sizingLBar = true;
          }
        }
      }
    }

    ANCHOR::EndGroup();

    if (regionRect.Contains(io.MousePos)) {
      bool overCustomDraw = false;
      for (auto &custom : customDraws) {
        if (custom.customRect.Contains(io.MousePos)) {
          overCustomDraw = true;
        }
      }
      if (overCustomDraw) {
      } else {
#if 0
            frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos[0] - (float)legendWidth - canvas_pos[0]) / (canvas_size[0] - legendWidth)));
            //frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

            /**firstFrame -= frameOverCursor;
            *firstFrame *= framePixelWidthTarget / framePixelWidth;
            *firstFrame += frameOverCursor;*/
            if (io.MouseWheel < -FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 1.1f);
               framePixelWidthTarget *= 0.9f;
               *firstFrame += frameOverCursor;
            }

            if (io.MouseWheel > FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 0.9f);
               framePixelWidthTarget *= 1.1f;
               *firstFrame += frameOverCursor;
            }
#endif
      }
    }

    if (expanded) {
      bool overExpanded = SequencerAddDelButton(draw_list,
                                                wabi::GfVec2f(canvas_pos[0] + 2, canvas_pos[1] + 2),
                                                !*expanded);
      if (overExpanded && io.MouseReleased[0])
        *expanded = !*expanded;
    }

    if (delEntry != -1) {
      sequence->Del(delEntry);
      if (selectedEntry &&
          (*selectedEntry == delEntry || *selectedEntry >= sequence->GetItemCount()))
        *selectedEntry = -1;
    }

    if (dupEntry != -1) {
      sequence->Duplicate(dupEntry);
    }
    return ret;
  }
}  // namespace AnchorSequencer
