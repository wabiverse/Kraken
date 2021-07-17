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

#include "ImSequencer.h"
#include "ANCHOR_api.h"
#include "imgui_internal.h"
#include <cstdlib>

namespace ImSequencer
{
#ifndef ANCHOR_DEFINE_MATH_OPERATORS
   static GfVec2f operator+(const GfVec2f& a, const GfVec2f& b) {
      return GfVec2f(a.x + b.x, a.y + b.y);
   }
#endif
   static bool SequencerAddDelButton(AnchorDrawList* draw_list, GfVec2f pos, bool add = true)
   {
      AnchorIO& io = Anchor::GetIO();
      AnchorRect delRect(pos, GfVec2f(pos.x + 16, pos.y + 16));
      bool overDel = delRect.Contains(io.MousePos);
      int delColor = overDel ? 0xFFAAAAAA : 0x50000000;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
      draw_list->AddLine(GfVec2f(delRect.Min.x + 3, midy), GfVec2f(delRect.Max.x - 3, midy), delColor, 2);
      if (add)
         draw_list->AddLine(GfVec2f(midx, delRect.Min.y + 3), GfVec2f(midx, delRect.Max.y - 3), delColor, 2);
      return overDel;
   }

   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions)
   {
      bool ret = false;
      AnchorIO& io = Anchor::GetIO();
      int cx = (int)(io.MousePos.x);
      int cy = (int)(io.MousePos.y);
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
      Anchor::BeginGroup();

      AnchorDrawList* draw_list = Anchor::GetWindowDrawList();
      GfVec2f canvas_pos = Anchor::GetCursorScreenPos();            // AnchorDrawList API uses screen coordinates!
      GfVec2f canvas_size = Anchor::GetContentRegionAvail();        // Resize canvas to what's available
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
         AnchorRect customRect;
         AnchorRect legendRect;
         AnchorRect clippingRect;
         AnchorRect legendClippingRect;
      };
      ImVector<CustomDraw> customDraws;
      ImVector<CustomDraw> compactCustomDraws;
      // zoom in/out
      const int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
      const float barWidthRatio = AnchorMin(visibleFrameCount / (float)frameCount, 1.f);
      const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

      AnchorRect regionRect(canvas_pos, canvas_pos + canvas_size);

      static bool panningView = false;
      static GfVec2f panningViewSource;
      static int panningViewFrame;
      if (Anchor::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
      {
         if (!panningView)
         {
            panningViewSource = io.MousePos;
            panningView = true;
            panningViewFrame = *firstFrame;
         }
         *firstFrame = panningViewFrame - int((io.MousePos.x - panningViewSource.x) / framePixelWidth);
         *firstFrame = AnchorClamp(*firstFrame, sequence->GetFrameMin(), sequence->GetFrameMax() - visibleFrameCount);
      }
      if (panningView && !io.MouseDown[2])
      {
         panningView = false;
      }
      framePixelWidthTarget = AnchorClamp(framePixelWidthTarget, 0.1f, 50.f);

      framePixelWidth = AnchorLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

      frameCount = sequence->GetFrameMax() - sequence->GetFrameMin();
      if (visibleFrameCount >= frameCount && firstFrame)
         *firstFrame = sequence->GetFrameMin();


      // --
      if (expanded && !*expanded)
      {
         Anchor::InvisibleButton("canvas", GfVec2f(canvas_size.x - canvas_pos.x, (float)ItemHeight));
         draw_list->AddRectFilled(canvas_pos, GfVec2f(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
         char tmps[512];
         AnchorFormatString(tmps, IM_ARRAYSIZE(tmps), "%d Frames / %d entries", frameCount, sequenceCount);
         draw_list->AddText(GfVec2f(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
      }
      else
      {
         bool hasScrollBar(true);
         /*
         int framesPixelWidth = int(frameCount * framePixelWidth);
         if ((framesPixelWidth + legendWidth) >= canvas_size.x)
         {
             hasScrollBar = true;
         }
         */
         // test scroll area
         GfVec2f headerSize(canvas_size.x, (float)ItemHeight);
         GfVec2f scrollBarSize(canvas_size.x, 14.f);
         Anchor::InvisibleButton("topBar", headerSize);
         draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
         GfVec2f childFramePos = Anchor::GetCursorScreenPos();
         GfVec2f childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
         Anchor::PushStyleColor(AnchorCol_FrameBg, 0);
         Anchor::BeginChildFrame(889, childFrameSize);
         sequence->focused = Anchor::IsWindowFocused();
         Anchor::InvisibleButton("contentBar", GfVec2f(canvas_size.x, float(controlHeight)));
         const GfVec2f contentMin = Anchor::GetItemRectMin();
         const GfVec2f contentMax = Anchor::GetItemRectMax();
         const AnchorRect contentRect(contentMin, contentMax);
         const float contentHeight = contentMax.y - contentMin.y;

         // full background
         draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);

         // current frame top
         AnchorRect topRect(GfVec2f(canvas_pos.x + legendWidth, canvas_pos.y), GfVec2f(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

         if (!MovingCurrentFrame && !MovingScrollBar && movingEntry == -1 && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
         {
            MovingCurrentFrame = true;
         }
         if (MovingCurrentFrame)
         {
            if (frameCount)
            {
               *currentFrame = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed;
               if (*currentFrame < sequence->GetFrameMin())
                  *currentFrame = sequence->GetFrameMin();
               if (*currentFrame >= sequence->GetFrameMax())
                  *currentFrame = sequence->GetFrameMax();
            }
            if (!io.MouseDown[0])
               MovingCurrentFrame = false;
         }

         //header
         draw_list->AddRectFilled(canvas_pos, GfVec2f(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
         if (sequenceOptions & SEQUENCER_ADD)
         {
            if (SequencerAddDelButton(draw_list, GfVec2f(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
               Anchor::OpenPopup("addEntry");

            if (Anchor::BeginPopup("addEntry"))
            {
               for (int i = 0; i < sequence->GetItemTypeCount(); i++)
                  if (Anchor::Selectable(sequence->GetItemTypeName(i)))
                  {
                     sequence->Add(i);
                     *selectedEntry = sequence->GetItemCount() - 1;
                  }

               Anchor::EndPopup();
               popupOpened = true;
            }
         }

         //header frame number and lines
         int modFrameCount = 10;
         int frameStep = 1;
         while ((modFrameCount * framePixelWidth) < 150)
         {
            modFrameCount *= 2;
            frameStep *= 2;
         };
         int halfModFrameCount = modFrameCount / 2;

         auto drawLine = [&](int i, int regionHeight) {
            bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence->GetFrameMax() || i == sequence->GetFrameMin());
            bool halfIndex = (i % halfModFrameCount) == 0;
            int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
            int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
            int tiretEnd = baseIndex ? regionHeight : ItemHeight;

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               draw_list->AddLine(GfVec2f((float)px, canvas_pos.y + (float)tiretStart), GfVec2f((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(GfVec2f((float)px, canvas_pos.y + (float)ItemHeight), GfVec2f((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
            }

            if (baseIndex && px > (canvas_pos.x + legendWidth))
            {
               char tmps[512];
               AnchorFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
               draw_list->AddText(GfVec2f((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
            }

         };

         auto drawLineContent = [&](int i, int /*regionHeight*/) {
            int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
            int tiretStart = int(contentMin.y);
            int tiretEnd = int(contentMax.y);

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               //draw_list->AddLine(GfVec2f((float)px, canvas_pos.y + (float)tiretStart), GfVec2f((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(GfVec2f(float(px), float(tiretStart)), GfVec2f(float(px), float(tiretEnd)), 0x30606060, 1);
            }
         };
         for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
         {
            drawLine(i, ItemHeight);
         }
         drawLine(sequence->GetFrameMin(), ItemHeight);
         drawLine(sequence->GetFrameMax(), ItemHeight);
         /*
                  draw_list->AddLine(canvas_pos, GfVec2f(canvas_pos.x, canvas_pos.y + controlHeight), 0xFF000000, 1);
                  draw_list->AddLine(GfVec2f(canvas_pos.x, canvas_pos.y + ItemHeight), GfVec2f(canvas_size.x, canvas_pos.y + ItemHeight), 0xFF000000, 1);
                  */
                  // clip content

         draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

         // draw item names in the legend rect on the left
         size_t customHeight = 0;
         for (int i = 0; i < sequenceCount; i++)
         {
            int type;
            sequence->Get(i, NULL, NULL, &type, NULL);
            GfVec2f tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
            draw_list->AddText(tpos, 0xFFFFFFFF, sequence->GetItemLabel(i));

            if (sequenceOptions & SEQUENCER_DEL)
            {
               bool overDel = SequencerAddDelButton(draw_list, GfVec2f(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false);
               if (overDel && io.MouseReleased[0])
                  delEntry = i;

               bool overDup = SequencerAddDelButton(draw_list, GfVec2f(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true);
               if (overDup && io.MouseReleased[0])
                  dupEntry = i;
            }
            customHeight += sequence->GetCustomHeight(i);
         }

         // clipping rect so items bars are not visible in the legend on the left when scrolled
         //

         // slots background
         customHeight = 0;
         for (int i = 0; i < sequenceCount; i++)
         {
            unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

            size_t localCustomHeight = sequence->GetCustomHeight(i);
            GfVec2f pos = GfVec2f(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
            GfVec2f sz = GfVec2f(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);
            if (!popupOpened && cy >= pos.y && cy < pos.y + (ItemHeight + localCustomHeight) && movingEntry == -1 && cx>contentMin.x && cx < contentMin.x + canvas_size.x)
            {
               col += 0x80201008;
               pos.x -= legendWidth;
            }
            draw_list->AddRectFilled(pos, sz, col, 0);
            customHeight += localCustomHeight;
         }

         draw_list->PushClipRect(childFramePos + GfVec2f(float(legendWidth), 0.f), childFramePos + childFrameSize);

         // vertical frame lines in content area
         for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
         {
            drawLineContent(i, int(contentHeight));
         }
         drawLineContent(sequence->GetFrameMin(), int(contentHeight));
         drawLineContent(sequence->GetFrameMax(), int(contentHeight));

         // selection
         bool selected = selectedEntry && (*selectedEntry >= 0);
         if (selected)
         {
            customHeight = 0;
            for (int i = 0; i < *selectedEntry; i++)
               customHeight += sequence->GetCustomHeight(i);;
            draw_list->AddRectFilled(GfVec2f(contentMin.x, contentMin.y + ItemHeight * *selectedEntry + customHeight), GfVec2f(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selectedEntry + 1) + customHeight), 0x801080FF, 1.f);
         }

         // slots
         customHeight = 0;
         for (int i = 0; i < sequenceCount; i++)
         {
            int* start, * end;
            unsigned int color;
            sequence->Get(i, &start, &end, NULL, &color);
            size_t localCustomHeight = sequence->GetCustomHeight(i);

            GfVec2f pos = GfVec2f(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
            GfVec2f slotP1(pos.x + *start * framePixelWidth, pos.y + 2);
            GfVec2f slotP2(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
            GfVec2f slotP3(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2 + localCustomHeight);
            unsigned int slotColor = color | 0xFF000000;
            unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

            if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
            {
               draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
               draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
            }
            if (AnchorRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
            {
               sequence->DoubleClick(i);
            }
            AnchorRect rects[3] = { AnchorRect(slotP1, GfVec2f(slotP1.x + framePixelWidth / 2, slotP2.y))
                , AnchorRect(GfVec2f(slotP2.x - framePixelWidth / 2, slotP1.y), slotP2)
                , AnchorRect(slotP1, slotP2) };

            const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (selected ? 0 : 0x202020) };
            if (movingEntry == -1 && (sequenceOptions & SEQUENCER_EDIT_STARTEND))// TODOFOCUS && backgroundRect.Contains(io.MousePos))
            {
               for (int j = 2; j >= 0; j--)
               {
                  AnchorRect& rc = rects[j];
                  if (!rc.Contains(io.MousePos))
                     continue;
                  draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
               }

               for (int j = 0; j < 3; j++)
               {
                  AnchorRect& rc = rects[j];
                  if (!rc.Contains(io.MousePos))
                     continue;
                  if (!AnchorRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
                     continue;
                  if (Anchor::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
                  {
                     movingEntry = i;
                     movingPos = cx;
                     movingPart = j + 1;
                     sequence->BeginEdit(movingEntry);
                     break;
                  }
               }
            }

            // custom draw
            if (localCustomHeight > 0)
            {
               GfVec2f rp(canvas_pos.x, contentMin.y + ItemHeight * i + 1 + customHeight);
               AnchorRect customRect(rp + GfVec2f(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(ItemHeight)),
                  rp + GfVec2f(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + ItemHeight)));
               AnchorRect clippingRect(rp + GfVec2f(float(legendWidth), float(ItemHeight)), rp + GfVec2f(canvas_size.x, float(localCustomHeight + ItemHeight)));

               AnchorRect legendRect(rp + GfVec2f(0.f, float(ItemHeight)), rp + GfVec2f(float(legendWidth), float(localCustomHeight)));
               AnchorRect legendClippingRect(canvas_pos + GfVec2f(0.f, float(ItemHeight)), canvas_pos + GfVec2f(float(legendWidth), float(localCustomHeight + ItemHeight)));
               customDraws.push_back({ i, customRect, legendRect, clippingRect, legendClippingRect });
            }
            else
            {
               GfVec2f rp(canvas_pos.x, contentMin.y + ItemHeight * i + customHeight);
               AnchorRect customRect(rp + GfVec2f(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(0.f)),
                  rp + GfVec2f(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
               AnchorRect clippingRect(rp + GfVec2f(float(legendWidth), float(0.f)), rp + GfVec2f(canvas_size.x, float(ItemHeight)));

               compactCustomDraws.push_back({ i, customRect, AnchorRect(), clippingRect, AnchorRect() });
            }
            customHeight += localCustomHeight;
         }


         // moving
         if (/*backgroundRect.Contains(io.MousePos) && */movingEntry >= 0)
         {
            Anchor::CaptureMouseFromApp();
            int diffFrame = int((cx - movingPos) / framePixelWidth);
            if (std::abs(diffFrame) > 0)
            {
               int* start, * end;
               sequence->Get(movingEntry, &start, &end, NULL, NULL);
               if (selectedEntry)
                  *selectedEntry = movingEntry;
               int& l = *start;
               int& r = *end;
               if (movingPart & 1)
                  l += diffFrame;
               if (movingPart & 2)
                  r += diffFrame;
               if (l < 0)
               {
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
            if (!io.MouseDown[0])
            {
               // single select
               if (!diffFrame && movingPart && selectedEntry)
               {
                  *selectedEntry = movingEntry;
                  ret = true;
               }

               movingEntry = -1;
               sequence->EndEdit();
            }
         }

         // cursor
         if (currentFrame && firstFrame && *currentFrame >= *firstFrame && *currentFrame <= sequence->GetFrameMax())
         {
            static const float cursorWidth = 8.f;
            float cursorOffset = contentMin.x + legendWidth + (*currentFrame - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
            draw_list->AddLine(GfVec2f(cursorOffset, canvas_pos.y), GfVec2f(cursorOffset, contentMax.y), 0xA02A2AFF, cursorWidth);
            char tmps[512];
            AnchorFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *currentFrame);
            draw_list->AddText(GfVec2f(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
         }

         draw_list->PopClipRect();
         draw_list->PopClipRect();

         for (auto& customDraw : customDraws)
            sequence->CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect);
         for (auto& customDraw : compactCustomDraws)
            sequence->CustomDrawCompact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);

         // copy paste
         if (sequenceOptions & SEQUENCER_COPYPASTE)
         {
            AnchorRect rectCopy(GfVec2f(contentMin.x + 100, canvas_pos.y + 2)
               , GfVec2f(contentMin.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectCopy = rectCopy.Contains(io.MousePos);
            unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectCopy.Min, copyColor, "Copy");

            AnchorRect rectPaste(GfVec2f(contentMin.x + 140, canvas_pos.y + 2)
               , GfVec2f(contentMin.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectPaste = rectPaste.Contains(io.MousePos);
            unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

            if (inRectCopy && io.MouseReleased[0])
            {
               sequence->Copy();
            }
            if (inRectPaste && io.MouseReleased[0])
            {
               sequence->Paste();
            }
         }
         //

         Anchor::EndChildFrame();
         Anchor::PopStyleColor();
         if (hasScrollBar)
         {
            Anchor::InvisibleButton("scrollBar", scrollBarSize);
            GfVec2f scrollBarMin = Anchor::GetItemRectMin();
            GfVec2f scrollBarMax = Anchor::GetItemRectMax();

            // ratio = number of frames visible in control / number to total frames

            float startFrameOffset = ((float)(firstFrameUsed - sequence->GetFrameMin()) / (float)frameCount) * (canvas_size.x - legendWidth);
            GfVec2f scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
            GfVec2f scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

            AnchorRect scrollBarRect(scrollBarA, scrollBarB);
            bool inScrollBar = scrollBarRect.Contains(io.MousePos);

            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


            GfVec2f scrollBarC(scrollBarMin.x + legendWidth + startFrameOffset, scrollBarMin.y);
            GfVec2f scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
            draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

            AnchorRect barHandleLeft(scrollBarC, GfVec2f(scrollBarC.x + 14, scrollBarD.y));
            AnchorRect barHandleRight(GfVec2f(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

            bool onLeft = barHandleLeft.Contains(io.MousePos);
            bool onRight = barHandleRight.Contains(io.MousePos);

            static bool sizingRBar = false;
            static bool sizingLBar = false;

            draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
            draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

            AnchorRect scrollBarThumb(scrollBarC, scrollBarD);
            static const float MinBarWidth = 44.f;
            if (sizingRBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingRBar = false;
               }
               else
               {
                  float barNewWidth = AnchorMax(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
                  float barRatio = barNewWidth / barWidthInPixels;
                  framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                  int newVisibleFrameCount = int((canvas_size.x - legendWidth) / framePixelWidthTarget);
                  int lastFrame = *firstFrame + newVisibleFrameCount;
                  if (lastFrame > sequence->GetFrameMax())
                  {
                     framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(sequence->GetFrameMax() - *firstFrame);
                  }
               }
            }
            else if (sizingLBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingLBar = false;
               }
               else
               {
                  if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
                  {
                     float barNewWidth = AnchorMax(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
                     float barRatio = barNewWidth / barWidthInPixels;
                     float previousFramePixelWidthTarget = framePixelWidthTarget;
                     framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                     int newVisibleFrameCount = int(visibleFrameCount / barRatio);
                     int newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
                     newFirstFrame = AnchorClamp(newFirstFrame, sequence->GetFrameMin(), AnchorMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                     if (newFirstFrame == *firstFrame)
                     {
                        framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
                     }
                     else
                     {
                        *firstFrame = newFirstFrame;
                     }
                  }
               }
            }
            else
            {
               if (MovingScrollBar)
               {
                  if (!io.MouseDown[0])
                  {
                     MovingScrollBar = false;
                  }
                  else
                  {
                     float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
                     *firstFrame = int((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
                     *firstFrame = AnchorClamp(*firstFrame, sequence->GetFrameMin(), AnchorMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                  }
               }
               else
               {
                  if (scrollBarThumb.Contains(io.MousePos) && Anchor::IsMouseClicked(0) && firstFrame && !MovingCurrentFrame && movingEntry == -1)
                  {
                     MovingScrollBar = true;
                     panningViewSource = io.MousePos;
                     panningViewFrame = -*firstFrame;
                  }
                  if (!sizingRBar && onRight && Anchor::IsMouseClicked(0))
                     sizingRBar = true;
                  if (!sizingLBar && onLeft && Anchor::IsMouseClicked(0))
                     sizingLBar = true;

               }
            }
         }
      }

      Anchor::EndGroup();

      if (regionRect.Contains(io.MousePos))
      {
         bool overCustomDraw = false;
         for (auto& custom : customDraws)
         {
            if (custom.customRect.Contains(io.MousePos))
            {
               overCustomDraw = true;
            }
         }
         if (overCustomDraw)
         {
         }
         else
         {
#if 0
            frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
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

      if (expanded)
      {
         bool overExpanded = SequencerAddDelButton(draw_list, GfVec2f(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
         if (overExpanded && io.MouseReleased[0])
            *expanded = !*expanded;
      }

      if (delEntry != -1)
      {
         sequence->Del(delEntry);
         if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= sequence->GetItemCount()))
            *selectedEntry = -1;
      }

      if (dupEntry != -1)
      {
         sequence->Duplicate(dupEntry);
      }
      return ret;
   }
}
