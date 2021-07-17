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
#define ANCHOR_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <math.h>
#include <vector>
#include <float.h>
#include <array>
#include "GraphEditor.h"

namespace GraphEditor {

static inline float Distance(const GfVec2f& a, const GfVec2f& b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static inline float sign(float v)
{
    return (v >= 0.f) ? 1.f : -1.f;
}

static GfVec2f GetInputSlotPos(Delegate& delegate, const Node& node, SlotIndex slotIndex, float factor)
{
    GfVec2f Size = node.mRect.GetSize() * factor;
    size_t InputsCount = delegate.GetTemplate(node.mTemplateIndex).mInputCount;
    return GfVec2f(node.mRect.Min.x * factor,
                  node.mRect.Min.y * factor + Size.y * ((float)slotIndex + 1) / ((float)InputsCount + 1) + 8.f);
}

static GfVec2f GetOutputSlotPos(Delegate& delegate, const Node& node, SlotIndex slotIndex, float factor)
{
    GfVec2f Size = node.mRect.GetSize() * factor;
    size_t OutputsCount = delegate.GetTemplate(node.mTemplateIndex).mOutputCount;
    return GfVec2f(node.mRect.Min.x * factor + Size.x,
                  node.mRect.Min.y * factor + Size.y * ((float)slotIndex + 1) / ((float)OutputsCount + 1) + 8.f);
}

static AnchorRect GetNodeRect(const Node& node, float factor)
{
    GfVec2f Size = node.mRect.GetSize() * factor;
    return AnchorRect(node.mRect.Min * factor, node.mRect.Min * factor + Size);
}

static GfVec2f editingNodeSource;
static bool editingInput = false;
static GfVec2f captureOffset;

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

static void HandleZoomScroll(AnchorRect regionRect, ViewState& viewState, const Options& options)
{
    AnchorIO& io = Anchor::GetIO();

    if (regionRect.Contains(io.MousePos))
    {
        if (io.MouseWheel < -FLT_EPSILON)
        {
            viewState.mFactorTarget *= 1.f - options.mZoomRatio;
        }

        if (io.MouseWheel > FLT_EPSILON)
        {
            viewState.mFactorTarget *= 1.0f + options.mZoomRatio;
        }
    }

    GfVec2f mouseWPosPre = (io.MousePos - Anchor::GetCursorScreenPos()) / viewState.mFactor;
    viewState.mFactorTarget = AnchorClamp(viewState.mFactorTarget, options.mMinZoom, options.mMaxZoom);
    viewState.mFactor = AnchorLerp(viewState.mFactor, viewState.mFactorTarget, options.mZoomLerpFactor);
    GfVec2f mouseWPosPost = (io.MousePos - Anchor::GetCursorScreenPos()) / viewState.mFactor;
    if (Anchor::IsMousePosValid())
    {
        viewState.mPosition += mouseWPosPost - mouseWPosPre;
    }
}

void GraphEditorClear()
{
    nodeOperation = NO_None;
}

static void FitNodes(Delegate& delegate, ViewState& viewState, const GfVec2f viewSize, bool selectedNodesOnly)
{
    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount)
    {
        return;
    }

    bool validNode = false;
    GfVec2f min(FLT_MAX, FLT_MAX);
    GfVec2f max(-FLT_MAX, -FLT_MAX);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        
        if (selectedNodesOnly && !node.mSelected)
        {
            continue;
        }
        
        min = AnchorMin(min, node.mRect.Min);
        min = AnchorMin(min, node.mRect.Max);
        max = AnchorMax(max, node.mRect.Min);
        max = AnchorMax(max, node.mRect.Max);
        validNode = true;
    }
    
    if (!validNode)
    {
        return;
    }
    
    min -= viewSize * 0.05f;
    max += viewSize * 0.05f;
    GfVec2f nodesSize = max - min;
    GfVec2f nodeCenter = (max + min) * 0.5f;
    
    float ratioY = viewSize.y / nodesSize.y;
    float ratioX = viewSize.x / nodesSize.x;

    viewState.mFactor = viewState.mFactorTarget = AnchorMin(AnchorMin(ratioY, ratioX), 1.f);
    viewState.mPosition = GfVec2f(-nodeCenter.x, -nodeCenter.y) + (viewSize * 0.5f) / viewState.mFactorTarget;
}

