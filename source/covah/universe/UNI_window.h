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
 * Universe.
 * Set the Stage.
 */

#include "UNI_context.h"

#include <wabi/wabi.h>

#include <wabi/base/tf/hashmap.h>

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usd/attribute.h>
#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usdUI/window.h>

WABI_NAMESPACE_BEGIN

struct wmWindow : public UsdUIWindow {

  SdfPath winhash;
  wmWindow *parent;

  UsdAttribute title;
  UsdAttribute icon;
  UsdAttribute state;
  UsdAttribute dpi;
  UsdAttribute dpifac;
  UsdAttribute widgetunit;
  UsdAttribute scale;
  UsdAttribute linewidth;
  UsdAttribute pixelsz;
  UsdAttribute cursor;
  UsdAttribute pos;
  UsdAttribute alignment;
  UsdAttribute size;
  UsdAttribute type;

  UsdUIWorkspace workspace;
  UsdUIScreen screen;

  /** Active scene for this window. */
  TfToken scene;

  /** Active session layer display name. */
  char view_layer_name[64];

  /** Anchor system backend pointer. */
  void *anchorwin;

  inline wmWindow(const SdfPath &path = SdfPath(STRINGIFY(COVAH_WINDOW)),
                  const SdfPath &wspace = SdfPath(STRINGIFY(COVAH_WORKSPACES_LAYOUT)),
                  const SdfPath &screen = SdfPath(STRINGIFY(COVAH_SCREEN_LAYOUT)));

 private:
  SdfPath m_sdf_wspace;
  SdfPath m_sdf_screen;
};

/**
 * Note: A workspace and a screen
 * must always exist on a Window,
 * and a screen must always exist
 * on a workspace. Therefore, the
 * paths get forced appends when
 * constructing a wmWindow. */
wmWindow::wmWindow(const SdfPath &path, const SdfPath &wspace, const SdfPath &screen)
  : UsdUIWindow(UNI.stage->DefinePrim(path)),
    winhash(path),
    m_sdf_wspace(winhash.AppendPath(wspace)),
    m_sdf_screen(m_sdf_wspace.AppendPath(screen)),
    title(CreateTitleAttr()),
    icon(CreateIconAttr()),
    state(CreateStateAttr()),
    dpi(CreateDpiAttr()),
    dpifac(CreateDpifacAttr()),
    widgetunit(CreateWidgetunitAttr()),
    scale(CreateScaleAttr()),
    linewidth(CreateLinewidthAttr()),
    pixelsz(CreatePixelszAttr()),
    cursor(CreateCursorAttr()),
    pos(CreatePosAttr()),
    alignment(CreateAlignmentAttr()),
    size(CreateSizeAttr()),
    type(CreateTypeAttr()),
    workspace(UsdUIWorkspace::Define(UNI.stage, path.AppendPath(wspace))),
    screen(UsdUIScreen::Define(UNI.stage, path.AppendPath(screen)))
{}

struct wmWindowManager {
  /** All windows this manager controls. */
  TfHashMap<SdfPath, wmWindow *, SdfPath::Hash> windows;
};

WABI_NAMESPACE_END