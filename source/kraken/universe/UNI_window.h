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

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_region.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

#include "WM_operators.h"

#include "KKE_context.h"
#include "KKE_robinhood.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usd/collectionAPI.h>
// #include <wabi/usd/usdUI/window.h>

#include <deque>

WABI_NAMESPACE_BEGIN

typedef std::deque<wmEvent *> wmEventQueue;

struct ScrAreaMap
{
  /** ScrVert. */
  std::vector<struct ScrVert *> verts;
  /** ScrEdge. */
  std::vector<struct ScrEdge *> edges;
  /** ScrArea. */
  std::vector<struct ScrArea *> areas;

  ScrAreaMap() : verts(EMPTY), edges(EMPTY), areas(EMPTY) {}
};

struct wmWindow : public KrakenPrim
{
  SdfPath path;
  wmWindow *parent;

  UsdAttribute title;
  UsdAttribute icon;
  UsdAttribute state;
  UsdAttribute dpi;
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
  Scene *scene;

  /** Anchor system backend pointer. */
  void *anchorwin;

  /** Set to one for an active window. */
  bool active;

  bool addmousemove;

  /** Storage for event system. */
  wmEvent *eventstate;
  wmEventQueue event_queue;
  std::vector<wmEventHandler *> modalhandlers;

  /** Runtime Window State. */
  char windowstate;
  int winid;

  WorkSpaceInstanceHook *workspace_hook;

  ScrAreaMap global_areas;

  inline wmWindow(kContext *C, const SdfPath &stagepath);

  inline wmWindow(kContext *C, wmWindow *prim, const SdfPath &stagepath);
};


wmWindow::wmWindow(kContext *C, const SdfPath &stagepath)
  : KrakenPrim(KRAKEN_LUXOVERSE_CREATE(C)),
    path(stagepath),
    parent(NULL),
    title(EMPTY /*CreateTitleAttr()*/),
    icon(EMPTY /*CreateIconAttr()*/),
    state(EMPTY /*CreateStateAttr()*/),
    dpi(EMPTY /*CreateDpiAttr()*/),
    widgetunit(EMPTY /*CreateWidgetunitAttr()*/),
    scale(EMPTY /*CreateScaleAttr()*/),
    linewidth(EMPTY /*CreateLinewidthAttr()*/),
    pixelsz(EMPTY /*CreatePixelszAttr()*/),
    cursor(EMPTY /*CreateCursorAttr()*/),
    pos(EMPTY /*CreatePosAttr()*/),
    alignment(EMPTY /*CreateAlignmentAttr()*/),
    size(EMPTY /*CreateSizeAttr()*/),
    type(EMPTY /*CreateTypeAttr()*/),
    workspace_rel(EMPTY /*CreateUiWindowWorkspaceRel()*/),
    anchorwin(nullptr),
    active(true),
    addmousemove(false),
    eventstate(new wmEvent()),
    windowstate(0),
    winid(0),
    workspace_hook(nullptr)
{}

wmWindow::wmWindow(kContext *C, wmWindow *prim, const SdfPath &stagepath)
  : KrakenPrim(KRAKEN_LUXOVERSE_CREATE_CHILD(C)),
    path(KrakenPrim::GetPath()),
    parent(prim),
    title(EMPTY /*CreateTitleAttr()*/),
    icon(EMPTY /*CreateIconAttr()*/),
    state(EMPTY /*CreateStateAttr()*/),
    dpi(EMPTY /*CreateDpiAttr()*/),
    widgetunit(EMPTY /*CreateWidgetunitAttr()*/),
    scale(EMPTY /*CreateScaleAttr()*/),
    linewidth(EMPTY /*CreateLinewidthAttr()*/),
    pixelsz(EMPTY /*CreatePixelszAttr()*/),
    cursor(EMPTY /*CreateCursorAttr()*/),
    pos(EMPTY /*CreatePosAttr()*/),
    alignment(EMPTY /*CreateAlignmentAttr()*/),
    size(EMPTY /*CreateSizeAttr()*/),
    type(EMPTY /*CreateTypeAttr()*/),
    workspace_rel(EMPTY /*CreateUiWindowWorkspaceRel()*/),
    anchorwin(nullptr),
    active(true),
    addmousemove(false),
    eventstate(new wmEvent()),
    windowstate(0),
    winid(0),
    workspace_hook(nullptr)
{}

struct wmNotifier
{
  wmNotifier();

  wmWindow *window;
  unsigned int category, data, subtype, action;
  void *reference;

  TfNotice notice;

  void Push();
};

struct wmSpaceTypeListenerParams
{
  struct wmWindow *window;
  struct ScrArea *area;
  struct wmNotifier *notifier;
  const struct Scene *scene;

  wmSpaceTypeListenerParams()
    : window(POINTER_ZERO),
      area(POINTER_ZERO),
      notifier(POINTER_ZERO),
      scene(POINTER_ZERO)
  {}
};

typedef std::deque<wmNotifier *> wmNotifierQueue;

struct wmWindowManager : public KrakenPrim
{
  SdfPath path;

  /** All windows this manager controls. */
  TfHashMap<SdfPath, wmWindow *, SdfPath::Hash> windows;

  wmWindow *windrawable, *winactive;

  wmNotifierQueue notifier_queue;

  int op_undo_depth;

  /** Active dragged items. */
  std::vector<void *> drags;

  bool file_saved;

  inline wmWindowManager();
};

wmWindowManager::wmWindowManager()
  : path(KRAKEN_PATH_DEFAULTS::KRAKEN_WM),
    windows(EMPTY),
    windrawable(POINTER_ZERO),
    winactive(POINTER_ZERO),
    notifier_queue(EMPTY),
    op_undo_depth(0),
    drags(EMPTY),
    file_saved(VALUE_ZERO)
{}

WABI_NAMESPACE_END