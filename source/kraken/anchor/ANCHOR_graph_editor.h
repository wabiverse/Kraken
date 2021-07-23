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

#include <vector>
#include <stdint.h>
#include <string>
#include "ANCHOR_api.h"
#include "ANCHOR_internal.h"

namespace AnchorGraphEditor
{

  typedef size_t NodeIndex;
  typedef size_t SlotIndex;
  typedef size_t LinkIndex;
  typedef size_t TemplateIndex;

  // Force the view to be respositionned and zoom to fit nodes with Show function.
  // Parameter value will be changed to Fit_None by the function.
  enum FitOnScreen
  {
    Fit_None,
    Fit_AllNodes,
    Fit_SelectedNodes
  };

  // Display options and colors
  struct Options
  {
    AnchorBBox mMinimap{
      {0.75f, 0.8f, 0.99f, 0.99f}
    };                             // rectangle coordinates of minimap
    AnchorU32 mBackgroundColor{ANCHOR_COL32(40, 40, 40, 255)};  // full background color
    AnchorU32 mGridColor{ANCHOR_COL32(0, 0, 0, 60)};            // grid lines color
    AnchorU32 mGridColor2{ANCHOR_COL32(0, 0, 0, 160)};          // grid lines color every 10th
    AnchorU32 mSelectedNodeBorderColor{
      ANCHOR_COL32(255, 130, 30, 255)};                              // node border color when it's selected
    AnchorU32 mNodeBorderColor{ANCHOR_COL32(100, 100, 100, 0)};      // node border color when it's not selected
    AnchorU32 mQuadSelection{ANCHOR_COL32(255, 32, 32, 64)};         // quad selection inside color
    AnchorU32 mQuadSelectionBorder{ANCHOR_COL32(255, 32, 32, 255)};  // quad selection border color
    AnchorU32 mDefaultSlotColor{
      ANCHOR_COL32(128, 128, 128, 255)};                     // when no color is provided in node template, use this value
    AnchorU32 mFrameFocus{ANCHOR_COL32(64, 128, 255, 255)};  // rectangle border when graph editor has focus
    float mLineThickness{5};                                 // links width in pixels when zoom value is 1
    float mGridSize{64.f};                                   // background grid size in pixels when zoom value is 1
    float mRounding{3.f};                                    // rounding at node corners
    float mZoomRatio{0.1f};                                  // factor per mouse wheel delta
    float mZoomLerpFactor{0.25f};                            // the smaller, the smoother
    float mBorderSelectionThickness{6.f};                    // thickness of selection border around nodes
    float mBorderThickness{6.f};                             // thickness of selection border around nodes
    float mNodeSlotRadius{8.f};                              // circle radius for inputs and outputs
    float mNodeSlotHoverFactor{1.2f};                        // increase size when hovering
    float mMinZoom{0.2f}, mMaxZoom{1.1f};
    float mSnap{5.f};
    bool mDisplayLinksAsCurves{true};  // false is straight and 45deg lines
    bool mAllowQuadSelection{true};    // multiple selection using drag and drop
    bool mRenderGrid{true};            // grid or nothing
    bool mDrawIONameOnHover{true};     // only draw node input/output when hovering
  };

  // View state: scroll position and zoom factor
  struct ViewState
  {
    wabi::GfVec2f mPosition{0.0f, 0.0f};  // scroll position
    float mFactor{1.0f};                  // current zoom factor
    float mFactorTarget{1.0f};            // targeted zoom factor interpolated using Options.mZoomLerpFactor
  };

  struct Template
  {
    AnchorU32 mHeaderColor;
    AnchorU32 mBackgroundColor;
    AnchorU32 mBackgroundColorOver;
    AnchorU8 mInputCount;
    const char **mInputNames;  // can be nullptr. No text displayed.
    AnchorU32 *mInputColors;   // can be nullptr, default slot color will be used.
    AnchorU8 mOutputCount;
    const char **mOutputNames;  // can be nullptr. No text displayed.
    AnchorU32 *mOutputColors;   // can be nullptr, default slot color will be used.
  };

  struct Node
  {
    const char *mName;
    TemplateIndex mTemplateIndex;
    AnchorBBox mRect;
    bool mSelected{false};
  };

  struct Link
  {
    NodeIndex mInputNodeIndex;
    SlotIndex mInputSlotIndex;
    NodeIndex mOutputNodeIndex;
    SlotIndex mOutputSlotIndex;
  };

  struct Delegate
  {
    virtual bool AllowedLink(NodeIndex from, NodeIndex to) = 0;

    virtual void SelectNode(NodeIndex nodeIndex, bool selected) = 0;
    virtual void MoveSelectedNodes(const wabi::GfVec2f delta) = 0;

    virtual void AddLink(NodeIndex inputNodeIndex,
                         SlotIndex inputSlotIndex,
                         NodeIndex outputNodeIndex,
                         SlotIndex outputSlotIndex) = 0;
    virtual void DelLink(LinkIndex linkIndex) = 0;

    // user is responsible for clipping
    virtual void CustomDraw(AnchorDrawList *drawList, AnchorBBox rectangle, NodeIndex nodeIndex) = 0;

    // use mouse position to open context menu
    // if nodeIndex != -1, right click happens on the specified node
    virtual void RightClick(NodeIndex nodeIndex, SlotIndex slotIndexInput, SlotIndex slotIndexOutput) = 0;

    virtual const size_t GetTemplateCount() = 0;
    virtual const Template GetTemplate(TemplateIndex index) = 0;

    virtual const size_t GetNodeCount() = 0;
    virtual const Node GetNode(NodeIndex index) = 0;

    virtual const size_t GetLinkCount() = 0;
    virtual const Link GetLink(LinkIndex index) = 0;
  };

  void Show(Delegate &delegate,
            const Options &options,
            ViewState &viewState,
            bool enabled,
            FitOnScreen *fit = nullptr);
  void GraphEditorClear();

  bool EditOptions(Options &options);

}  // namespace AnchorGraphEditor
