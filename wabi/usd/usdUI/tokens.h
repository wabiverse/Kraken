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

#include "wabi/base/tf/staticData.h"
#include "wabi/base/tf/token.h"
#include "wabi/usd/usdUI/api.h"
#include "wabi/wabi.h"
#include <vector>

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
 *   gprim.GetMyTokenValuedAttr().Set(UsdUITokens->closed);
 * @endcode
 */
struct UsdUITokensType {

  USDUI_API
  UsdUITokensType();

  /**
   * @brief "closed"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken closed;

  /**
   * @brief "minimized"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken minimized;

  /**
   * @brief "open"
   *
   * Possible value for UsdUINodeGraphNodeAPI::GetExpansionStateAttr() */
  const TfToken open;

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
   * @brief "ui:title"
   *
   * UsdUIWindow */
  const TfToken uiTitle;

  /**
   * @brief "ui:windowCoords"
   *
   * UsdUIWindow */
  const TfToken uiWindowCoords;

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
