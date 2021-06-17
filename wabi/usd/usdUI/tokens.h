/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */

/* clang-format off */

#ifndef USDUI_TOKENS_H
#define USDUI_TOKENS_H

/**
 * @file usdUI/tokens.h */

/**
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *
 *  This is an automatically generated file.
 *  Do not hand-edit!
 *
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

#  include "wabi/wabi.h"
#  include "wabi/usd/usdUI/api.h"
#  include "wabi/base/tf/staticData.h"
#  include "wabi/base/tf/token.h"
#  include <vector>

WABI_NAMESPACE_BEGIN

/**
 * @class UsdUITokensType
 *
 * @link UsdUITokens @endlink provides static, efficient
 * @link TfToken TfTokens\endlink for use in all public USD API.
 *
 * These tokens are auto-generated from the module's schema, representing
 * property names, for when you need to fetch an attribute or relationship
 * directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
 * manner, and allow the compiler to verify that you spelled the name
 * correctly.
 *
 * UsdUITokens also contains all of the @em allowedTokens values
 * declared for schema builtin attributes of 'token' scene description type.
 *
 * Use UsdUITokens like so:
 *
 * @code
 *   gprim.GetMyTokenValuedAttr().Set(UsdUITokens->areas);
 * @endcode
 */
struct UsdUITokensType {

  USDUI_API
  UsdUITokensType();

  /**
   * @brief "areas"
   *
   *  This token represents the collection name to use with UsdCollectionAPI to represent groups of UI layout areas of a UsdUIScreen prim.  */
  const TfToken areas;

  /**
   * @brief "bottom"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken bottom;

  /**
   * @brief "closed"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken closed;

  /**
   * @brief "copy"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken copy;

  /**
   * @brief "cross"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken cross;

  /**
   * @brief "default"
   *
   * Possible value for UsdUIWindow::GetCursorAttr(), Default value for UsdUIWindow::GetCursorAttr() */
  const TfToken default_;

  /**
   * @brief "detached"
   *
   * Possible value for UsdUIWindow::GetTypeAttr() */
  const TfToken detached;

  /**
   * @brief "dialog"
   *
   * Possible value for UsdUIWindow::GetTypeAttr() */
  const TfToken dialog;

  /**
   * @brief "dot"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken dot;

  /**
   * @brief "edit"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken edit;

  /**
   * @brief "embedded"
   *
   * Possible value for UsdUIWindow::GetStateAttr() */
  const TfToken embedded;

  /**
   * @brief "eraser"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken eraser;

  /**
   * @brief "eyedropper"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken eyedropper;

  /**
   * @brief "fullscreen"
   *
   * Possible value for UsdUIWindow::GetStateAttr() */
  const TfToken fullscreen;

  /**
   * @brief "hand"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken hand;

  /**
   * @brief "horizontalSplit"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken horizontalSplit;

  /**
   * @brief "hSplit"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken hSplit;

  /**
   * @brief "knife"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken knife;

  /**
   * @brief "left"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken left;

  /**
   * @brief "maximized"
   *
   * Possible value for UsdUIWindow::GetStateAttr() */
  const TfToken maximized;

  /**
   * @brief "minimized"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr(), Possible value for UsdUIWindow::GetStateAttr() */
  const TfToken minimized;

  /**
   * @brief "none"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr(), Default value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken none;

  /**
   * @brief "normal"
   *
   * Possible value for UsdUIWindow::GetStateAttr(), Default value for UsdUIWindow::GetStateAttr(), Possible value for UsdUIWindow::GetTypeAttr(), Default value for UsdUIWindow::GetTypeAttr() */
  const TfToken normal;

  /**
   * @brief "open"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken open;

  /**
   * @brief "paint"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken paint;

  /**
   * @brief "paintBrush"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken paintBrush;

  /**
   * @brief "right"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken right;

  /**
   * @brief "stop"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken stop;

  /**
   * @brief "swapArea"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken swapArea;

  /**
   * @brief "textEdit"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken textEdit;

  /**
   * @brief "top"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken top;

  /**
   * @brief "ui:area:icon"
   *
   * UsdUIArea */
  const TfToken uiAreaIcon;

  /**
   * @brief "ui:area:name"
   *
   * UsdUIArea */
  const TfToken uiAreaName;

  /**
   * @brief "ui:area:pos"
   *
   * UsdUIArea */
  const TfToken uiAreaPos;

  /**
   * @brief "ui:area:size"
   *
   * UsdUIArea */
  const TfToken uiAreaSize;

