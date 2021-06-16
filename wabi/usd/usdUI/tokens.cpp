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

#include "wabi/usd/usdUI/tokens.h"

WABI_NAMESPACE_BEGIN

UsdUITokensType::UsdUITokensType()
  : area("area", TfToken::Immortal),
    attached("attached", TfToken::Immortal),
    bottom("bottom", TfToken::Immortal),
    closed("closed", TfToken::Immortal),
    console("Console", TfToken::Immortal),
    detached("detached", TfToken::Immortal),
    dialog("dialog", TfToken::Immortal),
    dopeSheetEditor("DopeSheetEditor", TfToken::Immortal),
    empty("Empty", TfToken::Immortal),
    fileBrowser("FileBrowser", TfToken::Immortal),
    horizontalSplit("horizontalSplit", TfToken::Immortal),
    imageEditor("ImageEditor", TfToken::Immortal),
    info("Info", TfToken::Immortal),
    left("left", TfToken::Immortal),
    minimized("minimized", TfToken::Immortal),
    movieEditor("MovieEditor", TfToken::Immortal),
    nodeGraph("NodeGraph", TfToken::Immortal),
    none("none", TfToken::Immortal),
    open("open", TfToken::Immortal),
    outliner("Outliner", TfToken::Immortal),
    preferences("Preferences", TfToken::Immortal),
    properties("Properties", TfToken::Immortal),
    region("region", TfToken::Immortal),
    right("right", TfToken::Immortal),
    sequenceEditor("SequenceEditor", TfToken::Immortal),
    spreadsheet("Spreadsheet", TfToken::Immortal),
    statusBar("StatusBar", TfToken::Immortal),
    textEditor("TextEditor", TfToken::Immortal),
    top("top", TfToken::Immortal),
    topBar("TopBar", TfToken::Immortal),
    uiDescription("ui:description", TfToken::Immortal),
    uiDisplayGroup("ui:displayGroup", TfToken::Immortal),
    uiDisplayName("ui:displayName", TfToken::Immortal),
    uiNodegraphNodeDisplayColor("ui:nodegraph:node:displayColor", TfToken::Immortal),
    uiNodegraphNodeExpansionState("ui:nodegraph:node:expansionState", TfToken::Immortal),
    uiNodegraphNodeIcon("ui:nodegraph:node:icon", TfToken::Immortal),
    uiNodegraphNodePos("ui:nodegraph:node:pos", TfToken::Immortal),
    uiNodegraphNodeSize("ui:nodegraph:node:size", TfToken::Immortal),
    uiNodegraphNodeStackingOrder("ui:nodegraph:node:stackingOrder", TfToken::Immortal),
    uiScreenAreaIcon("ui:screen:area:icon", TfToken::Immortal),
    uiScreenAreaLayout("ui:screen:area:layout", TfToken::Immortal),
    uiScreenAreaName("ui:screen:area:name", TfToken::Immortal),
    uiScreenAreaPos("ui:screen:area:pos", TfToken::Immortal),
    uiScreenAreaPurpose("ui:screen:area:purpose", TfToken::Immortal),
    uiScreenAreaRegion("ui:screen:area:region", TfToken::Immortal),
    uiScreenAreaShowMenus("ui:screen:area:showMenus", TfToken::Immortal),
    uiScreenAreaSize("ui:screen:area:size", TfToken::Immortal),
    uiScreenAreaType("ui:screen:area:type", TfToken::Immortal),
    uiScreenAreaWorkspace("ui:screen:area:workspace", TfToken::Immortal),
    uiWindowIcon("ui:windowIcon", TfToken::Immortal),
    uiWindowPos("ui:windowPos", TfToken::Immortal),
    uiWindowSize("ui:windowSize", TfToken::Immortal),
    uiWindowTitle("ui:windowTitle", TfToken::Immortal),
    uiWindowType("ui:windowType", TfToken::Immortal),
    verticalSplit("verticalSplit", TfToken::Immortal),
    view3D("View3D", TfToken::Immortal),
    allTokens({
      area,
      attached,
      bottom,
      closed,
      console,
      detached,
      dialog,
      dopeSheetEditor,
      empty,
      fileBrowser,
      horizontalSplit,
      imageEditor,
      info,
      left,
      minimized,
      movieEditor,
      nodeGraph,
      none,
      open,
      outliner,
      preferences,
      properties,
      region,
      right,
      sequenceEditor,
      spreadsheet,
      statusBar,
      textEditor,
      top,
      topBar,
      uiDescription,
      uiDisplayGroup,
      uiDisplayName,
      uiNodegraphNodeDisplayColor,
      uiNodegraphNodeExpansionState,
      uiNodegraphNodeIcon,
      uiNodegraphNodePos,
      uiNodegraphNodeSize,
      uiNodegraphNodeStackingOrder,
      uiScreenAreaIcon,
      uiScreenAreaLayout,
      uiScreenAreaName,
      uiScreenAreaPos,
      uiScreenAreaPurpose,
      uiScreenAreaRegion,
      uiScreenAreaShowMenus,
      uiScreenAreaSize,
      uiScreenAreaType,
      uiScreenAreaWorkspace,
      uiWindowIcon,
      uiWindowPos,
      uiWindowSize,
      uiWindowTitle,
      uiWindowType,
      verticalSplit,
      view3D})
{}

TfStaticData<UsdUITokensType> UsdUITokens;

WABI_NAMESPACE_END
