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
 * Universe.
 * Set the Stage.
 */

#include "USD_wm_types.h"
#include "USD_context.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_workspace.h"
#include "USD_ID.h"

#include "WM_operators.h"

#include "KKE_context.h"
#include "KKE_robinhood.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usdUI/window.h>

#include <deque>

KRAKEN_NAMESPACE_BEGIN

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

struct wmWindow : public wabi::UsdUIWindow
{
  wabi::SdfPath path;
  wmWindow *parent;

  wabi::UsdAttribute title;
  wabi::UsdAttribute icon;
  wabi::UsdAttribute state;
  wabi::UsdAttribute dpi;
  wabi::UsdAttribute widgetunit;
  wabi::UsdAttribute scale;
  wabi::UsdAttribute linewidth;
  wabi::UsdAttribute pixelsz;
  wabi::UsdAttribute cursor;
  wabi::UsdAttribute pos;
  wabi::UsdAttribute alignment;
  wabi::UsdAttribute size;
  wabi::UsdAttribute type;

  wabi::UsdRelationship workspace_rel;

  /** Active scene for this window. */
  Scene *scene;

  /** Anchor system backend pointer. */
  void *anchorwin;

  /** Set to one for an active window. */
  bool active;

  /** Previous cursor when setting modal one. */
  short lastcursor;
  /** The current modal cursor. */
  short modalcursor;
  /** Cursor grab mode. */
  short grabcursor;
  /** Internal: tag this for extra mouse-move event,
   * makes cursors/buttons active on UI switching. */
  bool addmousemove;

  short modifier;

  /** Storage for event system. */
  wmEvent *eventstate;
  wmEventQueue event_queue;
  std::vector<wmEventHandler *> modalhandlers;
  std::vector<wmEventHandler *> handlers;
// uiPopupBlockHandle
  /** Runtime Window State. */
  char windowstate;
  int winid;

  WorkSpaceInstanceHook *workspace_hook;

  ScrAreaMap global_areas;

  inline wmWindow(kContext *C, const SdfPath &stagepath);

  inline wmWindow(kContext *C, wmWindow *prim, const SdfPath &stagepath);
};


wmWindow::wmWindow(kContext *C, const SdfPath &stagepath)
  : wabi::UsdUIWindow(KRAKEN_STAGE_CREATE(C)),
    path(GetPath()),
    parent(NULL),
    title(CreateTitleAttr()),
    icon(CreateIconAttr()),
    state(CreateStateAttr()),
    dpi(CreateDpiAttr()),
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
    lastcursor(0),
    modalcursor(0),
    grabcursor(0),
    addmousemove(false),
    eventstate(new wmEvent()),
    windowstate(0),
    winid(0),
    workspace_hook(nullptr)
{}

wmWindow::wmWindow(kContext *C, wmWindow *prim, const SdfPath &stagepath)
  : wabi::UsdUIWindow(KRAKEN_STAGE_CREATE_CHILD(C)),
    path(GetPath()),
    parent(prim),
    title(CreateTitleAttr()),
    icon(CreateIconAttr()),
    state(CreateStateAttr()),
    dpi(CreateDpiAttr()),
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
    windowstate(0),
    winid(0),
    workspace_hook(nullptr)
{}

struct wmNotifier
{
  wmWindow *window;
  unsigned int category, data, subtype, action;
  void *reference;

  // TfNotice notice;

  void Push();
};

struct wmSpaceTypeListenerParams
{
  wmWindow *window;
  struct ScrArea *area;
  wmNotifier *notifier;
  const Scene *scene;

  wmSpaceTypeListenerParams()
    : window(POINTER_ZERO),
      area(POINTER_ZERO),
      notifier(POINTER_ZERO),
      scene(POINTER_ZERO)
  {}
};

typedef std::deque<wmNotifier *> wmNotifierQueue;

struct wmWindowManager : public wabi::UsdPrim
{
  ID id;

  SdfPath path;

  /** All windows this manager controls. */
  wabi::TfHashMap<SdfPath, wmWindow *, wabi::SdfPath::Hash> windows;

  wmWindow *windrawable, *winactive;

  wmNotifierQueue notifier_queue;

  int op_undo_depth;

  /** Active dragged items. */
  std::vector<void *> drags;

  struct ReportList reports;

  std::vector<wmKeyConfig *> keyconfigs;

  /** Default configuration. */
  struct wmKeyConfig *defaultconf;
  /** Addon configuration. */
  struct wmKeyConfig *addonconf;
  /** User configuration. */
  struct wmKeyConfig *userconf;

  /** Active timers. */
  std::vector<struct wmTimer *> timers;

  bool file_saved;

  struct RSet *notifier_queue_set;

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

KRAKEN_NAMESPACE_END