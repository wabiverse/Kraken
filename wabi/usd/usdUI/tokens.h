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
 *   gprim.GetMyTokenValuedAttr().Set(UsdUITokens->area);
 * @endcode
 */
struct UsdUITokensType {

  USDUI_API
  UsdUITokensType();

  /**
   * @brief "area"
   *
   * Possible value for UsdUIScreenAPI::GetPurposeAttr(), Default value for UsdUIScreenAPI::GetPurposeAttr() */
  const TfToken area;

  /**
   * @brief "attached"
   *
   * Possible value for UsdUIWindow::GetWindowTypeAttr(), Default value for UsdUIWindow::GetWindowTypeAttr() */
  const TfToken attached;

  /**
   * @brief "bottom"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken bottom;

  /**
   * @brief "closed"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken closed;

  /**
   * @brief "Console"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken console;

  /**
   * @brief "detached"
   *
   * Possible value for UsdUIWindow::GetWindowTypeAttr() */
  const TfToken detached;

  /**
   * @brief "dialog"
   *
   * Possible value for UsdUIWindow::GetWindowTypeAttr() */
  const TfToken dialog;

  /**
   * @brief "DopeSheetEditor"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken dopeSheetEditor;

  /**
   * @brief "Empty"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr(), Default value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken empty;

  /**
   * @brief "FileBrowser"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken fileBrowser;

  /**
   * @brief "horizontalSplit"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken horizontalSplit;

  /**
   * @brief "ImageEditor"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken imageEditor;

  /**
   * @brief "Info"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken info;

  /**
   * @brief "left"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken left;

  /**
   * @brief "minimized"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken minimized;

  /**
   * @brief "MovieEditor"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken movieEditor;

  /**
   * @brief "NodeGraph"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken nodeGraph;

  /**
   * @brief "none"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr(), Default value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken none;

  /**
   * @brief "open"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken open;

  /**
   * @brief "Outliner"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken outliner;

  /**
   * @brief "Preferences"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken preferences;

  /**
   * @brief "Properties"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken properties;

  /**
   * @brief "region"
   *
   * Possible value for UsdUIScreenAPI::GetPurposeAttr() */
  const TfToken region;

  /**
   * @brief "right"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken right;

  /**
   * @brief "SequenceEditor"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken sequenceEditor;

  /**
   * @brief "Spreadsheet"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken spreadsheet;

  /**
   * @brief "StatusBar"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken statusBar;

  /**
   * @brief "TextEditor"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken textEditor;

  /**
   * @brief "top"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken top;

  /**
   * @brief "TopBar"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken topBar;

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
   * @brief "ui:screen:area:icon"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaIcon;

  /**
   * @brief "ui:screen:area:layout"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaLayout;

  /**
   * @brief "ui:screen:area:name"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaName;

  /**
   * @brief "ui:screen:area:pos"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaPos;

  /**
   * @brief "ui:screen:area:purpose"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaPurpose;

  /**
   * @brief "ui:screen:area:region"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaRegion;

  /**
   * @brief "ui:screen:area:showMenus"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaShowMenus;

  /**
   * @brief "ui:screen:area:size"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaSize;

  /**
   * @brief "ui:screen:area:type"
   *
   * UsdUIScreenAPI */
  const TfToken uiScreenAreaType;

  /**
   * @brief "ui:screen:area:workspace"
   *
   * UsdUIWorkspace, UsdUIScreenAPI */
  const TfToken uiScreenAreaWorkspace;

  /**
   * @brief "ui:windowIcon"
   *
   * UsdUIWindow */
  const TfToken uiWindowIcon;

  /**
   * @brief "ui:windowPos"
   *
   * UsdUIWindow */
  const TfToken uiWindowPos;

  /**
   * @brief "ui:windowSize"
   *
   * UsdUIWindow */
  const TfToken uiWindowSize;

  /**
   * @brief "ui:windowTitle"
   *
   * UsdUIWindow */
  const TfToken uiWindowTitle;

  /**
   * @brief "ui:windowType"
   *
   * UsdUIWindow */
  const TfToken uiWindowType;

  /**
   * @brief "verticalSplit"
   *
   * Possible value for UsdUIScreenAPI::GetLayoutAttr() */
  const TfToken verticalSplit;

  /**
   * @brief "View3D"
   *
   * Possible value for UsdUIScreenAPI::GetTypeAttr() */
  const TfToken view3D;

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