  /**
   * @brief "ui:description"
   *
   * UsdUIBackdrop */
  const TfToken uiDescription;

  /**
   * @brief "ui:displayGroup"
   *
   * UsdUISceneGraphPrimAPI */
  const TfToken uiDisplayGroup;

  /**
   * @brief "ui:displayName"
   *
   * UsdUISceneGraphPrimAPI */
  const TfToken uiDisplayName;

  /**
   * @brief "ui:nodegraph:node:displayColor"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodeDisplayColor;

  /**
   * @brief "ui:nodegraph:node:expansionState"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodeExpansionState;

  /**
   * @brief "ui:nodegraph:node:icon"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodeIcon;

  /**
   * @brief "ui:nodegraph:node:pos"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodePos;

  /**
   * @brief "ui:nodegraph:node:size"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodeSize;

  /**
   * @brief "ui:nodegraph:node:stackingOrder"
   *
   * UsdUINodeGraphNodeAPI */
  const TfToken uiNodegraphNodeStackingOrder;

  /**
   * @brief "ui:screen:alignment"
   *
   * UsdUIScreen */
  const TfToken uiScreenAlignment;

  /**
   * @brief "ui:screen:areas"
   *
   * UsdUIScreen */
  const TfToken uiScreenAreas;

  /**
   * @brief "ui:screen:collection:areas:includeRoot"
   *
   * UsdUIScreen */
  const TfToken uiScreenCollectionAreasIncludeRoot;

  /**
   * @brief "ui:window:cursor"
   *
   * UsdUIWindow */
  const TfToken uiWindowCursor;

  /**
   * @brief "ui:window:dpi"
   *
   * UsdUIWindow */
  const TfToken uiWindowDpi;

  /**
   * @brief "ui:window:dpifac"
   *
   * UsdUIWindow */
  const TfToken uiWindowDpifac;

  /**
   * @brief "ui:window:icon"
   *
   * UsdUIWindow */
  const TfToken uiWindowIcon;

  /**
   * @brief "ui:window:linewidth"
   *
   * UsdUIWindow */
  const TfToken uiWindowLinewidth;

  /**
   * @brief "ui:window:pixelsz"
   *
   * UsdUIWindow */
  const TfToken uiWindowPixelsz;

  /**
   * @brief "ui:window:pos"
   *
   * UsdUIWindow */
  const TfToken uiWindowPos;

  /**
   * @brief "ui:window:scale"
   *
   * UsdUIWindow */
  const TfToken uiWindowScale;

  /**
   * @brief "ui:window:size"
   *
   * UsdUIWindow */
  const TfToken uiWindowSize;

  /**
   * @brief "ui:window:state"
   *
   * UsdUIWindow */
  const TfToken uiWindowState;

  /**
   * @brief "ui:window:title"
   *
   * UsdUIWindow */
  const TfToken uiWindowTitle;

  /**
   * @brief "ui:window:type"
   *
   * UsdUIWindow */
  const TfToken uiWindowType;

  /**
   * @brief "ui:window:widgetunit"
   *
   * UsdUIWindow */
  const TfToken uiWindowWidgetunit;

  /**
   * @brief "ui:window:workspace"
   *
   * UsdUIWindow */
  const TfToken uiWindowWorkspace;

  /**
   * @brief "ui:workspace:name"
   *
   * UsdUIWorkspace */
  const TfToken uiWorkspaceName;

  /**
   * @brief "ui:workspace:screen"
   *
   * UsdUIWorkspace */
  const TfToken uiWorkspaceScreen;

  /**
   * @brief "verticalSplit"
   *
   * Possible value for UsdUIScreen::GetAlignmentAttr() */
  const TfToken verticalSplit;

  /**
   * @brief "vloop"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken vloop;

  /**
   * @brief "vSplit"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken vSplit;

  /**
   * @brief "wait"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken wait;

  /**
   * @brief "xMove"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken xMove;

  /**
   * @brief "yMove"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken yMove;

  /**
   * @brief "zoomIn"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken zoomIn;

  /**
   * @brief "zoomOut"
   *
   * Possible value for UsdUIWindow::GetCursorAttr() */
  const TfToken zoomOut;

  /**
   * A vector of all of the tokens listed above. */
  const std::vector<TfToken> allTokens;
};

/**
 * @var UsdUITokens
 *
 * A global variable with static, efficient @link TfToken
 * TfTokens\endlink for use in all public USD API.
 * 
 * @sa UsdUITokensType */
extern USDUI_API TfStaticData<UsdUITokensType> UsdUITokens;

WABI_NAMESPACE_END

#endif