static void DisplayLinks(Delegate& delegate,
                         AnchorDrawList* drawList,
                         const GfVec2f offset,
                         const float factor,
                         const AnchorRect regionRect,
                         NodeIndex hoveredNode,
                         const Options& options)
{
    const size_t linkCount = delegate.GetLinkCount();
    for (LinkIndex linkIndex = 0; linkIndex < linkCount; linkIndex++)
    {
        const auto link = delegate.GetLink(linkIndex);
        const auto nodeInput = delegate.GetNode(link.mInputNodeIndex);
        const auto nodeOutput = delegate.GetNode(link.mOutputNodeIndex);
        GfVec2f p1 = offset + GetOutputSlotPos(delegate, nodeInput, link.mInputSlotIndex, factor);
        GfVec2f p2 = offset + GetInputSlotPos(delegate, nodeOutput, link.mOutputSlotIndex, factor);

        // con. view clipping
        if ((p1.y < 0.f && p2.y < 0.f) || (p1.y > regionRect.Max.y && p2.y > regionRect.Max.y) ||
            (p1.x < 0.f && p2.x < 0.f) || (p1.x > regionRect.Max.x && p2.x > regionRect.Max.x))
            continue;

        bool highlightCons = hoveredNode == link.mInputNodeIndex || hoveredNode == link.mOutputNodeIndex;
        uint32_t col = delegate.GetTemplate(nodeInput.mTemplateIndex).mHeaderColor | (highlightCons ? 0xF0F0F0 : 0);
        if (options.mDisplayLinksAsCurves)
        {
            // curves
             drawList->AddBezierCurve(p1, p1 + GfVec2f(50, 0) * factor, p2 + GfVec2f(-50, 0) * factor, p2, 0xFF000000, options.mLineThickness * 1.5f * factor);
             drawList->AddBezierCurve(p1, p1 + GfVec2f(50, 0) * factor, p2 + GfVec2f(-50, 0) * factor, p2, col, options.mLineThickness * 1.5f * factor);
             /*
            GfVec2f p10 = p1 + GfVec2f(20.f * factor, 0.f);
            GfVec2f p20 = p2 - GfVec2f(20.f * factor, 0.f);

            GfVec2f dif = p20 - p10;
            GfVec2f p1a, p1b;
            if (fabsf(dif.x) > fabsf(dif.y))
            {
                p1a = p10 + GfVec2f(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5 * sign(dif.x), 0.f);
                p1b = p1a + GfVec2f(fabsf(dif.y) * sign(dif.x) , dif.y);
            }
            else
            {
                p1a = p10 + GfVec2f(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5 * sign(dif.y));
                p1b = p1a + GfVec2f(dif.x, fabsf(dif.x) * sign(dif.y));
            }
            drawList->AddLine(p1,  p10, col, 3.f * factor);
            drawList->AddLine(p10, p1a, col, 3.f * factor);
            drawList->AddLine(p1a, p1b, col, 3.f * factor);
            drawList->AddLine(p1b, p20, col, 3.f * factor);
            drawList->AddLine(p20,  p2, col, 3.f * factor);
            */
        }
        else
        {
            // straight lines
            std::array<GfVec2f, 6> pts;
            int ptCount = 0;
            GfVec2f dif = p2 - p1;

            GfVec2f p1a, p1b;
            const float limitx = 12.f * factor;
            if (dif.x < limitx)
            {
                GfVec2f p10 = p1 + GfVec2f(limitx, 0.f);
                GfVec2f p20 = p2 - GfVec2f(limitx, 0.f);

                dif = p20 - p10;
                p1a = p10 + GfVec2f(0.f, dif.y * 0.5f);
                p1b = p1a + GfVec2f(dif.x, 0.f);

                pts = { p1, p10, p1a, p1b, p20, p2 };
                ptCount = 6;
            }
            else
            {
                if (fabsf(dif.y) < 1.f)
                {
                    pts = { p1, (p1 + p2) * 0.5f, p2 };
                    ptCount = 3;
                }
                else
                {
                    if (fabsf(dif.y) < 10.f)
                    {
                        if (fabsf(dif.x) > fabsf(dif.y))
                        {
                            p1a = p1 + GfVec2f(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5f * sign(dif.x), 0.f);
                            p1b = p1a + GfVec2f(fabsf(dif.y) * sign(dif.x), dif.y);
                        }
                        else
                        {
                            p1a = p1 + GfVec2f(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5f * sign(dif.y));
                            p1b = p1a + GfVec2f(dif.x, fabsf(dif.x) * sign(dif.y));
                        }
                    }
                    else
                    {
                        if (fabsf(dif.x) > fabsf(dif.y))
                        {
                            float d = fabsf(dif.y) * sign(dif.x) * 0.5f;
                            p1a = p1 + GfVec2f(d, dif.y * 0.5f);
                            p1b = p1a + GfVec2f(fabsf(fabsf(dif.x) - fabsf(d) * 2.f) * sign(dif.x), 0.f);
                        }
                        else
                        {
                            float d = fabsf(dif.x) * sign(dif.y) * 0.5f;
                            p1a = p1 + GfVec2f(dif.x * 0.5f, d);
                            p1b = p1a + GfVec2f(0.f, fabsf(fabsf(dif.y) - fabsf(d) * 2.f) * sign(dif.y));
                        }
                    }
                    pts = { p1, p1a, p1b, p2 };
                    ptCount = 4;
                }
            }
            float highLightFactor = factor * (highlightCons ? 2.0f : 1.f);
            for (int pass = 0; pass < 2; pass++)
            {
                drawList->AddPolyline(pts.data(), ptCount, pass ? col : 0xFF000000, false, (pass ? options.mLineThickness : (options.mLineThickness * 1.5f)) * highLightFactor);
            }
        }
    }
}

static void HandleQuadSelection(Delegate& delegate, AnchorDrawList* drawList, const GfVec2f offset, const float factor, AnchorRect contentRect, const Options& options)
{
    if (!options.mAllowQuadSelection)
    {
        return;
    }
    AnchorIO& io = Anchor::GetIO();
    static GfVec2f quadSelectPos;
    //auto& nodes = delegate->GetNodes();
    auto nodeCount = delegate.GetNodeCount();

    if (nodeOperation == NO_QuadSelecting && Anchor::IsWindowFocused())
    {
        const GfVec2f bmin = AnchorMin(quadSelectPos, io.MousePos);
        const GfVec2f bmax = AnchorMax(quadSelectPos, io.MousePos);
        drawList->AddRectFilled(bmin, bmax, options.mQuadSelection, 1.f);
        drawList->AddRect(bmin, bmax, options.mQuadSelectionBorder, 1.f);
        if (!io.MouseDown[0])
        {
            if (!io.KeyCtrl && !io.KeyShift)
            {
                for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
                {
                    delegate.SelectNode(nodeIndex, false);
                }
            }

            nodeOperation = NO_None;
            AnchorRect selectionRect(bmin, bmax);
            for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
            {
                const auto node = delegate.GetNode(nodeIndex);
                GfVec2f nodeRectangleMin = offset + node.mRect.Min * factor;
                GfVec2f nodeRectangleMax = nodeRectangleMin + node.mRect.GetSize() * factor;
                if (selectionRect.Overlaps(AnchorRect(nodeRectangleMin, nodeRectangleMax)))
                {
                    if (io.KeyCtrl)
                    {
                        delegate.SelectNode(nodeIndex, false);
                    }
                    else
                    {
                        delegate.SelectNode(nodeIndex, true);
                    }
                }
                else
                {
                    if (!io.KeyShift)
                    {
                        delegate.SelectNode(nodeIndex, false);
                    }
                }
            }
        }
    }
    else if (nodeOperation == NO_None && io.MouseDown[0] && Anchor::IsWindowFocused() &&
             contentRect.Contains(io.MousePos))
    {
        nodeOperation = NO_QuadSelecting;
        quadSelectPos = io.MousePos;
    }
}

