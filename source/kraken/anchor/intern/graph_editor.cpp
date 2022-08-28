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

#ifndef NOMINMAX
#  define NOMINMAX
#endif
#define ANCHOR_DEFINE_MATH_OPERATORS

#include "ANCHOR_api.h"
#include "ANCHOR_internal.h"
#include "ANCHOR_graph_editor.h"

#include <math.h>
#include <vector>
#include <float.h>
#include <array>

KRAKEN_NAMESPACE_USING

namespace AnchorGraphEditor
{

  static inline float Distance(const wabi::GfVec2f &a, const wabi::GfVec2f &b)
  {
    return sqrtf((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));
  }

  static inline float sign(float v)
  {
    return (v >= 0.f) ? 1.f : -1.f;
  }

  static wabi::GfVec2f GetInputSlotPos(Delegate &delegate,
                                 const Node &node,
                                 SlotIndex slotIndex,
                                 float factor)
  {
    wabi::GfVec2f Size = node.mRect.GetSize() * factor;
    size_t InputsCount = delegate.GetTemplate(node.mTemplateIndex).mInputCount;
    return wabi::GfVec2f(node.mRect.Min[0] * factor,
                   node.mRect.Min[1] * factor +
                     Size[1] * ((float)slotIndex + 1) / ((float)InputsCount + 1) + 8.f);
  }

  static wabi::GfVec2f GetOutputSlotPos(Delegate &delegate,
                                  const Node &node,
                                  SlotIndex slotIndex,
                                  float factor)
  {
    wabi::GfVec2f Size = node.mRect.GetSize() * factor;
    size_t OutputsCount = delegate.GetTemplate(node.mTemplateIndex).mOutputCount;
    return wabi::GfVec2f(node.mRect.Min[0] * factor + Size[0],
                   node.mRect.Min[1] * factor +
                     Size[1] * ((float)slotIndex + 1) / ((float)OutputsCount + 1) + 8.f);
  }

  static AnchorBBox GetNodeRect(const Node &node, float factor)
  {
    wabi::GfVec2f Size = node.mRect.GetSize() * factor;
    return AnchorBBox(node.mRect.Min * factor, node.mRect.Min * factor + Size);
  }

  static wabi::GfVec2f editingNodeSource;
  static bool editingInput = false;
  static wabi::GfVec2f captureOffset;

  enum NodeOperation
  {
    NO_None,
    NO_EditingLink,
    NO_QuadSelecting,
    NO_MovingNodes,
    NO_EditInput,
    NO_PanView,
  };
  static NodeOperation nodeOperation = NO_None;

  static void HandleZoomScroll(AnchorBBox regionRect, ViewState &viewState, const Options &options)
  {
    AnchorIO &io = ANCHOR::GetIO();

    if (regionRect.Contains(io.MousePos)) {
      if (io.MouseWheel < -FLT_EPSILON) {
        viewState.mFactorTarget *= 1.f - options.mZoomRatio;
      }

      if (io.MouseWheel > FLT_EPSILON) {
        viewState.mFactorTarget *= 1.0f + options.mZoomRatio;
      }
    }

    wabi::GfVec2f mouseWPosPre = (io.MousePos - ANCHOR::GetCursorScreenPos()) / viewState.mFactor;
    viewState.mFactorTarget = AnchorClamp(viewState.mFactorTarget,
                                          options.mMinZoom,
                                          options.mMaxZoom);
    viewState.mFactor = AnchorLerp(viewState.mFactor,
                                   viewState.mFactorTarget,
                                   options.mZoomLerpFactor);
    wabi::GfVec2f mouseWPosPost = (io.MousePos - ANCHOR::GetCursorScreenPos()) / viewState.mFactor;
    if (ANCHOR::IsMousePosValid()) {
      viewState.mPosition += mouseWPosPost - mouseWPosPre;
    }
  }

  void GraphEditorClear()
  {
    nodeOperation = NO_None;
  }

  static void FitNodes(Delegate &delegate,
                       ViewState &viewState,
                       const wabi::GfVec2f viewSize,
                       bool selectedNodesOnly)
  {
    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount) {
      return;
    }

    bool validNode = false;
    wabi::GfVec2f mapmin(FLT_MAX, FLT_MAX);
    wabi::GfVec2f mapmax(-FLT_MAX, -FLT_MAX);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
      const Node &node = delegate.GetNode(nodeIndex);

      if (selectedNodesOnly && !node.mSelected) {
        continue;
      }

