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
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_api.h"

#include <wabi/usd/usdUI/tokens.h>

/**
 * Just some quick & handy inline tools,
 * out of the way of the main WindowManager
 * logic. */

WABI_NAMESPACE_BEGIN

inline TfToken wm_verify_spacetype(const TfToken &query)
{
  if (query == UsdUITokens->spaceView3D) {
    return UsdUITokens->spaceView3D;
  } else if (query == UsdUITokens->spaceGraph) {
    return UsdUITokens->spaceGraph;
  } else if (query == UsdUITokens->spaceOutliner) {
    return UsdUITokens->spaceOutliner;
  } else if (query == UsdUITokens->spaceProperties) {
    return UsdUITokens->spaceProperties;
  } else if (query == UsdUITokens->spaceFile) {
    return UsdUITokens->spaceFile;
  } else if (query == UsdUITokens->spaceImage) {
    return UsdUITokens->spaceImage;
  } else if (query == UsdUITokens->spaceInfo) {
    return UsdUITokens->spaceInfo;
  } else if (query == UsdUITokens->spaceSequence) {
    return UsdUITokens->spaceSequence;
  } else if (query == UsdUITokens->spaceText) {
    return UsdUITokens->spaceText;
  } else if (query == UsdUITokens->spaceNode) {
    return UsdUITokens->spaceNode;
  } else if (query == UsdUITokens->spaceConsole) {
    return UsdUITokens->spaceConsole;
  } else if (query == UsdUITokens->spacePref) {
    return UsdUITokens->spacePref;
  } else if (query == UsdUITokens->spaceClip) {
    return UsdUITokens->spaceClip;
  } else if (query == UsdUITokens->spaceTopbar) {
    return UsdUITokens->spaceTopbar;
  } else if (query == UsdUITokens->spaceStatusbar) {
    return UsdUITokens->spaceStatusbar;
  } else if (query == UsdUITokens->spaceSpreadsheet) {
    return UsdUITokens->spaceSpreadsheet;
  } else {
    return UsdUITokens->spaceEmpty;
  }
}

WABI_NAMESPACE_END