static bool HandleConnections(AnchorDrawList* drawList,
                       NodeIndex nodeIndex,
                       const GfVec2f offset,
                       const float factor,
                       Delegate& delegate,
                       const Options& options,
                       bool bDrawOnly,
                       SlotIndex& inputSlotOver,
                       SlotIndex& outputSlotOver,
                       const bool inMinimap)
{
    static NodeIndex editingNodeIndex;
    static SlotIndex editingSlotIndex;

    AnchorIO& io = Anchor::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const auto linkCount = delegate.GetLinkCount();

    size_t InputsCount = nodeTemplate.mInputCount;
    size_t OutputsCount = nodeTemplate.mOutputCount;
    inputSlotOver = -1;
    outputSlotOver = -1;

    // draw/use inputs/outputs
    bool hoverSlot = false;
    for (int i = 0; i < 2; i++)
    {
        float closestDistance = FLT_MAX;
        SlotIndex closestConn = -1;
        GfVec2f closestTextPos;
        GfVec2f closestPos;
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (SlotIndex slotIndex = 0; slotIndex < slotCount[i]; slotIndex++)
        {
            const char** con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
            const char* conText = (con && con[slotIndex]) ? con[slotIndex] : "";

            GfVec2f p =
                offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) : GetInputSlotPos(delegate, node, slotIndex, factor));
            float distance = Distance(p, io.MousePos);
            bool overCon = (nodeOperation == NO_None || nodeOperation == NO_EditingLink) &&
                           (distance < options.mNodeSlotRadius * 2.f) && (distance < closestDistance);

            
            GfVec2f textSize;
            textSize = Anchor::CalcTextSize(conText);
            GfVec2f textPos =
                p + GfVec2f(-options.mNodeSlotRadius * (i ? -1.f : 1.f) * (overCon ? 3.f : 2.f) - (i ? 0 : textSize.x),
                           -textSize.y / 2);

            AnchorRect nodeRect = GetNodeRect(node, factor);
            if (!inMinimap && (overCon || (nodeRect.Contains(io.MousePos - offset) && closestConn == -1 &&
                            (editingInput == (i != 0)) && nodeOperation == NO_EditingLink)))
            {
                closestDistance = distance;
                closestConn = slotIndex;
                closestTextPos = textPos;
                closestPos = p;
                
                if (i)
                {
                    outputSlotOver = slotIndex;
                }
                else
                {
                    inputSlotOver = slotIndex;
                }
            }
            else
            {
               const AnchorU32* slotColorSource = i ? nodeTemplate.mOutputColors : nodeTemplate.mInputColors;
               const AnchorU32 slotColor = slotColorSource ? slotColorSource[slotIndex] : options.mDefaultSlotColor;
               drawList->AddCircleFilled(p, options.mNodeSlotRadius, IM_COL32(0, 0, 0, 200));
               drawList->AddCircleFilled(p, options.mNodeSlotRadius * 0.75f, slotColor);
               if (!options.mDrawIONameOnHover)
               {
                    drawList->AddText(io.FontDefault, 14, textPos + GfVec2f(2, 2), IM_COL32(0, 0, 0, 255), conText);
                    drawList->AddText(io.FontDefault, 14, textPos, IM_COL32(150, 150, 150, 255), conText);
               }
            }
        }

        if (closestConn != -1)
        {
            const char** con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
            const char* conText = (con && con[closestConn]) ? con[closestConn] : "";
            const AnchorU32* slotColorSource = i ? nodeTemplate.mOutputColors : nodeTemplate.mInputColors;
            const AnchorU32 slotColor = slotColorSource ? slotColorSource[closestConn] : options.mDefaultSlotColor;
            hoverSlot = true;
            drawList->AddCircleFilled(closestPos, options.mNodeSlotRadius * options.mNodeSlotHoverFactor * 0.75f, IM_COL32(0, 0, 0, 200));
            drawList->AddCircleFilled(closestPos, options.mNodeSlotRadius * options.mNodeSlotHoverFactor, slotColor);
            drawList->AddText(io.FontDefault, 16, closestTextPos + GfVec2f(1, 1), IM_COL32(0, 0, 0, 255), conText);
            drawList->AddText(io.FontDefault, 16, closestTextPos, IM_COL32(250, 250, 250, 255), conText);
            bool inputToOutput = (!editingInput && !i) || (editingInput && i);
            if (nodeOperation == NO_EditingLink && !io.MouseDown[0] && !bDrawOnly)
            {
                if (inputToOutput)
                {
                    // check loopback
                    Link nl;
                    if (editingInput)
                        nl = Link{nodeIndex, closestConn, editingNodeIndex, editingSlotIndex};
                    else
                        nl = Link{editingNodeIndex, editingSlotIndex, nodeIndex, closestConn};

                    if (!delegate.AllowedLink(nl.mOutputNodeIndex, nl.mInputNodeIndex))
                    {
                        break;
                    }
                    bool alreadyExisting = false;
                    for (size_t linkIndex = 0; linkIndex < linkCount; linkIndex++)
                    {
                        const auto link = delegate.GetLink(linkIndex);
                        if (!memcmp(&link, &nl, sizeof(Link)))
                        {
                            alreadyExisting = true;
                            break;
                        }
                    }

                    if (!alreadyExisting)
                    {
                        for (int linkIndex = 0; linkIndex < linkCount; linkIndex++)
                        {
                            const auto link = delegate.GetLink(linkIndex);
                            if (link.mOutputNodeIndex == nl.mOutputNodeIndex && link.mOutputSlotIndex == nl.mOutputSlotIndex)
                            {
                                delegate.DelLink(linkIndex);
                                
                                break;
                            }
                        }

                        delegate.AddLink(nl.mInputNodeIndex, nl.mInputSlotIndex, nl.mOutputNodeIndex, nl.mOutputSlotIndex);
                    }
                }
            }
            // when Anchor::IsWindowHovered() && !Anchor::IsAnyItemActive() is uncommented, one can't click the node
            // input/output when mouse is over the node itself.
            if (nodeOperation == NO_None &&
                /*Anchor::IsWindowHovered() && !Anchor::IsAnyItemActive() &&*/ io.MouseClicked[0] && !bDrawOnly)
            {
                nodeOperation = NO_EditingLink;
                editingInput = i == 0;
                editingNodeSource = closestPos;
                editingNodeIndex = nodeIndex;
                editingSlotIndex = closestConn;
                if (editingInput)
                {
                    // remove existing link
                    for (int linkIndex = 0; linkIndex < linkCount; linkIndex++)
                    {
                        const auto link = delegate.GetLink(linkIndex);
                        if (link.mOutputNodeIndex == nodeIndex && link.mOutputSlotIndex == closestConn)
                        {
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

static void DrawGrid(AnchorDrawList* drawList, GfVec2f windowPos, const ViewState& viewState, const GfVec2f canvasSize, AnchorU32 gridColor, AnchorU32 gridColor2, float gridSize)
{
    float gridSpace = gridSize * viewState.mFactor;
    int divx = static_cast<int>(-viewState.mPosition.x / gridSize);
    int divy = static_cast<int>(-viewState.mPosition.y / gridSize);
    for (float x = fmodf(viewState.mPosition.x * viewState.mFactor, gridSpace); x < canvasSize.x; x += gridSpace, divx ++)
    {
        bool tenth = !(divx % 10);
        drawList->AddLine(GfVec2f(x, 0.0f) + windowPos, GfVec2f(x, canvasSize.y) + windowPos, tenth ? gridColor2 : gridColor);
    }
    for (float y = fmodf(viewState.mPosition.y * viewState.mFactor, gridSpace); y < canvasSize.y; y += gridSpace, divy ++)
    {
        bool tenth = !(divy % 10);
        drawList->AddLine(GfVec2f(0.0f, y) + windowPos, GfVec2f(canvasSize.x, y) + windowPos, tenth ? gridColor2 : gridColor);
    }
}

// return true if node is hovered
static bool DrawNode(AnchorDrawList* drawList,
                     NodeIndex nodeIndex,
                     const GfVec2f offset,
                     const float factor,
                     Delegate& delegate,
                     bool overInput,
                     const Options& options,
                     const bool inMinimap,
                     const AnchorRect& viewPort)
{
    AnchorIO& io = Anchor::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const GfVec2f nodeRectangleMin = offset + node.mRect.Min * factor;

    const bool old_any_active = Anchor::IsAnyItemActive();
    Anchor::SetCursorScreenPos(nodeRectangleMin);
    const GfVec2f nodeSize = node.mRect.GetSize() * factor;

    // test nested IO
    drawList->ChannelsSetCurrent(1); // Background
    const size_t InputsCount = nodeTemplate.mInputCount;
    const size_t OutputsCount = nodeTemplate.mOutputCount;

    /*
    for (int i = 0; i < 2; i++)
    {
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (size_t slotIndex = 0; slotIndex < slotCount[i]; slotIndex++)
        {
            const char* con = i ? nodeTemplate.mOutputNames[slotIndex] : nodeTemplate.mInputNames[slotIndex];//node.mOutputs[slot_idx] : node->mInputs[slot_idx];
            if (!delegate->IsIOPinned(nodeIndex, slot_idx, i == 1))
            {
               
            }
            continue;

            GfVec2f p = offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) : GetInputSlotPos(delegate, node, slotIndex, factor));
            const float arc = 28.f * (float(i) * 0.3f + 1.0f) * (i ? 1.f : -1.f);
            const float ofs = 0.f;

            GfVec2f pts[3] = {p + GfVec2f(arc + ofs, 0.f), p + GfVec2f(0.f + ofs, -arc), p + GfVec2f(0.f + ofs, arc)};
            drawList->AddTriangleFilled(pts[0], pts[1], pts[2], i ? 0xFFAA5030 : 0xFF30AA50);
            drawList->AddTriangle(pts[0], pts[1], pts[2], 0xFF000000, 2.f);
        }
    }
    */

    Anchor::SetCursorScreenPos(nodeRectangleMin);
    float maxHeight = AnchorMin(viewPort.Max.y, nodeRectangleMin.y + nodeSize.y) - nodeRectangleMin.y;
    float maxWidth = AnchorMin(viewPort.Max.x, nodeRectangleMin.x + nodeSize.x) - nodeRectangleMin.x;
    Anchor::InvisibleButton("node", GfVec2f(maxWidth, maxHeight));
    // must be called right after creating the control we want to be able to move
    bool nodeMovingActive = Anchor::IsItemActive();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool nodeWidgetsActive = (!old_any_active && Anchor::IsAnyItemActive());
    GfVec2f nodeRectangleMax = nodeRectangleMin + nodeSize;

    bool nodeHovered = false;
    if (Anchor::IsItemHovered() && nodeOperation == NO_None && !overInput)
    {
        nodeHovered = true;
    }

    if (Anchor::IsWindowFocused())
    {
        if ((nodeWidgetsActive || nodeMovingActive) && !inMinimap)
        {
            if (!node.mSelected)
            {
                if (!io.KeyShift)
                {
                    const auto nodeCount = delegate.GetNodeCount();
                    for (size_t i = 0; i < nodeCount; i++)
                    {
                        delegate.SelectNode(i, false);
                    }
                }
                delegate.SelectNode(nodeIndex, true);
            }
        }
    }
    if (nodeMovingActive && io.MouseDown[0] && nodeHovered && !inMinimap)
    {
        if (nodeOperation != NO_MovingNodes)
        {
            nodeOperation = NO_MovingNodes;
        }
    }

    const bool currentSelectedNode = node.mSelected;
    const AnchorU32 node_bg_color = nodeHovered ? nodeTemplate.mBackgroundColorOver : nodeTemplate.mBackgroundColor;

    drawList->AddRect(nodeRectangleMin,
                      nodeRectangleMax,
                      currentSelectedNode ? options.mSelectedNodeBorderColor : options.mNodeBorderColor,
                      options.mRounding,
                      AnchorDrawFlags_RoundCornersAll,
                      currentSelectedNode ? options.mBorderSelectionThickness : options.mBorderThickness);

    GfVec2f imgPos = nodeRectangleMin + GfVec2f(14, 25);
    GfVec2f imgSize = nodeRectangleMax + GfVec2f(-5, -5) - imgPos;
    float imgSizeComp = std::min(imgSize.x, imgSize.y);

    drawList->AddRectFilled(nodeRectangleMin, nodeRectangleMax, node_bg_color, options.mRounding);
    /*float progress = delegate->NodeProgress(nodeIndex);
    if (progress > FLT_EPSILON && progress < 1.f - FLT_EPSILON)
    {
        GfVec2f progressLineA = nodeRectangleMax - GfVec2f(nodeSize.x - 2.f, 3.f);
        GfVec2f progressLineB = progressLineA + GfVec2f(nodeSize.x * factor - 4.f, 0.f);
        drawList->AddLine(progressLineA, progressLineB, 0xFF400000, 3.f);
        drawList->AddLine(progressLineA, AnchorLerp(progressLineA, progressLineB, progress), 0xFFFF0000, 3.f);
    }*/
    GfVec2f imgPosMax = imgPos + GfVec2f(imgSizeComp, imgSizeComp);

    //GfVec2f imageSize = delegate->GetEvaluationSize(nodeIndex);
    /*float imageRatio = 1.f;
    if (imageSize.x > 0.f && imageSize.y > 0.f)
    {
        imageRatio = imageSize.y / imageSize.x;
    }
    GfVec2f quadSize = imgPosMax - imgPos;
    GfVec2f marge(0.f, 0.f);
    if (imageRatio > 1.f)
    {
        marge.x = (quadSize.x - quadSize.y / imageRatio) * 0.5f;
    }
    else
    {
        marge.y = (quadSize.y - quadSize.y * imageRatio) * 0.5f;
    }*/

    //delegate->DrawNodeImage(drawList, AnchorRect(imgPos, imgPosMax), marge, nodeIndex);

    drawList->AddRectFilled(nodeRectangleMin,
                            GfVec2f(nodeRectangleMax.x, nodeRectangleMin.y + 20),
                            nodeTemplate.mHeaderColor, options.mRounding);

    drawList->PushClipRect(nodeRectangleMin, GfVec2f(nodeRectangleMax.x, nodeRectangleMin.y + 20), true);
    drawList->AddText(nodeRectangleMin + GfVec2f(2, 2), IM_COL32(0, 0, 0, 255), node.mName);
    drawList->PopClipRect();

    AnchorRect customDrawRect(nodeRectangleMin + GfVec2f(options.mRounding, 20 + options.mRounding), nodeRectangleMax - GfVec2f(options.mRounding, options.mRounding));
    if (customDrawRect.Max.y > customDrawRect.Min.y && customDrawRect.Max.x > customDrawRect.Min.x)
    {
        delegate.CustomDraw(drawList, customDrawRect, nodeIndex);
    }
/*
    const ImTextureID bmpInfo = (ImTextureID)(uint64_t)delegate->GetBitmapInfo(nodeIndex).idx;
    if (bmpInfo)
    {
        GfVec2f bmpInfoPos(nodeRectangleMax - GfVec2f(26, 12));
        GfVec2f bmpInfoSize(20, 20);
        if (delegate->NodeIsCompute(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + GfVec2f(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + GfVec2f(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIs2D(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + GfVec2f(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + GfVec2f(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIsCubemap(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos + GfVec2f(0., bmpInfoSize.y),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + GfVec2f(bmpInfoSize.x, 0.f),
                                   bmpInfoPos);
        }
    }*/
    return nodeHovered;
}

bool DrawMiniMap(AnchorDrawList* drawList, Delegate& delegate, ViewState& viewState, const Options& options, const GfVec2f windowPos, const GfVec2f canvasSize)
{
    if (Distance(options.mMinimap.Min, options.mMinimap.Max) <= FLT_EPSILON)
    {
        return false;
    }

    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount)
    {
        return false;
    }

    GfVec2f min(FLT_MAX, FLT_MAX);
    GfVec2f max(-FLT_MAX, -FLT_MAX);
    const GfVec2f margin(50, 50);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        min = AnchorMin(min, node.mRect.Min - margin);
        min = AnchorMin(min, node.mRect.Max + margin);
        max = AnchorMax(max, node.mRect.Min - margin);
        max = AnchorMax(max, node.mRect.Max + margin);
    }

    // add view in world space
    const GfVec2f worldSizeView = canvasSize / viewState.mFactor;
    const GfVec2f viewMin(-viewState.mPosition.x, -viewState.mPosition.y);
    const GfVec2f viewMax = viewMin + worldSizeView;
    min = AnchorMin(min, viewMin);
    max = AnchorMax(max, viewMax);
    const GfVec2f nodesSize = max - min;
    const GfVec2f middleWorld = (min + max) * 0.5f;
    const GfVec2f minScreen = windowPos + options.mMinimap.Min * canvasSize;
    const GfVec2f maxScreen = windowPos + options.mMinimap.Max * canvasSize;
    const GfVec2f viewSize = maxScreen - minScreen;
    const GfVec2f middleScreen = (minScreen + maxScreen) * 0.5f;
    const float ratioY = viewSize.y / nodesSize.y;
    const float ratioX = viewSize.x / nodesSize.x;
    const float factor = AnchorMin(AnchorMin(ratioY, ratioX), 1.f);

    drawList->AddRectFilled(minScreen, maxScreen, IM_COL32(30, 30, 30, 200), 3, AnchorDrawFlags_RoundCornersAll);

    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);

        AnchorRect rect = node.mRect;
        rect.Min -= middleWorld;
        rect.Min *= factor;
        rect.Min += middleScreen;

        rect.Max -= middleWorld;
        rect.Max *= factor;
        rect.Max += middleScreen;

        drawList->AddRectFilled(rect.Min, rect.Max, nodeTemplate.mBackgroundColor, 1, AnchorDrawFlags_RoundCornersAll);
        if (node.mSelected)
        {
            drawList->AddRect(rect.Min, rect.Max, options.mSelectedNodeBorderColor, 1, AnchorDrawFlags_RoundCornersAll);
        }
    }

    // add view
    GfVec2f viewMinScreen = (viewMin - middleWorld) * factor + middleScreen;
    GfVec2f viewMaxScreen = (viewMax - middleWorld) * factor + middleScreen;
    drawList->AddRectFilled(viewMinScreen, viewMaxScreen, IM_COL32(255, 255, 255, 32), 1, AnchorDrawFlags_RoundCornersAll);
    drawList->AddRect(viewMinScreen, viewMaxScreen, IM_COL32(255, 255, 255, 128), 1, AnchorDrawFlags_RoundCornersAll);
    
    AnchorIO& io = Anchor::GetIO();
    const bool mouseInMinimap = AnchorRect(minScreen, maxScreen).Contains(io.MousePos);
    if (mouseInMinimap && io.MouseClicked[0])
    {
        const GfVec2f clickedRatio = (io.MousePos - minScreen) / viewSize;
        const GfVec2f worldPosCenter = GfVec2f(AnchorLerp(min.x, max.x, clickedRatio.x), AnchorLerp(min.y, max.y, clickedRatio.y));
        
        GfVec2f worldPosViewMin = worldPosCenter - worldSizeView * 0.5;
        GfVec2f worldPosViewMax = worldPosCenter + worldSizeView * 0.5;
        if (worldPosViewMin.x < min.x)
        {
            worldPosViewMin.x = min.x;
            worldPosViewMax.x = worldPosViewMin.x + worldSizeView.x;
        }
        if (worldPosViewMin.y < min.y)
        {
            worldPosViewMin.y = min.y;
            worldPosViewMax.y = worldPosViewMin.y + worldSizeView.y;
        }
        if (worldPosViewMax.x > max.x)
        {
            worldPosViewMax.x = max.x;
            worldPosViewMin.x = worldPosViewMax.x - worldSizeView.x;
        }
        if (worldPosViewMax.y > max.y)
        {
            worldPosViewMax.y = max.y;
            worldPosViewMin.y = worldPosViewMax.y - worldSizeView.y;
        }
        viewState.mPosition = GfVec2f(-worldPosViewMin.x, -worldPosViewMin.y);
    }
    return mouseInMinimap;
}

void Show(Delegate& delegate, const Options& options, ViewState& viewState, bool enabled, FitOnScreen* fit)
{
    Anchor::PushStyleVar(AnchorStyleVar_ChildBorderSize, 0.f);
    Anchor::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(0.f, 0.f));
    Anchor::PushStyleVar(AnchorStyleVar_FrameBorderSize, 0.f);

    const GfVec2f windowPos = Anchor::GetCursorScreenPos();
    const GfVec2f canvasSize = Anchor::GetContentRegionAvail();
    const GfVec2f scrollRegionLocalPos(0, 0);

    AnchorRect regionRect(windowPos, windowPos + canvasSize);

    HandleZoomScroll(regionRect, viewState, options);
    GfVec2f offset = Anchor::GetCursorScreenPos() + viewState.mPosition * viewState.mFactor;
    captureOffset = viewState.mPosition * viewState.mFactor;

    //Anchor::InvisibleButton("GraphEditorButton", canvasSize);
    Anchor::BeginChildFrame(71711, canvasSize);

    Anchor::SetCursorPos(windowPos);
    Anchor::BeginGroup();

    AnchorIO& io = Anchor::GetIO();

    // Create our child canvas
    Anchor::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(1, 1));
    Anchor::PushStyleVar(AnchorStyleVar_WindowPadding, GfVec2f(0, 0));
    Anchor::PushStyleColor(AnchorCol_ChildBg, IM_COL32(30, 30, 30, 200));

    AnchorDrawList* drawList = Anchor::GetWindowDrawList();
    Anchor::PushClipRect(regionRect.Min, regionRect.Max, true);
    drawList->AddRectFilled(windowPos, windowPos + canvasSize, options.mBackgroundColor);

    // Background or Display grid
    if (options.mRenderGrid)
    {
        DrawGrid(drawList, windowPos, viewState, canvasSize, options.mGridColor, options.mGridColor2, options.mGridSize);
    }
    
    // Fit view
    if (fit && ((*fit == Fit_AllNodes) || (*fit == Fit_SelectedNodes)))
    {
        FitNodes(delegate, viewState, canvasSize, (*fit == Fit_SelectedNodes));
    }

    if (enabled)
    {
        static NodeIndex hoveredNode = -1;
        // Display links
        drawList->ChannelsSplit(3);

        // minimap
        drawList->ChannelsSetCurrent(2); // minimap
        const bool inMinimap = DrawMiniMap(drawList, delegate, viewState, options, windowPos, canvasSize);

        // Focus rectangle
        if (Anchor::IsWindowFocused())
        {
           drawList->AddRect(regionRect.Min, regionRect.Max, options.mFrameFocus, 1.f, 0, 2.f);
        }

        drawList->ChannelsSetCurrent(1); // Background

        // Links
        DisplayLinks(delegate, drawList, offset, viewState.mFactor, regionRect, hoveredNode, options);

        // edit node link
        if (nodeOperation == NO_EditingLink)
        {
            GfVec2f p1 = editingNodeSource;
            GfVec2f p2 = io.MousePos;
            drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 3.0f);
        }

        // Display nodes
        drawList->PushClipRect(regionRect.Min, regionRect.Max, true);
        hoveredNode = -1;
        
        SlotIndex inputSlotOver = -1;
        SlotIndex outputSlotOver = -1;
        NodeIndex nodeOver = -1;

        const auto nodeCount = delegate.GetNodeCount();
        for (int i = 0; i < 2; i++)
        {
            for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
            {
                //const auto* node = &nodes[nodeIndex];
                const auto node = delegate.GetNode(nodeIndex);
                if (node.mSelected != (i != 0))
                {
                    continue;
                }

                // node view clipping
                AnchorRect nodeRect = GetNodeRect(node, viewState.mFactor);
                nodeRect.Min += offset;
                nodeRect.Max += offset;
                if (!regionRect.Overlaps(nodeRect))
                {
                    continue;
                }

                Anchor::PushID((int)nodeIndex);
                SlotIndex inputSlot = -1;
                SlotIndex outputSlot = -1;

                bool overInput = (!inMinimap) && HandleConnections(drawList, nodeIndex, offset, viewState.mFactor, delegate, options, false, inputSlot, outputSlot, inMinimap);

                // shadow
                /*
                GfVec2f shadowOffset = GfVec2f(30, 30);
                GfVec2f shadowPivot = (nodeRect.Min + nodeRect.Max) /2.f;
                GfVec2f shadowPointMiddle = shadowPivot + shadowOffset;
                GfVec2f shadowPointTop = GfVec2f(shadowPivot.x, nodeRect.Min.y) + shadowOffset;
                GfVec2f shadowPointBottom = GfVec2f(shadowPivot.x, nodeRect.Max.y) + shadowOffset;
                GfVec2f shadowPointLeft = GfVec2f(nodeRect.Min.x, shadowPivot.y) + shadowOffset;
                GfVec2f shadowPointRight = GfVec2f(nodeRect.Max.x, shadowPivot.y) + shadowOffset;

                // top left
                drawList->AddRectFilledMultiColor(nodeRect.Min + shadowOffset, shadowPointMiddle, IM_COL32(0 ,0, 0, 0), IM_COL32(0,0,0,0), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 0));

                // top right
                drawList->AddRectFilledMultiColor(shadowPointTop, shadowPointRight, IM_COL32(0 ,0, 0, 0), IM_COL32(0,0,0,0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 255));

                // bottom left
                drawList->AddRectFilledMultiColor(shadowPointLeft, shadowPointBottom, IM_COL32(0 ,0, 0, 0), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 0), IM_COL32(0,0,0,0));

                // bottom right
                drawList->AddRectFilledMultiColor(shadowPointMiddle, nodeRect.Max + shadowOffset, IM_COL32(0, 0, 0, 255), IM_COL32(0 ,0, 0, 0), IM_COL32(0,0,0,0), IM_COL32(0, 0, 0, 0));
                */
                if (DrawNode(drawList, nodeIndex, offset, viewState.mFactor, delegate, overInput, options, inMinimap, regionRect))
                {
                    hoveredNode = nodeIndex;
                }

                HandleConnections(drawList, nodeIndex, offset, viewState.mFactor, delegate, options, true, inputSlot, outputSlot, inMinimap);
                if (inputSlot != -1 || outputSlot != -1)
                {
                    inputSlotOver = inputSlot;
                    outputSlotOver = outputSlot;
                    nodeOver = nodeIndex;
                }

                Anchor::PopID();
            }
        }
        

        
        drawList->PopClipRect();

        if (nodeOperation == NO_MovingNodes)
        {
            if (Anchor::IsMouseDragging(0, 1))
            {
                GfVec2f delta = io.MouseDelta / viewState.mFactor;
                if (fabsf(delta.x) >= 1.f || fabsf(delta.y) >= 1.f)
                {
                    delegate.MoveSelectedNodes(delta);
                }
            }
        }

        drawList->ChannelsSetCurrent(0);

        // quad selection
        if (!inMinimap)
        {
            HandleQuadSelection(delegate, drawList, offset, viewState.mFactor, regionRect, options);
        }

        drawList->ChannelsMerge();

        // releasing mouse button means it's done in any operation
        if (nodeOperation == NO_PanView)
        {
            if (!io.MouseDown[2])
            {
                nodeOperation = NO_None;
            }
        }
        else if (nodeOperation != NO_None && !io.MouseDown[0])
        {
            nodeOperation = NO_None;
        }

        // right click
        if (!inMinimap && nodeOperation == NO_None && regionRect.Contains(io.MousePos) &&
                (Anchor::IsMouseClicked(1) /*|| (Anchor::IsWindowFocused() && Anchor::IsKeyPressedMap(AnchorKey_Tab))*/))
        {
            delegate.RightClick(nodeOver, inputSlotOver, outputSlotOver);
        }

        // Scrolling
        if (Anchor::IsWindowHovered() && !Anchor::IsAnyItemActive() && io.MouseClicked[2] && nodeOperation == NO_None)
        {
            nodeOperation = NO_PanView;
        }
        if (nodeOperation == NO_PanView)
        {
            viewState.mPosition += io.MouseDelta / viewState.mFactor;
        }
    }

    Anchor::PopClipRect();

    Anchor::PopStyleColor(1);
    Anchor::PopStyleVar(2);
    Anchor::EndGroup();
    Anchor::EndChildFrame();

    Anchor::PopStyleVar(3);
    
    // change fit to none
    if (fit)
    {
        *fit = Fit_None;
    }
}