      mapmin = AnchorMin(mapmin, node.mRect.Min);
      mapmin = AnchorMin(mapmin, node.mRect.Max);
      mapmax = AnchorMax(mapmax, node.mRect.Min);
      mapmax = AnchorMax(mapmax, node.mRect.Max);
      validNode = true;
    }

    if (!validNode) {
      return;
    }

    mapmin -= viewSize * 0.05f;
    mapmax += viewSize * 0.05f;
    wabi::GfVec2f nodesSize = mapmax - mapmin;
    wabi::GfVec2f nodeCenter = (mapmax + mapmin) * 0.5f;

    float ratioY = viewSize[1] / nodesSize[1];
    float ratioX = viewSize[0] / nodesSize[0];

    viewState.mFactor = viewState.mFactorTarget = AnchorMin(AnchorMin(ratioY, ratioX), 1.f);
    viewState.mPosition = wabi::GfVec2f(-nodeCenter[0], -nodeCenter[1]) +
                          (viewSize * 0.5f) / viewState.mFactorTarget;
  }

  static void DisplayLinks(Delegate &delegate,
                           AnchorDrawList *drawList,
                           const wabi::GfVec2f offset,
                           const float factor,
                           const AnchorBBox regionRect,
                           NodeIndex hoveredNode,
                           const Options &options)
  {
    const size_t linkCount = delegate.GetLinkCount();
    for (LinkIndex linkIndex = 0; linkIndex < linkCount; linkIndex++) {
      const auto link = delegate.GetLink(linkIndex);
      const auto nodeInput = delegate.GetNode(link.mInputNodeIndex);
      const auto nodeOutput = delegate.GetNode(link.mOutputNodeIndex);
      wabi::GfVec2f p1 = offset + GetOutputSlotPos(delegate, nodeInput, link.mInputSlotIndex, factor);
      wabi::GfVec2f p2 = offset + GetInputSlotPos(delegate, nodeOutput, link.mOutputSlotIndex, factor);

      // con. view clipping
      if ((p1[1] < 0.f && p2[1] < 0.f) ||
          (p1[1] > regionRect.Max[1] && p2[1] > regionRect.Max[1]) ||
          (p1[0] < 0.f && p2[0] < 0.f) || (p1[0] > regionRect.Max[0] && p2[0] > regionRect.Max[0]))
        continue;

      bool highlightCons = hoveredNode == link.mInputNodeIndex ||
                           hoveredNode == link.mOutputNodeIndex;
      uint32_t col = delegate.GetTemplate(nodeInput.mTemplateIndex).mHeaderColor |
                     (highlightCons ? 0xF0F0F0 : 0);
      if (options.mDisplayLinksAsCurves) {
        // curves
        drawList->AddBezierCurve(p1,
                                 p1 + wabi::GfVec2f(50, 0) * factor,
                                 p2 + wabi::GfVec2f(-50, 0) * factor,
                                 p2,
                                 0xFF000000,
                                 options.mLineThickness * 1.5f * factor);
        drawList->AddBezierCurve(p1,
                                 p1 + wabi::GfVec2f(50, 0) * factor,
                                 p2 + wabi::GfVec2f(-50, 0) * factor,
                                 p2,
                                 col,
                                 options.mLineThickness * 1.5f * factor);
        /*
              wabi::GfVec2f p10 = p1 + wabi::GfVec2f(20.f * factor, 0.f);
              wabi::GfVec2f p20 = p2 - wabi::GfVec2f(20.f * factor, 0.f);

              wabi::GfVec2f dif = p20 - p10;
              wabi::GfVec2f p1a, p1b;
              if (fabsf(dif[0]) > fabsf(dif[1]))
              {
                  p1a = p10 + wabi::GfVec2f(fabsf(fabsf(dif[0]) - fabsf(dif[1])) * 0.5 * sign(dif[0]),
           0.f); p1b = p1a + wabi::GfVec2f(fabsf(dif[1]) * sign(dif[0]) , dif[1]);
              }
              else
              {
                  p1a = p10 + wabi::GfVec2f(0.f, fabsf(fabsf(dif[1]) - fabsf(dif[0])) * 0.5 *
           sign(dif[1])); p1b = p1a + wabi::GfVec2f(dif[0], fabsf(dif[0]) * sign(dif[1]));
              }
              drawList->AddLine(p1,  p10, col, 3.f * factor);
              drawList->AddLine(p10, p1a, col, 3.f * factor);
              drawList->AddLine(p1a, p1b, col, 3.f * factor);
              drawList->AddLine(p1b, p20, col, 3.f * factor);
              drawList->AddLine(p20,  p2, col, 3.f * factor);
              */
      } else {
        // straight lines
        std::array<wabi::GfVec2f, 6> pts;
        int ptCount = 0;
        wabi::GfVec2f dif = p2 - p1;

        wabi::GfVec2f p1a, p1b;
        const float limitx = 12.f * factor;
        if (dif[0] < limitx) {
          wabi::GfVec2f p10 = p1 + wabi::GfVec2f(limitx, 0.f);
          wabi::GfVec2f p20 = p2 - wabi::GfVec2f(limitx, 0.f);

          dif = p20 - p10;
          p1a = p10 + wabi::GfVec2f(0.f, dif[1] * 0.5f);
          p1b = p1a + wabi::GfVec2f(dif[0], 0.f);

          pts = {p1, p10, p1a, p1b, p20, p2};
          ptCount = 6;
        } else {
          if (fabsf(dif[1]) < 1.f) {
            pts = {p1, (p1 + p2) * 0.5f, p2};
            ptCount = 3;
          } else {
            if (fabsf(dif[1]) < 10.f) {
              if (fabsf(dif[0]) > fabsf(dif[1])) {
                p1a = p1 +
                      wabi::GfVec2f(fabsf(fabsf(dif[0]) - fabsf(dif[1])) * 0.5f * sign(dif[0]), 0.f);
                p1b = p1a + wabi::GfVec2f(fabsf(dif[1]) * sign(dif[0]), dif[1]);
              } else {
                p1a = p1 +
                      wabi::GfVec2f(0.f, fabsf(fabsf(dif[1]) - fabsf(dif[0])) * 0.5f * sign(dif[1]));
                p1b = p1a + wabi::GfVec2f(dif[0], fabsf(dif[0]) * sign(dif[1]));
              }
            } else {
              if (fabsf(dif[0]) > fabsf(dif[1])) {
                float d = fabsf(dif[1]) * sign(dif[0]) * 0.5f;
                p1a = p1 + wabi::GfVec2f(d, dif[1] * 0.5f);
                p1b = p1a + wabi::GfVec2f(fabsf(fabsf(dif[0]) - fabsf(d) * 2.f) * sign(dif[0]), 0.f);
              } else {
                float d = fabsf(dif[0]) * sign(dif[1]) * 0.5f;
                p1a = p1 + wabi::GfVec2f(dif[0] * 0.5f, d);
                p1b = p1a + wabi::GfVec2f(0.f, fabsf(fabsf(dif[1]) - fabsf(d) * 2.f) * sign(dif[1]));
              }
            }
            pts = {p1, p1a, p1b, p2};
            ptCount = 4;
          }
        }
        float highLightFactor = factor * (highlightCons ? 2.0f : 1.f);
        for (int pass = 0; pass < 2; pass++) {
          drawList->AddPolyline(pts.data(),
                                ptCount,
                                pass ? col : 0xFF000000,
                                false,
                                (pass ? options.mLineThickness : (options.mLineThickness * 1.5f)) *
                                  highLightFactor);
        }
      }
    }
  }

  static void HandleQuadSelection(Delegate &delegate,
                                  AnchorDrawList *drawList,
                                  const wabi::GfVec2f offset,
                                  const float factor,
                                  AnchorBBox contentRect,
                                  const Options &options)
  {
    if (!options.mAllowQuadSelection) {
      return;
    }
    AnchorIO &io = ANCHOR::GetIO();
    static wabi::GfVec2f quadSelectPos;
    // auto& nodes = delegate->GetNodes();
    auto nodeCount = delegate.GetNodeCount();

    if (nodeOperation == NO_QuadSelecting && ANCHOR::IsWindowFocused()) {
      const wabi::GfVec2f bmin = AnchorMin(quadSelectPos, io.MousePos);
      const wabi::GfVec2f bmax = AnchorMax(quadSelectPos, io.MousePos);
      drawList->AddRectFilled(bmin, bmax, options.mQuadSelection, 1.f);
      drawList->AddRect(bmin, bmax, options.mQuadSelectionBorder, 1.f);
      if (!io.MouseDown[0]) {
        if (!io.KeyCtrl && !io.KeyShift) {
          for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            delegate.SelectNode(nodeIndex, false);
          }
        }

        nodeOperation = NO_None;
        AnchorBBox selectionRect(bmin, bmax);
        for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
          const auto node = delegate.GetNode(nodeIndex);
          wabi::GfVec2f nodeRectangleMin = offset + node.mRect.Min * factor;
          wabi::GfVec2f nodeRectangleMax = nodeRectangleMin + node.mRect.GetSize() * factor;
          if (selectionRect.Overlaps(AnchorBBox(nodeRectangleMin, nodeRectangleMax))) {
            if (io.KeyCtrl) {
              delegate.SelectNode(nodeIndex, false);
            } else {
              delegate.SelectNode(nodeIndex, true);
            }
          } else {
            if (!io.KeyShift) {
              delegate.SelectNode(nodeIndex, false);
            }
          }
        }
      }
    } else if (nodeOperation == NO_None && io.MouseDown[0] && ANCHOR::IsWindowFocused() &&
               contentRect.Contains(io.MousePos)) {
      nodeOperation = NO_QuadSelecting;
      quadSelectPos = io.MousePos;
    }
  }

  static bool HandleConnections(AnchorDrawList *drawList,
                                NodeIndex nodeIndex,
                                const wabi::GfVec2f offset,
                                const float factor,
                                Delegate &delegate,
                                const Options &options,
                                bool bDrawOnly,
                                SlotIndex &inputSlotOver,
                                SlotIndex &outputSlotOver,
                                const bool inMinimap)
  {
    static NodeIndex editingNodeIndex;
    static SlotIndex editingSlotIndex;

    AnchorIO &io = ANCHOR::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const auto linkCount = delegate.GetLinkCount();

    size_t InputsCount = nodeTemplate.mInputCount;
    size_t OutputsCount = nodeTemplate.mOutputCount;
    inputSlotOver = -1;
    outputSlotOver = -1;

    // draw/use inputs/outputs
    bool hoverSlot = false;
    for (int i = 0; i < 2; i++) {
      float closestDistance = FLT_MAX;
      SlotIndex closestConn = -1;
      wabi::GfVec2f closestTextPos;
      wabi::GfVec2f closestPos;
      const size_t slotCount[2] = {InputsCount, OutputsCount};

      for (SlotIndex slotIndex = 0; slotIndex < slotCount[i]; slotIndex++) {
        const char **con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
        const char *conText = (con && con[slotIndex]) ? con[slotIndex] : "";

        wabi::GfVec2f p = offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) :
                                  GetInputSlotPos(delegate, node, slotIndex, factor));
        float distance = Distance(p, io.MousePos);
        bool overCon = (nodeOperation == NO_None || nodeOperation == NO_EditingLink) &&
                       (distance < options.mNodeSlotRadius * 2.f) && (distance < closestDistance);


        wabi::GfVec2f textSize;
        textSize = ANCHOR::CalcTextSize(conText);
        wabi::GfVec2f textPos = p + wabi::GfVec2f(-options.mNodeSlotRadius * (i ? -1.f : 1.f) *
                                          (overCon ? 3.f : 2.f) -
                                        (i ? 0 : textSize[0]),
                                      -textSize[1] / 2);

        AnchorBBox nodeRect = GetNodeRect(node, factor);
        if (!inMinimap &&
            (overCon || (nodeRect.Contains(io.MousePos - offset) && closestConn == -1 &&
                         (editingInput == (i != 0)) && nodeOperation == NO_EditingLink))) {
          closestDistance = distance;
          closestConn = slotIndex;
          closestTextPos = textPos;
          closestPos = p;

          if (i) {
            outputSlotOver = slotIndex;
          } else {
            inputSlotOver = slotIndex;
          }
        } else {
          const AnchorU32 *slotColorSource = i ? nodeTemplate.mOutputColors :
                                                 nodeTemplate.mInputColors;
          const AnchorU32 slotColor = slotColorSource ? slotColorSource[slotIndex] :
                                                        options.mDefaultSlotColor;
          drawList->AddCircleFilled(p, options.mNodeSlotRadius, ANCHOR_COL32(0, 0, 0, 200));
          drawList->AddCircleFilled(p, options.mNodeSlotRadius * 0.75f, slotColor);
          if (!options.mDrawIONameOnHover) {
            drawList->AddText(io.FontDefault,
                              14,
                              textPos + wabi::GfVec2f(2, 2),
                              ANCHOR_COL32(0, 0, 0, 255),
                              conText);
            drawList->AddText(io.FontDefault,
                              14,
                              textPos,
                              ANCHOR_COL32(150, 150, 150, 255),
                              conText);
          }
        }
      }

      if (closestConn != -1) {
        const char **con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
        const char *conText = (con && con[closestConn]) ? con[closestConn] : "";
        const AnchorU32 *slotColorSource = i ? nodeTemplate.mOutputColors :
                                               nodeTemplate.mInputColors;
        const AnchorU32 slotColor = slotColorSource ? slotColorSource[closestConn] :
                                                      options.mDefaultSlotColor;
        hoverSlot = true;
        drawList->AddCircleFilled(closestPos,
                                  options.mNodeSlotRadius * options.mNodeSlotHoverFactor * 0.75f,
                                  ANCHOR_COL32(0, 0, 0, 200));
        drawList->AddCircleFilled(closestPos,
                                  options.mNodeSlotRadius * options.mNodeSlotHoverFactor,
                                  slotColor);
        drawList->AddText(io.FontDefault,
                          16,
                          closestTextPos + wabi::GfVec2f(1, 1),
                          ANCHOR_COL32(0, 0, 0, 255),
                          conText);
        drawList->AddText(io.FontDefault,
                          16,
                          closestTextPos,
                          ANCHOR_COL32(250, 250, 250, 255),
                          conText);
        bool inputToOutput = (!editingInput && !i) || (editingInput && i);
        if (nodeOperation == NO_EditingLink && !io.MouseDown[0] && !bDrawOnly) {
          if (inputToOutput) {
            // check loopback
            Link nl;
            if (editingInput)
              nl = Link{nodeIndex, closestConn, editingNodeIndex, editingSlotIndex};
            else
              nl = Link{editingNodeIndex, editingSlotIndex, nodeIndex, closestConn};

            if (!delegate.AllowedLink(nl.mOutputNodeIndex, nl.mInputNodeIndex)) {
              break;
            }
            bool alreadyExisting = false;
            for (size_t linkIndex = 0; linkIndex < linkCount; linkIndex++) {
              const auto link = delegate.GetLink(linkIndex);
              if (!memcmp(&link, &nl, sizeof(Link))) {
                alreadyExisting = true;
                break;
              }
            }

            if (!alreadyExisting) {
              for (int linkIndex = 0; linkIndex < linkCount; linkIndex++) {
                const auto link = delegate.GetLink(linkIndex);
                if (link.mOutputNodeIndex == nl.mOutputNodeIndex &&
                    link.mOutputSlotIndex == nl.mOutputSlotIndex) {
                  delegate.DelLink(linkIndex);

                  break;
                }
              }

              delegate.AddLink(nl.mInputNodeIndex,
                               nl.mInputSlotIndex,
                               nl.mOutputNodeIndex,
                               nl.mOutputSlotIndex);
            }
          }
        }
        // when ANCHOR::IsWindowHovered() && !ANCHOR::IsAnyItemActive() is uncommented, one can't
        // click the node input/output when mouse is over the node itself.
        if (nodeOperation == NO_None &&
            /*ANCHOR::IsWindowHovered() && !ANCHOR::IsAnyItemActive() &&*/ io.MouseClicked[0] &&
            !bDrawOnly) {
          nodeOperation = NO_EditingLink;
          editingInput = i == 0;
          editingNodeSource = closestPos;
          editingNodeIndex = nodeIndex;
          editingSlotIndex = closestConn;
          if (editingInput) {
            // remove existing link
            for (int linkIndex = 0; linkIndex < linkCount; linkIndex++) {
              const auto link = delegate.GetLink(linkIndex);
              if (link.mOutputNodeIndex == nodeIndex && link.mOutputSlotIndex == closestConn) {
                delegate.DelLink(linkIndex);
                break;
              }
            }
          }
        }
      }
    }
    return hoverSlot;
  }

  static void DrawGrid(AnchorDrawList *drawList,
                       wabi::GfVec2f windowPos,
                       const ViewState &viewState,
                       const wabi::GfVec2f canvasSize,
                       AnchorU32 gridColor,
                       AnchorU32 gridColor2,
                       float gridSize)
  {
    float gridSpace = gridSize * viewState.mFactor;
    int divx = static_cast<int>(-viewState.mPosition[0] / gridSize);
    int divy = static_cast<int>(-viewState.mPosition[1] / gridSize);
    for (float x = fmodf(viewState.mPosition[0] * viewState.mFactor, gridSpace); x < canvasSize[0];
         x += gridSpace, divx++) {
      bool tenth = !(divx % 10);
      drawList->AddLine(wabi::GfVec2f(x, 0.0f) + windowPos,
                        wabi::GfVec2f(x, canvasSize[1]) + windowPos,
                        tenth ? gridColor2 : gridColor);
    }
    for (float y = fmodf(viewState.mPosition[1] * viewState.mFactor, gridSpace); y < canvasSize[1];
         y += gridSpace, divy++) {
      bool tenth = !(divy % 10);
      drawList->AddLine(wabi::GfVec2f(0.0f, y) + windowPos,
                        wabi::GfVec2f(canvasSize[0], y) + windowPos,
                        tenth ? gridColor2 : gridColor);
    }
  }

  // return true if node is hovered
  static bool DrawNode(AnchorDrawList *drawList,
                       NodeIndex nodeIndex,
                       const wabi::GfVec2f offset,
                       const float factor,
                       Delegate &delegate,
                       bool overInput,
                       const Options &options,
                       const bool inMinimap,
                       const AnchorBBox &viewPort)
  {
    AnchorIO &io = ANCHOR::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const wabi::GfVec2f nodeRectangleMin = offset + node.mRect.Min * factor;

    const bool old_any_active = ANCHOR::IsAnyItemActive();
    ANCHOR::SetCursorScreenPos(nodeRectangleMin);
    const wabi::GfVec2f nodeSize = node.mRect.GetSize() * factor;

    // test nested IO
    drawList->ChannelsSetCurrent(1);  // Background
    const size_t InputsCount = nodeTemplate.mInputCount;
    const size_t OutputsCount = nodeTemplate.mOutputCount;

    /*
      for (int i = 0; i < 2; i++)
      {
          const size_t slotCount[2] = {InputsCount, OutputsCount};

          for (size_t slotIndex = 0; slotIndex < slotCount[i]; slotIndex++)
          {
              const char* con = i ? nodeTemplate.mOutputNames[slotIndex] :
      nodeTemplate.mInputNames[slotIndex];//node.mOutputs[slot_idx] : node->mInputs[slot_idx]; if
      (!delegate->IsIOPinned(nodeIndex, slot_idx, i == 1))
              {

              }
              continue;

              wabi::GfVec2f p = offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) :
      GetInputSlotPos(delegate, node, slotIndex, factor)); const float arc = 28.f * (float(i) *
      0.3f + 1.0f)
      * (i ? 1.f : -1.f); const float ofs = 0.f;

              wabi::GfVec2f pts[3] = {p + wabi::GfVec2f(arc + ofs, 0.f), p + wabi::GfVec2f(0.f + ofs, -arc), p +
      wabi::GfVec2f(0.f + ofs, arc)}; drawList->AddTriangleFilled(pts[0], pts[1], pts[2], i ? 0xFFAA5030
      : 0xFF30AA50); drawList->AddTriangle(pts[0], pts[1], pts[2], 0xFF000000, 2.f);
          }
      }
      */

    ANCHOR::SetCursorScreenPos(nodeRectangleMin);
    float maxHeight = AnchorMin(viewPort.Max[1], nodeRectangleMin[1] + nodeSize[1]) -
                      nodeRectangleMin[1];
    float maxWidth = AnchorMin(viewPort.Max[0], nodeRectangleMin[0] + nodeSize[0]) -
                     nodeRectangleMin[0];
    ANCHOR::InvisibleButton("node", wabi::GfVec2f(maxWidth, maxHeight));
    // must be called right after creating the control we want to be able to move
    bool nodeMovingActive = ANCHOR::IsItemActive();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool nodeWidgetsActive = (!old_any_active && ANCHOR::IsAnyItemActive());
    wabi::GfVec2f nodeRectangleMax = nodeRectangleMin + nodeSize;

    bool nodeHovered = false;
    if (ANCHOR::IsItemHovered() && nodeOperation == NO_None && !overInput) {
      nodeHovered = true;
    }

    if (ANCHOR::IsWindowFocused()) {
      if ((nodeWidgetsActive || nodeMovingActive) && !inMinimap) {
        if (!node.mSelected) {
          if (!io.KeyShift) {
            const auto nodeCount = delegate.GetNodeCount();
            for (size_t i = 0; i < nodeCount; i++) {
              delegate.SelectNode(i, false);
            }
          }
          delegate.SelectNode(nodeIndex, true);
        }
      }
    }
    if (nodeMovingActive && io.MouseDown[0] && nodeHovered && !inMinimap) {
      if (nodeOperation != NO_MovingNodes) {
        nodeOperation = NO_MovingNodes;
      }
    }

    const bool currentSelectedNode = node.mSelected;
    const AnchorU32 node_bg_color = nodeHovered ? nodeTemplate.mBackgroundColorOver :
                                                  nodeTemplate.mBackgroundColor;

    drawList->AddRect(
      nodeRectangleMin,
      nodeRectangleMax,
      currentSelectedNode ? options.mSelectedNodeBorderColor : options.mNodeBorderColor,
      options.mRounding,
      AnchorDrawFlags_RoundCornersAll,
      currentSelectedNode ? options.mBorderSelectionThickness : options.mBorderThickness);

    wabi::GfVec2f imgPos = nodeRectangleMin + wabi::GfVec2f(14, 25);
    wabi::GfVec2f imgSize = nodeRectangleMax + wabi::GfVec2f(-5, -5) - imgPos;
    float imgSizeComp = std::min(imgSize[0], imgSize[1]);

    drawList->AddRectFilled(nodeRectangleMin, nodeRectangleMax, node_bg_color, options.mRounding);
    /*float progress = delegate->NodeProgress(nodeIndex);
      if (progress > FLT_EPSILON && progress < 1.f - FLT_EPSILON)
      {
          wabi::GfVec2f progressLineA = nodeRectangleMax - wabi::GfVec2f(nodeSize[0] - 2.f, 3.f);
          wabi::GfVec2f progressLineB = progressLineA + wabi::GfVec2f(nodeSize[0] * factor - 4.f, 0.f);
          drawList->AddLine(progressLineA, progressLineB, 0xFF400000, 3.f);
          drawList->AddLine(progressLineA, AnchorLerp(progressLineA, progressLineB, progress),
      0xFFFF0000, 3.f);
      }*/
    wabi::GfVec2f imgPosMax = imgPos + wabi::GfVec2f(imgSizeComp, imgSizeComp);

    // wabi::GfVec2f imageSize = delegate->GetEvaluationSize(nodeIndex);
    /*float imageRatio = 1.f;
      if (imageSize[0] > 0.f && imageSize[1] > 0.f)
      {
          imageRatio = imageSize[1] / imageSize[0];
      }
      wabi::GfVec2f quadSize = imgPosMax - imgPos;
      wabi::GfVec2f marge(0.f, 0.f);
      if (imageRatio > 1.f)
      {
          marge[0] = (quadSize[0] - quadSize[1] / imageRatio) * 0.5f;
      }
      else
      {
          marge[1] = (quadSize[1] - quadSize[1] * imageRatio) * 0.5f;
      }*/

    // delegate->DrawNodeImage(drawList, AnchorBBox(imgPos, imgPosMax), marge, nodeIndex);

    drawList->AddRectFilled(nodeRectangleMin,
                            wabi::GfVec2f(nodeRectangleMax[0], nodeRectangleMin[1] + 20),
                            nodeTemplate.mHeaderColor,
                            options.mRounding);

    drawList->PushClipRect(nodeRectangleMin,
                           wabi::GfVec2f(nodeRectangleMax[0], nodeRectangleMin[1] + 20),
                           true);
    drawList->AddText(nodeRectangleMin + wabi::GfVec2f(2, 2), ANCHOR_COL32(0, 0, 0, 255), node.mName);
    drawList->PopClipRect();

    AnchorBBox customDrawRect(nodeRectangleMin +
                                wabi::GfVec2f(options.mRounding, 20 + options.mRounding),
                              nodeRectangleMax - wabi::GfVec2f(options.mRounding, options.mRounding));
    if (customDrawRect.Max[1] > customDrawRect.Min[1] &&
        customDrawRect.Max[0] > customDrawRect.Min[0]) {
      delegate.CustomDraw(drawList, customDrawRect, nodeIndex);
    }
    /*
      const ImTextureID bmpInfo = (ImTextureID)(uint64_t)delegate->GetBitmapInfo(nodeIndex).idx;
      if (bmpInfo)
      {
          wabi::GfVec2f bmpInfoPos(nodeRectangleMax - wabi::GfVec2f(26, 12));
          wabi::GfVec2f bmpInfoSize(20, 20);
          if (delegate->NodeIsCompute(nodeIndex))
          {
              drawList->AddImageQuad(bmpInfo,
                                     bmpInfoPos,
                                     bmpInfoPos + wabi::GfVec2f(bmpInfoSize[0], 0.f),
                                     bmpInfoPos + bmpInfoSize,
                                     bmpInfoPos + wabi::GfVec2f(0., bmpInfoSize[1]));
          }
          else if (delegate->NodeIs2D(nodeIndex))
          {
              drawList->AddImageQuad(bmpInfo,
                                     bmpInfoPos,
                                     bmpInfoPos + wabi::GfVec2f(bmpInfoSize[0], 0.f),
                                     bmpInfoPos + bmpInfoSize,
                                     bmpInfoPos + wabi::GfVec2f(0., bmpInfoSize[1]));
          }
          else if (delegate->NodeIsCubemap(nodeIndex))
          {
              drawList->AddImageQuad(bmpInfo,
                                     bmpInfoPos + wabi::GfVec2f(0., bmpInfoSize[1]),
                                     bmpInfoPos + bmpInfoSize,
                                     bmpInfoPos + wabi::GfVec2f(bmpInfoSize[0], 0.f),
                                     bmpInfoPos);
          }
      }*/
    return nodeHovered;
  }

  bool DrawMiniMap(AnchorDrawList *drawList,
                   Delegate &delegate,
                   ViewState &viewState,
                   const Options &options,
                   const wabi::GfVec2f windowPos,
                   const wabi::GfVec2f canvasSize)
  {
    if (Distance(options.mMinimap.Min, options.mMinimap.Max) <= FLT_EPSILON) {
      return false;
    }

    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount) {
      return false;
    }

    wabi::GfVec2f mapmin(FLT_MAX, FLT_MAX);
    wabi::GfVec2f mapmax(-FLT_MAX, -FLT_MAX);
    const wabi::GfVec2f margin(50, 50);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
      const Node &node = delegate.GetNode(nodeIndex);
      mapmin = AnchorMin(mapmin, node.mRect.Min - margin);
      mapmin = AnchorMin(mapmin, node.mRect.Max + margin);
      mapmax = AnchorMax(mapmax, node.mRect.Min - margin);
      mapmax = AnchorMax(mapmax, node.mRect.Max + margin);
    }

    // add view in world space
    const wabi::GfVec2f worldSizeView = canvasSize / viewState.mFactor;
    const wabi::GfVec2f viewMin(-viewState.mPosition[0], -viewState.mPosition[1]);
    const wabi::GfVec2f viewMax = viewMin + worldSizeView;
    mapmin = AnchorMin(mapmin, viewMin);
    mapmax = AnchorMax(mapmax, viewMax);
    const wabi::GfVec2f nodesSize = mapmax - mapmin;
    const wabi::GfVec2f middleWorld = (mapmin + mapmax) * 0.5f;
    const wabi::GfVec2f minScreen = windowPos + wabi::GfVec2f(options.mMinimap.Min * canvasSize);
    const wabi::GfVec2f maxScreen = windowPos + wabi::GfVec2f(options.mMinimap.Max * canvasSize);
    const wabi::GfVec2f viewSize = maxScreen - minScreen;
    const wabi::GfVec2f middleScreen = (minScreen + maxScreen) * 0.5f;
    const float ratioY = viewSize[1] / nodesSize[1];
    const float ratioX = viewSize[0] / nodesSize[0];
    const float factor = AnchorMin(AnchorMin(ratioY, ratioX), 1.f);

    drawList->AddRectFilled(minScreen,
                            maxScreen,
                            ANCHOR_COL32(30, 30, 30, 200),
                            3,
                            AnchorDrawFlags_RoundCornersAll);

    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
      const Node &node = delegate.GetNode(nodeIndex);
      const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);

      AnchorBBox rect = node.mRect;
      rect.Min -= middleWorld;
      rect.Min *= factor;
      rect.Min += middleScreen;

      rect.Max -= middleWorld;
      rect.Max *= factor;
      rect.Max += middleScreen;

      drawList->AddRectFilled(rect.Min,
                              rect.Max,
                              nodeTemplate.mBackgroundColor,
                              1,
                              AnchorDrawFlags_RoundCornersAll);
      if (node.mSelected) {
        drawList->AddRect(rect.Min,
                          rect.Max,
                          options.mSelectedNodeBorderColor,
                          1,
                          AnchorDrawFlags_RoundCornersAll);
      }
    }

    // add view
    wabi::GfVec2f viewMinScreen = (viewMin - middleWorld) * factor + middleScreen;
    wabi::GfVec2f viewMaxScreen = (viewMax - middleWorld) * factor + middleScreen;
    drawList->AddRectFilled(viewMinScreen,
                            viewMaxScreen,
                            ANCHOR_COL32(255, 255, 255, 32),
                            1,
                            AnchorDrawFlags_RoundCornersAll);
    drawList->AddRect(viewMinScreen,
                      viewMaxScreen,
                      ANCHOR_COL32(255, 255, 255, 128),
                      1,
                      AnchorDrawFlags_RoundCornersAll);

    AnchorIO &io = ANCHOR::GetIO();
    const bool mouseInMinimap = AnchorBBox(minScreen, maxScreen).Contains(io.MousePos);
    if (mouseInMinimap && io.MouseClicked[0]) {
      const wabi::GfVec2f clickedRatio = (io.MousePos - minScreen) / viewSize;
      const wabi::GfVec2f worldPosCenter = wabi::GfVec2f(AnchorLerp(mapmin[0], mapmax[0], clickedRatio[0]),
                                             AnchorLerp(mapmin[1], mapmax[1], clickedRatio[1]));

      wabi::GfVec2f worldPosViewMin = worldPosCenter - worldSizeView * 0.5;
      wabi::GfVec2f worldPosViewMax = worldPosCenter + worldSizeView * 0.5;
      if (worldPosViewMin[0] < mapmin[0]) {
        worldPosViewMin[0] = mapmin[0];
        worldPosViewMax[0] = worldPosViewMin[0] + worldSizeView[0];
      }
      if (worldPosViewMin[1] < mapmin[1]) {
        worldPosViewMin[1] = mapmin[1];
        worldPosViewMax[1] = worldPosViewMin[1] + worldSizeView[1];
      }
      if (worldPosViewMax[0] > mapmax[0]) {
        worldPosViewMax[0] = mapmax[0];
        worldPosViewMin[0] = worldPosViewMax[0] - worldSizeView[0];
      }
      if (worldPosViewMax[1] > mapmax[1]) {
        worldPosViewMax[1] = mapmax[1];
        worldPosViewMin[1] = worldPosViewMax[1] - worldSizeView[1];
      }
      viewState.mPosition = wabi::GfVec2f(-worldPosViewMin[0], -worldPosViewMin[1]);
    }
    return mouseInMinimap;
  }

  void Show(Delegate &delegate,
            const Options &options,
            ViewState &viewState,
            bool enabled,
            FitOnScreen *fit)
  {
    ANCHOR::PushStyleVar(AnchorStyleVar_ChildBorderSize, 0.f);
    ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, wabi::GfVec2f(0.f, 0.f));
    ANCHOR::PushStyleVar(AnchorStyleVar_FrameBorderSize, 0.f);

    const wabi::GfVec2f windowPos = ANCHOR::GetCursorScreenPos();
    const wabi::GfVec2f canvasSize = ANCHOR::GetContentRegionAvail();
    const wabi::GfVec2f scrollRegionLocalPos(0, 0);

    AnchorBBox regionRect(windowPos, windowPos + canvasSize);

    HandleZoomScroll(regionRect, viewState, options);
    wabi::GfVec2f offset = ANCHOR::GetCursorScreenPos() + viewState.mPosition * viewState.mFactor;
    captureOffset = viewState.mPosition * viewState.mFactor;

    // ANCHOR::InvisibleButton("GraphEditorButton", canvasSize);
    ANCHOR::BeginChildFrame(71711, canvasSize);

    ANCHOR::SetCursorPos(windowPos);
    ANCHOR::BeginGroup();

    AnchorIO &io = ANCHOR::GetIO();

    // Create our child canvas
    ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, wabi::GfVec2f(1, 1));
    ANCHOR::PushStyleVar(AnchorStyleVar_WindowPadding, wabi::GfVec2f(0, 0));
    ANCHOR::PushStyleColor(AnchorCol_ChildBg, ANCHOR_COL32(30, 30, 30, 200));

    AnchorDrawList *drawList = ANCHOR::GetWindowDrawList();
    ANCHOR::PushClipRect(regionRect.Min, regionRect.Max, true);
    drawList->AddRectFilled(windowPos, windowPos + canvasSize, options.mBackgroundColor);

    // Background or Display grid
    if (options.mRenderGrid) {
      DrawGrid(drawList,
               windowPos,
               viewState,
               canvasSize,
               options.mGridColor,
               options.mGridColor2,
               options.mGridSize);
    }

    // Fit view
    if (fit && ((*fit == Fit_AllNodes) || (*fit == Fit_SelectedNodes))) {
      FitNodes(delegate, viewState, canvasSize, (*fit == Fit_SelectedNodes));
    }

    if (enabled) {
      static NodeIndex hoveredNode = -1;
      // Display links
      drawList->ChannelsSplit(3);

      // minimap
      drawList->ChannelsSetCurrent(2);  // minimap
      const bool inMinimap =
        DrawMiniMap(drawList, delegate, viewState, options, windowPos, canvasSize);

      // Focus rectangle
      if (ANCHOR::IsWindowFocused()) {
        drawList->AddRect(regionRect.Min, regionRect.Max, options.mFrameFocus, 1.f, 0, 2.f);
      }

      drawList->ChannelsSetCurrent(1);  // Background

      // Links
      DisplayLinks(delegate,
                   drawList,
                   offset,
                   viewState.mFactor,
                   regionRect,
                   hoveredNode,
                   options);

      // edit node link
      if (nodeOperation == NO_EditingLink) {
        wabi::GfVec2f p1 = editingNodeSource;
        wabi::GfVec2f p2 = io.MousePos;
        drawList->AddLine(p1, p2, ANCHOR_COL32(200, 200, 200, 255), 3.0f);
      }

      // Display nodes
      drawList->PushClipRect(regionRect.Min, regionRect.Max, true);
      hoveredNode = -1;

      SlotIndex inputSlotOver = -1;
      SlotIndex outputSlotOver = -1;
      NodeIndex nodeOver = -1;

      const auto nodeCount = delegate.GetNodeCount();
      for (int i = 0; i < 2; i++) {
        for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
          // const auto* node = &nodes[nodeIndex];
          const auto node = delegate.GetNode(nodeIndex);
          if (node.mSelected != (i != 0)) {
            continue;
          }

          // node view clipping
          AnchorBBox nodeRect = GetNodeRect(node, viewState.mFactor);
          nodeRect.Min += offset;
          nodeRect.Max += offset;
          if (!regionRect.Overlaps(nodeRect)) {
            continue;
          }

          ANCHOR::PushID((int)nodeIndex);
          SlotIndex inputSlot = -1;
          SlotIndex outputSlot = -1;

          bool overInput = (!inMinimap) && HandleConnections(drawList,
                                                             nodeIndex,
                                                             offset,
                                                             viewState.mFactor,
                                                             delegate,
                                                             options,
                                                             false,
                                                             inputSlot,
                                                             outputSlot,
                                                             inMinimap);

#ifdef WITH_ANCHOR_CURVE_EDIT_SHADOW
          wabi::GfVec2f shadowOffset = wabi::GfVec2f(30, 30);
          wabi::GfVec2f shadowPivot = (nodeRect.Min + nodeRect.Max) / 2.f;
          wabi::GfVec2f shadowPointMiddle = shadowPivot + shadowOffset;
          wabi::GfVec2f shadowPointTop = wabi::GfVec2f(shadowPivot[0], nodeRect.Min[1]) + shadowOffset;
          wabi::GfVec2f shadowPointBottom = wabi::GfVec2f(shadowPivot[0], nodeRect.Max[1]) + shadowOffset;
          wabi::GfVec2f shadowPointLeft = wabi::GfVec2f(nodeRect.Min[0], shadowPivot[1]) + shadowOffset;
          wabi::GfVec2f shadowPointRight = wabi::GfVec2f(nodeRect.Max[0], shadowPivot[1]) + shadowOffset;

          /* top left */
          drawList->AddRectFilledMultiColor(nodeRect.Min + shadowOffset,
                                            shadowPointMiddle,
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 255),
                                            ANCHOR_COL32(0, 0, 0, 0));

          /* top right */
          drawList->AddRectFilledMultiColor(shadowPointTop,
                                            shadowPointRight,
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 255));

          /* bottom left */
          drawList->AddRectFilledMultiColor(shadowPointLeft,
                                            shadowPointBottom,
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 255),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0));

          /* bottom right */
          drawList->AddRectFilledMultiColor(shadowPointMiddle,
                                            nodeRect.Max + shadowOffset,
                                            ANCHOR_COL32(0, 0, 0, 255),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0),
                                            ANCHOR_COL32(0, 0, 0, 0));
