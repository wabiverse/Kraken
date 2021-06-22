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
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

#include "WM_operators.h"

#include "CKE_context.h"
#include "CKE_robinhood.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usdUI/window.h>

#include <deque>

WABI_NAMESPACE_BEGIN

typedef std::deque<wmEvent *> wmEventQueue;

struct wmWindow : public UsdUIWindow, public CovahObject
{
  SdfPath path;
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

  UsdRelationship workspace_rel;

  /** Active scene for this window. */
  TfToken scene;

  /** Anchor system backend pointer. */
  void *anchorwin;

  /** Set to one for an active window. */
  bool active;

  bool addmousemove;

  /** Storage for event system. */
  wmEvent *eventstate;
  wmEventQueue event_queue;

  struct
  {
    WorkSpace *workspace;
    cScreen *screen;
  } prims;

  inline wmWindow(cContext *C,
                  const SdfPath &stagepath = SdfPath(COVAH_PATH_DEFAULTS::COVAH_WINDOW),
                  const SdfPath &wspace = SdfPath(COVAH_PATH_DEFAULTS::COVAH_WORKSPACES_LAYOUT),
                  const SdfPath &screen = SdfPath(COVAH_PATH_DEFAULTS::COVAH_SCREEN_LAYOUT));

  inline wmWindow(cContext *C, wmWindow *prim, const SdfPath &stagepath);
};


wmWindow::wmWindow(cContext *C,
                   const SdfPath &stagepath,
                   const SdfPath &wspace,
                   const SdfPath &screen)
  : UsdUIWindow(COVAH_UNIVERSE_CREATE(C)),
    path(stagepath),
    parent(NULL),
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
    workspace_rel(CreateUiWindowWorkspaceRel()),
    anchorwin(nullptr),
    active(true),
    addmousemove(false),
    eventstate(new wmEvent()),
    prims({.workspace = new WorkSpace(C, wspace),
           .screen = new cScreen(C, screen)})
{}

wmWindow::wmWindow(cContext *C, wmWindow *prim, const SdfPath &stagepath)
  : UsdUIWindow(COVAH_UNIVERSE_CREATE_CHILD(C)),
    path(GetPath()),
    parent(prim),
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
    workspace_rel(CreateUiWindowWorkspaceRel()),
    anchorwin(nullptr),
    active(true),
    addmousemove(false),
    eventstate(new wmEvent()),
    prims({.workspace = prim->prims.workspace,
           .screen = prim->prims.screen})
{}

struct wmNotifier
{
  wmWindow *window;

  unsigned int category, data, subtype, action;

  void *reference;

  inline wmNotifier();
};

wmNotifier::wmNotifier()
  : window(nullptr),
    category(0),
    data(0),
    subtype(0),
    action(0),
    reference(nullptr)
{}

typedef std::vector<wmNotifier *> wmNotifierQueue;

struct wmWindowManager : public CovahObject
{
  /** All windows this manager controls. */
  TfHashMap<SdfPath, wmWindow *, SdfPath::Hash> windows;

  wmWindow *windrawable, *winactive;

  wmNotifierQueue notifier_queue;

  int op_undo_depth;

  inline wmWindowManager();
};

wmWindowManager::wmWindowManager()
  : windows(),
    windrawable(nullptr),
    winactive(nullptr),
    notifier_queue(),
    op_undo_depth(0)
{}

WABI_NAMESPACE_END