bool EditOptions(Options& options)
{
    bool updated = false;
    if (Anchor::CollapsingHeader("Colors", nullptr))
    {
        ImColor backgroundColor(options.mBackgroundColor);
        ImColor gridColor(options.mGridColor);
        ImColor selectedNodeBorderColor(options.mSelectedNodeBorderColor);
        ImColor nodeBorderColor(options.mNodeBorderColor);
        ImColor quadSelection(options.mQuadSelection);
        ImColor quadSelectionBorder(options.mQuadSelectionBorder);
        ImColor defaultSlotColor(options.mDefaultSlotColor);
        ImColor frameFocus(options.mFrameFocus);

        updated |= Anchor::ColorEdit4("Background", (float*)&backgroundColor);
        updated |= Anchor::ColorEdit4("Grid", (float*)&gridColor);
        updated |= Anchor::ColorEdit4("Selected Node Border", (float*)&selectedNodeBorderColor);
        updated |= Anchor::ColorEdit4("Node Border", (float*)&nodeBorderColor);
        updated |= Anchor::ColorEdit4("Quad Selection", (float*)&quadSelection);
        updated |= Anchor::ColorEdit4("Quad Selection Border", (float*)&quadSelectionBorder);
        updated |= Anchor::ColorEdit4("Default Slot", (float*)&defaultSlotColor);
        updated |= Anchor::ColorEdit4("Frame when has focus", (float*)&frameFocus);

        options.mBackgroundColor = backgroundColor;
        options.mGridColor = gridColor;
        options.mSelectedNodeBorderColor = selectedNodeBorderColor;
        options.mNodeBorderColor = nodeBorderColor;
        options.mQuadSelection = quadSelection;
        options.mQuadSelectionBorder = quadSelectionBorder;
        options.mDefaultSlotColor = defaultSlotColor;
        options.mFrameFocus = frameFocus;
    }

    if (Anchor::CollapsingHeader("Options", nullptr))
    {
        updated |= Anchor::InputFloat4("Minimap", &options.mMinimap.Min.x);
        updated |= Anchor::InputFloat("Line Thickness", &options.mLineThickness);
        updated |= Anchor::InputFloat("Grid Size", &options.mGridSize);
        updated |= Anchor::InputFloat("Rounding", &options.mRounding);
        updated |= Anchor::InputFloat("Zoom Ratio", &options.mZoomRatio);
        updated |= Anchor::InputFloat("Zoom Lerp Factor", &options.mZoomLerpFactor);
        updated |= Anchor::InputFloat("Border Selection Thickness", &options.mBorderSelectionThickness);
        updated |= Anchor::InputFloat("Border Thickness", &options.mBorderThickness);
        updated |= Anchor::InputFloat("Slot Radius", &options.mNodeSlotRadius);
        updated |= Anchor::InputFloat("Slot Hover Factor", &options.mNodeSlotHoverFactor);
        updated |= Anchor::InputFloat2("Zoom min/max", &options.mMinZoom);
        updated |= Anchor::InputFloat("Slot Hover Factor", &options.mSnap);
        
        if (Anchor::RadioButton("Curved Links", options.mDisplayLinksAsCurves))
        {
            options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
            updated = true;
        }
        if (Anchor::RadioButton("Straight Links", !options.mDisplayLinksAsCurves))
        {
            options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
            updated = true;
        }

        updated |= Anchor::Checkbox("Allow Quad Selection", &options.mAllowQuadSelection);
        updated |= Anchor::Checkbox("Render Grid", &options.mRenderGrid);
        updated |= Anchor::Checkbox("Draw IO names on hover", &options.mDrawIONameOnHover);
    }

    return updated;
}

} // namespace