#endif /* WITH_ANCHOR_CURVE_EDIT_SHADOW */

          if (DrawNode(drawList,
                       nodeIndex,
                       offset,
                       viewState.mFactor,
                       delegate,
                       overInput,
                       options,
                       inMinimap,
                       regionRect)) {
            hoveredNode = nodeIndex;
          }

          HandleConnections(drawList,
                            nodeIndex,
                            offset,
                            viewState.mFactor,
                            delegate,
                            options,
                            true,
                            inputSlot,
                            outputSlot,
                            inMinimap);
          if (inputSlot != -1 || outputSlot != -1) {
            inputSlotOver = inputSlot;
            outputSlotOver = outputSlot;
            nodeOver = nodeIndex;
          }

          ANCHOR::PopID();
        }
      }


      drawList->PopClipRect();

      if (nodeOperation == NO_MovingNodes) {
        if (ANCHOR::IsMouseDragging(0, 1)) {
          wabi::GfVec2f delta = io.MouseDelta / viewState.mFactor;
          if (fabsf(delta[0]) >= 1.f || fabsf(delta[1]) >= 1.f) {
            delegate.MoveSelectedNodes(delta);
          }
        }
      }

      drawList->ChannelsSetCurrent(0);

      // quad selection
      if (!inMinimap) {
        HandleQuadSelection(delegate, drawList, offset, viewState.mFactor, regionRect, options);
      }

      drawList->ChannelsMerge();

      // releasing mouse button means it's done in any operation
      if (nodeOperation == NO_PanView) {
        if (!io.MouseDown[2]) {
          nodeOperation = NO_None;
        }
      } else if (nodeOperation != NO_None && !io.MouseDown[0]) {
        nodeOperation = NO_None;
      }

      // right click
      if (!inMinimap && nodeOperation == NO_None && regionRect.Contains(io.MousePos) &&
          (ANCHOR::IsMouseClicked(
            1) /*|| (ANCHOR::IsWindowFocused() && ANCHOR::IsKeyPressedMap(AnchorKey_Tab))*/)) {
        delegate.RightClick(nodeOver, inputSlotOver, outputSlotOver);
      }

      // Scrolling
      if (ANCHOR::IsWindowHovered() && !ANCHOR::IsAnyItemActive() && io.MouseClicked[2] &&
          nodeOperation == NO_None) {
        nodeOperation = NO_PanView;
      }
      if (nodeOperation == NO_PanView) {
        viewState.mPosition += io.MouseDelta / viewState.mFactor;
      }
    }

    ANCHOR::PopClipRect();

    ANCHOR::PopStyleColor(1);
    ANCHOR::PopStyleVar(2);
    ANCHOR::EndGroup();
    ANCHOR::EndChildFrame();

    ANCHOR::PopStyleVar(3);

    // change fit to none
    if (fit) {
      *fit = Fit_None;
    }
  }

  bool EditOptions(Options &options)
  {
    bool updated = false;
    if (ANCHOR::CollapsingHeader("Colors", nullptr)) {
      AnchorColor backgroundColor(options.mBackgroundColor);
      AnchorColor gridColor(options.mGridColor);
      AnchorColor selectedNodeBorderColor(options.mSelectedNodeBorderColor);
      AnchorColor nodeBorderColor(options.mNodeBorderColor);
      AnchorColor quadSelection(options.mQuadSelection);
      AnchorColor quadSelectionBorder(options.mQuadSelectionBorder);
      AnchorColor defaultSlotColor(options.mDefaultSlotColor);
      AnchorColor frameFocus(options.mFrameFocus);

      updated |= ANCHOR::ColorEdit4("Background", (float *)&backgroundColor);
      updated |= ANCHOR::ColorEdit4("Grid", (float *)&gridColor);
      updated |= ANCHOR::ColorEdit4("Selected Node Border", (float *)&selectedNodeBorderColor);
      updated |= ANCHOR::ColorEdit4("Node Border", (float *)&nodeBorderColor);
      updated |= ANCHOR::ColorEdit4("Quad Selection", (float *)&quadSelection);
      updated |= ANCHOR::ColorEdit4("Quad Selection Border", (float *)&quadSelectionBorder);
      updated |= ANCHOR::ColorEdit4("Default Slot", (float *)&defaultSlotColor);
      updated |= ANCHOR::ColorEdit4("Frame when has focus", (float *)&frameFocus);

      options.mBackgroundColor = backgroundColor;
      options.mGridColor = gridColor;
      options.mSelectedNodeBorderColor = selectedNodeBorderColor;
      options.mNodeBorderColor = nodeBorderColor;
      options.mQuadSelection = quadSelection;
      options.mQuadSelectionBorder = quadSelectionBorder;
      options.mDefaultSlotColor = defaultSlotColor;
      options.mFrameFocus = frameFocus;
    }

    if (ANCHOR::CollapsingHeader("Options", nullptr)) {
      updated |= ANCHOR::InputFloat4("Minimap", &options.mMinimap.Min[0]);
      updated |= ANCHOR::InputFloat("Line Thickness", &options.mLineThickness);
      updated |= ANCHOR::InputFloat("Grid Size", &options.mGridSize);
      updated |= ANCHOR::InputFloat("Rounding", &options.mRounding);
      updated |= ANCHOR::InputFloat("Zoom Ratio", &options.mZoomRatio);
      updated |= ANCHOR::InputFloat("Zoom Lerp Factor", &options.mZoomLerpFactor);
      updated |= ANCHOR::InputFloat("Border Selection Thickness",
                                    &options.mBorderSelectionThickness);
      updated |= ANCHOR::InputFloat("Border Thickness", &options.mBorderThickness);
      updated |= ANCHOR::InputFloat("Slot Radius", &options.mNodeSlotRadius);
      updated |= ANCHOR::InputFloat("Slot Hover Factor", &options.mNodeSlotHoverFactor);
      updated |= ANCHOR::InputFloat2("Zoom min/max", &options.mMinZoom);
      updated |= ANCHOR::InputFloat("Slot Hover Factor", &options.mSnap);

      if (ANCHOR::RadioButton("Curved Links", options.mDisplayLinksAsCurves)) {
        options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
        updated = true;
      }
      if (ANCHOR::RadioButton("Straight Links", !options.mDisplayLinksAsCurves)) {
        options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
        updated = true;
      }

      updated |= ANCHOR::Checkbox("Allow Quad Selection", &options.mAllowQuadSelection);
      updated |= ANCHOR::Checkbox("Render Grid", &options.mRenderGrid);
      updated |= ANCHOR::Checkbox("Draw IO names on hover", &options.mDrawIONameOnHover);
    }

    return updated;
  }

}  // namespace AnchorGraphEditor
