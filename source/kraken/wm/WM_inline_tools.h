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
 * Window Manager.
 * Making GUI Fly.
 */

#include "KLI_compiler_compat.h"

#include "USD_space_types.h"

#include <wabi/usd/usdUI/tokens.h>

/**
 * Just some quick & handy inline tools,
 * out of the way of the main WindowManager
 * logic. */

KLI_INLINE wabi::TfToken wm_verify_spacetype(const wabi::TfToken &query)
{
  if (query == wabi::UsdUITokens->spaceView3D) {
    return wabi::UsdUITokens->spaceView3D;
  } else if (query == wabi::UsdUITokens->spaceGraph) {
    return wabi::UsdUITokens->spaceGraph;
  } else if (query == wabi::UsdUITokens->spaceOutliner) {
    return wabi::UsdUITokens->spaceOutliner;
  } else if (query == wabi::UsdUITokens->spaceProperties) {
    return wabi::UsdUITokens->spaceProperties;
  } else if (query == wabi::UsdUITokens->spaceFile) {
    return wabi::UsdUITokens->spaceFile;
  } else if (query == wabi::UsdUITokens->spaceImage) {
    return wabi::UsdUITokens->spaceImage;
  } else if (query == wabi::UsdUITokens->spaceInfo) {
    return wabi::UsdUITokens->spaceInfo;
  } else if (query == wabi::UsdUITokens->spaceSequence) {
    return wabi::UsdUITokens->spaceSequence;
  } else if (query == wabi::UsdUITokens->spaceText) {
    return wabi::UsdUITokens->spaceText;
  } else if (query == wabi::UsdUITokens->spaceNode) {
    return wabi::UsdUITokens->spaceNode;
  } else if (query == wabi::UsdUITokens->spaceConsole) {
    return wabi::UsdUITokens->spaceConsole;
  } else if (query == wabi::UsdUITokens->spacePref) {
    return wabi::UsdUITokens->spacePref;
  } else if (query == wabi::UsdUITokens->spaceClip) {
    return wabi::UsdUITokens->spaceClip;
  } else if (query == wabi::UsdUITokens->spaceTopbar) {
    return wabi::UsdUITokens->spaceTopbar;
  } else if (query == wabi::UsdUITokens->spaceStatusbar) {
    return wabi::UsdUITokens->spaceStatusbar;
  } else if (query == wabi::UsdUITokens->spaceSpreadsheet) {
    return wabi::UsdUITokens->spaceSpreadsheet;
  } else {
    return wabi::UsdUITokens->spaceEmpty;
  }
}

KLI_INLINE eSpaceType WM_spacetype_enum_from_token(const wabi::TfToken &query)
{
  if (query.IsEmpty()) {
    return SPACE_EMPTY;
  }

  if (query == wabi::UsdUITokens->spaceView3D) {
    return SPACE_VIEW3D;
  } else if (query == wabi::UsdUITokens->spaceGraph) {
    return SPACE_GRAPH;
  } else if (query == wabi::UsdUITokens->spaceOutliner) {
    return SPACE_OUTLINER;
  } else if (query == wabi::UsdUITokens->spaceProperties) {
    return SPACE_PROPERTIES;
  } else if (query == wabi::UsdUITokens->spaceFile) {
    return SPACE_FILE;
  } else if (query == wabi::UsdUITokens->spaceImage) {
    return SPACE_IMAGE;
  } else if (query == wabi::UsdUITokens->spaceInfo) {
    return SPACE_INFO;
  } else if (query == wabi::UsdUITokens->spaceSequence) {
    return SPACE_SEQ;
  } else if (query == wabi::UsdUITokens->spaceText) {
    return SPACE_TEXT;
  } else if (query == wabi::UsdUITokens->spaceNode) {
    return SPACE_NODE;
  } else if (query == wabi::UsdUITokens->spaceConsole) {
    return SPACE_CONSOLE;
  } else if (query == wabi::UsdUITokens->spacePref) {
    return SPACE_USERPREF;
  } else if (query == wabi::UsdUITokens->spaceClip) {
    return SPACE_CLIP;
  } else if (query == wabi::UsdUITokens->spaceTopbar) {
    return SPACE_TOPBAR;
  } else if (query == wabi::UsdUITokens->spaceStatusbar) {
    return SPACE_STATUSBAR;
  } else if (query == wabi::UsdUITokens->spaceSpreadsheet) {
    return SPACE_SPREADSHEET;
  } else {
    return SPACE_EMPTY;
  }
}

